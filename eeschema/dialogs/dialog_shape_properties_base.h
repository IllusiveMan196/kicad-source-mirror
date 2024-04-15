///////////////////////////////////////////////////////////////////////////
// C++ code generated with wxFormBuilder (version 4.0.0-0-g0efcecf)
// http://www.wxformbuilder.org/
//
// PLEASE DO *NOT* EDIT THIS FILE!
///////////////////////////////////////////////////////////////////////////

#pragma once

#include <wx/artprov.h>
#include <wx/xrc/xmlres.h>
#include <wx/intl.h>
class COLOR_SWATCH;
class WX_INFOBAR;

#include "dialog_shim.h"
#include <wx/infobar.h>
#include <wx/gdicmn.h>
#include <wx/font.h>
#include <wx/colour.h>
#include <wx/settings.h>
#include <wx/string.h>
#include <wx/checkbox.h>
#include <wx/stattext.h>
#include <wx/textctrl.h>
#include <wx/sizer.h>
#include <wx/panel.h>
#include <wx/bmpcbox.h>
#include <wx/gbsizer.h>
#include <wx/radiobut.h>
#include <wx/statbox.h>
#include <wx/simplebook.h>
#include <wx/button.h>
#include <wx/dialog.h>

///////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
/// Class DIALOG_SHAPE_PROPERTIES_BASE
///////////////////////////////////////////////////////////////////////////////
class DIALOG_SHAPE_PROPERTIES_BASE : public DIALOG_SHIM
{
	DECLARE_EVENT_TABLE()
	private:

		// Private event handlers
		void _wxFB_onBorderChecked( wxCommandEvent& event ){ onBorderChecked( event ); }
		void _wxFB_onFillChecked( wxCommandEvent& event ){ onFillChecked( event ); }
		void _wxFB_onFillRadioButton( wxCommandEvent& event ){ onFillRadioButton( event ); }
		void _wxFB_onFill( wxCommandEvent& event ){ onFill( event ); }


	protected:
		enum
		{
			NO_FILL = 1000,
			FILLED_SHAPE,
			FILLED_WITH_BG_BODYCOLOR,
			FILLED_WITH_COLOR
		};

		WX_INFOBAR* m_infoBar;
		wxGridBagSizer* m_borderSizer;
		wxCheckBox* m_borderCheckbox;
		wxStaticText* m_borderWidthLabel;
		wxTextCtrl* m_borderWidthCtrl;
		wxStaticText* m_borderWidthUnits;
		wxStaticText* m_borderColorLabel;
		wxPanel* m_panelBorderColor;
		COLOR_SWATCH* m_borderColorSwatch;
		wxStaticText* m_borderStyleLabel;
		wxBitmapComboBox* m_borderStyleCombo;
		wxStaticText* m_helpLabel1;
		wxSimplebook* m_fillBook;
		wxPanel* m_schematicPage;
		wxGridBagSizer* m_fillSizer;
		wxCheckBox* m_filledCtrl;
		wxStaticText* m_fillColorLabel;
		wxPanel* m_panelFillColor;
		COLOR_SWATCH* m_fillColorSwatch;
		wxStaticText* m_helpLabel2;
		wxPanel* m_symbolEditorPage;
		wxRadioButton* m_rbFillNone;
		wxRadioButton* m_rbFillOutline;
		wxRadioButton* m_rbFillBackground;
		wxRadioButton* m_rbFillCustom;
		COLOR_SWATCH* m_customColorSwatch;
		wxBoxSizer* m_symbolEditorSizer;
		wxCheckBox* m_privateCheckbox;
		wxCheckBox* m_checkApplyToAllUnits;
		wxCheckBox* m_checkApplyToAllBodyStyles;
		wxStdDialogButtonSizer* m_sdbSizer;
		wxButton* m_sdbSizerOK;
		wxButton* m_sdbSizerCancel;

		// Virtual event handlers, override them in your derived class
		virtual void onBorderChecked( wxCommandEvent& event ) { event.Skip(); }
		virtual void onFillChecked( wxCommandEvent& event ) { event.Skip(); }
		virtual void onFillRadioButton( wxCommandEvent& event ) { event.Skip(); }
		virtual void onFill( wxCommandEvent& event ) { event.Skip(); }


	public:

		DIALOG_SHAPE_PROPERTIES_BASE( wxWindow* parent, wxWindowID id = wxID_ANY, const wxString& title = _("%s Properties"), const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxSize( -1,-1 ), long style = wxDEFAULT_DIALOG_STYLE|wxRESIZE_BORDER );

		~DIALOG_SHAPE_PROPERTIES_BASE();

};

