/*
 * This program source code file is part of KiCad, a free EDA CAD application.
 *
 * Copyright (C) 2016 Jean-Pierre Charras, jp.charras at wanadoo.fr
 * Copyright (C) 2013 Wayne Stambaugh <stambaughw@verizon.net>
 * Copyright (C) 1992-2016 KiCad Developers, see AUTHORS.txt for contributors.
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

/**
 * @file netlist_object.cpp
 * @brief Class NETLIST_OBJECT to handle 1 item connected (in netlist and erc calculations)
 */

#include <fctsys.h>
#include <macros.h>
#include <list>

#include <sch_component.h>
#include <sch_connection.h>
#include <netlist_object.h>
#include <sch_edit_frame.h>
#include <schematic.h>

#if defined(DEBUG)

#include <iostream>
const char* ShowType( NETLIST_ITEM aType )
{
    const char* ret;

    switch( aType )
    {
    case NETLIST_ITEM::SEGMENT:
        ret = "segment";            break;

    case NETLIST_ITEM::BUS:
        ret = "bus";                break;

    case NETLIST_ITEM::JUNCTION:
        ret = "junction";           break;

    case NETLIST_ITEM::LABEL:
        ret = "label";              break;

    case NETLIST_ITEM::HIERLABEL:
        ret = "hierlabel";          break;

    case NETLIST_ITEM::GLOBLABEL:
        ret = "glabel";             break;

    case NETLIST_ITEM::BUSLABELMEMBER:
        ret = "buslblmember";       break;

    case NETLIST_ITEM::HIERBUSLABELMEMBER:
        ret = "hierbuslblmember";   break;

    case NETLIST_ITEM::GLOBBUSLABELMEMBER:
        ret = "gbuslblmember";      break;

    case NETLIST_ITEM::SHEETBUSLABELMEMBER:
        ret = "sbuslblmember";      break;

    case NETLIST_ITEM::SHEETLABEL:
        ret = "sheetlabel";         break;

    case NETLIST_ITEM::PINLABEL:
        ret = "pinlabel";           break;

    case NETLIST_ITEM::PIN:
        ret = "pin";                break;

    case NETLIST_ITEM::NOCONNECT:
        ret = "noconnect";          break;

    default:
        ret = "??";                 break;
    }

    return ret;
}


void NETLIST_OBJECT::Show( std::ostream& out, int ndx ) const
{
    wxString path = m_SheetPath.PathHumanReadable();

    out << "<netItem ndx=\"" << ndx << '"' <<
    " type=\"" << ShowType( m_Type ) << '"' <<
    " netCode=\"" << GetNet() << '"' <<
    " sheet=\"" << TO_UTF8( path ) << '"' <<
    ">\n";

    out << " <start " << m_Start << "/> <end " << m_End << "/>\n";

    if( !m_Label.IsEmpty() )
        out << " <label>" << m_Label.mb_str() << "</label>\n";

    out << " <sheetpath>" << m_SheetPath.PathHumanReadable().mb_str() << "</sheetpath>\n";

    switch( m_Type )
    {
    case NETLIST_ITEM::PIN:
        /* GetRef() needs to be const
        out << " <refOfComp>" << GetComponentParent()->GetRef(&m_SheetPath).mb_str()
            << "</refOfComp>\n";
        */

        if( m_Comp )
            m_Comp->Show( 1, out );

        break;

    default:
        // not all the m_Comp classes have working Show functions.
        ;
    }

/*  was segfault-ing
    if( m_Comp )
        m_Comp->Show( 1, out );     // labels may not have good Show() funcs?
    else
        out << " m_Comp==NULL\n";
*/

    out << "</netItem>\n";
}

#endif


NETLIST_OBJECT::NETLIST_OBJECT()
{
    m_Type = NETLIST_ITEM::ITEM_UNSPECIFIED; /* Type of this item (see NETLIST_ITEM_T enum) */
    m_Comp = NULL;    /* Pointer on the library item that created this net object
                       * (the parent)*/
    m_Link = NULL;    /* For SCH_SHEET_PIN:
                       * Pointer to the hierarchy sheet that contains this
                       * SCH_SHEET_PIN For Pins: pointer to the component that
                       * contains this pin
                       */
    m_Flag    = 0;    /* flag used in calculations */
    m_netCode = 0;    /* net code for all items except BUS labels because a BUS
                       * label has as many net codes as bus members
                       */
    m_BusNetCode = 0; /* Used for BUS connections */
    m_Member     = 0; /* for labels type NETLIST_ITEM::BUSLABELMEMBER ( bus member created
                       * from the BUS label )  member number
                       */
    m_ConnectionType = NET_CONNECTION::UNCONNECTED;
    m_ElectricalPinType =
            ELECTRICAL_PINTYPE::PT_INPUT; /* Has meaning only for Pins: electrical type of the pin
                                        * used to detect conflicts between pins in ERC
                                        */
    m_netNameCandidate = NULL;         /* a pointer to a NETLIST_OBJECT type label connected to this
                                        * object used to give a name to the net
                                        */
}


