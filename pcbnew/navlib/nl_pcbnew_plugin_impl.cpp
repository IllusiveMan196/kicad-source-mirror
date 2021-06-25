/*
 * This program source code file is part of KiCad, a free EDA CAD application.
 *
 * Copyright (C) 2021 3Dconnexion
 * Copyright (C) 2021 KiCad Developers, see AUTHORS.txt for contributors.
 *
 * This program is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation, either version 3 of the License, or (at your
 * option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "nl_pcbnew_plugin_impl.h"

// KiCAD includes
#include <board.h>
#include <pcb_base_frame.h>
#include <bitmaps.h>
#include <class_draw_panel_gal.h>
#include <view/view.h>
#include <view/wx_view_controls.h>
#include <tool/action_manager.h>
#include <tool/tool_action.h>
#include <tool/tool_manager.h>

// stdlib
#include <list>
#include <map>
#include <memory>
#include <utility>
#include <vector>
#include <cfloat>

#include <wx/log.h>
#include <wx/mstream.h>

/**
 * Template to compare two floating point values for equality within a required epsilon.
 *
 * @param aFirst value to compare.
 * @param aSecond value to compare.
 * @param aEpsilon allowed error.
 * @return true if the values considered equal within the specified epsilon, otherwise false.
 */
template <class T>
bool equals( T aFirst, T aSecond, T aEpsilon = static_cast<T>( FLT_EPSILON ) )
{
    T diff = fabs( aFirst - aSecond );
    if( diff < aEpsilon )
    {
        return true;
    }
    aFirst = fabs( aFirst );
    aSecond = fabs( aSecond );
    T largest = aFirst > aSecond ? aFirst : aSecond;
    if( diff <= largest * aEpsilon )
    {
        return true;
    }
    return false;
}

/**
 * Template to compare two VECTOR2<T> values for equality within a required epsilon.
 *
 * @param aFirst value to compare.
 * @param aSecond value to compare.
 * @param aEpsilon allowed error.
 * @return true if the values considered equal within the specified epsilon, otherwise false.
 */
template <class T>
bool equals( VECTOR2<T> const& aFirst, VECTOR2<T> const& aSecond,
             T aEpsilon = static_cast<T>( FLT_EPSILON ) )
{
    if( !equals( aFirst.x, aSecond.x, aEpsilon ) )
    {
        return false;
    }

    return equals( aFirst.y, aSecond.y, aEpsilon );
}

/**
 * Flag to enable the NL_PCBNEW_PLUGIN debug tracing.
 *
 * Use "KI_TRACE_NL_PCBNEW_PLUGIN" to enable.
 *
 * @ingroup trace_env_vars
 */
const wxChar* NL_PCBNEW_PLUGIN_IMPL::m_logTrace = wxT( "KI_TRACE_NL_PCBNEW_PLUGIN" );

NL_PCBNEW_PLUGIN_IMPL::NL_PCBNEW_PLUGIN_IMPL( PCB_DRAW_PANEL_GAL* aViewport ) :
        CNavigation3D( false, false ), m_viewport2D( aViewport ), m_isMoving( false )
{
    m_view = m_viewport2D->GetView();
    m_viewportWidth = m_view->GetBoundary().GetWidth();

    PutProfileHint( "KiCAD PCB" );

    // Use the default settings for the connexion to the 3DMouse navigation
    // They are use a single-threaded threading model and row vectors.
    EnableNavigation( true );

    // Use the SpaceMouse internal timing source for the frame rate.
    PutFrameTimingSource( TimingSource::SpaceMouse );

    exportCommandsAndImages();
}

NL_PCBNEW_PLUGIN_IMPL::~NL_PCBNEW_PLUGIN_IMPL()
{
    EnableNavigation( false );
}

void NL_PCBNEW_PLUGIN_IMPL::SetFocus( bool aFocus )
{
    wxLogTrace( m_logTrace, "NL_PCBNEW_PLUGIN_IMPL::SetFocus %d", aFocus );
    NAV_3D::Write( navlib::focus_k, aFocus );
}

