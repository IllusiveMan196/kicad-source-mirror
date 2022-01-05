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

#ifndef LIB_SHAPE_H
#define LIB_SHAPE_H

#include <lib_item.h>
#include <eda_shape.h>


class LIB_SHAPE : public LIB_ITEM, public EDA_SHAPE
{
public:
    LIB_SHAPE( LIB_SYMBOL* aParent, SHAPE_T aShape, int aLineWidth = 0,
               FILL_T aFillType = FILL_T::NO_FILL );

    // Do not create a copy constructor.  The one generated by the compiler is adequate.

    ~LIB_SHAPE() { }

    wxString GetClass() const override
    {
        return wxT( "LIB_SHAPE" );
    }

    wxString GetTypeName() const override
    {
        return ShowShape();
    }

    STROKE_PARAMS GetStroke() const { return m_stroke; }
    void SetStroke( const STROKE_PARAMS& aStroke ) { m_stroke = aStroke; }

    bool HitTest( const VECTOR2I& aPosition, int aAccuracy = 0 ) const override;
    bool HitTest( const EDA_RECT& aRect, bool aContained, int aAccuracy = 0 ) const override;

    int GetPenWidth() const override;

    int GetEffectivePenWidth( const RENDER_SETTINGS* aSettings ) const override
    {
        // For historical reasons, a stored value of 0 means "default width" and negative
        // numbers meant "don't stroke".

        if( GetPenWidth() < 0 && GetFillMode() != FILL_T::NO_FILL )
            return 0;
        else if( GetPenWidth() == 0 )
            return aSettings->GetDefaultPenWidth();
        else
            return std::max( GetPenWidth(), aSettings->GetMinPenWidth() );
    }

    const EDA_RECT GetBoundingBox() const override;

    void GetMsgPanelInfo( EDA_DRAW_FRAME* aFrame, std::vector<MSG_PANEL_ITEM>& aList ) override;

    void BeginEdit( const VECTOR2I& aStartPoint ) override  { beginEdit( aStartPoint ); }
    bool ContinueEdit( const VECTOR2I& aPosition ) override { return continueEdit( aPosition ); }
    void CalcEdit( const VECTOR2I& aPosition ) override     { calcEdit( aPosition ); }

    /**
     * The base EndEdit() removes the last point in the polyline, so don't call that here
     */
    void EndEdit() override                                { }
    void SetEditState( int aState )                        { setEditState( aState ); }

    void AddPoint( const VECTOR2I& aPosition );

    void Offset( const VECTOR2I& aOffset ) override;

    void MoveTo( const VECTOR2I& aPosition ) override;

    VECTOR2I GetPosition() const override { return getPosition(); }
    void     SetPosition( const VECTOR2I& aPosition ) override { setPosition( aPosition ); }

    VECTOR2I GetCenter() const { return getCenter(); }

    void CalcArcAngles( int& aStartAngle, int& aEndAngle ) const;

    void MirrorHorizontal( const VECTOR2I& aCenter ) override;
    void MirrorVertical( const VECTOR2I& aCenter ) override;
    void Rotate( const VECTOR2I& aCenter, bool aRotateCCW = true ) override;

    void Plot( PLOTTER* aPlotter, const VECTOR2I& aOffset, bool aFill,
               const TRANSFORM& aTransform ) const override;

    wxString GetSelectMenuText( EDA_UNITS aUnits ) const override;

    BITMAPS GetMenuImage() const override;

    EDA_ITEM* Clone() const override;

private:
    /**
     * @copydoc LIB_ITEM::compare()
     *
     * The circle specific sort order is as follows:
     *      - Circle horizontal (X) position.
     *      - Circle vertical (Y) position.
     *      - Circle radius.
     */
    int compare( const LIB_ITEM& aOther,
            LIB_ITEM::COMPARE_FLAGS aCompareFlags = LIB_ITEM::COMPARE_FLAGS::NORMAL ) const override;

    void print( const RENDER_SETTINGS* aSettings, const VECTOR2I& aOffset, void* aData,
                const TRANSFORM& aTransform ) override;

    double getParentOrientation() const override { return 0.0; }
    VECTOR2I getParentPosition() const override { return VECTOR2I(); }
};


#endif    // LIB_SHAPE_H