// Copy constructor
NETLIST_OBJECT::NETLIST_OBJECT( NETLIST_OBJECT& aSource )
{
    *this = aSource;
}


NETLIST_OBJECT::~NETLIST_OBJECT()
{
}


// return true if the object is a label of any type
bool NETLIST_OBJECT::IsLabelType() const
{
    return m_Type == NETLIST_ITEM::LABEL || m_Type == NETLIST_ITEM::GLOBLABEL
           || m_Type == NETLIST_ITEM::HIERLABEL || m_Type == NETLIST_ITEM::BUSLABELMEMBER
           || m_Type == NETLIST_ITEM::GLOBBUSLABELMEMBER
           || m_Type == NETLIST_ITEM::HIERBUSLABELMEMBER || m_Type == NETLIST_ITEM::PINLABEL;
}

bool NETLIST_OBJECT::IsLabelConnected( NETLIST_OBJECT* aNetItem )
{
    if( aNetItem == this )   // Don't compare the same net list object.
        return false;

    NETLIST_ITEM at = m_Type;
    NETLIST_ITEM bt = aNetItem->m_Type;

    if( ( at == NETLIST_ITEM::HIERLABEL || at == NETLIST_ITEM::HIERBUSLABELMEMBER )
            && ( bt == NETLIST_ITEM::SHEETLABEL || bt == NETLIST_ITEM::SHEETBUSLABELMEMBER ) )
    {
        if( m_SheetPath == aNetItem->m_SheetPathInclude )
        {
            return true; //connected!
        }
    }
    else if( ( at == NETLIST_ITEM::GLOBLABEL ) && ( bt == NETLIST_ITEM::GLOBLABEL ) )
    {
        if( m_Label == aNetItem->m_Label )
            return true; //connected!
    }

    return false; //these two are unconnected
}


void NETLIST_OBJECT::ConvertBusToNetListItems( NETLIST_OBJECT_LIST& aNetListItems )
{
    SCH_CONNECTION conn;
    wxCHECK_RET( conn.IsBusLabel( m_Label ),
                 wxT( "<" ) + m_Label + wxT( "> is not a valid bus label." ) );

    if( m_Type == NETLIST_ITEM::HIERLABEL )
        m_Type = NETLIST_ITEM::HIERBUSLABELMEMBER;
    else if( m_Type == NETLIST_ITEM::GLOBLABEL )
        m_Type = NETLIST_ITEM::GLOBBUSLABELMEMBER;
    else if( m_Type == NETLIST_ITEM::SHEETLABEL )
        m_Type = NETLIST_ITEM::SHEETBUSLABELMEMBER;
    else if( m_Type == NETLIST_ITEM::LABEL )
        m_Type = NETLIST_ITEM::BUSLABELMEMBER;
    else
        wxCHECK_RET( false, wxT( "Net list object type is not valid." ) );

    // NOTE: all netlist objects generated from a single bus definition need to have different
    // member codes set.  For bus vectors, the member code matches the vector index, but for
    // bus groups (including with nested vectors) the code is something arbitrary.
    long member_offset = 0;

    auto alias = static_cast<SCH_ITEM*>( m_Comp )->Schematic()->GetBusAlias( m_Label );
    wxString group_name;
    bool self_set = false;
    std::vector<wxString> bus_contents_vec;

    if( alias || SCH_CONNECTION::ParseBusGroup( m_Label, &group_name, &bus_contents_vec ) )
    {
        if( alias )
        {
            for( const wxString& member : alias->Members() )
                bus_contents_vec.emplace_back( member );
        }

        // For named bus groups, like "USB{DP DM}"
        wxString group_prefix = ( group_name != "" ) ? ( group_name + "." ) : "";

        std::list<wxString> bus_contents( bus_contents_vec.begin(), bus_contents_vec.end() );

        for( const auto& bus_member : bus_contents )
        {
            wxString prefix;
            std::vector<wxString> members;

            // Nested bus vector inside a bus group
            if( SCH_CONNECTION::ParseBusVector( bus_member, &prefix, &members ) )
            {
                long begin, end;

                prefix = group_prefix + prefix;
                begin = conn.VectorStart();
                end = conn.VectorEnd();

                if( !self_set )
                {
                    m_Label = members[0];
                    m_Member = ( begin++ ) + ( member_offset++ );

                    self_set = true;
                    begin++;
                }

                fillBusVector( aNetListItems, prefix, begin, end, member_offset );
                member_offset += std::abs( end - begin );
            }
            else if( auto nested_alias = static_cast<SCH_ITEM*>( m_Comp )->Schematic()->GetBusAlias(
                             bus_member ) )
            {
                // Nested alias inside a group
                for( const auto& alias_member : nested_alias->Members() )
                {
                    bus_contents.push_back( alias_member );
                }
            }
            else
            {
                if( !self_set )
                {
                    m_Label = group_prefix + bus_member;
                    m_Member = member_offset++;
                    self_set = true;
                }
                else
                {
                    auto item = new NETLIST_OBJECT( *this );
                    item->m_Label = group_prefix + bus_member;
                    item->m_Member = member_offset++;
                    aNetListItems.push_back( item );
                }
            }
        }
    }
    else if( SCH_CONNECTION::ParseBusVector( m_Label, &group_name, &bus_contents_vec ) )
    {
        long begin = conn.VectorStart();
        long end = conn.VectorEnd();

        m_Label = bus_contents_vec[0];
        m_Member = begin;

        fillBusVector( aNetListItems, group_name, begin + 1, end, 0 );
    }
}