// temporary store for the command categories
typedef std::map<std::string, TDx::CCommandTreeNode*> CATEGORY_STORE;

/**
 * Add a category to the store.
 *
 * The function adds category paths of the format "A.B" where B is a sub-category of A.
 *
 * @param aCategoryPath is the std::string representation of the category.
 * @param aCategoryStore is the CATEGORY_STORE instance to add to.
 * @return a CATEGORY_STORE::iterator where the category was added.
 */
static CATEGORY_STORE::iterator add_category( std::string     aCategoryPath,
                                              CATEGORY_STORE& aCategoryStore )
{
    using TDx::SpaceMouse::CCategory;

    CATEGORY_STORE::iterator parent_iter = aCategoryStore.begin();
    std::string::size_type   pos = aCategoryPath.find_last_of( '.' );

    if( pos != std::string::npos )
    {
        std::string parentPath = aCategoryPath.substr( 0, pos );
        parent_iter = aCategoryStore.find( parentPath );
        if( parent_iter == aCategoryStore.end() )
        {
            parent_iter = add_category( parentPath, aCategoryStore );
        }
    }

    std::string                name = aCategoryPath.substr( pos + 1 );
    std::unique_ptr<CCategory> categoryNode =
            std::make_unique<CCategory>( aCategoryPath.c_str(), name.c_str() );

    CATEGORY_STORE::iterator iter = aCategoryStore.insert(
            aCategoryStore.end(), CATEGORY_STORE::value_type( aCategoryPath, categoryNode.get() ) );

    parent_iter->second->push_back( std::move( categoryNode ) );
    return iter;
}

/**
  * Export the invocable actions and images to the 3Dconnexion UI.
  */
void NL_PCBNEW_PLUGIN_IMPL::exportCommandsAndImages()
{
    wxLogTrace( m_logTrace, "NL_PCBNEW_PLUGIN_IMPL::exportCommandsAndImages" );

    std::list<TOOL_ACTION*> actions = ACTION_MANAGER::GetActionList();

    if( actions.size() == 0 )
    {
        return;
    }

    using TDx::SpaceMouse::CCommand;
    using TDx::SpaceMouse::CCommandSet;

    // The root action set node
    CCommandSet commandSet( "PCB_DRAW_PANEL_GAL", "PCB Viewer" );

    // Activate the command set
    NAV_3D::PutActiveCommands( commandSet.GetId() );

    // temporary store for the categories
    CATEGORY_STORE categoryStore;

    std::vector<TDx::CImage> vImages;

    // add the action set to the category_store
    CATEGORY_STORE::iterator iter = categoryStore.insert(
            categoryStore.end(), CATEGORY_STORE::value_type( ".", &commandSet ) );

    std::list<TOOL_ACTION*>::const_iterator it;

    for( it = actions.begin(); it != actions.end(); ++it )
    {
        const TOOL_ACTION* action = *it;
        std::string        label = action->GetLabel().ToStdString();
        if( label.empty() )
        {
            continue;
        }

        std::string name = action->GetName();

        // Do no export commands for the 3DViewer app.
        if( name.rfind( "3DViewer.", 0 ) == 0 )
        {
            continue;
        }

        std::string              strCategory = action->GetToolName();
        CATEGORY_STORE::iterator iter = categoryStore.find( strCategory );
        if( iter == categoryStore.end() )
        {
            iter = add_category( std::move( strCategory ), categoryStore );
        }

        std::string description = action->GetDescription().ToStdString();

        // Arbitrary 8-bit data stream
        wxMemoryOutputStream imageStream;
        if( action->GetIcon() != BITMAPS::INVALID_BITMAP )
        {
            wxImage image = KiBitmap( action->GetIcon() ).ConvertToImage();
            image.SaveFile( imageStream, wxBitmapType::wxBITMAP_TYPE_PNG );
            image.Destroy();

            if( imageStream.GetSize() )
            {
                wxStreamBuffer* streamBuffer = imageStream.GetOutputStreamBuffer();
                TDx::CImage     image = TDx::CImage::FromData( "", 0, name.c_str() );
                image.AssignImage( std::string( reinterpret_cast<const char*>(
                                                        streamBuffer->GetBufferStart() ),
                                                streamBuffer->GetBufferSize() ),
                                   0 );

                wxLogTrace( m_logTrace, "Adding image for : %s", name );
                vImages.push_back( std::move( image ) );
            }
        }

        wxLogTrace( m_logTrace, "Inserting command: %s,  description: %s,  in category:  %s", name,
                    description, iter->first );

        iter->second->push_back(
                CCommand( std::move( name ), std::move( label ), std::move( description ) ) );
    }

    NAV_3D::AddCommandSet( commandSet );
    NAV_3D::AddImages( vImages );
}

