/*
 * This program source code file is part of KiCad, a free EDA CAD application.
 *
 * Copyright (C) 2019 CERN
 * Copyright (C) 2019-2020 KiCad Developers, see AUTHORS.txt for contributors.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, you may find one here:
 * http://www.gnu.org/licenses/old-licenses/gpl-2.0.html
 * or you may search the http://www.gnu.org website for the version 2 license,
 * or you may write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA
 */

#include "sch_drawing_tools.h"
#include "ee_selection_tool.h"
#include "ee_point_editor.h"
#include <ee_actions.h>
#include <sch_edit_frame.h>
#include <project.h>
#include <id.h>
#include <eeschema_id.h>
#include <confirm.h>
#include <view/view_controls.h>
#include <view/view.h>
#include <sch_component.h>
#include <sch_no_connect.h>
#include <sch_line.h>
#include <sch_junction.h>
#include <sch_bus_entry.h>
#include <sch_text.h>
#include <sch_sheet.h>
#include <sch_bitmap.h>
#include <schematic.h>
#include <class_library.h>
#include <eeschema_settings.h>
#include <dialogs/dialog_edit_label.h>

SCH_DRAWING_TOOLS::SCH_DRAWING_TOOLS() :
        EE_TOOL_BASE<SCH_EDIT_FRAME>( "eeschema.InteractiveDrawing" ),
        m_lastGlobalLabelShape( PINSHEETLABEL_SHAPE::PS_INPUT ),
        m_lastTextOrientation( LABEL_SPIN_STYLE::LEFT ),
        m_lastTextBold( false ),
        m_lastTextItalic( false )
{
}


bool SCH_DRAWING_TOOLS::Init()
{
    EE_TOOL_BASE::Init();

    auto belowRootSheetCondition =
            [&]( const SELECTION& aSel )
            {
                return m_frame->GetCurrentSheet().Last() != &m_frame->Schematic().Root();
            };

    auto& ctxMenu = m_menu.GetMenu();
    ctxMenu.AddItem( EE_ACTIONS::leaveSheet, belowRootSheetCondition, 2 );

    return true;
}


