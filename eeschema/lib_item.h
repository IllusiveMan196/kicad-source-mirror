/*
 * This program source code file is part of KiCad, a free EDA CAD application.
 *
 * Copyright (C) 2015 Jean-Pierre Charras, jaen-pierre.charras at wanadoo.fr
 * Copyright (C) 2015 Wayne Stambaugh <stambaughw@gmail.com>
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

#ifndef _LIB_ITEM_H_
#define _LIB_ITEM_H_

#include <eda_item.h>
#include <eda_rect.h>
#include <fill_type.h>
#include <transform.h>
#include <render_settings.h>

class LINE_READER;
class OUTPUTFORMATTER;
class LIB_SYMBOL;
class PLOTTER;
class LIB_PIN;
class MSG_PANEL_ITEM;

using KIGFX::RENDER_SETTINGS;

extern const int fill_tab[];


#define MINIMUM_SELECTION_DISTANCE 2 // Minimum selection distance in internal units


/**
 * Helper for defining a list of pin object pointers.  The list does not
 * use a Boost pointer class so the object pointers do not accidentally get
 * deleted when the container is deleted.
 */
typedef std::vector< LIB_PIN* > LIB_PINS;


/**
 * The base class for drawable items used by schematic library components.
 */
class LIB_ITEM : public EDA_ITEM
{
public:
    LIB_ITEM( KICAD_T aType, LIB_SYMBOL* aSymbol = nullptr, int aUnit = 0, int aConvert = 0,
              FILL_TYPE aFillType = FILL_TYPE::NO_FILL );

    // Do not create a copy constructor.  The one generated by the compiler is adequate.

    virtual ~LIB_ITEM() { }

    // Define the enums for basic
    enum LIB_CONVERT : int  { BASE = 1, DEMORGAN = 2 };

    /**
     * The list of flags used by the #compare function.
     *
     * - NORMAL This compares everything between two #LIB_ITEM objects.
     * - UNIT This compare flag ignores unit and convert and pin number information when
     *        comparing #LIB_ITEM objects for unit comparison.
     */
    enum COMPARE_FLAGS : int { NORMAL = 0x00, UNIT = 0x01 };

    /**
     * Provide a user-consumable name of the object type.  Perform localization when
     * called so that run-time language selection works.
     */
    virtual wxString GetTypeName() const = 0;

    /**
     * Begin drawing a component library draw item at \a aPosition.
     *
     * It typically would be called on a left click when a draw tool is selected in
     * the component library editor and one of the graphics tools is selected.
     *
     * @param aPosition The position in drawing coordinates where the drawing was started.
     *                  May or may not be required depending on the item being drawn.
     */
    virtual void BeginEdit( const wxPoint& aPosition ) {}

    /**
     * Continue an edit in progress at \a aPosition.
     *
     * This is used to perform the next action while drawing an item.  This would be
     * called for each additional left click when the mouse is captured while the item
     * is being drawn.
     *
     * @param aPosition The position of the mouse left click in drawing coordinates.
     * @return True if additional mouse clicks are required to complete the edit in progress.
     */
    virtual bool ContinueEdit( const wxPoint& aPosition ) { return false; }

    /**
     * End an object editing action.
     *
     * This is used to end or abort an edit action in progress initiated by BeginEdit().
     */
    virtual void EndEdit() {}

    /**
     * Calculate the attributes of an item at \a aPosition when it is being edited.
     *
     * This method gets called by the Draw() method when the item is being edited.  This
     * probably should be a pure virtual method but bezier curves are not yet editable in
     * the component library editor.  Therefore, the default method does nothing.
     *
     * @param aPosition The current mouse position in drawing coordinates.
     */
    virtual void CalcEdit( const wxPoint& aPosition ) {}

    /**
     * Draw an item
     *
     * @param aDC Device Context (can be null)
     * @param aOffset Offset to draw
     * @param aData Value or pointer used to pass others parameters, depending on body items.
     *              Used for some items to force to force no fill mode ( has meaning only for
     *              items what can be filled ). used in printing or moving objects mode or to
     *              pass reference to the lib component for pins.
     * @param aTransform Transform Matrix (rotation, mirror ..)
     */
    virtual void Print( const RENDER_SETTINGS* aSettings, const wxPoint &aOffset,
                        void* aData, const TRANSFORM& aTransform );

    virtual int GetPenWidth() const = 0;

    LIB_SYMBOL* GetParent() const
    {
        return (LIB_SYMBOL*) m_parent;
    }

    void ViewGetLayers( int aLayers[], int& aCount ) const override;

    bool HitTest( const wxPoint& aPosition, int aAccuracy = 0 ) const override
    {
        // This is just here to prevent annoying compiler warnings about hidden overloaded
        // virtual functions
        return EDA_ITEM::HitTest( aPosition, aAccuracy );
    }

    bool HitTest( const EDA_RECT& aRect, bool aContained, int aAccuracy = 0 ) const override;

    /**
     * @return the boundary box for this, in library coordinates
     */
    const EDA_RECT GetBoundingBox() const override { return EDA_ITEM::GetBoundingBox(); }

    /**
     * Display basic info (type, part and convert) about the current item in message panel.
     * <p>
     * This base function is used to display the information common to the
     * all library items.  Call the base class from the derived class or the
     * common information will not be updated in the message panel.
     * </p>
     * @param aList is the list to populate.
     */
    void GetMsgPanelInfo( EDA_DRAW_FRAME* aFrame, std::vector<MSG_PANEL_ITEM>& aList ) override;