long NL_PCBNEW_PLUGIN_IMPL::GetCameraMatrix( navlib::matrix_t& matrix ) const
{
    if( m_view == nullptr )
    {
        return navlib::make_result_code( navlib::navlib_errc::no_data_available );
    }

    m_viewPosition = m_view->GetCenter();

    double x = m_view->IsMirroredX() ? -1 : 1;
    double y = m_view->IsMirroredY() ? 1 : -1;

    // x * y * z = 1 for a right-handed coordinate system.
    double z = x * y;

    // Note: the connexion has been configured as row vectors, the coordinate system is defined in
    // NL_PCBNEW_PLUGIN_IMPL::GetCoordinateSystem and the front view in NL_PCBNEW_PLUGIN_IMPL::GetFrontView.
    matrix = { x, 0, 0, 0, 0, y, 0, 0, 0, 0, z, 0, m_viewPosition.x, m_viewPosition.y, 0, 1 };
    return 0;
}

long NL_PCBNEW_PLUGIN_IMPL::GetPointerPosition( navlib::point_t& position ) const
{
    if( m_view == nullptr )
    {
        return navlib::make_result_code( navlib::navlib_errc::no_data_available );
    }

    VECTOR2D mouse_pointer = m_viewport2D->GetViewControls()->GetMousePosition();

    position.x = mouse_pointer.x;
    position.y = mouse_pointer.y;
    position.z = 0;

    return 0;
}

long NL_PCBNEW_PLUGIN_IMPL::GetViewExtents( navlib::box_t& extents ) const
{
    if( m_view == nullptr )
    {
        return navlib::make_result_code( navlib::navlib_errc::no_data_available );
    }

    double scale = m_viewport2D->GetGAL()->GetWorldScale();
    BOX2D  box = m_view->GetViewport();

    m_viewportWidth = box.GetWidth();

    extents = navlib::box_t{ -box.GetWidth() / 2.0,
                             -box.GetHeight() / 2.0,
                             m_viewport2D->GetGAL()->GetMinDepth() / scale,
                             box.GetWidth() / 2.0,
                             box.GetHeight() / 2.0,
                             m_viewport2D->GetGAL()->GetMaxDepth() / scale };
    return 0;
}

long NL_PCBNEW_PLUGIN_IMPL::GetIsViewPerspective( navlib::bool_t& perspective ) const
{
    perspective = false;

    return 0;
}

long NL_PCBNEW_PLUGIN_IMPL::SetCameraMatrix( const navlib::matrix_t& matrix )
{
    if( m_view == nullptr )
    {
        return navlib::make_result_code( navlib::navlib_errc::invalid_operation );
    }

    long     result = 0;
    VECTOR2D viewPos( matrix.m4x4[3][0], matrix.m4x4[3][1] );

    if( !equals( m_view->GetCenter(), m_viewPosition ) )
    {
        m_view->SetCenter( viewPos + m_view->GetCenter() - m_viewPosition );
        result = navlib::make_result_code( navlib::navlib_errc::error );
    }
    else
    {
        m_view->SetCenter( viewPos );
    }

    m_viewPosition = viewPos;

    return result;
}