int SCH_DRAWING_TOOLS::PlaceComponent(  const TOOL_EVENT& aEvent  )
{
    SCH_COMPONENT*                    component = aEvent.Parameter<SCH_COMPONENT*>();
    SCHLIB_FILTER                     filter;
    std::vector<COMPONENT_SELECTION>* historyList = nullptr;

    if( aEvent.IsAction( &EE_ACTIONS::placeSymbol ) )
        historyList = &m_symbolHistoryList;
    else if (aEvent.IsAction( &EE_ACTIONS::placePower ) )
    {
        historyList = &m_powerHistoryList;
        filter.FilterPowerParts( true );
    }
    else
        wxFAIL_MSG( "PlaceCompontent(): unexpected request" );

    getViewControls()->ShowCursor( true );

    // If a component was passed in get it ready for placement.
    if( component )
    {
        component->SetFlags( IS_NEW | IS_MOVED );

        m_toolMgr->RunAction( EE_ACTIONS::clearSelection, true );
        m_selectionTool->AddItemToSel( component );
    }

    std::string tool = aEvent.GetCommandStr().get();
    m_frame->PushTool( tool );
    Activate();

    // Prime the pump
    if( component )
    {
        getViewControls()->WarpCursor( getViewControls()->GetMousePosition( false ) );
        m_toolMgr->RunAction( ACTIONS::refreshPreview );
    }
    else if( aEvent.HasPosition() )
        m_toolMgr->RunAction( EE_ACTIONS::cursorClick );

    // Main loop: keep receiving events
    while( TOOL_EVENT* evt = Wait() )
    {
        m_frame->GetCanvas()->SetCurrentCursor( component ? wxCURSOR_ARROW : wxCURSOR_PENCIL );
        VECTOR2I cursorPos = getViewControls()->GetCursorPosition( !evt->Modifier( MD_ALT ) );

        auto cleanup = [&] () {
            m_toolMgr->RunAction( EE_ACTIONS::clearSelection, true );
            m_view->ClearPreview();
            delete component;
            component = nullptr;
        };

        if( evt->IsCancelInteractive() )
        {
            if( component )
                cleanup();
            else
            {
                m_frame->PopTool( tool );
                break;
            }
        }
        else if( evt->IsActivate() )
        {
            if( component )
                cleanup();

            if( evt->IsMoveTool() )
            {
                // leave ourselves on the stack so we come back after the move
                break;
            }
            else
            {
                m_frame->PopTool( tool );
                break;
            }
        }
        else if( evt->IsClick( BUT_LEFT ) )
        {
            if( !component )
            {
                m_toolMgr->RunAction( EE_ACTIONS::clearSelection, true );

                // Pick the module to be placed
                bool footprintPreviews = m_frame->eeconfig()->m_Appearance.footprint_preview;
                COMPONENT_SELECTION sel = m_frame->SelectCompFromLibTree( &filter, *historyList,
                                                                          true, 1, 1,
                                                                          footprintPreviews );

                // Restore cursor after dialog
                getViewControls()->WarpCursor( getViewControls()->GetCursorPosition(), true );

                LIB_PART* part = sel.LibId.IsValid() ? m_frame->GetLibPart( sel.LibId ) : nullptr;

                if( !part )
                    continue;

                component = new SCH_COMPONENT(
                        *part, &m_frame->GetCurrentSheet(), sel, (wxPoint) cursorPos );
                component->SetParent( m_frame->GetCurrentSheet().LastScreen() );
                component->SetFlags( IS_NEW | IS_MOVED );

                if( m_frame->eeconfig()->m_AutoplaceFields.enable )
                    component->AutoplaceFields( /* aScreen */ NULL, /* aManual */ false );

                m_frame->SaveCopyForRepeatItem( component );

                m_view->ClearPreview();
                m_view->AddToPreview( component->Clone() );
                m_selectionTool->AddItemToSel( component );
            }
            else
            {
                SCH_COMPONENT* next_comp = nullptr;

                m_view->ClearPreview();
                m_frame->AddItemToScreenAndUndoList( component );

                EE_SELECTION new_sel;
                new_sel.Add( component );

                m_toolMgr->RunAction( EE_ACTIONS::addNeededJunctions, true, &new_sel );
                m_frame->OnModify();

                if( m_frame->eeconfig()->m_SymChooserPanel.place_all_units
                        || m_frame->eeconfig()->m_SymChooserPanel.keep_symbol )
                {
                    int new_unit = component->GetUnit();

                    if( m_frame->eeconfig()->m_SymChooserPanel.place_all_units
                            && component->GetUnit() < component->GetUnitCount() )
                        new_unit++;
                    else
                        new_unit = 1;

                    // We are either stepping to the next unit or next component
                    if( m_frame->eeconfig()->m_SymChooserPanel.keep_symbol || new_unit > 1 )
                    {
                        // Deselect the last placed symbol: obviously we do not want to
                        // apply some changes (like rotation, mirror...) to previously placed
                        // symbols.
                        m_selectionTool->ClearSelection();

                        next_comp = static_cast<SCH_COMPONENT*>( component->Duplicate() );
                        next_comp->SetFlags( IS_NEW | IS_MOVED );
                        next_comp->SetUnit( new_unit );
                        next_comp->SetUnitSelection( &m_frame->GetCurrentSheet(), new_unit );

                        if( m_frame->eeconfig()->m_AutoplaceFields.enable )
                            component->AutoplaceFields( /* aScreen */ NULL, /* aManual */ false );

                        m_frame->SaveCopyForRepeatItem( next_comp );
                        m_view->AddToPreview( next_comp->Clone() );
                        m_selectionTool->AddItemToSel( next_comp );
                    }
                }

                component = next_comp;
            }
        }
        else if( evt->IsClick( BUT_RIGHT ) )
        {
            // Warp after context menu only if dragging...
            if( !component )
                m_toolMgr->VetoContextMenuMouseWarp();

            m_menu.ShowContextMenu( m_selectionTool->GetSelection() );
        }
        else if( evt->Category() == TC_COMMAND && evt->Action() == TA_CHOICE_MENU_CHOICE )
        {
            if( evt->GetCommandId().get() >= ID_POPUP_SCH_SELECT_UNIT_CMP
                && evt->GetCommandId().get() <= ID_POPUP_SCH_SELECT_UNIT_CMP_MAX )
            {
                int unit = evt->GetCommandId().get() - ID_POPUP_SCH_SELECT_UNIT_CMP;

                if( component )
                {
                    m_frame->SelectUnit( component, unit );
                    m_toolMgr->RunAction( ACTIONS::refreshPreview );
                }
            }
        }
        else if( component && ( evt->IsAction( &ACTIONS::refreshPreview ) || evt->IsMotion() ) )
        {
            component->SetPosition( (wxPoint)cursorPos );
            m_view->ClearPreview();
            m_view->AddToPreview( component->Clone() );
        }
        else
            evt->SetPassEvent();

        // Enable autopanning and cursor capture only when there is a module to be placed
        getViewControls()->SetAutoPan( component != nullptr );
        getViewControls()->CaptureCursor( component != nullptr );
    }

    return 0;
}


