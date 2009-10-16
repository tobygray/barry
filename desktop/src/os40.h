///
/// \file	os40.h
///		Wrapper class for opensync 0.4x syncing behaviour
///

/*
    Copyright (C) 2009, Net Direct Inc. (http://www.netdirect.ca/)

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

#ifndef __BARRYDESKTOP_OS40_H__
#define __BARRYDESKTOP_OS40_H__

#include "dlopen.h"
#include "osbase.h"
#include <memory>

namespace OpenSync {

class OpenSync40Private;

class OpenSync40 : public DlOpen, public OpenSync::API
{
public:

private:
	// private opensync 0.40 function pointers and data
	OpenSync40Private *m_priv;

protected:
	void SetupEnvironment(OpenSync40Private *p);

public:
	OpenSync40();
	~OpenSync40();

	//
	// Virtual API overrides
	//

	// General engine information
	const char* GetVersion() const;
	void GetPluginNames(string_list_type &plugins);
	void GetFormats(format_list_type &formats);

	// Information about configured groups
	void GetGroupNames(string_list_type &groups);
	void GetMembers(const std::string &group_name,
		member_list_type &members);

	// Group configuration
	void AddGroup(const std::string &group_name);
	void DeleteGroup(const std::string &group_name);

	// Member configuration
	void AddMember(const std::string &group_name,
		const std::string &plugin_name,
		const std::string &member_name);
	void DeleteMember(const std::string &group_name, long member_id);
	void DeleteMember(const std::string &group_name,
		const std::string &plugin_name);

	// Syncing
};

} // namespace OpenSync

#endif

