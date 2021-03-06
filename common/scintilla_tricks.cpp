/*
 * This program source code file is part of KiCad, a free EDA CAD application.
 *
 * Copyright (C) 2020 KiCad Developers, see change_log.txt for contributors.
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
#include <scintilla_tricks.h>
#include <gal/color4d.h>
#include <dialog_shim.h>

SCINTILLA_TRICKS::SCINTILLA_TRICKS( wxStyledTextCtrl* aScintilla, const wxString& aBraces ) :
        m_te( aScintilla ),
        m_braces( aBraces ),
        m_lastCaretPos( -1 )
{
    // A hack which causes Scintilla to auto-size the text editor canvas
    // See: https://github.com/jacobslusser/ScintillaNET/issues/216
    m_te->SetScrollWidth( 1 );
    m_te->SetScrollWidthTracking( true );

    // Set up the brace highlighting
    wxColour highlight = wxSystemSettings::GetColour( wxSYS_COLOUR_HIGHLIGHT );
   	wxColour highlightText = wxSystemSettings::GetColour( wxSYS_COLOUR_WINDOWTEXT );

    if( KIGFX::COLOR4D( highlightText ).GetBrightness() > 0.5 )
        highlight = highlight.ChangeLightness( 80 );
    else
        highlight = highlight.ChangeLightness( 120 );

    m_te->StyleSetForeground( wxSTC_STYLE_BRACELIGHT, highlightText );
    m_te->StyleSetBackground( wxSTC_STYLE_BRACELIGHT, highlight );
    m_te->StyleSetForeground( wxSTC_STYLE_BRACEBAD, *wxRED );

    // Set up autocomplete
    m_te->AutoCompSetIgnoreCase( true );
    m_te->AutoCompSetFillUps( m_braces[1] );
    m_te->AutoCompSetMaxHeight( 20 );

    // Hook up events
    m_te->Bind( wxEVT_STC_UPDATEUI, &SCINTILLA_TRICKS::onScintillaUpdateUI, this );

    // Dispatch command-keys in Scintilla control.
    m_te->Bind( wxEVT_CHAR_HOOK, &SCINTILLA_TRICKS::onCharHook, this );
}


bool SCINTILLA_TRICKS::isCtrl( int aChar, const wxKeyEvent& e )
{
    return e.GetKeyCode() == aChar && e.ControlDown() && !e.AltDown() &&
            !e.ShiftDown() && !e.MetaDown();
}


bool SCINTILLA_TRICKS::isShiftCtrl( int aChar, const wxKeyEvent& e )
{
    return e.GetKeyCode() == aChar && e.ControlDown() && !e.AltDown() &&
            e.ShiftDown() && !e.MetaDown();
}


void SCINTILLA_TRICKS::onCharHook( wxKeyEvent& aEvent )
{
    if( aEvent.GetKeyCode() == WXK_TAB )
    {
        if( aEvent.ControlDown() )
        {
            int flags = 0;

            if( !aEvent.ShiftDown() )
                flags |= wxNavigationKeyEvent::IsForward;

            wxWindow* parent = m_te->GetParent();

            while( parent && dynamic_cast<DIALOG_SHIM*>( parent ) == nullptr )
                parent = parent->GetParent();

            if( parent )
                parent->NavigateIn( flags );
        }
        else
        {
            m_te->Tab();
        }
    }
    else if( isCtrl( 'Z', aEvent ) )
    {
        m_te->Undo();
    }
    else if( isShiftCtrl( 'Z', aEvent ) || isCtrl( 'Y', aEvent ) )
    {
        m_te->Redo();
    }
    else if( isCtrl( 'X', aEvent ) )
    {
        m_te->Cut();
    }
    else if( isCtrl( 'C', aEvent ) )
    {
        m_te->Copy();
    }
    else if( isCtrl( 'V', aEvent ) )
    {
        m_te->Paste();
    }
    else if( aEvent.GetKeyCode() == WXK_BACK )
    {
        m_te->DeleteBack();
    }
    else if( aEvent.GetKeyCode() == WXK_DELETE )
    {
        if( m_te->GetSelectionEnd() > m_te->GetSelectionStart() )
            m_te->DeleteBack();
    }
    else
    {
        aEvent.Skip();
    }
}


void SCINTILLA_TRICKS::onScintillaUpdateUI( wxStyledTextEvent& aEvent )
{
    auto isBrace = [this]( int c ) -> bool
                   {
                       return m_braces.Find( (wxChar) c ) >= 0;
                   };

    // Has the caret changed position?
    int caretPos = m_te->GetCurrentPos();

    if( m_lastCaretPos != caretPos )
    {
        m_lastCaretPos = caretPos;
        int bracePos1 = -1;
        int bracePos2 = -1;

        // Is there a brace to the left or right?
        if( caretPos > 0 && isBrace( m_te->GetCharAt( caretPos-1 ) ) )
            bracePos1 = ( caretPos - 1 );
        else if( isBrace( m_te->GetCharAt( caretPos ) ) )
            bracePos1 = caretPos;

        if( bracePos1 >= 0 )
        {
            // Find the matching brace
            bracePos2 = m_te->BraceMatch( bracePos1 );

            if( bracePos2 == -1 )
            {
                m_te->BraceBadLight( bracePos1 );
                m_te->SetHighlightGuide( 0 );
            }
            else
            {
                m_te->BraceHighlight( bracePos1, bracePos2 );
                m_te->SetHighlightGuide( m_te->GetColumn( bracePos1 ) );
            }
        }
        else
        {
            // Turn off brace matching
            m_te->BraceHighlight( -1, -1 );
            m_te->SetHighlightGuide( 0 );
        }
    }
}


void SCINTILLA_TRICKS::DoAutocomplete( const wxString& aPartial, const wxArrayString& aTokens )
{
    wxArrayString matchedTokens;

    wxString filter = wxT( "*" ) + aPartial.Lower() + wxT( "*" );

    for( const wxString& token : aTokens )
    {
        if( token.Lower().Matches( filter ) )
            matchedTokens.push_back( token );
    }

    if( matchedTokens.size() > 0 )
    {
        // NB: tokens MUST be in alphabetical order because the Scintilla engine is going
        // to do a binary search on them
        matchedTokens.Sort();

        m_te->AutoCompShow( aPartial.size(), wxJoin( matchedTokens, ' ' ) );
    }
}