int SCH_DRAWING_TOOLS::PlaceImage( const TOOL_EVENT& aEvent )
{
    SCH_BITMAP* image = aEvent.Parameter<SCH_BITMAP*>();
    bool        immediateMode = image;
    VECTOR2I    cursorPos = getViewControls()->GetCursorPosition();

    m_toolMgr->RunAction( EE_ACTIONS::clearSelection, true );
    getViewControls()->ShowCursor( true );

    // Add all the drawable parts to preview
    if( image )
    {
        image->SetPosition( (wxPoint)cursorPos );
        m_view->ClearPreview();
        m_view->AddToPreview( image->Clone() );
    }

    std::string tool = aEvent.GetCommandStr().get();
    m_frame->PushTool( tool );
    Activate();

    // Prime the pump
    if( image )
        m_toolMgr->RunAction( ACTIONS::refreshPreview );
    else if( aEvent.HasPosition() )
        m_toolMgr->RunAction( ACTIONS::cursorClick );

    // Main loop: keep receiving events
    while( TOOL_EVENT* evt = Wait() )
    {
        m_frame->GetCanvas()->SetCurrentCursor( image ? wxCURSOR_ARROW : wxCURSOR_PENCIL );
        cursorPos = getViewControls()->GetCursorPosition( !evt->Modifier( MD_ALT ) );

        auto cleanup = [&] () {
            m_toolMgr->RunAction( EE_ACTIONS::clearSelection, true );
            m_view->ClearPreview();
            delete image;
            image = nullptr;
        };

        if( evt->IsCancelInteractive() )
        {
            if( image )
                cleanup();
            else
            {
                m_frame->PopTool( tool );
                break;
            }

            if( immediateMode )
            {
                m_frame->PopTool( tool );
                break;
            }
        }
        else if( evt->IsActivate() )
        {
            if( image )
                cleanup();

            if( evt->IsMoveTool() )
            {
                // leave ourselves on the stack so we come back after the move
                break;
            }
            else
            {
                m_frame->PopTool( tool );
                break;
            }
        }
        else if( evt->IsClick( BUT_LEFT ) )
        {
            if( !image )
            {
                m_toolMgr->RunAction( EE_ACTIONS::clearSelection, true );
                wxFileDialog dlg( m_frame, _( "Choose Image" ), wxEmptyString, wxEmptyString,
                                  _( "Image Files " ) + wxImage::GetImageExtWildcard(), wxFD_OPEN );

                if( dlg.ShowModal() != wxID_OK )
                    continue;

                // Restore cursor after dialog
                getViewControls()->WarpCursor( getViewControls()->GetCursorPosition(), true );

                wxString fullFilename = dlg.GetPath();

                if( wxFileExists( fullFilename ) )
                    image = new SCH_BITMAP( (wxPoint)cursorPos );

                if( !image || !image->ReadImageFile( fullFilename ) )
                {
                    wxMessageBox( _( "Couldn't load image from \"%s\"" ), fullFilename );
                    delete image;
                    image = nullptr;
                    continue;
                }

                image->SetFlags( IS_NEW | IS_MOVED );

                m_frame->SaveCopyForRepeatItem( image );

                m_view->ClearPreview();
                m_view->AddToPreview( image->Clone() );
                m_view->RecacheAllItems();  // Bitmaps are cached in Opengl

                m_selectionTool->AddItemToSel( image );

                getViewControls()->SetCursorPosition( cursorPos, false );
            }
            else
            {
                m_frame->AddItemToScreenAndUndoList( image );
                image = nullptr;
                m_toolMgr->RunAction( ACTIONS::activatePointEditor );

                m_view->ClearPreview();

                if( immediateMode )
                {
                    m_frame->PopTool( tool );
                    break;
                }
            }
        }
        else if( evt->IsClick( BUT_RIGHT ) )
        {
            // Warp after context menu only if dragging...
            if( !image )
                m_toolMgr->VetoContextMenuMouseWarp();

            m_menu.ShowContextMenu( m_selectionTool->GetSelection() );
        }
        else if( image && ( evt->IsAction( &ACTIONS::refreshPreview ) || evt->IsMotion() ) )
        {
            image->SetPosition( (wxPoint)cursorPos );
            m_view->ClearPreview();
            m_view->AddToPreview( image->Clone() );
            m_view->RecacheAllItems();  // Bitmaps are cached in Opengl
        }
        else
            evt->SetPassEvent();

        // Enable autopanning and cursor capture only when there is a module to be placed
        getViewControls()->SetAutoPan( image != nullptr );
        getViewControls()->CaptureCursor( image != nullptr );
    }

    return 0;
}


