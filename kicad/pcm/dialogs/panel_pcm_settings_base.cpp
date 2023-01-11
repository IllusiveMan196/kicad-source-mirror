///////////////////////////////////////////////////////////////////////////
// C++ code generated with wxFormBuilder (version 3.10.1-0-g8feb16b)
// http://www.wxformbuilder.org/
//
// PLEASE DO *NOT* EDIT THIS FILE!
///////////////////////////////////////////////////////////////////////////

#include "panel_pcm_settings_base.h"

///////////////////////////////////////////////////////////////////////////

PANEL_PCM_SETTINGS_BASE::PANEL_PCM_SETTINGS_BASE( wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style, const wxString& name ) : wxPanel( parent, id, pos, size, style, name )
{
	wxBoxSizer* bSizer1;
	bSizer1 = new wxBoxSizer( wxVERTICAL );

	m_generalLabel = new wxStaticText( this, wxID_ANY, _("General"), wxDefaultPosition, wxDefaultSize, 0 );
	m_generalLabel->Wrap( -1 );
	bSizer1->Add( m_generalLabel, 0, wxTOP|wxRIGHT|wxLEFT, 13 );


	bSizer1->Add( 0, 3, 0, wxEXPAND, 5 );

	m_staticline1 = new wxStaticLine( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxLI_HORIZONTAL );
	bSizer1->Add( m_staticline1, 0, wxEXPAND|wxBOTTOM, 5 );

	wxBoxSizer* bSizer4;
	bSizer4 = new wxBoxSizer( wxVERTICAL );

	m_updateCheck = new wxCheckBox( this, wxID_ANY, _("Check for package updates on startup"), wxDefaultPosition, wxDefaultSize, 0 );
	m_updateCheck->SetValue(true);
	bSizer4->Add( m_updateCheck, 0, wxLEFT, 5 );


	bSizer1->Add( bSizer4, 0, wxEXPAND|wxTOP|wxLEFT, 5 );


	bSizer1->Add( 0, 20, 0, wxEXPAND, 5 );

	m_staticText4 = new wxStaticText( this, wxID_ANY, _("Library package handling"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText4->Wrap( -1 );
	bSizer1->Add( m_staticText4, 0, wxTOP|wxRIGHT|wxLEFT, 13 );


	bSizer1->Add( 0, 3, 0, wxEXPAND, 5 );

	m_staticline2 = new wxStaticLine( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxLI_HORIZONTAL );
	bSizer1->Add( m_staticline2, 0, wxEXPAND|wxBOTTOM, 5 );

	wxBoxSizer* bSizer3;
	bSizer3 = new wxBoxSizer( wxVERTICAL );

	m_libAutoAdd = new wxCheckBox( this, wxID_ANY, _("Automatically add installed libraries to global lib table"), wxDefaultPosition, wxDefaultSize, 0 );
	m_libAutoAdd->SetValue(true);
	bSizer3->Add( m_libAutoAdd, 0, wxALL, 5 );

	m_libAutoRemove = new wxCheckBox( this, wxID_ANY, _("Automatically remove uninstalled libraries"), wxDefaultPosition, wxDefaultSize, 0 );
	m_libAutoRemove->SetValue(true);
	bSizer3->Add( m_libAutoRemove, 0, wxALL, 5 );

	wxBoxSizer* bSizer2;
	bSizer2 = new wxBoxSizer( wxHORIZONTAL );

	m_staticText1 = new wxStaticText( this, wxID_ANY, _("Library nickname prefix:"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText1->Wrap( -1 );
	bSizer2->Add( m_staticText1, 0, wxALL|wxALIGN_CENTER_VERTICAL, 5 );

	m_libPrefix = new wxTextCtrl( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
	bSizer2->Add( m_libPrefix, 0, wxALL|wxALIGN_CENTER_VERTICAL, 5 );


	bSizer3->Add( bSizer2, 0, wxEXPAND, 5 );

	m_libHelp = new wxStaticText( this, wxID_ANY, _("After packages are (un)installed KiCad may need to be restarted to reflect changes in the global library table."), wxDefaultPosition, wxDefaultSize, 0 );
	m_libHelp->Wrap( -1 );
	bSizer3->Add( m_libHelp, 0, wxALL, 5 );


	bSizer1->Add( bSizer3, 1, wxEXPAND|wxTOP|wxLEFT, 5 );


	this->SetSizer( bSizer1 );
	this->Layout();
	bSizer1->Fit( this );
}

PANEL_PCM_SETTINGS_BASE::~PANEL_PCM_SETTINGS_BASE()
{
}
