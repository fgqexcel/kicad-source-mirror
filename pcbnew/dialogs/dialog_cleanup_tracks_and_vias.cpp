/*
 * This program source code file is part of KiCad, a free EDA CAD application.
 *
 * Copyright (C) 1992-2020 KiCad Developers, see AUTHORS.txt for contributors.
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
#include <wx/wx.h>

#include <board_commit.h>
#include <dialog_cleanup_tracks_and_vias.h>
#include <kiface_i.h>
#include <pcb_edit_frame.h>
#include <pcbnew_settings.h>
#include <reporter.h>
#include <tool/tool_manager.h>
#include <tools/pcb_actions.h>
#include <tracks_cleaner.h>
#include <drc/drc_item.h>
#include <drc/drc_provider.h>

DIALOG_CLEANUP_TRACKS_AND_VIAS::DIALOG_CLEANUP_TRACKS_AND_VIAS( PCB_EDIT_FRAME* aParentFrame ):
        DIALOG_CLEANUP_TRACKS_AND_VIAS_BASE( aParentFrame ),
    m_parentFrame( aParentFrame )
{
    auto cfg = m_parentFrame->GetPcbNewSettings();

    m_cleanViasOpt->SetValue( cfg->m_Cleanup.cleanup_vias );
    m_mergeSegmOpt->SetValue( cfg->m_Cleanup.merge_segments );
    m_deleteUnconnectedOpt->SetValue( cfg->m_Cleanup.cleanup_unconnected );
    m_cleanShortCircuitOpt->SetValue( cfg->m_Cleanup.cleanup_short_circuits );
    m_deleteTracksInPadsOpt->SetValue( cfg->m_Cleanup.cleanup_tracks_in_pad );

    m_changesTreeModel = new RC_TREE_MODEL( m_parentFrame, m_changesDataView );
    m_changesDataView->AssociateModel( m_changesTreeModel );

    m_changesTreeModel->SetSeverities( RPT_SEVERITY_ACTION );

    // We use a sdbSizer to get platform-dependent ordering of the action buttons, but
    // that requires us to correct the button labels here.
    m_sdbSizerOK->SetLabel( _( "Update PCB" ) );

    m_sdbSizerOK->SetDefault();
    GetSizer()->SetSizeHints(this);
    Centre();
}


DIALOG_CLEANUP_TRACKS_AND_VIAS::~DIALOG_CLEANUP_TRACKS_AND_VIAS()
{
    auto cfg = m_parentFrame->GetPcbNewSettings();

    cfg->m_Cleanup.cleanup_vias           = m_cleanViasOpt->GetValue();
    cfg->m_Cleanup.merge_segments         = m_mergeSegmOpt->GetValue();
    cfg->m_Cleanup.cleanup_unconnected    = m_deleteUnconnectedOpt->GetValue();
    cfg->m_Cleanup.cleanup_short_circuits = m_cleanShortCircuitOpt->GetValue();
    cfg->m_Cleanup.cleanup_tracks_in_pad  = m_deleteTracksInPadsOpt->GetValue();

    for( DRC_ITEM* item : m_items )
        delete item;

    m_changesTreeModel->DecRef();
}


void DIALOG_CLEANUP_TRACKS_AND_VIAS::OnCheckBox( wxCommandEvent& anEvent )
{
    doCleanup( true );
}


bool DIALOG_CLEANUP_TRACKS_AND_VIAS::TransferDataToWindow()
{
    doCleanup( true );

    return true;
}


bool DIALOG_CLEANUP_TRACKS_AND_VIAS::TransferDataFromWindow()
{
    doCleanup( false );

    return true;
}


void DIALOG_CLEANUP_TRACKS_AND_VIAS::doCleanup( bool aDryRun )
{
    wxBusyCursor busy;
    BOARD_COMMIT commit( m_parentFrame );
    TRACKS_CLEANER cleaner( m_parentFrame->GetUserUnits(), m_parentFrame->GetBoard(), commit );

    if( !aDryRun )
    {
        // Clear current selection list to avoid selection of deleted items
        m_parentFrame->GetToolManager()->RunAction( PCB_ACTIONS::selectionClear, true );

        // ... and to keep the treeModel from trying to refresh a deleted item
        m_changesTreeModel->SetProvider( nullptr );
    }

    for( DRC_ITEM* item : m_items )
        delete item;

    m_items.clear();

    // Old model has to be refreshed, GAL normally does not keep updating it
    m_parentFrame->Compile_Ratsnest( false );

    bool modified = cleaner.CleanupBoard( aDryRun, &m_items,
                                          m_cleanShortCircuitOpt->GetValue(),
                                          m_cleanViasOpt->GetValue(),
                                          m_mergeSegmOpt->GetValue(),
                                          m_deleteUnconnectedOpt->GetValue(),
                                          m_deleteTracksInPadsOpt->GetValue() );

    if( aDryRun )
    {
        RC_ITEMS_PROVIDER* provider = new VECTOR_DRC_ITEMS_PROVIDER( m_parentFrame, &m_items );
        m_changesTreeModel->SetProvider( provider );
    }
    else if( modified )
    {
        // Clear undo and redo lists to avoid inconsistencies between lists
        commit.Push( _( "Board cleanup" ) );
        m_parentFrame->GetCanvas()->Refresh( true );
    }
}


void DIALOG_CLEANUP_TRACKS_AND_VIAS::OnSelectItem( wxDataViewEvent& aEvent )
{
    const KIID&   itemID = RC_TREE_MODEL::ToUUID( aEvent.GetItem() );
    BOARD_ITEM*   item = m_parentFrame->GetBoard()->GetItem( itemID );
    WINDOW_THAWER thawer( m_parentFrame );

    m_parentFrame->FocusOnItem( item );
    m_parentFrame->GetCanvas()->Refresh();

    aEvent.Skip();
}


void DIALOG_CLEANUP_TRACKS_AND_VIAS::OnLeftDClickItem( wxMouseEvent& event )
{
    event.Skip();

    if( m_changesDataView->GetCurrentItem().IsOk() )
    {
        if( !IsModal() )
            Show( false );
    }
}