long NL_PCBNEW_PLUGIN_IMPL::SetViewExtents( const navlib::box_t& extents )
{
    if( m_view == nullptr )
    {
        return navlib::make_result_code( navlib::navlib_errc::invalid_operation );
    }

    long result = 0;
    if( m_viewportWidth != m_view->GetViewport().GetWidth() )
    {
        result = navlib::make_result_code( navlib::navlib_errc::error );
    }

    double width = m_viewportWidth;
    m_viewportWidth = extents.max_x - extents.min_x;

    double scale = width / m_viewportWidth * m_view->GetScale();
    m_view->SetScale( scale, m_view->GetCenter() );

    if( !equals( m_view->GetScale(), scale ) )
    {
        result = navlib::make_result_code( navlib::navlib_errc::error );
    }

    return result;
}

long NL_PCBNEW_PLUGIN_IMPL::SetViewFOV( double fov )
{
    return navlib::make_result_code( navlib::navlib_errc::invalid_operation );
}

long NL_PCBNEW_PLUGIN_IMPL::SetViewFrustum( const navlib::frustum_t& frustum )
{
    return navlib::make_result_code( navlib::navlib_errc::invalid_operation );
}

long NL_PCBNEW_PLUGIN_IMPL::GetModelExtents( navlib::box_t& extents ) const
{
    if( m_view == nullptr )
    {
        return navlib::make_result_code( navlib::navlib_errc::no_data_available );
    }

    BOX2I box = static_cast<PCB_BASE_FRAME*>( m_viewport2D->GetParent() )->GetDocumentExtents();
    box.Normalize();

    double half_depth = 0.1 / m_viewport2D->GetGAL()->GetWorldScale();
    if( box.GetWidth() == 0 && box.GetHeight() == 0 )
    {
        half_depth = 0;
    }

    extents = { static_cast<double>( box.GetOrigin().x ),
                static_cast<double>( box.GetOrigin().y ),
                -half_depth,
                static_cast<double>( box.GetEnd().x ),
                static_cast<double>( box.GetEnd().y ),
                half_depth };

    return 0;
}

long NL_PCBNEW_PLUGIN_IMPL::GetCoordinateSystem( navlib::matrix_t& matrix ) const
{
    // The coordinate system is defined as x to the right, y down and z into the screen.
    matrix = { 1, 0, 0, 0, 0, -1, 0, 0, 0, 0, -1, 0, 0, 0, 0, 1 };
    return 0;
}

long NL_PCBNEW_PLUGIN_IMPL::GetFrontView( navlib::matrix_t& matrix ) const
{
    matrix = { 1, 0, 0, 0, 0, -1, 0, 0, 0, 0, -1, 0, 0, 0, 0, 1 };
    return 0;
}

long NL_PCBNEW_PLUGIN_IMPL::GetIsSelectionEmpty( navlib::bool_t& empty ) const
{
    empty = true;
    return 0;
}

long NL_PCBNEW_PLUGIN_IMPL::GetIsViewRotatable( navlib::bool_t& isRotatable ) const
{
    isRotatable = false;
    return 0;
}

long NL_PCBNEW_PLUGIN_IMPL::SetActiveCommand( std::string commandId )
{
    if( commandId.empty() )
    {
        return 0;
    }

    std::list<TOOL_ACTION*> actions = ACTION_MANAGER::GetActionList();
    TOOL_ACTION*            context = nullptr;

    for( std::list<TOOL_ACTION*>::const_iterator it = actions.begin(); it != actions.end(); it++ )
    {
        TOOL_ACTION* action = *it;
        std::string  nm = action->GetName();

        if( commandId == nm )
        {
            context = action;
        }
    }

    if( context != nullptr )
    {
        wxWindow* parent = m_viewport2D->GetParent();

        // Only allow command execution if the window is enabled. i.e. there is not a modal dialog
        // currently active.
        if( parent->IsEnabled() )
        {
            TOOL_MANAGER* tool_manager = static_cast<PCB_BASE_FRAME*>( parent )->GetToolManager();

            // Get the selection to use to test if the action is enabled
            SELECTION& sel = tool_manager->GetToolHolder()->GetCurrentSelection();

            bool runAction = true;

            if( const ACTION_CONDITIONS* aCond =
                        tool_manager->GetActionManager()->GetCondition( *context ) )
            {
                runAction = aCond->enableCondition( sel );
            }

            if( runAction )
            {
                tool_manager->RunAction( *context, true );
            }
        }
        else
        {
            return navlib::make_result_code( navlib::navlib_errc::invalid_operation );
        }
    }

    return 0;
}

