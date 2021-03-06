/*
 * This program source code file is part of KiCad, a free EDA CAD application.
 *
 * Copyright (C) 2012 Miguel Angel Ajo Pelayo, miguelangel@nbee.es
 * Copyright (C) 2018 Jean-Pierre Charras, jp.charras at wanadoo.fr
 * Copyright (C) 2004-2018 KiCad Developers, see AUTHORS.txt for contributors.
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
 * @file footprint_wizard_frame.h
 */

#ifndef FOOTPRINT_WIZARD_FRAME_H_
#define FOOTPRINT_WIZARD_FRAME_H_


#include <wx/gdicmn.h>
#include <footprint_wizard.h>
class wxSashLayoutWindow;
class wxListBox;
class WX_GRID;
class wxGridEvent;
class FOOTPRINT_EDIT_FRAME;


enum WizardParameterColumnNames
{
    WIZ_COL_NAME = 0,
    WIZ_COL_VALUE,
    WIZ_COL_UNITS
};

/**
 * FOOTPRINT_WIZARD_FRAME
 */
class FOOTPRINT_WIZARD_FRAME : public PCB_BASE_FRAME
{
private:
    wxPanel*        m_parametersPanel;      ///< Panel for the page list and parameter grid
    wxListBox*      m_pageList;             ///< The list of pages
    WX_GRID*        m_parameterGrid;        ///< The list of parameters
    int             m_parameterGridPage;    ///< the page currently displayed by m_parameterGrid
                                            ///< it is most of time the m_pageList selection, but can differ
                                            ///< during transitions between pages.
    wxTextCtrl*     m_buildMessageBox;

    wxString        m_auiPerspective;       ///< Encoded string describing the AUI layout

    bool            m_wizardListShown;      ///< A show-once flag for the wizard list

protected:
    wxString        m_wizardName;           ///< name of the current wizard
    wxString        m_wizardDescription;    ///< description of the wizard
    wxString        m_wizardStatus;         ///< current wizard status

public:

    FOOTPRINT_WIZARD_FRAME( KIWAY* aKiway, wxWindow* parent, FRAME_T aFrameType );

    ~FOOTPRINT_WIZARD_FRAME();

    ///> @copydoc PCB_BASE_FRAME::GetModel()
    BOARD_ITEM_CONTAINER* GetModel() const override;

    MODULE*             GetBuiltFootprint();

    /**
     * Reload the Python plugins if they are newer than
     * the already loaded, and load new plugins if any
     * Do nothing if KICAD_SCRIPTING is not defined
     */
    void PythonPluginsReload();

    COLOR_SETTINGS* GetColorSettings() override;

private:

    void                OnSize( wxSizeEvent& event ) override;
    void                OnGridSize( wxSizeEvent& aSizeEvent );

    /**
     * redraws the message panel.
     * display the current footprint info, or
     * clear the message panel if nothing is loaded
     */
    void UpdateMsgPanel() override;

    /**
     * rebuild the GAL view (reint tool manager, colors and drawings)
     * must be run after any footprint change.
     */
    void updateView();

    /**
     * Function ExportSelectedFootprint();
     * will let the caller exit from the wait loop, and get the built footprint
     *
     */
    void                ExportSelectedFootprint( wxCommandEvent& aEvent );

    /**
     * Function OnSashDrag
     * resizes the child windows when dragging a sash window border.
     */

    void                OnSashDrag( wxSashEvent& event );

    /**
     * Function ReCreatePageList
     * Creates or recreates the list of parameter pages for the current wizard.
     * This list is sorted
     */
    void                ReCreatePageList();

    /**
     * Function ReCreateParameterList
     * Creates the list of parameters for the current page
     */
    void                ReCreateParameterList();

    /**
     * Expand the 'Value' column to fill available
     */
    void                ResizeParamColumns();

    /**
     * Function initParameterGrid
     * Prepare the grid where parameters are displayed
     */
    void                initParameterGrid();

    /**
     * Function SelectFootprintWizard
     * Shows the list of footprint wizards available into the system
     */
    void                SelectFootprintWizard();

    /**
     * Function ReloadFootprint
     * Reloads the current footprint
     */
    void                ReloadFootprint();

    /**
     * Function DisplayBuildMessages
     * Display the message generated by the python build footprint script
     */
    void                DisplayBuildMessage( wxString& aMessage );

    /**
     * Function GetMyWizard
     * Reloads the wizard by name
     */
    FOOTPRINT_WIZARD*   GetMyWizard();


    void                Process_Special_Functions( wxCommandEvent& event );

    /**
     * Function DisplayWizardInfos
     * Shows all the details about the current wizard
     */
    void                DisplayWizardInfos();


    void                OnCloseWindow( wxCloseEvent& Event ) override;
    void                ReCreateHToolbar() override;
    void                ReCreateVToolbar() override;
    void                ClickOnPageList( wxCommandEvent& event );

    void                LoadSettings( APP_SETTINGS_BASE* aCfg ) override;
    void                SaveSettings( APP_SETTINGS_BASE* aCfg ) override;

    WINDOW_SETTINGS*    GetWindowSettings( APP_SETTINGS_BASE* aCfg ) override;

            /**
             * Function OnActivate
             * is called when the frame frame is activate to reload the libraries and component lists
             * that can be changed by the schematic editor or the library editor.
             */
    void                OnActivate( wxActivateEvent& event );

    void                SelectCurrentWizard( wxCommandEvent& event );

    void                DefaultParameters( wxCommandEvent& event );

    /**
     * Function ParametersUpdated
     * Update the footprint python parameters values from the values in grid
     */
    void                ParametersUpdated( wxGridEvent& event );

    /**
     * Function Update3D_Frame
     * must be called after a footprint selection
     * Updates the 3D view and 3D frame title.
     * @param aForceReload = true to reload data immediately
     * @param aTitle (optional) the window title to set for the viewer
     */
    void                Update3DView( bool aForceReload, const wxString* aTitle ) override;

    /*
     * Virtual functions, not used here, but needed by PCB_BASE_FRAME
     * (virtual pure functions )
     */
    void SaveCopyInUndoList( BOARD_ITEM*, UNDO_REDO_T, const wxPoint& ) override {}
    void SaveCopyInUndoList( const PICKED_ITEMS_LIST&, UNDO_REDO_T, const wxPoint& ) override {}


    DECLARE_EVENT_TABLE()
};



#endif    // FOOTPRINT_WIZARD_FRM_H_
