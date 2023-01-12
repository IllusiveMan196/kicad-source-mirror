/*
 * This program source code file is part of KiCad, a free EDA CAD application.
 *
 * Copyright (C) 2004 Jean-Pierre Charras, jaen-pierre.charras@gipsa-lab.inpg.com
 * Copyright (C) 2004-2022 KiCad Developers, see AUTHORS.txt for contributors.
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

#ifndef LIB_TEXT_H
#define LIB_TEXT_H

#include <eda_text.h>
#include <lib_item.h>


/**
 * Define a symbol library graphical text item.
 *
 * This is only a graphical text item.  Field text like the reference designator,
 * symbol value, etc. are not LIB_TEXT items.  See the #LIB_FIELD class for the
 * field item definition.
 */
class LIB_TEXT : public LIB_ITEM, public EDA_TEXT
{
public:
    LIB_TEXT( LIB_SYMBOL* aParent );

    // Do not create a copy constructor.  The one generated by the compiler is adequate.

    ~LIB_TEXT() { }

    wxString GetClass() const override
    {
        return wxT( "LIB_TEXT" );
    }

    wxString GetTypeName() const override
    {
        return _( "Text" );
    }

    void ViewGetLayers( int aLayers[], int& aCount ) const override;

    bool HitTest( const VECTOR2I& aPosition, int aAccuracy = 0 ) const override;

    bool HitTest( const BOX2I& aRect, bool aContained, int aAccuracy = 0 ) const override
    {
        if( m_flags & (STRUCT_DELETED | SKIP_STRUCT ) )
            return false;

        BOX2I rect = aRect;

        rect.Inflate( aAccuracy );

        BOX2I textBox = GetTextBox();
        textBox.RevertYAxis();

        if( aContained )
            return rect.Contains( textBox );

        return rect.Intersects( textBox, GetTextAngle() );
    }

    int GetPenWidth() const override;

    KIFONT::FONT* getDrawFont() const override;

    const BOX2I GetBoundingBox() const override;

    void BeginEdit( const VECTOR2I& aStartPoint ) override;
    void CalcEdit( const VECTOR2I& aPosition ) override;

    void Offset( const VECTOR2I& aOffset ) override;

    void MoveTo( const VECTOR2I& aPosition ) override;

    VECTOR2I GetPosition() const override { return EDA_TEXT::GetTextPos(); }

    void MirrorHorizontal( const VECTOR2I& aCenter ) override;
    void MirrorVertical( const VECTOR2I& aCenter ) override;
    void Rotate( const VECTOR2I& aCenter, bool aRotateCCW = true ) override;

    void NormalizeJustification( bool inverse );

    void Plot( PLOTTER* aPlotter, bool aBackground, const VECTOR2I& aOffset,
               const TRANSFORM& aTransform, bool aDimmed ) const override;

    wxString GetItemDescription( UNITS_PROVIDER* aUnitsProvider ) const override;
    void GetMsgPanelInfo( EDA_DRAW_FRAME* aFrame, std::vector<MSG_PANEL_ITEM>& aList ) override;

    BITMAPS GetMenuImage() const override;

    EDA_ITEM* Clone() const override;

private:
    /**
     * @copydoc LIB_ITEM::compare()
     *
     * The text specific sort order is as follows:
     *      - Text string, case insensitive compare.
     *      - Text horizontal (X) position.
     *      - Text vertical (Y) position.
     *      - Text width.
     *      - Text height.
     */
    int compare( const LIB_ITEM& aOther, int aCompareFlags = 0 ) const override;

    void print( const RENDER_SETTINGS* aSettings, const VECTOR2I& aOffset, void* aData,
                const TRANSFORM& aTransform, bool aDimmed ) override;
};


#endif    // LIB_TEXT_H
