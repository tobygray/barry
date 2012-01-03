///
/// \file	EvoCfgDlg.cc
///		The configuration dialog used to configure Evolution sources
///

/*
    Copyright (C) 2011-2012, Net Direct Inc. (http://www.netdirect.ca/)

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

    See the GNU General Public License in the COPYING file at the
    root directory of this project for more details.
*/

#include "EvoCfgDlg.h"
#include "osconfig.h"
#include "windowids.h"

using namespace std;

EvoCfgDlg::EvoCfgDlg(wxWindow *parent,
			const OpenSync::Config::Evolution &ec,
			const EvoSources &es)
	: wxDialog(parent, Dialog_EvoCfg, _T("Evolution Plugin Configuration"))
	, m_address_path(ec.GetAddressPath())
	, m_calendar_path(ec.GetCalendarPath())
	, m_tasks_path(ec.GetTasksPath())
	, m_memos_path(ec.GetMemosPath())
	, m_sources(es)
{
	CreateLayout();
}

void EvoCfgDlg::CreateLayout()
{
	m_topsizer = new wxBoxSizer(wxVERTICAL);

	m_topsizer->Add(
		new wxStaticText(this, wxID_ANY, _T("Address Book:")),
		0, wxALIGN_LEFT | wxTOP | wxLEFT | wxRIGHT, 10);
	AddCombo(&m_address_combo, wxID_ANY,
		m_address_path, m_sources.GetAddressBook());
	m_topsizer->Add(m_address_combo, 0, wxEXPAND | wxLEFT | wxRIGHT, 10);

	m_topsizer->Add(
		new wxStaticText(this, wxID_ANY, _T("Calendar:")),
		0, wxALIGN_LEFT | wxTOP | wxLEFT | wxRIGHT, 10);
	AddCombo(&m_calendar_combo, wxID_ANY,
		m_calendar_path, m_sources.GetEvents());
	m_topsizer->Add(m_calendar_combo, 0, wxEXPAND | wxLEFT | wxRIGHT, 10);

	m_topsizer->Add(
		new wxStaticText(this, wxID_ANY, _T("Tasks:")),
		0, wxALIGN_LEFT | wxTOP | wxLEFT | wxRIGHT, 10);
	AddCombo(&m_tasks_combo, wxID_ANY,
		m_tasks_path, m_sources.GetTasks());
	m_topsizer->Add(m_tasks_combo, 0, wxEXPAND | wxLEFT | wxRIGHT, 10);

	m_topsizer->Add(
		new wxStaticText(this, wxID_ANY, _T("Memos:")),
		0, wxALIGN_LEFT | wxTOP | wxLEFT | wxRIGHT, 10);
	AddCombo(&m_memos_combo, wxID_ANY,
		m_memos_path, m_sources.GetMemos());
	m_topsizer->Add(m_memos_combo, 0, wxEXPAND | wxLEFT | wxRIGHT, 10);

	wxSizer *button = CreateSeparatedButtonSizer(wxOK | wxCANCEL);
	m_topsizer->Add(button, 0, wxALL | wxEXPAND | wxALIGN_RIGHT, 10);

	SetSizer(m_topsizer);
	m_topsizer->SetSizeHints(this);
	m_topsizer->Layout();
}

void EvoCfgDlg::AddCombo(wxComboBox **combo, int  id,
			const std::string &current_path,
			const EvoSources::List &list)
{
	// is the current path in the list?
	bool in_list = false;
	for( EvoSources::List::const_iterator i = list.begin(); current_path.size() && i != list.end(); ++i ) {
		if( i->m_SourcePath == current_path ) {
			in_list = true;
			break;
		}
	}

	// create an array of choices, and add current_path as first in list
	// if it is not already there
	wxArrayString choices;
	if( current_path.size() && !in_list ) {
		choices.Add(wxString(current_path.c_str(), wxConvUTF8));
	}

	// add the sources list
	for( EvoSources::List::const_iterator i = list.begin(); i!=list.end(); ++i ) {
		if( i->m_SourcePath.size() ) {
			choices.Add(wxString(i->m_SourcePath.c_str(), wxConvUTF8));
		}
	}

	// create the combo box
	*combo = new wxComboBox(this, id,
		choices.GetCount() ? choices[0] : wxString(),
		wxDefaultPosition, wxDefaultSize, // wxSize(250, -1), //wxDefaultSize,
		choices,
		wxCB_DROPDOWN);
}

void EvoCfgDlg::SetPaths(OpenSync::Config::Evolution &ec) const
{
	ec.SetAddressPath(m_address_path);
	ec.SetCalendarPath(m_calendar_path);
	ec.SetTasksPath(m_tasks_path);
	ec.SetMemosPath(m_memos_path);
}

wxString EvoCfgDlg::CheckPath(const wxString &name,
				const std::string &path) const
{
	if( path.size() > 7 && path.substr(0, 7) == "file://" ) {
		std::string p = path.substr(7);
		if( !EvoSources::PathExists(p) ) {
			wxString msg = name + _T(" does not exist.");
			return msg;
		}
	}

	return _T("");
}

wxString EvoCfgDlg::ValidatePaths() const
{
	if( m_address_path.empty() || m_calendar_path.empty() || m_tasks_path.empty() )
		return _T("Address book, Event, and Tasks must be set.");

	wxString msg;
	msg = CheckPath(_T("Address book path"), m_address_path);
	if( msg.size() )
		return msg;
	msg = CheckPath(_T("Event path"), m_calendar_path);
	if( msg.size() )
		return msg;
	msg = CheckPath(_T("Tasks path"), m_tasks_path);
	if( msg.size() )
		return msg;
	msg = CheckPath(_T("Memos path"), m_memos_path);
	return msg;
}

bool EvoCfgDlg::TransferDataFromWindow()
{
	m_address_path = string(m_address_combo->GetValue().utf8_str());
	m_calendar_path = string(m_calendar_combo->GetValue().utf8_str());
	m_tasks_path = string(m_tasks_combo->GetValue().utf8_str());
	m_memos_path = string(m_memos_combo->GetValue().utf8_str());

	wxString msg = ValidatePaths();
	if( msg.size() ) {
		wxMessageBox(msg, _T("Minimum Config Required"),
			wxOK | wxICON_ERROR, this);
		return false;
	}

	return true;
}

int EvoCfgDlg::ShowModal()
{
	return wxDialog::ShowModal();
}

