///
/// \file	ModemDlg.cc
///		Dialog class to handle modem functionality
///

/*
    Copyright (C) 2012, Net Direct Inc. (http://www.netdirect.ca/)

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

#include "ModemDlg.h"
#include "windowids.h"
#include "exechelper.h"
#include "barrydesktop.h"
#include "tempdir.h"
#include <iostream>
#include <sstream>
#include <algorithm>
#include <unistd.h>
#include <sys/types.h>
#include <grp.h>
#include <limits.h>
#include <stdlib.h>
#include <wx/dir.h>
#include <wx/filename.h>
#include <barry/barry.h>

using namespace std;

// begin wxGlade: ::extracode
// end wxGlade


ModemDlg::ModemDlg(wxWindow* parent,
			const std::vector<std::string> &peers,
			const std::string &default_peer)
	: wxDialog(parent, Dialog_Modem, _T("Modem Kickstart"))
{
	bottom_buttons = CreateButtonSizer(wxOK | wxCANCEL);

	// begin wxGlade: ModemDlg::ModemDlg
	sizer_5_staticbox = new wxStaticBox(this, -1, wxT("Device"));
	sizer_1_staticbox = new wxStaticBox(this, -1, wxT("Providers"));
	const wxString list_box_1_choices[] = {
        wxT("barry-minimal"),
        wxT("barry-rogers"),
        wxT("barry-testing")
    };
	list_box_1 = new wxListBox(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, 3, list_box_1_choices, wxLB_SINGLE);
	label_1 = new wxStaticText(this, wxID_ANY, wxT("Password:"));
	text_ctrl_1 = new wxTextCtrl(this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_PASSWORD);

	set_properties();
	do_layout();
	// end wxGlade

	// add all peers to the listbox
	list_box_1->Clear();
	int default_index = -1, index = 0;
	for( std::vector<std::string>::const_iterator i = peers.begin();
		i != peers.end(); ++i, ++index )
	{
		if( default_peer == *i )
			default_index = index;
		list_box_1->Append(wxString(i->c_str(), wxConvUTF8));
	}

	if( default_index >= 0 ) {
		list_box_1->SetSelection(default_index);
	}
	else {
		list_box_1->SetSelection(0);
	}
}

void ModemDlg::set_properties()
{
	// begin wxGlade: ModemDlg::set_properties
	SetTitle(wxT("Modem Starter"));
	list_box_1->SetMinSize(wxSize(-1, 150));
	list_box_1->SetSelection(0);
	text_ctrl_1->SetMinSize(wxSize(150, -1));
	// end wxGlade
}


void ModemDlg::do_layout()
{
	// begin wxGlade: ModemDlg::do_layout
	wxBoxSizer* sizer_2 = new wxBoxSizer(wxHORIZONTAL);
	wxBoxSizer* sizer_3 = new wxBoxSizer(wxVERTICAL);
	wxStaticBoxSizer* sizer_5 = new wxStaticBoxSizer(sizer_5_staticbox, wxHORIZONTAL);
	wxStaticBoxSizer* sizer_1 = new wxStaticBoxSizer(sizer_1_staticbox, wxVERTICAL);
	sizer_1->Add(list_box_1, 0, wxEXPAND, 0);
	sizer_3->Add(sizer_1, 0, wxEXPAND, 0);
	sizer_5->Add(label_1, 0, wxRIGHT|wxALIGN_CENTER_VERTICAL, 2);
	sizer_5->Add(text_ctrl_1, 0, 0, 0);
	sizer_3->Add(sizer_5, 1, wxEXPAND, 0);
	sizer_2->Add(sizer_3, 0, wxALL|wxEXPAND, 7);
	// end wxGlade

	sizer_3->Add(bottom_buttons, 0, wxTOP|wxEXPAND, 5);

	SetSizer(sizer_2);
	sizer_2->Fit(this);
	Layout();
}

//
// ProgramDetect
//
/// Searches for the given program name, in various directories, and
/// stores the following info:
///	Exists() - file exists, and if true, then:
///	GetPath() - returns full path of file, and:
///	IsExecutable() - returns true if file is a runnable program
///	IsSuid() - returns true if file is suid
///	GetGroup() - returns the group name of the file
///
/// For pppd, for example, this may return on Debian:
///	exists? true
///	path? /usr/sbin/pppd
///	executable? true if user in group, false if not
///	suid? true, suid root on Debian
///	group? uses the 'dip' group on Debian
///
/// On Fedora:
///	exists? true
///	path? /usr/sbin/pppd
///	exec? true
///	suid? false
///	group? root
///
class ProgramDetect
{
	std::string m_path;
	std::string m_group;
	bool m_executable;
	bool m_suid;

public:
	void Dump(std::ostream &os)
	{
		os << "Path: " << m_path << "\n";
		os << "Group: " << m_group << "\n";
		os << "Exec: " << (m_executable ? "true" : "false") << "\n";
		os << "Suid: " << (m_suid ? "true" : "false") << "\n";
	}

	const std::string& GetPath() const { return m_path; }
	const std::string& GetGroup() const { return m_group; }
	bool Exists() const { return m_path.size() > 0; }
	bool IsExecutable() const { return m_executable; }
	bool IsSuid() const { return m_suid; }

	bool CheckPath(const std::string &path)
	{
		if( access(path.c_str(), F_OK) == 0 ) {
			m_path = path;
			return true;
		}
		return false;
	}

	void CheckAll(const char *prog, const std::string &path)
	{
		istringstream iss(path);
		string p;

		while( getline(iss, p, ':') ) {
			string f = p + "/" + prog;
			if( CheckPath(f) )
				return;
		}
	}

	ProgramDetect(const char *prog, const std::string &path)
		: m_executable(false)
		, m_suid(false)
	{
		// find the program first
		CheckAll(prog, path);

		// does it exist?
		if( !m_path.size() )
			return;	// nope

		// executable?
		if( access(m_path.c_str(), X_OK) == 0 ) {
			m_executable = true;
		}

		// suid?
		struct stat s;
		if( stat(m_path.c_str(), &s) == 0 ) {
			if( s.st_mode & S_ISUID ) {
				m_suid = true;

			}

			struct group *g = getgrgid(s.st_gid);
			if( g ) {
				m_group = g->gr_name;
			}
		}
	}
};

std::string ModemDlg::GetPeerName() const
{
	return string(list_box_1->GetStringSelection().utf8_str());
}

std::string ModemDlg::GetPassword() const
{
	return string(text_ctrl_1->GetValue().utf8_str());
}

void ModemDlg::DoModem(wxWindow *parent, const Barry::Pin &pin)
{
	// test whether xterm is in the path
	ProgramDetect xterm("xterm", getenv("PATH"));
	if( !xterm.Exists() || !xterm.IsExecutable() ) {
		wxMessageBox(_T("Cannot locate the xterm program.  This is used to display modem connection output.  Please install it and try again."), _T("Xterm Not Found"), wxOK | wxICON_ERROR, parent);
		return;
	}

	// in preparation for pppob search, find sbin, relative to our
	// current application path
	wxFileName app_path(wxTheApp->argv[0]);
	string app_sbin(app_path.GetPath().utf8_str());
	app_sbin += "/../sbin";
	if( app_sbin[0] != '/' ) {
		char cwdbuf[PATH_MAX];
		getcwd(cwdbuf, PATH_MAX);

		// insert current directory
		app_sbin.insert(0, string(cwdbuf) + "/");
	}
	char *app_real = realpath(app_sbin.c_str(), NULL);
	app_sbin = app_real;
	free(app_real);
cout << "app_sbin = " << app_sbin << endl;

	// test whether pppob is available... use app directory as
	// part of search as well, in case Barry is installed in an
	// unusual place
	ProgramDetect pppob("pppob", app_sbin + ":/usr/sbin:" + getenv("PATH"));
	if( !pppob.Exists() || !pppob.IsExecutable() ) {
		wxMessageBox(_T("Cannot find pppob.  Please make sure Barry's command line utilities are installed."), _T("pppob Not Found"), wxOK | wxICON_ERROR, parent);
		return;
	}
cout << pppob.GetPath() << endl;

	// test whether we can run pppd, probably need to use full
	// /usr/sbin/pppd path to reach it... check for return code
	// of 0 or 2
	ProgramDetect pppd("pppd", "/usr/sbin:/usr/bin");
	if( !pppd.Exists() ) {
	    	wxMessageBox(_T("Cannot find pppd.  Please install it for modem use."), _T("pppd Not Found"), wxOK | wxICON_ERROR, parent);
		return;
	}

	string need_sudo;

	if( !(pppd.IsSuid() && pppd.GetGroup() != "root") ) {
		// No group / suid setup, so need sudo
		// Use gtk's gui sudo, since we're using GTK's wxwidgets
		// and probably safe here
		// FIXME - need a portable method for all of this someday :-(
		need_sudo = "gksu ";
	}

	ExecHelper eh(0);
	wxString cmd((need_sudo + pppd.GetPath() + " help").c_str(), wxConvUTF8);
	if( !eh.Run(0, "", cmd) ) {
		if( need_sudo.size() ) {
			wxMessageBox(_T("Unable to run pppd.  Please make sure you have it enabled in your sudo config, or put pppd in a group of its own."), _T("Cannot Run pppd"), wxOK | wxICON_ERROR, parent);
		}
		else {
			wxString msg = wxString::Format(_T("Unable to run pppd.  Please make sure your user account is included in the '%s' group."), wxString(pppd.GetGroup().c_str(), wxConvUTF8).c_str());

			wxMessageBox(msg, _T("Cannot Run pppd"), wxOK | wxICON_ERROR, parent);
		}

		return;
	}

	eh.WaitForChild();
	if( !(eh.GetChildExitCode() == 0 || eh.GetChildExitCode() == 2) ) {
		wxString msg = wxString::Format(_T("Unable to run pppd correctly.  Unexpected error code: %d, %s"), eh.GetChildExitCode(), cmd.c_str());
		wxMessageBox(msg, _T("Error Code"), wxOK | wxICON_ERROR, parent);
		return;
	}

	// do a search in /etc/ppp/peers for all barry-* files and
	// store in a vector
	std::vector<std::string> peers;
	wxDir dir(wxString("/etc/ppp/peers", wxConvUTF8));
	if( !dir.IsOpened() ) {
		wxMessageBox(_T("Unable to access files in /etc/ppp/peers.  Do you have the correct permissions?"), _T("Cannot Open Peers"), wxOK | wxICON_ERROR, parent);
		return;
	}
	wxString filename;
	bool cont = dir.GetFirst(&filename, _T("barry-*"), wxDIR_FILES);
	while( cont ) {
		peers.push_back( string(filename.utf8_str()) );
		cont = dir.GetNext(&filename);
	}

	// anything available?
	if( !peers.size() ) {
		wxMessageBox(_T("No providers found.  Make sure Barry was properly installed, with peer files in /etc/ppp/peers."), _T("No Providers"), wxOK | wxICON_ERROR, parent);
		return;
	}

	// sort the vector
	sort(peers.begin(), peers.end());

	// do an access or file open test on the first barry-* file
	// to make sure that we have access to peers/
	string testfile = "/etc/ppp/peers/" + peers[0];
	if( access(testfile.c_str(), R_OK) != 0 ) {
		wxString msg = wxString::Format(_T("Cannot read provider files under /etc/ppp/peers. Please check your file permissions. (Access failed for %s)"), wxString(testfile.c_str(), wxConvUTF8).c_str());
		wxMessageBox(msg, _T("Permissions Error"), wxOK | wxICON_ERROR, parent);
		return;
	}

	// fetch default peer choice
	Barry::GlobalConfigFile &config = wxGetApp().GetGlobalConfig();
	string key = pin.Str() + "-DefaultPeer";
	string default_peer = config.GetKey(key);

	// show the dialog
	ModemDlg dlg(parent, peers, default_peer);
	if( dlg.ShowModal() == wxID_OK ) {
		string password = dlg.GetPassword();
		string peer = dlg.GetPeerName();
		string peerfile = "/etc/ppp/peers/" + peer;
		if( !peer.size() || access(peerfile.c_str(), R_OK) != 0 ) {
			wxString msg = wxString::Format(_T("Unable to open peer file: %s"), wxString(peerfile.c_str(), wxConvUTF8).c_str());
			wxMessageBox(msg, _T("Invalid Peer"), wxOK | wxICON_ERROR, parent);
			return;
		}

		// save peer selection as default for this device
		config.SetKey(key, peer);

		// create shell script which calls pppd with proper
		// pty pppob override, and password if needed
		TempDir tempdir("BarryDesktopModem");
		string tmp_path = tempdir.GetNewFilename();

		ofstream otmp(tmp_path.c_str());
		otmp << "#!/bin/sh" << endl;
		otmp << "echo Starting pppd for device PIN "
			<< pin.Str() << "... " << endl;
//		otmp << need_sudo << pppd.GetPath()
// FIXME - need gksu here, for the pty override :-(  Need method to
// pass options to pppob without root
		otmp << "gksu " << pppd.GetPath()
			<< " call " << peer
			<< " pty \"" << pppob.GetPath()
				<< " -p " << pin.Str();
		if( password.size() )
			otmp << " -P " << password;
		otmp << '"' << endl;
		otmp << "echo Press enter to close window..." << endl;
		otmp << "read" << endl;
		otmp.close();

		chmod(tmp_path.c_str(), S_IRUSR | S_IRGRP | S_IROTH |
			S_IXUSR | S_IXGRP | S_IXOTH);

		// create command line using xterm as display
		wxString xterm_cmd((xterm.GetPath() + " " + tmp_path).c_str(),
			wxConvUTF8);

		// run! and go back to main screen
		ExecHelper run(0);
		run.Run(parent, "modem", xterm_cmd);

		// let run, then let TempDir cleanup
		sleep(2);
	}
}