int SCH_DRAWING_TOOLS::SingleClickPlace( const TOOL_EVENT& aEvent )
{
    wxPoint   cursorPos;
    KICAD_T   type = aEvent.Parameter<KICAD_T>();

    if( type == SCH_JUNCTION_T && aEvent.HasPosition() )
    {
        EE_SELECTION& selection = m_selectionTool->GetSelection();
        SCH_LINE*     wire = dynamic_cast<SCH_LINE*>( selection.Front() );

        if( wire )
        {
            SEG seg( wire->GetStartPoint(), wire->GetEndPoint() );
            VECTOR2I nearest = seg.NearestPoint( getViewControls()->GetCursorPosition() );
            getViewControls()->SetCrossHairCursorPosition( nearest, false );
            getViewControls()->WarpCursor( getViewControls()->GetCursorPosition(), true );
        }
    }

    m_toolMgr->RunAction( EE_ACTIONS::clearSelection, true );
    getViewControls()->ShowCursor( true );
    getViewControls()->SetSnapping( true );

    SCH_ITEM* previewItem;
    switch( type )
    {
    case SCH_NO_CONNECT_T:
        previewItem = new SCH_NO_CONNECT( cursorPos );
        break;
    case SCH_JUNCTION_T:
        previewItem = new SCH_JUNCTION( cursorPos );
        break;
    case SCH_BUS_WIRE_ENTRY_T:
        previewItem = new SCH_BUS_WIRE_ENTRY( cursorPos );
        break;
    default:
        wxASSERT_MSG( false, "Unknown item type in SCH_DRAWING_TOOLS::SingleClickPlace" );
        return 0;
    }

    previewItem->SetParent( m_frame->GetScreen() );

    m_view->ClearPreview();
    m_view->AddToPreview( previewItem->Clone() );

    std::string tool = aEvent.GetCommandStr().get();
    m_frame->PushTool( tool );
    Activate();

    // Prime the pump
    if( aEvent.HasPosition() )
        m_toolMgr->RunAction( ACTIONS::cursorClick );

    // Main loop: keep receiving events
    while( TOOL_EVENT* evt = Wait() )
    {
        m_frame->GetCanvas()->SetCurrentCursor( wxCURSOR_ARROW );
        cursorPos = (wxPoint) getViewControls()->GetCursorPosition( !evt->Modifier( MD_ALT ) );

        if( evt->IsCancelInteractive() )
        {
            m_frame->PopTool( tool );
            break;
        }
        else if( evt->IsActivate() )
        {
            if( evt->IsMoveTool() )
            {
                // leave ourselves on the stack so we come back after the move
                break;
            }
            else
            {
                m_frame->PopTool( tool );
                break;
            }
        }
        else if( evt->IsClick( BUT_LEFT ) )
        {
            if( !m_frame->GetScreen()->GetItem( cursorPos, 0, type ) )
            {
                if( type == SCH_JUNCTION_T )
                    m_frame->AddJunction( cursorPos );
                else
                {
                    SCH_ITEM* newItem = static_cast<SCH_ITEM*>( previewItem->Clone() );
                    newItem->SetPosition( cursorPos );
                    newItem->SetFlags( IS_NEW );

                    m_frame->AddItemToScreenAndUndoList( newItem );
                    m_frame->SaveCopyForRepeatItem( newItem );

                    m_frame->SchematicCleanUp();
                    m_frame->OnModify();
                }
            }
        }
        else if( evt->IsClick( BUT_RIGHT ) )
        {
            m_menu.ShowContextMenu( m_selectionTool->GetSelection() );
        }
        else if( evt->IsAction( &ACTIONS::refreshPreview ) || evt->IsMotion() )
        {
            previewItem->SetPosition( (wxPoint)cursorPos );
            m_view->ClearPreview();
            m_view->AddToPreview( previewItem->Clone() );
        }
        else if( evt->Category() == TC_COMMAND )
        {
            if( ( type == SCH_BUS_WIRE_ENTRY_T )
                    && (   evt->IsAction( &EE_ACTIONS::rotateCW )
                        || evt->IsAction( &EE_ACTIONS::rotateCCW )
                        || evt->IsAction( &EE_ACTIONS::mirrorX )
                        || evt->IsAction( &EE_ACTIONS::mirrorY ) ) )
            {
                SCH_BUS_ENTRY_BASE* busItem = static_cast<SCH_BUS_ENTRY_BASE*>( previewItem );

                // The bus entries only rotate in one direction
                if( evt->IsAction( &EE_ACTIONS::rotateCW )
                        || evt->IsAction( &EE_ACTIONS::rotateCCW ) )
                    busItem->Rotate( busItem->GetPosition() );
                else if( evt->IsAction( &EE_ACTIONS::mirrorX ) )
                    busItem->MirrorX( busItem->GetPosition().x );
                else if( evt->IsAction( &EE_ACTIONS::mirrorY ) )
                    busItem->MirrorY( busItem->GetPosition().y );

                m_view->ClearPreview();
                m_view->AddToPreview( previewItem->Clone() );
            }
        }
        else
            evt->SetPassEvent();
    }

    delete previewItem;
    m_view->ClearPreview();

    return 0;
}


