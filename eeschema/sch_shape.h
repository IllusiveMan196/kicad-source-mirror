/*
 * This program source code file is part of KiCad, a free EDA CAD application.
 *
 * Copyright (C) 2004 Jean-Pierre Charras, jaen-pierre.charras@gipsa-lab.inpg.com
 * Copyright (C) 2004-2021 KiCad Developers, see AUTHORS.txt for contributors.
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

#ifndef SCH_SHAPE_H
#define SCH_SHAPE_H

#include <sch_item.h>
#include <eda_shape.h>


class SCH_SHAPE : public SCH_ITEM, public EDA_SHAPE
{
public:
    SCH_SHAPE( SHAPE_T aShape, int aLineWidth = 0, FILL_T aFillType = FILL_T::NO_FILL,
               KICAD_T aType = SCH_SHAPE_T );

    // Do not create a copy constructor.  The one generated by the compiler is adequate.

    ~SCH_SHAPE() { }

    wxString GetClass() const override
    {
        return wxT( "SCH_SHAPE" );
    }

    void SwapData( SCH_ITEM* aItem ) override;

    bool HitTest( const VECTOR2I& aPosition, int aAccuracy = 0 ) const override
    {
        return hitTest( aPosition, aAccuracy );
    }

    bool HitTest( const EDA_RECT& aRect, bool aContained, int aAccuracy = 0 ) const override
    {
        return hitTest( aRect, aContained, aAccuracy );
    }

    int GetPenWidth() const override;

    bool HasLineStroke() const override               { return true; }
    STROKE_PARAMS GetStroke() const override          { return m_stroke; }
    void SetStroke( const STROKE_PARAMS& aStroke ) override;

    PLOT_DASH_TYPE GetEffectiveLineStyle() const
    {
        if( m_stroke.GetPlotStyle() == PLOT_DASH_TYPE::DEFAULT )
            return PLOT_DASH_TYPE::DASH;
        else
            return m_stroke.GetPlotStyle();
    }

    const EDA_RECT GetBoundingBox() const override    { return getBoundingBox(); }

    VECTOR2I GetPosition() const override { return getPosition(); }
    void     SetPosition( const VECTOR2I& aPos ) override { setPosition( aPos ); }

    VECTOR2I GetCenter() const { return getCenter(); }

    void BeginEdit( const VECTOR2I& aStartPoint ) { beginEdit( aStartPoint ); }
    bool ContinueEdit( const VECTOR2I& aPosition ) { return continueEdit( aPosition ); }
    void CalcEdit( const VECTOR2I& aPosition ) { calcEdit( aPosition ); }
    void EndEdit()                                    { endEdit(); }
    void SetEditState( int aState )                   { setEditState( aState ); }

    void Move( const VECTOR2I& aOffset ) override;

    void MirrorHorizontally( int aCenter ) override;
    void MirrorVertically( int aCenter ) override;
    void Rotate( const VECTOR2I& aCenter ) override;

    void AddPoint( const VECTOR2I& aPosition );

    void Plot( PLOTTER* aPlotter ) const override;

    void GetMsgPanelInfo( EDA_DRAW_FRAME* aFrame, std::vector<MSG_PANEL_ITEM>& aList ) override;

    wxString GetSelectMenuText( EDA_UNITS aUnits ) const override;

    BITMAPS GetMenuImage() const override;

    EDA_ITEM* Clone() const override;

    void ViewGetLayers( int aLayers[], int& aCount ) const override;

#if defined(DEBUG)
    void Show( int nestLevel, std::ostream& os ) const override { ShowDummy( os ); }
#endif

private:
    void Print( const RENDER_SETTINGS* aSettings, const VECTOR2I& aOffset ) override;

    EDA_ANGLE getParentOrientation() const override { return ANGLE_0; }
    VECTOR2I getParentPosition() const override { return VECTOR2I(); }
};


#endif    // SCH_SHAPE_H
