/*
 * This program source code file is part of KiCad, a free EDA CAD application.
 *
 * Copyright (C) 2015 Jean-Pierre Charras, jp.charras at wanadoo.fr
 * Copyright (C) 2015 SoftPLC Corporation, Dick Hollenbeck <dick@softplc.com>
 * Copyright (C) 2015 Wayne Stambaugh <stambaughw@gmail.com>
 * Copyright (C) 1992-2018 KiCad Developers, see AUTHORS.txt for contributors.
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
 * @file pcbnew/menubar_footprint_editor.cpp
 * @brief (Re)Create the main menubar for the footprint editor
 */


#include <menus_helpers.h>
#include <pgm_base.h>

#include "help_common_strings.h"
#include "hotkeys.h"
#include "footprint_edit_frame.h"
#include "pcbnew.h"
#include "pcbnew_id.h"


void FOOTPRINT_EDIT_FRAME::ReCreateMenuBar()
{
    // Create and try to get the current menubar
    wxMenuBar* menuBar = GetMenuBar();

    if( !menuBar )
        menuBar = new wxMenuBar();

    // Delete all existing menus so they can be rebuilt.
    // This allows language changes of the menu text on the fly.
    menuBar->Freeze();

    while( menuBar->GetMenuCount() )
        delete menuBar->Remove( 0 );

    // Recreate all menus:
    wxString text;

    // Menu File:
    wxMenu* fileMenu = new wxMenu;

    // Active library selection
    AddMenuItem( fileMenu, ID_MODEDIT_SELECT_CURRENT_LIB,
                 _("Set Acti&ve Library..."),
                 _( "Select active library" ),
                 KiBitmap( open_library_xpm ) );

    fileMenu->AppendSeparator();

    // New module
    text = AddHotkeyName( _( "&New Footprint..." ), m_hotkeysDescrList, HK_NEW );
    AddMenuItem( fileMenu, ID_MODEDIT_NEW_MODULE,
                 text, _( "Create new footprint" ),
                 KiBitmap( new_footprint_xpm ) );

    text = AddHotkeyName( _( "&Open Footprint..." ), m_hotkeysDescrList, HK_OPEN );
    AddMenuItem( fileMenu, ID_MODEDIT_LOAD_MODULE, text,
                 _( "Open a footprint from a library" ),
                 KiBitmap( module_xpm ) );

    fileMenu->AppendSeparator();

    // Save module
    text = AddHotkeyName( _( "&Save" ), m_hotkeysDescrList, HK_SAVE );
    AddMenuItem( fileMenu, ID_MODEDIT_SAVE_LIBMODULE, text,
                 _( "Save footprint" ),
                 KiBitmap( save_library_xpm ) );

    fileMenu->AppendSeparator();


    wxMenu* importSubMenu = new wxMenu();

    // from current Board
    AddMenuItem( importSubMenu, ID_MODEDIT_LOAD_MODULE_FROM_BOARD,
                 _( "Footprint from &Current Board..." ),
                 _( "Import a footprint from the current board" ),
                 KiBitmap( load_module_board_xpm ) );

    AddMenuItem( importSubMenu, ID_MODEDIT_IMPORT_PART,
                 _( "Footprint from &KiCad File..." ),
                 _( "Import a footprint from an existing footprint file" ),
                 KiBitmap( import_module_xpm ) );

    AddMenuItem( importSubMenu, ID_GEN_IMPORT_DXF_FILE,
                 _( "Footprint Outlines from &DXF File..." ),
                 _( "Import 2D Drawing DXF file to Footprint Editor on Drawings layer" ),
                 KiBitmap( import_xpm ) );

    AddMenuItem( fileMenu, importSubMenu, -1,
                 _( "&Import" ),
                 _( "Import files" ),
                 KiBitmap( import_xpm ) );


    wxMenu* exportSubMenu = new wxMenu();

    AddMenuItem( exportSubMenu, ID_MODEDIT_SAVE_LIBRARY_AS,
                 _( "&Active Library..." ),
                 _( "Export active library" ),
                 KiBitmap( library_archive_xpm ) );

    AddMenuItem( exportSubMenu, ID_MODEDIT_EXPORT_PART,
                 _( "&Footprint..." ),
                 _( "Export current footprint to a file" ),
                 KiBitmap( export_module_xpm ) );

    AddMenuItem( fileMenu, exportSubMenu, -1,
                 _( "E&xport" ),
                 _( "Export files" ),
                 KiBitmap( export_xpm ) );



    fileMenu->AppendSeparator();

    // Print
    text = AddHotkeyName( _( "&Print..." ), m_hotkeysDescrList, HK_PRINT );
    AddMenuItem( fileMenu, wxID_PRINT, text,
                 _( "Print current footprint" ),
                 KiBitmap( print_button_xpm ) );

    // Separator
    fileMenu->AppendSeparator();

    // Properties
    AddMenuItem( fileMenu, ID_MODEDIT_EDIT_MODULE_PROPERTIES,
                 _( "P&roperties..." ),
                 _( "Edit footprint properties" ),
                 KiBitmap( module_options_xpm ) );

    fileMenu->AppendSeparator();

    // Close editor
    AddMenuItem( fileMenu, wxID_EXIT,
                 _( "&Exit" ),
                 _( "Close footprint editor" ),
                 KiBitmap( exit_xpm ) );

    //----- Edit menu ------------------
    wxMenu* editMenu = new wxMenu;

    // Undo
    text = AddHotkeyName( _( "&Undo" ), m_hotkeysDescrList, HK_UNDO );
    AddMenuItem( editMenu, wxID_UNDO,
                 text, _( "Undo last action" ),
                 KiBitmap( undo_xpm ) );

    // Redo
    text = AddHotkeyName( _( "&Redo" ), m_hotkeysDescrList, HK_REDO );
    AddMenuItem( editMenu, wxID_REDO,
                 text, _( "Redo last action" ),
                 KiBitmap( redo_xpm ) );

    // Separator
    editMenu->AppendSeparator();

    if( IsGalCanvasActive() )
    {
        text = AddHotkeyName( _( "Cu&t" ), m_hotkeysDescrList, HK_EDIT_CUT );
        AddMenuItem( editMenu, ID_EDIT_CUT, text,
                     _( "Cuts the selected item(s) to the Clipboard" ), KiBitmap( cut_xpm ) );
        text = AddHotkeyName( _( "&Copy" ), m_hotkeysDescrList, HK_EDIT_COPY );
        AddMenuItem( editMenu, ID_EDIT_COPY, text,
                     _( "Copies the selected item(s) to the Clipboard" ), KiBitmap( copy_xpm ) );
        text = AddHotkeyName( _( "&Paste" ), m_hotkeysDescrList, HK_EDIT_PASTE );
        AddMenuItem( editMenu, ID_EDIT_PASTE, text,
                     _( "Pastes item(s) from the Clipboard" ), KiBitmap( paste_xpm ) );
    }

    // Delete items
    AddMenuItem( editMenu, ID_MODEDIT_DELETE_TOOL,
                 _( "&Delete" ), _( "Delete items" ),
                 KiBitmap( delete_xpm ) );

    editMenu->AppendSeparator();

    //--------- View menu ----------------
    wxMenu* viewMenu = new wxMenu;

    AddMenuItem( viewMenu, ID_OPEN_MODULE_VIEWER,
                 _( "&Library Browser" ),
                 _( "Open the Library Browser" ),
                 KiBitmap( modview_icon_xpm ) );

    text = AddHotkeyName( _( "&3D Viewer" ), m_hotkeysDescrList, HK_3D_VIEWER );
    AddMenuItem( viewMenu, ID_MENU_PCB_SHOW_3D_FRAME,
                 text, _( "Show footprint in 3D viewer" ),
                 KiBitmap( three_d_xpm ) );

    viewMenu->AppendSeparator();

    /* Important Note for ZOOM IN and ZOOM OUT commands from menubar:
     * we cannot add hotkey info here, because the hotkey HK_ZOOM_IN and HK_ZOOM_OUT
     * events(default = WXK_F1 and WXK_F2) are *NOT* equivalent to this menu command:
     * zoom in and out from hotkeys are equivalent to the pop up menu zoom
     * From here, zooming is made around the screen center
     * From hotkeys, zooming is made around the mouse cursor position
     * (obviously not possible from the toolbar or menubar command)
     *
     * in other words HK_ZOOM_IN and HK_ZOOM_OUT *are NOT* accelerators
     * for Zoom in and Zoom out sub menus
     */
    text = AddHotkeyName( _( "Zoom &In" ), m_hotkeysDescrList,
                          HK_ZOOM_IN, IS_ACCELERATOR );
    AddMenuItem( viewMenu, ID_ZOOM_IN, text, HELP_ZOOM_IN, KiBitmap( zoom_in_xpm ) );

    text = AddHotkeyName( _( "Zoom &Out" ), m_hotkeysDescrList,
                          HK_ZOOM_OUT, IS_ACCELERATOR );
    AddMenuItem( viewMenu, ID_ZOOM_OUT, text, HELP_ZOOM_OUT, KiBitmap( zoom_out_xpm ) );

    text = AddHotkeyName( _( "&Zoom to Fit" ), m_hotkeysDescrList,
                          HK_ZOOM_AUTO  );
    AddMenuItem( viewMenu, ID_ZOOM_PAGE, text, _( "Zoom to fit footprint" ),
                 KiBitmap( zoom_fit_in_page_xpm ) );

    text = AddHotkeyName( _( "Zoom to Selection" ), m_hotkeysDescrList, HK_ZOOM_SELECTION );

    AddMenuItem( viewMenu, ID_ZOOM_SELECTION, text, KiBitmap( zoom_area_xpm ) );

    text = AddHotkeyName( _( "&Redraw" ), m_hotkeysDescrList, HK_ZOOM_REDRAW );
    AddMenuItem( viewMenu, ID_ZOOM_REDRAW, text,
                 HELP_ZOOM_REDRAW, KiBitmap( zoom_redraw_xpm ) );

    viewMenu->AppendSeparator();

    AddMenuItem( viewMenu, ID_TB_OPTIONS_SHOW_GRID,
                 _( "Show &Grid" ), wxEmptyString,
                 KiBitmap( grid_xpm ), wxITEM_CHECK );

    AddMenuItem( viewMenu, ID_PCB_USER_GRID_SETUP,
                 _( "Grid &Settings..." ),_( "Adjust custom user-defined grid dimensions" ),
                 KiBitmap( grid_xpm ) );

    AddMenuItem( viewMenu, ID_TB_OPTIONS_SHOW_POLAR_COORD,
                 _( "Display &Polar Coordinates" ), wxEmptyString,
                 KiBitmap( polar_coord_xpm ), wxITEM_CHECK );

    // Units submenu
    wxMenu* unitsSubMenu = new wxMenu;
    AddMenuItem( unitsSubMenu, ID_TB_OPTIONS_SELECT_UNIT_INCH,
                 _( "&Imperial" ), _( "Use imperial units" ),
                 KiBitmap( unit_inch_xpm ), wxITEM_RADIO );

    AddMenuItem( unitsSubMenu, ID_TB_OPTIONS_SELECT_UNIT_MM,
                 _( "&Metric" ), _( "Use metric units" ),
                 KiBitmap( unit_mm_xpm ), wxITEM_RADIO );

    AddMenuItem( viewMenu, unitsSubMenu,
                 -1, _( "&Units" ),
                 _( "Select which units are displayed" ),
                 KiBitmap( unit_mm_xpm ) );

#ifndef __APPLE__
    AddMenuItem( viewMenu, ID_TB_OPTIONS_SELECT_CURSOR,
                 _( "Full Window Crosshair" ),
                 _( "Change cursor shape" ),
                 KiBitmap( cursor_shape_xpm ), wxITEM_CHECK );
#else
    AddMenuItem( viewMenu, ID_TB_OPTIONS_SELECT_CURSOR,
                 _( "Full Window Crosshair" ),
                 _( "Change cursor shape (not supported in Legacy Toolset)" ),
                 KiBitmap( cursor_shape_xpm ), wxITEM_CHECK );
#endif

    viewMenu->AppendSeparator();

    // Drawing Mode Submenu
    wxMenu* drawingModeSubMenu = new wxMenu;

    AddMenuItem( drawingModeSubMenu, ID_TB_OPTIONS_SHOW_PADS_SKETCH,
                 _( "Sketch &Pads" ), _( "Show pads in outline mode" ),
                 KiBitmap( pad_sketch_xpm ), wxITEM_CHECK );

    AddMenuItem( drawingModeSubMenu, ID_TB_OPTIONS_SHOW_MODULE_EDGE_SKETCH,
                 _( "Sketch Footprint &Edges" ), _( "Show footprint edges in outline mode" ),
                 KiBitmap( show_mod_edge_xpm ), wxITEM_CHECK );
    AddMenuItem( drawingModeSubMenu, ID_TB_OPTIONS_SHOW_MODULE_TEXT_SKETCH,
                 _( "Sketch Footprint Te&xt" ), _( "Show footprint text in outline mode" ),
                 KiBitmap( text_sketch_xpm ), wxITEM_CHECK );

    AddMenuItem( viewMenu, drawingModeSubMenu,
                 -1, _( "&Drawing Mode" ),
                 _( "Select how items are displayed" ),
                 KiBitmap( add_zone_xpm ) );

    // Contrast Mode Submenu
    wxMenu* contrastModeSubMenu = new wxMenu;

    text = AddHotkeyName( _( "&High Contrast Mode" ), m_hotkeysDescrList,
                          HK_SWITCH_HIGHCONTRAST_MODE );
    AddMenuItem( contrastModeSubMenu, ID_TB_OPTIONS_SHOW_HIGH_CONTRAST_MODE, text,
                 _( "Use high contrast display mode" ),
                 KiBitmap( contrast_mode_xpm ), wxITEM_CHECK );

    contrastModeSubMenu->AppendSeparator();

    text = AddHotkeyName( _( "&Decrease Layer Opacity" ), g_Pcbnew_Editor_Hotkeys_Descr,
                          HK_DEC_LAYER_ALPHA );
    AddMenuItem( contrastModeSubMenu, ID_DEC_LAYER_ALPHA,
                 text, _( "Make the current layer more transparent" ),
                 KiBitmap( contrast_mode_xpm ) );

    text = AddHotkeyName( _( "&Increase Layer Opacity" ), g_Pcbnew_Editor_Hotkeys_Descr,
                          HK_INC_LAYER_ALPHA );
    AddMenuItem( contrastModeSubMenu, ID_INC_LAYER_ALPHA,
                 text, _( "Make the current layer less transparent" ),
                 KiBitmap( contrast_mode_xpm ) );

    AddMenuItem( viewMenu, contrastModeSubMenu,
                 -1, _( "&Contrast Mode" ),
                 _( "Select how items are displayed" ),
                 KiBitmap( contrast_mode_xpm ) );

#ifdef __APPLE__
    viewMenu->AppendSeparator();
#endif


    //-------- Setup menu --------------------
    wxMenu* setupMenu = new wxMenu;

    // Sizes and Widths
    AddMenuItem( setupMenu, ID_PCB_DRAWINGS_WIDTHS_SETUP,
                 _( "Te&xts and Drawings..." ),
                 _( "Adjust dimensions for texts and drawings" ),
                 KiBitmap( text_xpm ) );

    // Pad settings
    AddMenuItem( setupMenu, ID_MODEDIT_PAD_SETTINGS,
                 _( "Default &Pad Properties..." ),
                 _( "Edit settings for new pads" ),
                 KiBitmap( pad_dimensions_xpm ) );

    //-------- Place menu --------------------
    wxMenu* placeMenu = new wxMenu;

    // Pad
    AddMenuItem( placeMenu, ID_MODEDIT_PAD_TOOL,
                 _( "&Pad" ), _( "Add pad" ),
                 KiBitmap( pad_xpm ) );

    placeMenu->AppendSeparator();

    // Text
    AddMenuItem( placeMenu, ID_MODEDIT_TEXT_TOOL,
                 _( "&Text" ), _( "Add graphic text" ),
                 KiBitmap( text_xpm ) );

    // Arc
    AddMenuItem( placeMenu, ID_MODEDIT_ARC_TOOL,
                 _( "&Arc" ), _( "Add graphic arc" ),
                 KiBitmap( add_arc_xpm ) );

    // Circle
    AddMenuItem( placeMenu, ID_MODEDIT_CIRCLE_TOOL,
                 _( "&Circle" ), _( "Add graphic circle" ),
                 KiBitmap( add_circle_xpm ) );

    // Line
    AddMenuItem( placeMenu, ID_MODEDIT_LINE_TOOL,
                 _( "&Line" ), _( "Add graphic line" ),
                 KiBitmap( add_graphical_segments_xpm ) );

    // Polygon
    AddMenuItem( placeMenu, ID_MODEDIT_POLYGON_TOOL,
                 _( "&Polygon" ), _( "Add graphic polygon" ),
                 KiBitmap( add_graphical_polygon_xpm ) );

    placeMenu->AppendSeparator();

    // Anchor
    AddMenuItem( placeMenu, ID_MODEDIT_ANCHOR_TOOL,
                 _( "A&nchor" ),
                 _( "Place footprint reference anchor" ),
                 KiBitmap( anchor_xpm ) );

    // Origin
    AddMenuItem( placeMenu, ID_MODEDIT_PLACE_GRID_COORD,
                 _( "&Grid Origin" ),
                 _( "Set grid origin point" ),
                 KiBitmap( grid_select_axis_xpm ) );


    //----- Inspect menu ---------------------
    wxMenu* inspectMenu = new wxMenu;

    AddMenuItem( inspectMenu, ID_MODEDIT_MEASUREMENT_TOOL,
                 _( "&Measure" ),
                 _( "Measure distance" ),
                 KiBitmap( measurement_xpm ) );

    //----- Tools menu ---------------------
    wxMenu* toolsMenu = new wxMenu;

    AddMenuItem( toolsMenu, ID_MODEDIT_UPDATE_MODULE_IN_BOARD,
                 _( "&Update Footprint on PCB" ),
                 _( "Push updated footprint through to current board" ),
                 KiBitmap( update_module_board_xpm ) );

    AddMenuItem( toolsMenu, ID_MODEDIT_INSERT_MODULE_IN_BOARD,
                 _( "&Insert Footprint on PCB" ),
                 _( "Insert footprint onto current board" ),
                 KiBitmap( insert_module_board_xpm ) );

    toolsMenu->AppendSeparator();

    AddMenuItem( toolsMenu, ID_MODEDIT_DELETE_PART,
                 _( "&Delete a Footprint in Active Library" ),
                 _( "Choose and delete a footprint from the active library" ),
                 KiBitmap( delete_xpm ) );


    //----- Preferences menu -----------------
    wxMenu* prefs_menu = new wxMenu;

    AddMenuItem( prefs_menu, ID_PCB_LIB_WIZARD,
                _( "&Footprint Library Wizard..." ), _( "Add footprint libraries with wizard" ),
                KiBitmap( wizard_add_fplib_small_xpm ) );

    AddMenuItem( prefs_menu, ID_PCB_LIB_TABLE_EDIT,
                _( "Footprint Li&brary Table..." ), _( "Configure footprint library table" ),
                KiBitmap( library_table_xpm ) );

    // Path configuration edit dialog.
    AddMenuItem( prefs_menu,
                 ID_PREFERENCES_CONFIGURE_PATHS,
                 _( "Configure Pa&ths..." ),
                 _( "Edit path configuration environment variables" ),
                 KiBitmap( path_xpm ) );

    // Settings
    AddMenuItem( prefs_menu, wxID_PREFERENCES,
                 _( "General &Settings..." ), _( "Change footprint editor settings." ),
                 KiBitmap( preference_xpm ) );

    prefs_menu->AppendSeparator();

    AddMenuItem( prefs_menu, ID_PCB_DISPLAY_OPTIONS_SETUP,
              _( "&Display Options..." ),
              _( "Graphics acceleration, grid and cursor settings." ),
              KiBitmap( display_options_xpm ) );

    text = AddHotkeyName( _( "Legacy Tool&set" ), m_hotkeysDescrList,
                          HK_CANVAS_LEGACY );
    AddMenuItem( prefs_menu, ID_MENU_CANVAS_LEGACY, text,
                 _( "Use Legacy Toolset (not all features will be available)" ),
                 KiBitmap( tools_xpm ), wxITEM_RADIO );

    text = AddHotkeyName( _( "Modern Toolset (&Accelerated)" ),
                          m_hotkeysDescrList, HK_CANVAS_OPENGL );
    AddMenuItem( prefs_menu, ID_MENU_CANVAS_OPENGL, text,
                 _( "Use Modern Toolset with hardware-accelerated graphics (recommended)" ),
                 KiBitmap( tools_xpm ), wxITEM_RADIO );

    text = AddHotkeyName( _( "Modern Toolset (&Fallback)" ),
                          m_hotkeysDescrList, HK_CANVAS_CAIRO );
    AddMenuItem( prefs_menu, ID_MENU_CANVAS_CAIRO, text,
                 _( "Use Modern Toolset with software graphics (fall-back)" ),
                 KiBitmap( tools_xpm ), wxITEM_RADIO );

    prefs_menu->AppendSeparator();

    // Language submenu
    Pgm().AddMenuLanguageList( prefs_menu );

    // Hotkey submenu
    AddHotkeyConfigMenu( prefs_menu );

    //----- Help menu --------------------
    wxMenu* helpMenu = new wxMenu;

    // Contents
    AddMenuItem( helpMenu, wxID_HELP,
                 _( "Pcbnew &Manual" ),
                 _( "Open the Pcbnew Manual" ),
                 KiBitmap( online_help_xpm ) );

    AddMenuItem( helpMenu, wxID_INDEX,
                 _( "&Getting Started in KiCad" ),
                 _( "Open the \"Getting Started in KiCad\" guide for beginners" ),
                 KiBitmap( help_xpm ) );

    text = AddHotkeyName( _( "&List Hotkeys..." ), m_hotkeysDescrList, HK_HELP );
    AddMenuItem( helpMenu, ID_PREFERENCES_HOTKEY_SHOW_CURRENT_LIST,
                 text,
                 _( "Displays current hotkeys table and corresponding commands" ),
                 KiBitmap( hotkeys_xpm ) );

    helpMenu->AppendSeparator();

    AddMenuItem( helpMenu, ID_HELP_GET_INVOLVED,
                 _( "Get &Involved" ),
                 _( "Contribute to KiCad (opens a web browser)" ),
                 KiBitmap( info_xpm ) );

    // About Pcbnew
    helpMenu->AppendSeparator();
    AddMenuItem( helpMenu, wxID_ABOUT,
                 _( "&About KiCad" ),
                 _( "About KiCad" ),
                 KiBitmap( about_xpm ) );

    // Append menus to the menubar
    menuBar->Append( fileMenu, _( "&File" ) );
    menuBar->Append( editMenu, _( "&Edit" ) );
    menuBar->Append( viewMenu, _( "&View" ) );
    menuBar->Append( setupMenu, _( "&Setup" ) );
    menuBar->Append( placeMenu, _( "&Place" ) );
    menuBar->Append( inspectMenu, _( "&Inspect" ) );
    menuBar->Append( toolsMenu, _( "&Tools" ) );
    menuBar->Append( prefs_menu, _( "P&references" ) );
    menuBar->Append( helpMenu, _( "&Help" ) );

    menuBar->Thaw();

    // Associate the menu bar with the frame, if no previous menubar
    if( GetMenuBar() == NULL )
        SetMenuBar( menuBar );
    else
        menuBar->Refresh();
}