void NETLIST_OBJECT::fillBusVector( NETLIST_OBJECT_LIST& aNetListItems,
                                    wxString aName, long aBegin, long aEnd, long aOffset )
{
    for( long member = aBegin; member <= aEnd; member++ )
    {
        auto item = new NETLIST_OBJECT( *this );

        item->m_Label = aName;
        item->m_Label << member;
        item->m_Member = member;

        aNetListItems.push_back( item );
    }
}


bool NETLIST_OBJECT::IsLabelGlobal() const
{
    // return true if the object is a global label
    // * a actual global label
    // * a pin label coming from a invisible power pin
    return ( m_Type == NETLIST_ITEM::PINLABEL ) || ( m_Type == NETLIST_ITEM::GLOBLABEL )
           || ( m_Type == NETLIST_ITEM::GLOBBUSLABELMEMBER );
}


bool NETLIST_OBJECT::IsLabelBusMemberType() const
{
    // return true if the object is a bus label member build from a
    // schematic bus label (like label[xx..yy)
    // They are labels with very specific properties, especially for connection
    // between them: 2 bus label members can be connected only
    // if they have the same member value.
    return ( m_Type == NETLIST_ITEM::SHEETBUSLABELMEMBER )
           || ( m_Type == NETLIST_ITEM::BUSLABELMEMBER )
           || ( m_Type == NETLIST_ITEM::HIERBUSLABELMEMBER )
           || ( m_Type == NETLIST_ITEM::GLOBBUSLABELMEMBER );
}


/*
 * return the net name of the item
 */
wxString NETLIST_OBJECT::GetNetName() const
{
    if( m_netNameCandidate == NULL )
        return wxEmptyString;

    wxString netName;

    if( m_netNameCandidate->m_Type == NETLIST_ITEM::PIN )
        return GetShortNetName();

    if( !m_netNameCandidate->IsLabelGlobal() )
    {
        // usual net name, prefix it by the sheet path
        netName = m_netNameCandidate->m_SheetPath.PathHumanReadable();
    }

    netName += m_netNameCandidate->m_Label;

    return netName;
}

/**
 * return the short net name of the item i.e. the net name
 * from the "best" label without any prefix.
 * 2 different nets can have the same short name
 */
wxString NETLIST_OBJECT::GetShortNetName() const
{
    if( m_netNameCandidate == NULL )
        return wxEmptyString;

    wxString netName;

    if( m_netNameCandidate->m_Type == NETLIST_ITEM::PIN )
    {
        SCH_COMPONENT* link = m_netNameCandidate->GetComponentParent();
        if( link )  // Should be always true
        {
            netName = wxT("Net-(");
            netName << link->GetRef( &m_netNameCandidate->m_SheetPath );
            netName << wxT("-Pad") << m_netNameCandidate->m_PinNum << wxT(")");
        }
    }
    else
        netName = m_netNameCandidate->m_Label;

    return netName;
}

/**
 * Set m_netNameCandidate to a connected item which will
 * be used to calcule the net name of the item
 * Obviously the candidate can be only a label
 * If there is no label on the net, a pad name will be
 * used to build a net name (something like Cmp<REF>_Pad<PAD_NAME>
 * @param aCandidate = the connected item candidate
 */
void NETLIST_OBJECT::SetNetNameCandidate( NETLIST_OBJECT* aCandidate )
{
    switch( aCandidate->m_Type )
    {
    case NETLIST_ITEM::HIERLABEL:
    case NETLIST_ITEM::LABEL:
    case NETLIST_ITEM::PINLABEL:
    case NETLIST_ITEM::GLOBLABEL:
    case NETLIST_ITEM::GLOBBUSLABELMEMBER:
    case NETLIST_ITEM::SHEETBUSLABELMEMBER:
    case NETLIST_ITEM::PIN:
        m_netNameCandidate = aCandidate;
        break;

    default:
        break;
    }
}


const wxString NETLIST_OBJECT::GetPinNameText() const
{
    wxString name;
    // returns the pin name, for NETLIST_ITEM::PIN (usual pin) item.
    if( m_Type == NETLIST_ITEM::PIN )
    {
        name = static_cast<LIB_PIN*>( m_Comp )->GetName();

        if( name == "~" )   //empty name
            name = wxEmptyString;
    }

    return name;
}
