/*
 * This program source code file is part of KiCad, a free EDA CAD application.
 *
 * Copyright (C) 2018 Jean-Pierre Charras, jp.charras at wanadoo.fr
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

#ifndef TYPE_SCH_MARKER_H_
#define TYPE_SCH_MARKER_H_

#include <erc_item.h>
#include <sch_item.h>
#include <marker_base.h>


class SCH_MARKER : public SCH_ITEM, public MARKER_BASE
{
public:
    SCH_MARKER( std::shared_ptr<ERC_ITEM> aItem, const VECTOR2I& aPos );

    ~SCH_MARKER();

    // Do not create a copy constructor.  The one generated by the compiler is adequate.

    static inline bool ClassOf( const EDA_ITEM* aItem )
    {
        return aItem && SCH_MARKER_T == aItem->Type();
    }

    wxString GetClass() const override
    {
        return wxT( "SCH_MARKER" );
    }

    const KIID GetUUID() const override { return m_Uuid; }

    void SwapData( SCH_ITEM* aItem ) override;

    wxString Serialize() const;
    static SCH_MARKER* Deserialize( const wxString& data );

    void ViewGetLayers( int aLayers[], int& aCount ) const override;

    SCH_LAYER_ID GetColorLayer() const;

    SEVERITY GetSeverity() const override;

    void Print( const RENDER_SETTINGS* aSettings, const VECTOR2I& aOffset ) override;

    void Plot( PLOTTER* /* aPlotter */, bool /* aBackground */ ) const override
    {
        // SCH_MARKERs should not be plotted. However, SCH_ITEM will fail an assertion if we
        // do not confirm this by locally implementing a no-op Plot().
    }

    BOX2I const GetBoundingBox() const override;

    // Geometric transforms (used in block operations):

    void Move( const VECTOR2I& aMoveVector ) override
    {
        m_Pos += aMoveVector;
    }

    void MirrorHorizontally( int aCenter ) override;
    void MirrorVertically( int aCenter ) override;
    void Rotate( const VECTOR2I& aCenter ) override;

    /**
     * Compare DRC marker main and auxiliary text against search string.
     *
     * @param[in] aSearchData is the criteria to search against.
     * @param[in] aAuxData is the optional data required for the search or NULL if not used.
     * @return True if the DRC main or auxiliary text matches the search criteria.
     */
    bool Matches( const EDA_SEARCH_DATA& aSearchData, void* aAuxDat ) const override;

    void GetMsgPanelInfo( EDA_DRAW_FRAME* aFrame, std::vector<MSG_PANEL_ITEM>& aList ) override;

    wxString GetItemDescription( UNITS_PROVIDER* aUnitsProvider ) const override
    {
        return wxString( _( "ERC Marker" ) );
    }

    BITMAPS GetMenuImage() const override;

    VECTOR2I GetPosition() const override { return m_Pos; }
    void     SetPosition( const VECTOR2I& aPosition ) override { m_Pos = aPosition; }

    bool HitTest( const VECTOR2I& aPosition, int aAccuracy = 0 ) const override;

    EDA_ITEM* Clone() const override;

#if defined(DEBUG)
    void Show( int nestLevel, std::ostream& os ) const override;
#endif

protected:
    KIGFX::COLOR4D getColor() const override;
};

#endif // TYPE_SCH_MARKER_H_