    /**
     * Test LIB_ITEM objects for equivalence.
     *
     * @param aOther Object to test against.
     * @return True if object is identical to this object.
     */
    bool operator==( const LIB_ITEM& aOther ) const;
    bool operator==( const LIB_ITEM* aOther ) const
    {
        return *this == *aOther;
    }

    /**
     * Test if another draw item is less than this draw object.
     *
     * @param aOther - Draw item to compare against.
     * @return - True if object is less than this object.
     */
    bool operator<( const LIB_ITEM& aOther) const;

    /**
     * Set the drawing object by \a aOffset from the current position.
     *
     * @param aOffset Coordinates to offset the item position.
     */
    virtual void Offset( const wxPoint& aOffset ) = 0;

    /**
     * Move a draw object to \a aPosition.
     *
     * @param aPosition Position to move draw item to.
     */
    virtual void MoveTo( const wxPoint& aPosition ) = 0;

    void SetPosition( const wxPoint& aPosition ) override { MoveTo( aPosition ); }

    /**
     * Mirror the draw object along the horizontal (X) axis about \a aCenter point.
     *
     * @param aCenter Point to mirror around.
     */
    virtual void MirrorHorizontal( const wxPoint& aCenter ) = 0;

    /**
     * Mirror the draw object along the MirrorVertical (Y) axis about \a aCenter point.
     *
     * @param aCenter Point to mirror around.
     */
    virtual void MirrorVertical( const wxPoint& aCenter ) = 0;

    /**
     * Rotate the object about \a aCenter point.
     *
     * @param aCenter Point to rotate around.
     * @param aRotateCCW True to rotate counter clockwise.  False to rotate clockwise.
     */
    virtual void Rotate( const wxPoint& aCenter, bool aRotateCCW = true ) = 0;

    /**
     * Plot the draw item using the plot object.
     *
     * @param aPlotter The plot object to plot to.
     * @param aOffset Plot offset position.
     * @param aFill Flag to indicate whether or not the object is filled.
     * @param aTransform The plot transform.
     */
    virtual void Plot( PLOTTER* aPlotter, const wxPoint& aOffset, bool aFill,
                       const TRANSFORM& aTransform ) const = 0;

    virtual int GetWidth() const = 0;
    virtual void SetWidth( int aWidth ) = 0;

    /**
     * Check if draw object can be filled.
     *
     * The default setting is false.  If the derived object support filling, set the
     * m_isFillable member to true.
     */
    bool IsFillable() const { return m_isFillable; }

    void SetUnit( int aUnit ) { m_unit = aUnit; }
    int GetUnit() const { return m_unit; }

    void SetConvert( int aConvert ) { m_convert = aConvert; }
    int GetConvert() const { return m_convert; }

    void SetFillMode( FILL_TYPE aFillMode ) { m_fill = aFillMode; }
    FILL_TYPE GetFillMode() const { return m_fill; }

#if defined(DEBUG)
    void Show( int nestLevel, std::ostream& os ) const override { ShowDummy( os ); }
#endif

protected:
    /**
     * Provide the draw object specific comparison called by the == and < operators.
     *
     * The base object sort order which always proceeds the derived object sort order
     * is as follows:
     *      - Component alternate part (DeMorgan) number.
     *      - Component part number.
     *      - KICAD_T enum value.
     *      - Result of derived classes comparison.
     *
     * @note Make sure you call down to #LIB_ITEM::compare before doing any derived object
     *       comparisons or you will break the sorting using the symbol library file format.
     *
     * @param aOther A reference to the other #LIB_ITEM to compare the arc against.
     * @param aCompareFlags The flags used to perform the comparison.
     *
     * @return An integer value less than 0 if the object is less than \a aOther object,
     *         zero if the object is equal to \a aOther object, or greater than 0 if the
     *         object is greater than \a aOther object.
     */
    virtual int compare( const LIB_ITEM& aOther,
            LIB_ITEM::COMPARE_FLAGS aCompareFlags = LIB_ITEM::COMPARE_FLAGS::NORMAL ) const;

    /**
     * Print the item to \a aDC.
     *
     * @param aOffset A reference to a wxPoint object containing the offset where to draw
     *                from the object's current position.
     * @param aData A pointer to any object specific data required to perform the draw.
     * @param aTransform A reference to a #TRANSFORM object containing drawing transform.
     */
    virtual void print( const RENDER_SETTINGS* aSettings, const wxPoint& aOffset, void* aData,
                        const TRANSFORM& aTransform ) = 0;

private:
    friend class LIB_SYMBOL;

protected:
    /**
     * Unit identification for multiple parts per package.  Set to 0 if the item is common
     * to all units.
     */
    int         m_unit;

    /**
     * Shape identification for alternate body styles.  Set 0 if the item is common to all
     * body styles.  This is typially used for representing DeMorgan variants in KiCad.
     */
    int         m_convert;

    /**
     * The body fill type.  This has meaning only for some items.  For a list of fill types
     * see #FILL_TYPE.
     */
    FILL_TYPE   m_fill;
    bool        m_isFillable;
};


#endif  //  _LIB_ITEM_H_