// History lists for placing labels and text

SCH_TEXT* SCH_DRAWING_TOOLS::getNextNewText()
{
    if( m_queuedTexts.empty() )
        return nullptr;

    auto next_text = std::move( m_queuedTexts.front() );
    m_queuedTexts.pop_front();

    return next_text.release();
}


SCH_TEXT* SCH_DRAWING_TOOLS::createNewText( const VECTOR2I& aPosition, int aType )
{
    SCHEMATIC*          schematic = getModel<SCHEMATIC>();
    SCHEMATIC_SETTINGS& settings = schematic->Settings();
    SCH_TEXT*           textItem = nullptr;

    m_queuedTexts.clear();

    switch( aType )
    {
    case LAYER_NOTES:
        textItem = new SCH_TEXT( (wxPoint) aPosition );
        break;

    case LAYER_LOCLABEL:
        textItem = new SCH_LABEL( (wxPoint) aPosition );
        break;

    case LAYER_HIERLABEL:
        textItem = new SCH_HIERLABEL( (wxPoint) aPosition );
        textItem->SetShape( m_lastGlobalLabelShape );
        break;

    case LAYER_GLOBLABEL:
        textItem = new SCH_GLOBALLABEL( (wxPoint) aPosition );
        textItem->SetShape( m_lastGlobalLabelShape );
        break;

    default:
        DisplayError( m_frame, wxT( "SCH_EDIT_FRAME::CreateNewText() Internal error" ) );
        return nullptr;
    }

    textItem->SetParent( schematic );
    textItem->SetBold( m_lastTextBold );
    textItem->SetItalic( m_lastTextItalic );
    textItem->SetLabelSpinStyle( m_lastTextOrientation );
    textItem->SetTextSize( wxSize( settings.m_DefaultTextSize, settings.m_DefaultTextSize ) );
    textItem->SetFlags( IS_NEW | IS_MOVED );

    DIALOG_LABEL_EDITOR dlg( m_frame, textItem );

    if( dlg.ShowModal() != wxID_OK || textItem->GetText().IsEmpty() )
    {
        delete textItem;
        return nullptr;
    }

    if( aType != LAYER_NOTES )
    {
        UTF8 text( textItem->GetText() );
        int  brace_count = 0;
        int  bracket_count = 0;
        bool last_space = false;
        UTF8 token;

        for( auto chIt = text.ubegin(); chIt != text.uend(); chIt++ )
        {
            switch( *chIt )
            {
            case '{':
                brace_count++;
                last_space = false;
                break;

            case '[':
                bracket_count++;
                last_space = false;
                break;

            case '}':
                brace_count = std::max( 0, brace_count - 1 );
                last_space = false;
                break;

            case ']':
                bracket_count = std::max( 0, bracket_count - 1 );
                last_space = false;
                break;

            case ' ':
            case '\n':
            case '\r':
            case '\t':
                if( !token.empty() && bracket_count == 0 && brace_count == 0 )
                {
                    std::unique_ptr<SCH_TEXT> nextitem( static_cast<SCH_TEXT*>( textItem->Clone() ) );
                    nextitem->SetText( token.wx_str() );
                    m_queuedTexts.push_back( std::move( nextitem ) );
                    token.clear();
                    continue;
                }

                // Skip leading whitespace
                if( token.empty() || last_space )
                    continue;

                last_space = true;
                break;

            default:
                last_space = false;
                break;
            }

            token += *chIt;
        }

        if( !token.empty() )
        {
            std::unique_ptr<SCH_TEXT> nextitem( static_cast<SCH_TEXT*>( textItem->Clone() ) );
            nextitem->SetText( token.wx_str() );
            m_queuedTexts.push_back( std::move( nextitem ) );
        }

        delete textItem;
        textItem = getNextNewText();

        if( !textItem )
            return nullptr;
    }

    m_lastTextBold = textItem->IsBold();
    m_lastTextItalic = textItem->IsItalic();
    m_lastTextOrientation = textItem->GetLabelSpinStyle();

    if( textItem->Type() == SCH_GLOBAL_LABEL_T || textItem->Type() == SCH_HIER_LABEL_T )
        m_lastGlobalLabelShape = textItem->GetShape();

    return textItem;
}


