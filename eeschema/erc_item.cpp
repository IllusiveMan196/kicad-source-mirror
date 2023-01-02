/*
 * This program source code file is part of KiCad, a free EDA CAD application.
 *
 * Copyright (C) 2020 KiCad Developers, see AUTHORS.txt for contributors.
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


#include "wx/html/m_templ.h"
#include "wx/html/styleparams.h"
#include <erc.h>
#include <erc_item.h>
#include <i18n_utility.h>


// These, being statically-defined, require specialized I18N handling.  We continue to
// use the _() macro so that string harvesting by the I18N framework doesn't have to be
// specialized, but we don't translate on initialization and instead do it in the getters.

#undef _
#define _(s) s

// NOTE: Avoid changing the settings key for an ERC item after it has been created

ERC_ITEM ERC_ITEM::heading_connections( 0, _( "Connections" ), "" );
ERC_ITEM ERC_ITEM::heading_conflicts( 0, _( "Conflicts" ), "" );
ERC_ITEM ERC_ITEM::heading_misc( 0, _( "Miscellaneous" ), "" );

ERC_ITEM ERC_ITEM::duplicateSheetName( ERCE_DUPLICATE_SHEET_NAME,
        _( "Duplicate sheet names within a given sheet" ),
        wxT( "duplicate_sheet_names" ) );

ERC_ITEM ERC_ITEM::endpointOffGrid( ERCE_ENDPOINT_OFF_GRID,
        _( "Symbol pin or wire end off grid" ),
        wxT( "endpoint_off_grid" ) );

ERC_ITEM ERC_ITEM::pinNotConnected( ERCE_PIN_NOT_CONNECTED,
        _( "Pin not connected" ),
        wxT( "pin_not_connected" ) );

ERC_ITEM ERC_ITEM::pinNotDriven( ERCE_PIN_NOT_DRIVEN,
        _( "Input pin not driven by any Output pins" ),
        wxT( "pin_not_driven" ) );

ERC_ITEM ERC_ITEM::powerpinNotDriven( ERCE_POWERPIN_NOT_DRIVEN,
        _( "Input Power pin not driven by any Output Power pins" ),
        wxT( "power_pin_not_driven" ) );

ERC_ITEM ERC_ITEM::pinTableWarning( ERCE_PIN_TO_PIN_WARNING,
        _( "Conflict problem between pins" ),
        wxT( "pin_to_pin" ) );

ERC_ITEM ERC_ITEM::pinTableError( ERCE_PIN_TO_PIN_ERROR,
        _( "Conflict problem between pins" ),
        wxT( "pin_to_pin" ) );

ERC_ITEM ERC_ITEM::hierLabelMismatch( ERCE_HIERACHICAL_LABEL,
        _( "Mismatch between hierarchical labels and sheet pins" ),
        wxT( "hier_label_mismatch" ) );

ERC_ITEM ERC_ITEM::noConnectConnected( ERCE_NOCONNECT_CONNECTED,
        _( "A pin with a \"no connection\" flag is connected" ),
        wxT( "no_connect_connected" ) );

ERC_ITEM ERC_ITEM::noConnectDangling( ERCE_NOCONNECT_NOT_CONNECTED,
        _( "Unconnected \"no connection\" flag" ),
        wxT( "no_connect_dangling" ) );

ERC_ITEM ERC_ITEM::labelDangling( ERCE_LABEL_NOT_CONNECTED,
        _( "Label not connected to anything" ),
        wxT( "label_dangling" ) );

ERC_ITEM ERC_ITEM::globalLabelDangling( ERCE_GLOBLABEL,
        _( "Global label not connected anywhere else in the schematic" ),
        wxT( "global_label_dangling" ) );

ERC_ITEM ERC_ITEM::similarLabels( ERCE_SIMILAR_LABELS,
        _( "Labels are similar (lower/upper case difference only)"),
        wxT( "similar_labels" ) );

ERC_ITEM ERC_ITEM::differentUnitFootprint( ERCE_DIFFERENT_UNIT_FP,
        _( "Different footprint assigned in another unit of the symbol" ),
        wxT( "different_unit_footprint" ) );

ERC_ITEM ERC_ITEM::differentUnitNet( ERCE_DIFFERENT_UNIT_NET,
        _( "Different net assigned to a shared pin in another unit of the symbol" ),
        wxT( "different_unit_net" ) );

ERC_ITEM ERC_ITEM::busDefinitionConflict( ERCE_BUS_ALIAS_CONFLICT,
        _( "Conflict between bus alias definitions across schematic sheets" ),
        wxT( "bus_definition_conflict" ) );

ERC_ITEM ERC_ITEM::multipleNetNames( ERCE_DRIVER_CONFLICT,
        _( "More than one name given to this bus or net" ),
        wxT( "multiple_net_names" ) );

ERC_ITEM ERC_ITEM::netclassConflict( ERCE_NETCLASS_CONFLICT,
        _( "Conflicting netclass assignments" ),
        wxT( "conflicting_netclasses" ) );

ERC_ITEM ERC_ITEM::netNotBusMember( ERCE_BUS_ENTRY_CONFLICT,
        _( "Net is graphically connected to a bus but not a bus member" ),
        wxT( "net_not_bus_member" ) );

ERC_ITEM ERC_ITEM::busLabelSyntax( ERCE_BUS_LABEL_ERROR,
        _( "Label attached to bus item does not describe a bus" ),
        wxT( "bus_label_syntax" ) );

ERC_ITEM ERC_ITEM::busToBusConflict( ERCE_BUS_TO_BUS_CONFLICT,
        _( "Buses are graphically connected but share no bus members" ),
        wxT( "bus_to_bus_conflict" ) );

ERC_ITEM ERC_ITEM::busToNetConflict( ERCE_BUS_TO_NET_CONFLICT,
        _( "Invalid connection between bus and net items" ),
        wxT( "bus_to_net_conflict" ) );

ERC_ITEM ERC_ITEM::unresolvedVariable( ERCE_UNRESOLVED_VARIABLE,
        _( "Unresolved text variable" ),
        wxT( "unresolved_variable" ) );

ERC_ITEM ERC_ITEM::simulationModelIssues( ERCE_SIMULATION_MODEL,
        _( "SPICE model issue" ),
        wxT( "simulation_model_issue" ) );

ERC_ITEM ERC_ITEM::wireDangling( ERCE_WIRE_DANGLING,
        _( "Wires not connected to anything" ),
        wxT( "wire_dangling" ) );

ERC_ITEM ERC_ITEM::libSymbolIssues( ERCE_LIB_SYMBOL_ISSUES,
        _( "Library symbol issue" ),
        wxT( "lib_symbol_issues" ) );

ERC_ITEM ERC_ITEM::unannotated( ERCE_UNANNOTATED,
        _( "Symbol is not annotated" ),
        wxT( "unannotated" ) );

ERC_ITEM ERC_ITEM::extraUnits( ERCE_EXTRA_UNITS,
        _( "Symbol has more units than are defined" ),
        wxT( "extra_units" ) );

ERC_ITEM ERC_ITEM::missingUnits( ERCE_MISSING_UNIT,
        _( "Symbol has units that are not placed" ),
        wxT( "missing_unit" ) );

ERC_ITEM ERC_ITEM::missingInputPin( ERCE_MISSING_INPUT_PIN,
        _( "Symbol has input pins that are not placed" ),
        wxT( "missing_input_pin" ) );

ERC_ITEM ERC_ITEM::missingBidiPin( ERCE_MISSING_BIDI_PIN,
        _( "Symbol has bidirectional pins that are not placed" ),
        wxT( "missing_bidi_pin" ) );

ERC_ITEM ERC_ITEM::missingPowerInputPin( ERCE_MISSING_POWER_INPUT_PIN,
        _( "Symbol has power input pins that are not placed" ),
        wxT( "missing_power_pin" ) );

ERC_ITEM ERC_ITEM::differentUnitValue( ERCE_DIFFERENT_UNIT_VALUE,
        _( "Units of same symbol have different values" ),
        wxT( "unit_value_mismatch" ) );

ERC_ITEM ERC_ITEM::duplicateReference( ERCE_DUPLICATE_REFERENCE,
        _( "Duplicate reference designators" ),
        wxT( "duplicate_reference" ) );

ERC_ITEM ERC_ITEM::busEntryNeeded( ERCE_BUS_ENTRY_NEEDED,
        _( "Bus Entry needed" ),
        wxT( "bus_entry_needed" ) );

std::vector<std::reference_wrapper<RC_ITEM>> ERC_ITEM::allItemTypes( {
                 ERC_ITEM::heading_connections,
                 ERC_ITEM::pinNotConnected,
                 ERC_ITEM::pinNotDriven,
                 ERC_ITEM::powerpinNotDriven,
                 ERC_ITEM::noConnectConnected,
                 ERC_ITEM::noConnectDangling,
                 ERC_ITEM::labelDangling,
                 ERC_ITEM::globalLabelDangling,
                 ERC_ITEM::wireDangling,
                 ERC_ITEM::busEntryNeeded,
                 ERC_ITEM::endpointOffGrid,

                 ERC_ITEM::heading_conflicts,
                 ERC_ITEM::duplicateReference,
                 ERC_ITEM::pinTableWarning,
                 ERC_ITEM::differentUnitValue,
                 ERC_ITEM::differentUnitFootprint,
                 ERC_ITEM::differentUnitNet,
                 ERC_ITEM::duplicateSheetName,
                 ERC_ITEM::hierLabelMismatch,
                 ERC_ITEM::multipleNetNames,
                 ERC_ITEM::busDefinitionConflict,
                 ERC_ITEM::busToBusConflict,
                 ERC_ITEM::busToNetConflict,
                 ERC_ITEM::netNotBusMember,
                 ERC_ITEM::netclassConflict,

                 ERC_ITEM::heading_misc,
                 ERC_ITEM::unannotated,
                 ERC_ITEM::unresolvedVariable,
                 ERC_ITEM::simulationModelIssues,
                 ERC_ITEM::similarLabels,
                 ERC_ITEM::busLabelSyntax,
                 ERC_ITEM::libSymbolIssues,
                 ERC_ITEM::extraUnits,
                 ERC_ITEM::missingUnits,
                 ERC_ITEM::missingInputPin,
                 ERC_ITEM::missingBidiPin,
                 ERC_ITEM::missingPowerInputPin
         } );



std::shared_ptr<ERC_ITEM> ERC_ITEM::Create( int aErrorCode )
{
    switch( aErrorCode )
    {
    case ERCE_DUPLICATE_SHEET_NAME:    return std::make_shared<ERC_ITEM>( duplicateSheetName );
    case ERCE_ENDPOINT_OFF_GRID:       return std::make_shared<ERC_ITEM>( endpointOffGrid );
    case ERCE_PIN_NOT_CONNECTED:       return std::make_shared<ERC_ITEM>( pinNotConnected );
    case ERCE_PIN_NOT_DRIVEN:          return std::make_shared<ERC_ITEM>( pinNotDriven );
    case ERCE_POWERPIN_NOT_DRIVEN:     return std::make_shared<ERC_ITEM>( powerpinNotDriven );
    case ERCE_PIN_TO_PIN_WARNING:      return std::make_shared<ERC_ITEM>( pinTableWarning );
    case ERCE_PIN_TO_PIN_ERROR:        return std::make_shared<ERC_ITEM>( pinTableError );
    case ERCE_HIERACHICAL_LABEL:       return std::make_shared<ERC_ITEM>( hierLabelMismatch );
    case ERCE_NOCONNECT_CONNECTED:     return std::make_shared<ERC_ITEM>( noConnectConnected );
    case ERCE_NOCONNECT_NOT_CONNECTED: return std::make_shared<ERC_ITEM>( noConnectDangling );
    case ERCE_LABEL_NOT_CONNECTED:     return std::make_shared<ERC_ITEM>( labelDangling );
    case ERCE_SIMILAR_LABELS:          return std::make_shared<ERC_ITEM>( similarLabels );
    case ERCE_DIFFERENT_UNIT_FP:       return std::make_shared<ERC_ITEM>( differentUnitFootprint );
    case ERCE_DIFFERENT_UNIT_NET:      return std::make_shared<ERC_ITEM>( differentUnitNet );
    case ERCE_BUS_ALIAS_CONFLICT:      return std::make_shared<ERC_ITEM>( busDefinitionConflict );
    case ERCE_DRIVER_CONFLICT:         return std::make_shared<ERC_ITEM>( multipleNetNames );
    case ERCE_BUS_ENTRY_CONFLICT:      return std::make_shared<ERC_ITEM>( netNotBusMember );
    case ERCE_BUS_LABEL_ERROR:         return std::make_shared<ERC_ITEM>( busLabelSyntax );
    case ERCE_BUS_TO_BUS_CONFLICT:     return std::make_shared<ERC_ITEM>( busToBusConflict );
    case ERCE_BUS_TO_NET_CONFLICT:     return std::make_shared<ERC_ITEM>( busToNetConflict );
    case ERCE_NETCLASS_CONFLICT:       return std::make_shared<ERC_ITEM>( netclassConflict );
    case ERCE_GLOBLABEL:               return std::make_shared<ERC_ITEM>( globalLabelDangling );
    case ERCE_UNRESOLVED_VARIABLE:     return std::make_shared<ERC_ITEM>( unresolvedVariable );
    case ERCE_SIMULATION_MODEL:        return std::make_shared<ERC_ITEM>( simulationModelIssues );
    case ERCE_WIRE_DANGLING:           return std::make_shared<ERC_ITEM>( wireDangling );
    case ERCE_LIB_SYMBOL_ISSUES:       return std::make_shared<ERC_ITEM>( libSymbolIssues );
    case ERCE_UNANNOTATED:             return std::make_shared<ERC_ITEM>( unannotated );
    case ERCE_EXTRA_UNITS:             return std::make_shared<ERC_ITEM>( extraUnits );
    case ERCE_DIFFERENT_UNIT_VALUE:    return std::make_shared<ERC_ITEM>( differentUnitValue );
    case ERCE_DUPLICATE_REFERENCE:     return std::make_shared<ERC_ITEM>( duplicateReference );
    case ERCE_BUS_ENTRY_NEEDED:        return std::make_shared<ERC_ITEM>( busEntryNeeded );
    case ERCE_MISSING_UNIT:            return std::make_shared<ERC_ITEM>( missingUnits );
    case ERCE_MISSING_INPUT_PIN:       return std::make_shared<ERC_ITEM>( missingInputPin );
    case ERCE_MISSING_POWER_INPUT_PIN: return std::make_shared<ERC_ITEM>( missingPowerInputPin );
    case ERCE_MISSING_BIDI_PIN:        return std::make_shared<ERC_ITEM>( missingBidiPin );
    case ERCE_UNSPECIFIED:
    default:
        wxFAIL_MSG( "Unknown ERC error code" );
        return nullptr;
    }

    return nullptr;
}
