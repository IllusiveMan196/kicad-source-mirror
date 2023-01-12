/*
 * This program source code file is part of KiCad, a free EDA CAD application.
 *
 * Copyright (C) 2004 Jean-Pierre Charras, jaen-pierre.charras@gipsa-lab.inpg.com
 * Copyright (C) 1992-2022 KiCad Developers, see AUTHORS.txt for contributors.
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

#ifndef FP_TEXT_H
#define FP_TEXT_H

#include <eda_text.h>
#include <board_item.h>

class LINE_READER;
class FOOTPRINT;
class MSG_PANEL_ITEM;
class PCB_BASE_FRAME;
class SHAPE;


class FP_TEXT : public BOARD_ITEM, public EDA_TEXT
{
public:
    /**
     * Footprint text type: there must be only one (and only one) for each of the reference
     * value texts in one footprint; others could be added for the user (DIVERS is French for
     * 'others'). Reference and value always live on silkscreen (on the footprint side); other
     * texts are planned to go on whatever layer the user wants.
     */
    enum TEXT_TYPE
    {
        TEXT_is_REFERENCE = 0,
        TEXT_is_VALUE     = 1,
        TEXT_is_DIVERS    = 2
    };

    FP_TEXT( FOOTPRINT* aParentFootprint, TEXT_TYPE text_type = TEXT_is_DIVERS );

    // Do not create a copy constructor & operator=.
    // The ones generated by the compiler are adequate.

    ~FP_TEXT();

    static inline bool ClassOf( const EDA_ITEM* aItem )
    {
        return aItem && aItem->Type() == PCB_FP_TEXT_T;
    }

    bool IsType( const std::vector<KICAD_T>& aScanTypes ) const override
    {
        if( BOARD_ITEM::IsType( aScanTypes ) )
            return true;

        for( KICAD_T scanType : aScanTypes )
        {
            if( scanType == PCB_LOCATE_TEXT_T )
                return true;
        }

        return false;
    }

    wxString GetParentAsString() const;

    bool Matches( const EDA_SEARCH_DATA& aSearchData, void* aAuxData ) const override
    {
        return BOARD_ITEM::Matches( GetShownText(), aSearchData );
    }

    virtual VECTOR2I GetPosition() const override
    {
        return EDA_TEXT::GetTextPos();
    }

    virtual void SetPosition( const VECTOR2I& aPos ) override
    {
        EDA_TEXT::SetTextPos( aPos );
        SetLocalCoord();
    }

    /**
     * Called when rotating the parent footprint.
     */
    void KeepUpright( const EDA_ANGLE& aOldOrientation, const EDA_ANGLE& aNewOrientation );

    void Rotate( const VECTOR2I& aOffset, const EDA_ANGLE& aAngle ) override;

    /// Flip entity during footprint flip
    void Flip( const VECTOR2I& aCentre, bool aFlipLeftRight ) override;

    bool IsParentFlipped() const;

    /**
     * Mirror text position.  Do not mirror the text itself, or change its layer.
     */
    void Mirror( const VECTOR2I& aCentre, bool aMirrorAroundXAxis );

    void Move( const VECTOR2I& aMoveVector ) override;

    /// @deprecated it seems (but the type is used to 'protect' reference and value from deletion,
    /// and for identification)
    void SetType( TEXT_TYPE aType )      { m_Type = aType; }
    TEXT_TYPE GetType() const            { return m_Type; }

    // The Pos0 accessors are for footprint-relative coordinates.
    void SetPos0( const VECTOR2I& aPos ) { m_Pos0 = aPos; SetDrawCoord(); }
    const VECTOR2I& GetPos0() const      { return m_Pos0; }

    int GetLength() const;        // text length

    /**
     * @return the text rotation for drawings and plotting the footprint rotation is taken
     *         in account.
     */
    virtual EDA_ANGLE GetDrawRotation() const override;

    // Virtual function
    const BOX2I GetBoundingBox() const override;

    ///< Set absolute coordinates.
    void SetDrawCoord();

    ///< Set relative coordinates.
    void SetLocalCoord();

    void GetMsgPanelInfo( EDA_DRAW_FRAME* aFrame, std::vector<MSG_PANEL_ITEM>& aList ) override;

    bool TextHitTest( const VECTOR2I& aPoint, int aAccuracy = 0 ) const override;
    bool TextHitTest( const BOX2I& aRect, bool aContains, int aAccuracy = 0 ) const override;

    bool HitTest( const VECTOR2I& aPosition, int aAccuracy ) const override
    {
        return TextHitTest( aPosition, aAccuracy );
    }

    bool HitTest( const BOX2I& aRect, bool aContained, int aAccuracy = 0 ) const override
    {
        return TextHitTest( aRect, aContained, aAccuracy );
    }

    void TransformShapeToPolygon( SHAPE_POLY_SET& aBuffer, PCB_LAYER_ID aLayer, int aClearance,
                                  int aError, ERROR_LOC aErrorLoc,
                                  bool aIgnoreLineWidth ) const override;

    void TransformTextToPolySet( SHAPE_POLY_SET& aBuffer, PCB_LAYER_ID aLayer, int aClearance,
                                 int aError, ERROR_LOC aErrorLoc ) const;

    // @copydoc BOARD_ITEM::GetEffectiveShape
    std::shared_ptr<SHAPE> GetEffectiveShape( PCB_LAYER_ID aLayer = UNDEFINED_LAYER,
                                              FLASHING aFlash = FLASHING::DEFAULT ) const override;

    wxString GetClass() const override
    {
        return wxT( "FP_TEXT" );
    }

    wxString GetItemDescription( UNITS_PROVIDER* aUnitsProvider ) const override;

    BITMAPS GetMenuImage() const override;

    EDA_ITEM* Clone() const override;

    virtual wxString GetShownText( int aDepth = 0, bool aAllowExtraText = true ) const override;

    virtual const BOX2I ViewBBox() const override;

    virtual void ViewGetLayers( int aLayers[], int& aCount ) const override;

    double ViewGetLOD( int aLayer, KIGFX::VIEW* aView ) const override;

#if defined(DEBUG)
    virtual void Show( int nestLevel, std::ostream& os ) const override { ShowDummy( os ); }
#endif

private:
    TEXT_TYPE m_Type;           ///< 0=ref, 1=val, etc.

    VECTOR2I  m_Pos0;           ///< text coordinates relative to the footprint anchor, orient 0.
                                ///< text coordinate ref point is the text center
};

#endif // FP_TEXT_H