int SCH_DRAWING_TOOLS::TwoClickPlace( const TOOL_EVENT& aEvent )
{
    EDA_ITEM* item = nullptr;
    bool      importMode = aEvent.IsAction( &EE_ACTIONS::importSheetPin );
    KICAD_T   type = aEvent.Parameter<KICAD_T>();

    m_toolMgr->RunAction( EE_ACTIONS::clearSelection, true );
    getViewControls()->ShowCursor( true );

    std::string tool = aEvent.GetCommandStr().get();
    m_frame->PushTool( tool );
    Activate();

    // Prime the pump
    if( aEvent.HasPosition() )
        m_toolMgr->RunAction( ACTIONS::cursorClick );

    // Main loop: keep receiving events
    while( TOOL_EVENT* evt = Wait() )
    {
        m_frame->GetCanvas()->SetCurrentCursor( item ? wxCURSOR_ARROW : wxCURSOR_PENCIL );
        VECTOR2I cursorPos = getViewControls()->GetCursorPosition( !evt->Modifier( MD_ALT ) );

        auto cleanup = [&] () {
            m_toolMgr->RunAction( EE_ACTIONS::clearSelection, true );
            m_view->ClearPreview();
            delete item;
            item = nullptr;
        };

        if( evt->IsCancelInteractive() )
        {
            if( item )
                cleanup();
            else
            {
                m_frame->PopTool( tool );
                break;
            }
        }
        else if( evt->IsActivate() )
        {
            if( item )
                cleanup();

            if( evt->IsPointEditor() )
            {
                // don't exit (the point editor runs in the background)
            }
            else if( evt->IsMoveTool() )
            {
                // leave ourselves on the stack so we come back after the move
                break;
            }
            else
            {
                m_frame->PopTool( tool );
                break;
            }
        }
        else if( evt->IsClick( BUT_LEFT ) )
        {
            // First click creates...
            if( !item )
            {
                m_toolMgr->RunAction( EE_ACTIONS::clearSelection, true );

                switch( type )
                {
                case SCH_LABEL_T:
                    item = createNewText( cursorPos, LAYER_LOCLABEL );
                    break;
                case SCH_HIER_LABEL_T:
                    item = createNewText( cursorPos, LAYER_HIERLABEL );
                    break;
                case SCH_GLOBAL_LABEL_T:
                    item = createNewText( cursorPos, LAYER_GLOBLABEL );
                    break;
                case SCH_TEXT_T:
                    item = createNewText( cursorPos, LAYER_NOTES );
                    break;
                case SCH_SHEET_PIN_T:
                {
                    SCH_HIERLABEL* label = nullptr;
                    SCH_SHEET*     sheet = nullptr;

                    if( m_selectionTool->SelectPoint( cursorPos, EE_COLLECTOR::SheetsOnly, &item ) )
                        sheet = dynamic_cast<SCH_SHEET*>( item );

                    item = nullptr;

                    if( !sheet )
                    {
                        m_statusPopup.reset( new STATUS_TEXT_POPUP( m_frame ) );
                        m_statusPopup->SetText( _( "Click over a sheet." ) );
                        m_statusPopup->Move( wxGetMousePosition() + wxPoint( 20, 20 ) );
                        m_statusPopup->PopupFor( 2000 );
                        break;
                    }

                    if( importMode )
                    {
                        label = m_frame->ImportHierLabel( sheet );

                        if( !label )
                        {
                            m_statusPopup.reset( new STATUS_TEXT_POPUP( m_frame ) );
                            m_statusPopup->SetText( _( "No new hierarchical labels found." ) );
                            m_statusPopup->Move( wxGetMousePosition() + wxPoint( 20, 20 ) );
                            m_statusPopup->PopupFor( 2000 );
                            break;
                        }
                    }

                    item = m_frame->CreateSheetPin( sheet, label );
                    break;
                }
                default:
                    break;
                }

                // Restore cursor after dialog
                getViewControls()->WarpCursor( getViewControls()->GetCursorPosition(), true );

                if( item )
                {
                    item->SetFlags( IS_NEW | IS_MOVED );
                    m_view->ClearPreview();
                    m_view->AddToPreview( item->Clone() );
                    m_selectionTool->AddItemToSel( item );
                }

                getViewControls()->SetCursorPosition( cursorPos, false );
            }

            // ... and second click places:
            else
            {
                item->ClearFlags( IS_MOVED );
                m_frame->AddItemToScreenAndUndoList( (SCH_ITEM*) item );
                item = getNextNewText();

                if( item )
                {
                    m_toolMgr->RunAction( EE_ACTIONS::clearSelection, true );
                    item->SetFlags( IS_NEW | IS_MOVED );
                    m_view->ClearPreview();
                    m_view->AddToPreview( item->Clone() );
                    m_selectionTool->AddItemToSel( item );
                }
                else
                {
                    m_view->ClearPreview();
                }
            }
        }
        else if( evt->IsClick( BUT_RIGHT ) )
        {
            // Warp after context menu only if dragging...
            if( !item )
                m_toolMgr->VetoContextMenuMouseWarp();

            m_menu.ShowContextMenu( m_selectionTool->GetSelection() );
        }
        else if( item && evt->IsSelectionEvent() )
        {
            // This happens if our text was replaced out from under us by ConvertTextType()
            EE_SELECTION& selection = m_selectionTool->GetSelection();

            if( selection.GetSize() == 1 )
            {
                item = (SCH_ITEM*) selection.Front();
                m_view->ClearPreview();
                m_view->AddToPreview( item->Clone() );
            }
            else
                item = nullptr;
        }
        else if( item && ( evt->IsAction( &ACTIONS::refreshPreview ) || evt->IsMotion() ) )
        {
            static_cast<SCH_ITEM*>( item )->SetPosition( (wxPoint) cursorPos );
            m_view->ClearPreview();
            m_view->AddToPreview( item->Clone() );
        }
        else
            evt->SetPassEvent();

        // Enable autopanning and cursor capture only when there is a module to be placed
        getViewControls()->SetAutoPan( item != nullptr );
        getViewControls()->CaptureCursor( item != nullptr );
    }

    return 0;
}