long NL_PCBNEW_PLUGIN_IMPL::SetSettingsChanged( long change )
{
    return 0;
}

long NL_PCBNEW_PLUGIN_IMPL::SetMotionFlag( bool value )
{
    m_isMoving = value;

    return 0;
}

long NL_PCBNEW_PLUGIN_IMPL::SetTransaction( long value )
{
    if( value == 0L )
    {
        m_viewport2D->ForceRefresh();
    }

    return 0;
}

long NL_PCBNEW_PLUGIN_IMPL::GetViewFOV( double& fov ) const
{
    return navlib::make_result_code( navlib::navlib_errc::invalid_operation );
}

long NL_PCBNEW_PLUGIN_IMPL::GetViewFrustum( navlib::frustum_t& frustum ) const
{
    return navlib::make_result_code( navlib::navlib_errc::invalid_operation );
}

long NL_PCBNEW_PLUGIN_IMPL::GetSelectionExtents( navlib::box_t& extents ) const
{
    return navlib::make_result_code( navlib::navlib_errc::no_data_available );
}

long NL_PCBNEW_PLUGIN_IMPL::GetSelectionTransform( navlib::matrix_t& transform ) const
{
    return navlib::make_result_code( navlib::navlib_errc::no_data_available );
}

long NL_PCBNEW_PLUGIN_IMPL::SetSelectionTransform( const navlib::matrix_t& matrix )
{
    return navlib::make_result_code( navlib::navlib_errc::invalid_operation );
}

long NL_PCBNEW_PLUGIN_IMPL::GetPivotPosition( navlib::point_t& position ) const
{
    return navlib::make_result_code( navlib::navlib_errc::invalid_operation );
}

long NL_PCBNEW_PLUGIN_IMPL::IsUserPivot( navlib::bool_t& userPivot ) const
{
    return navlib::make_result_code( navlib::navlib_errc::invalid_operation );
}

long NL_PCBNEW_PLUGIN_IMPL::SetPivotPosition( const navlib::point_t& position )
{
    return navlib::make_result_code( navlib::navlib_errc::invalid_operation );
}

long NL_PCBNEW_PLUGIN_IMPL::GetPivotVisible( navlib::bool_t& visible ) const
{
    return navlib::make_result_code( navlib::navlib_errc::invalid_operation );
}

long NL_PCBNEW_PLUGIN_IMPL::SetPivotVisible( bool visible )
{
    return navlib::make_result_code( navlib::navlib_errc::invalid_operation );
}

long NL_PCBNEW_PLUGIN_IMPL::GetHitLookAt( navlib::point_t& position ) const
{
    return navlib::make_result_code( navlib::navlib_errc::no_data_available );
}

long NL_PCBNEW_PLUGIN_IMPL::SetHitAperture( double aperture )
{
    return navlib::make_result_code( navlib::navlib_errc::invalid_operation );
}

long NL_PCBNEW_PLUGIN_IMPL::SetHitDirection( const navlib::vector_t& direction )
{
    return navlib::make_result_code( navlib::navlib_errc::invalid_operation );
}

long NL_PCBNEW_PLUGIN_IMPL::SetHitLookFrom( const navlib::point_t& eye )
{
    return navlib::make_result_code( navlib::navlib_errc::invalid_operation );
}

long NL_PCBNEW_PLUGIN_IMPL::SetHitSelectionOnly( bool onlySelection )
{
    return navlib::make_result_code( navlib::navlib_errc::invalid_operation );
}

long NL_PCBNEW_PLUGIN_IMPL::SetCameraTarget( const navlib::point_t& position )
{
    return navlib::make_result_code( navlib::navlib_errc::invalid_operation );
}
