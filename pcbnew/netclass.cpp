/*
 * This program source code file is part of KiCad, a free EDA CAD application.
 *
 * Copyright (C) 2009 SoftPLC Corporation, Dick Hollenbeck <dick@softplc.com>
 * Copyright (C) 2009 Jean-Pierre Charras, jean-pierre.charras@inpg.fr
 * Copyright (C) 2009 KiCad Developers, see change_log.txt for contributors.
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

#include <fctsys.h>
#include <common.h>
#include <kicad_string.h>
#include <pcbnew.h>
#include <richio.h>
#include <macros.h>

#include <class_board.h>
#include <netclass.h>

// This will get mapped to "kicad_default" in the specctra_export.
const char NETCLASS::Default[] = "Default";

// Initial values for netclass initialization
const int DEFAULT_CLEARANCE        = Millimeter2iu( 0.2 ); // track to track and track to pads clearance
const int DEFAULT_VIA_DIAMETER     = Millimeter2iu( 0.8 );
const int DEFAULT_VIA_DRILL        = Millimeter2iu( 0.4 );
const int DEFAULT_UVIA_DIAMETER    = Millimeter2iu( 0.3 );
const int DEFAULT_UVIA_DRILL       = Millimeter2iu( 0.1 );
const int DEFAULT_TRACK_WIDTH      = Millimeter2iu( 0.25 );
const int DEFAULT_DIFF_PAIR_WIDTH  = Millimeter2iu( 0.2 );
const int DEFAULT_DIFF_PAIR_GAP    = Millimeter2iu( 0.25 );
const int DEFAULT_DIFF_PAIR_VIAGAP = Millimeter2iu( 0.25 );


NETCLASS::NETCLASS( const wxString& aName ) :
    m_Name( aName )
{
    // Default settings
    SetClearance( DEFAULT_CLEARANCE );
    SetViaDrill( DEFAULT_VIA_DRILL );
    SetuViaDrill( DEFAULT_UVIA_DRILL );
    // These defaults will be overwritten by SetParams,
    // from the board design parameters, later
    SetTrackWidth( DEFAULT_TRACK_WIDTH );
    SetViaDiameter( DEFAULT_VIA_DIAMETER );
    SetuViaDiameter( DEFAULT_UVIA_DIAMETER );
    SetDiffPairWidth( DEFAULT_DIFF_PAIR_WIDTH );
    SetDiffPairGap( DEFAULT_DIFF_PAIR_GAP );
    SetDiffPairViaGap( DEFAULT_DIFF_PAIR_VIAGAP );
}


void NETCLASS::SetParams( const NETCLASS& aDefaults )
{
    SetClearance( aDefaults.GetClearance() );
    SetTrackWidth( aDefaults.GetTrackWidth() );
    SetViaDiameter( aDefaults.GetViaDiameter() );
    SetViaDrill( aDefaults.GetViaDrill() );
    SetuViaDiameter( aDefaults.GetuViaDiameter() );
    SetuViaDrill( aDefaults.GetuViaDrill() );
    SetDiffPairWidth( aDefaults.GetDiffPairWidth() );
    SetDiffPairGap( aDefaults.GetDiffPairGap() );
    SetDiffPairViaGap( aDefaults.GetDiffPairViaGap() );
}


NETCLASS::~NETCLASS()
{
}


NETCLASSES::NETCLASSES()
{
    m_default = std::make_shared<NETCLASS>( NETCLASS::Default );
}


NETCLASSES::~NETCLASSES()
{
}


bool NETCLASSES::Add( const NETCLASSPTR& aNetClass )
{
    const wxString& name = aNetClass->GetName();

    if( name == NETCLASS::Default )
    {
        m_default = aNetClass;
        return true;
    }

    // Test for an existing netclass:
    if( !Find( name ) )
    {
        // name not found, take ownership
        m_NetClasses[name] = aNetClass;

        return true;
    }
    else
    {
        // name already exists
        // do not "take ownership" and return false telling caller such.
        return false;
    }
}


NETCLASSPTR NETCLASSES::Remove( const wxString& aNetName )
{
    NETCLASS_MAP::iterator found = m_NetClasses.find( aNetName );

    if( found != m_NetClasses.end() )
    {
        std::shared_ptr<NETCLASS> netclass = found->second;
        m_NetClasses.erase( found );
        return netclass;
    }

    return NETCLASSPTR();
}


NETCLASSPTR NETCLASSES::Find( const wxString& aName ) const
{
    if( aName == NETCLASS::Default )
        return GetDefault();

    NETCLASS_MAP::const_iterator found = m_NetClasses.find( aName );

    if( found == m_NetClasses.end() )
        return NETCLASSPTR();
    else
        return found->second;
}


void BOARD::SynchronizeNetsAndNetClasses()
{
    NETCLASSES& netClasses = m_designSettings.m_NetClasses;
    NETCLASSPTR defaultNetClass = netClasses.GetDefault();

    // set all NETs to the default NETCLASS, then later override some
    // as we go through the NETCLASSes.

    for( NETINFO_LIST::iterator net( m_NetInfo.begin() ), netEnd( m_NetInfo.end() );
                net != netEnd; ++net )
    {
        net->SetClass( defaultNetClass );
    }

    // Add netclass name and pointer to nets.  If a net is in more than one netclass,
    // set the net's name and pointer to only the first netclass.  Subsequent
    // and therefore bogus netclass memberships will be deleted in logic below this loop.
    for( NETCLASSES::iterator clazz = netClasses.begin(); clazz != netClasses.end(); ++clazz )
    {
        NETCLASSPTR netclass = clazz->second;

        for( NETCLASS::const_iterator member = netclass->begin(); member != netclass->end(); ++member )
        {
            const wxString& netname = *member;

            // although this overall function seems to be adequately fast,
            // FindNet( wxString ) uses now a fast binary search and is fast
            // event for large net lists
            NETINFO_ITEM* net = FindNet( netname );

            if( net && net->GetClassName() == NETCLASS::Default )
            {
                net->SetClass( netclass );
            }
        }
    }

    // Finally, make sure that every NET is in a NETCLASS, even if that
    // means the Default NETCLASS.  And make sure that all NETCLASSes do not
    // contain netnames that do not exist, by deleting all netnames from
    // every netclass and re-adding them.

    for( NETCLASSES::iterator clazz = netClasses.begin(); clazz != netClasses.end(); ++clazz )
    {
        NETCLASSPTR netclass = clazz->second;

        netclass->Clear();
    }

    defaultNetClass->Clear();

    for( NETINFO_LIST::iterator net( m_NetInfo.begin() ), netEnd( m_NetInfo.end() );
            net != netEnd; ++net )
    {
        const wxString& classname = net->GetClassName();

        // because of the std:map<> this should be fast, and because of
        // prior logic, netclass should not be NULL.
        NETCLASSPTR netclass = netClasses.Find( classname );

        wxASSERT( netclass );

        netclass->Add( net->GetNetname() );
    }

    // Set initial values for custom track width & via size to match the default netclass settings
    m_designSettings.UseCustomTrackViaSize( false );
    m_designSettings.SetCustomTrackWidth( defaultNetClass->GetTrackWidth() );
    m_designSettings.SetCustomViaSize( defaultNetClass->GetViaDiameter() );
    m_designSettings.SetCustomViaDrill( defaultNetClass->GetViaDrill() );
    m_designSettings.SetCustomDiffPairWidth( defaultNetClass->GetDiffPairWidth() );
    m_designSettings.SetCustomDiffPairGap( defaultNetClass->GetDiffPairGap() );
    m_designSettings.SetCustomDiffPairViaGap( defaultNetClass->GetDiffPairViaGap() );

    InvokeListeners( &BOARD_LISTENER::OnBoardNetSettingsChanged, *this );
}


#if defined(DEBUG)

void NETCLASS::Show( int nestLevel, std::ostream& os ) const
{
    // for now, make it look like XML:
    //NestedSpace( nestLevel, os )

    os << '<' << GetClass().Lower().mb_str() << ">\n";

    for( const_iterator i = begin();  i!=end();  ++i )
    {
        // NestedSpace( nestLevel+1, os ) << *i;
        os << TO_UTF8( *i );
    }

    // NestedSpace( nestLevel, os )
    os << "</" << GetClass().Lower().mb_str() << ">\n";
}

#endif


void NETCLASS::Format( OUTPUTFORMATTER* aFormatter, int aNestLevel, int aControlBits ) const
{
    aFormatter->Print( aNestLevel, "(net_class %s %s\n",
                       aFormatter->Quotew( GetName() ).c_str(),
                       aFormatter->Quotew( GetDescription() ).c_str() );

    aFormatter->Print( aNestLevel+1, "(clearance %s)\n", FormatInternalUnits( GetClearance() ).c_str() );
    aFormatter->Print( aNestLevel+1, "(trace_width %s)\n", FormatInternalUnits( GetTrackWidth() ).c_str() );

    aFormatter->Print( aNestLevel+1, "(via_dia %s)\n", FormatInternalUnits( GetViaDiameter() ).c_str() );
    aFormatter->Print( aNestLevel+1, "(via_drill %s)\n", FormatInternalUnits( GetViaDrill() ).c_str() );

    aFormatter->Print( aNestLevel+1, "(uvia_dia %s)\n", FormatInternalUnits( GetuViaDiameter() ).c_str() );
    aFormatter->Print( aNestLevel+1, "(uvia_drill %s)\n", FormatInternalUnits( GetuViaDrill() ).c_str() );

    // Save the diff_pair_gap and diff_pair_width values only if not the default, to avoid unnecessary
    // incompatibility  with previous Pcbnew versions.
    if( ( DEFAULT_DIFF_PAIR_WIDTH != GetDiffPairWidth() ) ||
        ( DEFAULT_DIFF_PAIR_GAP != GetDiffPairGap() ) )
    {
        aFormatter->Print( aNestLevel+1, "(diff_pair_width %s)\n",
                FormatInternalUnits( GetDiffPairWidth() ).c_str() );
        aFormatter->Print( aNestLevel+1, "(diff_pair_gap %s)\n",
                FormatInternalUnits( GetDiffPairGap() ).c_str() );

        // 6.0 TODO: figure out what to do with DiffPairViaGap...
    }

    for( NETCLASS::const_iterator it = begin(); it != end(); ++it )
        aFormatter->Print( aNestLevel+1, "(add_net %s)\n", aFormatter->Quotew( *it ).c_str() );

    aFormatter->Print( aNestLevel, ")\n\n" );
}