int SCH_DRAWING_TOOLS::DrawSheet( const TOOL_EVENT& aEvent )
{
    EE_POINT_EDITOR* pointEditor = m_toolMgr->GetTool<EE_POINT_EDITOR>();
    SCH_SHEET*       sheet = nullptr;

    m_toolMgr->RunAction( EE_ACTIONS::clearSelection, true );
    getViewControls()->ShowCursor( true );

    std::string tool = aEvent.GetCommandStr().get();
    m_frame->PushTool( tool );
    Activate();

    // Prime the pump
    if( aEvent.HasPosition() )
        m_toolMgr->RunAction( ACTIONS::cursorClick );

    // Main loop: keep receiving events
    while( TOOL_EVENT* evt = Wait() )
    {
        if( !pointEditor->HasPoint() )
            m_frame->GetCanvas()->SetCurrentCursor( wxCURSOR_PENCIL );

        VECTOR2I cursorPos = getViewControls()->GetCursorPosition( !evt->Modifier( MD_ALT ) );

        auto cleanup = [&] () {
            m_toolMgr->RunAction( EE_ACTIONS::clearSelection, true );
            m_view->ClearPreview();
            delete sheet;
            sheet = nullptr;
        };

        if( evt->IsCancelInteractive() )
        {
            if( sheet )
                cleanup();
            else
            {
                m_frame->PopTool( tool );
                break;
            }
        }
        else if( evt->IsActivate() )
        {
            if( sheet )
                cleanup();

            if( evt->IsPointEditor() )
            {
                // don't exit (the point editor runs in the background)
            }
            else if( evt->IsMoveTool() )
            {
                // leave ourselves on the stack so we come back after the move
                break;
            }
            else
            {
                m_frame->PopTool( tool );
                break;
            }
        }

        else if( evt->IsClick( BUT_LEFT ) && !sheet )
        {
            EESCHEMA_SETTINGS* cfg = m_frame->eeconfig();

            m_toolMgr->RunAction( EE_ACTIONS::clearSelection, true );

            sheet = new SCH_SHEET( m_frame->GetCurrentSheet().Last(),
                                   static_cast<wxPoint>( cursorPos ) );
            sheet->SetFlags( IS_NEW | IS_RESIZED );
            sheet->SetScreen( NULL );
            sheet->SetBorderWidth( cfg->m_Drawing.default_line_thickness );
            sheet->SetBorderColor( cfg->m_Drawing.default_sheet_border_color );
            sheet->SetBackgroundColor( cfg->m_Drawing.default_sheet_background_color );
            sheet->GetFields()[ SHEETNAME ].SetText( "Untitled Sheet" );
            sheet->GetFields()[ SHEETFILENAME ].SetText( "untitled.kicad_sch" );
            sizeSheet( sheet, cursorPos );

            m_view->ClearPreview();
            m_view->AddToPreview( sheet->Clone() );
        }

        else if( sheet && ( evt->IsClick( BUT_LEFT )
                         || evt->IsAction( &EE_ACTIONS::finishSheet ) ) )
        {
            m_view->ClearPreview();
            getViewControls()->SetAutoPan( false );
            getViewControls()->CaptureCursor( false );

            if( m_frame->EditSheetProperties( static_cast<SCH_SHEET*>( sheet ),
                                              &m_frame->GetCurrentSheet(), nullptr ) )
            {
                sheet->AutoplaceFields( /* aScreen */ NULL, /* aManual */ false );

                m_frame->AddItemToScreenAndUndoList( sheet );
                m_frame->UpdateHierarchyNavigator();
                m_selectionTool->AddItemToSel( sheet );
            }
            else
            {
                delete sheet;
            }

            sheet = nullptr;
        }

        else if( sheet && ( evt->IsAction( &ACTIONS::refreshPreview ) || evt->IsMotion() ) )
        {
            sizeSheet( sheet, cursorPos );
            m_view->ClearPreview();
            m_view->AddToPreview( sheet->Clone() );
        }

        else if( evt->IsClick( BUT_RIGHT ) )
        {
            // Warp after context menu only if dragging...
            if( !sheet )
                m_toolMgr->VetoContextMenuMouseWarp();

            m_menu.ShowContextMenu( m_selectionTool->GetSelection() );
        }
        else
            evt->SetPassEvent();

        // Enable autopanning and cursor capture only when there is a sheet to be placed
        getViewControls()->SetAutoPan( sheet != nullptr );
        getViewControls()->CaptureCursor( sheet != nullptr );
    }

    return 0;
}


