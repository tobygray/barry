///
/// \file	StringSync.h
///		Class to easily manage the wxString / std::string boundary
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

#ifndef __BARRYDESKTOP_STRING_SYNC_H__
#define __BARRYDESKTOP_STRING_SYNC_H__

#include <wx/wx.h>
#include <list>

//
// StringSync
//
/// Provides a wxString or std::string while maintaining a link to
/// a corresponding external string of the opposite kind.  When
/// Sync() is called, the external linked string is updated with the
/// current contents of the returned string.
///
/// This class contains two lists of strings: one that maps external
/// std::strings to internal wxStrings, and another that maps external
/// wxStrings to internal std::strings.  These lists never mix.
/// Add or Refresh transfers data from external to internal, and Sync
/// transfers data from internal to external.
///
/// Normally, you would use Add and Refresh when setting up a dialog,
/// and once setup, never use them again.  Use Sync when extracting
/// data from the dialog, back into the original variables again.
///
/// Note that you must call Sync explicitly, as it is not called
/// in the destructor, since it is impossible to know when the
/// external strings are freed, and they may have been freed already
/// by the time ~StringSync() is called.
///
class StringSync
{
public:
	typedef std::pair<wxString, std::string*>	WxIsCopyType;
	typedef std::pair<std::string, wxString*>	StdIsCopyType;
	typedef std::list<WxIsCopyType>			WxIsCopyList;
	typedef std::list<StdIsCopyType>		StdIsCopyList;

private:
	WxIsCopyList m_wx;
	StdIsCopyList m_std;

public:
	/// On destruction, calls Sync()
	~StringSync();

	/// Creates an internal wxString, copies the contents of source
	/// into it, and returns its reference.
	wxString* Add(std::string &source);

	/// Does the opposite
	std::string* Add(wxString &source);

	/// Copies all internal wxString contents into the corresponding
	/// external std::strings.
	void SyncToStd();

	/// Copies all internal std::string contents into the corresponding
	/// external wxStrings.
	void SyncToWx();

	/// Calls both SyncToStd() and SyncToWx()
	void Sync();

	/// Copies the contents of all the external std::strings into
	/// their internal wxStrings, basically doing an Add() for each one
	/// without re-creating the wxStrings.
	void RefreshWx();

	/// Copies the contents of all the external wxStrings into their
	/// internal std::strings, basically doing an Add() for each one
	/// without re-creating the std::strings.
	void RefreshStd();

	/// Calls both RefreshStd() and RefreshWx()
	void Refresh();
};

#endif