void SCH_DRAWING_TOOLS::sizeSheet( SCH_SHEET* aSheet, VECTOR2I aPos )
{
    wxPoint pos = aSheet->GetPosition();
    wxPoint size = (wxPoint) aPos - pos;

    size.x = std::max( size.x, MIN_SHEET_WIDTH );
    size.y = std::max( size.y, MIN_SHEET_HEIGHT );

    wxPoint grid = m_frame->GetNearestGridPosition( pos + size );
    aSheet->Resize( wxSize( grid.x - pos.x, grid.y - pos.y ) );
}


void SCH_DRAWING_TOOLS::setTransitions()
{
    Go( &SCH_DRAWING_TOOLS::PlaceComponent,      EE_ACTIONS::placeSymbol.MakeEvent() );
    Go( &SCH_DRAWING_TOOLS::PlaceComponent,      EE_ACTIONS::placePower.MakeEvent() );
    Go( &SCH_DRAWING_TOOLS::SingleClickPlace,    EE_ACTIONS::placeNoConnect.MakeEvent() );
    Go( &SCH_DRAWING_TOOLS::SingleClickPlace,    EE_ACTIONS::placeJunction.MakeEvent() );
    Go( &SCH_DRAWING_TOOLS::SingleClickPlace,    EE_ACTIONS::placeBusWireEntry.MakeEvent() );
    Go( &SCH_DRAWING_TOOLS::TwoClickPlace,       EE_ACTIONS::placeLabel.MakeEvent() );
    Go( &SCH_DRAWING_TOOLS::TwoClickPlace,       EE_ACTIONS::placeHierLabel.MakeEvent() );
    Go( &SCH_DRAWING_TOOLS::TwoClickPlace,       EE_ACTIONS::placeGlobalLabel.MakeEvent() );
    Go( &SCH_DRAWING_TOOLS::DrawSheet,           EE_ACTIONS::drawSheet.MakeEvent() );
    Go( &SCH_DRAWING_TOOLS::TwoClickPlace,       EE_ACTIONS::importSheetPin.MakeEvent() );
    Go( &SCH_DRAWING_TOOLS::TwoClickPlace,       EE_ACTIONS::placeSchematicText.MakeEvent() );
    Go( &SCH_DRAWING_TOOLS::PlaceImage,          EE_ACTIONS::placeImage.MakeEvent() );
}
