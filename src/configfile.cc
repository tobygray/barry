///
/// \file	configfile.cc
///		Barry configuraion class, for one device PIN
///

/*
    Copyright (C) 2007-2012, Net Direct Inc. (http://www.netdirect.ca/)

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

#include "configfile.h"
#include "error.h"
#include "r_message.h"
#include "getpwuid.h"
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <fstream>
#include <sstream>
#include <iostream>
#include <iomanip>
#include <sys/types.h>
#include <sys/stat.h>

namespace Barry {

/// Creates a tar.gz filename using PIN + date + time + label.
/// Does not include any path, just returns a new filename.
std::string MakeBackupFilename(const Barry::Pin &pin,
				const std::string &label)
{
	using namespace std;

	time_t t = time(NULL);
	struct tm *lt = localtime(&t);

	std::string fileLabel = label;
	if( fileLabel.size() ) {
		// prepend a hyphen
		fileLabel.insert(fileLabel.begin(), '-');

		// translate all spaces and slashes
		for( size_t i = 0; i < fileLabel.size(); i++ ) {
			if( fileLabel[i] == ' ' )
				fileLabel[i] = '_';
			else if( fileLabel[i] == '/' )
				fileLabel[i] = '-';
			else if( fileLabel[i] == '\\' )
				fileLabel[i] = '-';
		}
	}

	ostringstream tarfilename;
	tarfilename << pin.Str() << "-"
		<< setw(4) << setfill('0') << (lt->tm_year + 1900)
		<< setw(2) << setfill('0') << (lt->tm_mon + 1)
		<< setw(2) << setfill('0') << lt->tm_mday
		<< "-"
		<< setw(2) << setfill('0') << lt->tm_hour
		<< setw(2) << setfill('0') << lt->tm_min
		<< setw(2) << setfill('0') << lt->tm_sec
		<< fileLabel
		<< ".tar.gz";
	return tarfilename.str();
}

bool ConfigFile::DBListType::IsSelected(const std::string &dbname) const
{
	const_iterator i = begin();
	for( ; i != end(); ++i ) {
		if( *i == dbname ) {
			return true;
		}
	}
	return false;
}

std::ostream& operator<< (std::ostream &os, const ConfigFile::DBListType &list)
{
	os << "DBListType dump:\n";

	for( ConfigFile::DBListType::const_iterator i = list.begin();
		i != list.end();
		++i )
	{
		os << "   " << *i << "\n";
	}
	return os;
}


//////////////////////////////////////////////////////////////////////////////
// ConfigFile class members

/// Loads config file for the given pin, and ends up in an
/// unenlightened state.  Throws ConfigFileError on error,
/// but it is not an error if the config does not exist.
/// Never use this if you have a DatabaseDatabase object!
/// This ctor is only for temporary loading of config data.
ConfigFile::ConfigFile(Barry::Pin pin)
	: m_pin(pin)
	, m_loaded(false)
	, m_promptBackupLabel(false)
	, m_autoSelectAll(false)
{
	if( m_pin == 0 )
		throw ConfigFileError("Configfile: empty pin");

	BuildFilename();
	BuildDefaultPath(); // this handles the situation that path is not set
	Load();
}

/// Opens and loads config file for given pin, and calls Enlighten
/// Throws ConfigFileError on error.  Should never fail unless
/// passed a bad pin.
ConfigFile::ConfigFile(Barry::Pin pin,
		       const Barry::DatabaseDatabase &db)
	: m_pin(pin)
	, m_loaded(false)
	, m_promptBackupLabel(false)
	, m_autoSelectAll(false)
{
	if( m_pin == 0 )
		throw ConfigFileError("Configfile: empty pin");

	BuildFilename();
	BuildDefaultPath();
	Load();
	Enlighten(db);
}

ConfigFile::~ConfigFile()
{
}

void ConfigFile::BuildFilename()
{
	size_t strsize = 255 * 5;
	char *strbuf = new char[strsize];
	struct passwd pwbuf;
	struct passwd *pw;

	getpwuid_r(getuid(), &pwbuf, strbuf, strsize, &pw);
	if( !pw ) {
		delete [] strbuf;
		throw ConfigFileError("BuildFilename: getpwuid failed", errno);
	}

	m_filename = pw->pw_dir;
	m_filename += "/.barry/backup/";
	m_filename += m_pin.Str();
	m_filename += "/config";

	delete [] strbuf;
}

void ConfigFile::BuildDefaultPath()
{
	struct passwd *pw = getpwuid(getuid());
	m_path = pw->pw_dir;
	m_path += "/.barry/backup/";
	m_path += m_pin.Str();
}

void ConfigFile::Clear()
{
	m_loaded = false;
	m_backupList.clear();
	m_restoreList.clear();
	m_deviceName.clear();
	m_promptBackupLabel = false;
	m_autoSelectAll = false;
}

/// Attempt to load the configuration file, but do not fail if not available
void ConfigFile::Load()
{
	// start fresh
	Clear();

	// open input file
	std::ifstream in(m_filename.c_str(), std::ios::in | std::ios::binary);
	if( !in )
		return;

	std::string line;
	DBListType *pList = 0;

	while( std::getline(in, line) ) {
		std::string keyword;
		std::istringstream iss(line);
		iss >> keyword;

		if( keyword == "backup_list" ) {
			pList = &m_backupList;
		}
		else if( keyword == "restore_list" ) {
			pList = &m_restoreList;
		}
		else if( line[0] == ' ' && pList ) {
			pList->push_back(line.c_str() + 1);
		}
		else {
			pList = 0;

			// add all remaining keyword checks here
			if( keyword == "device_name" ) {
				iss >> std::ws;
				std::getline(iss, m_deviceName);
				if( m_deviceName.size() == 0 ) {
					// if there is a device_name setting,
					// then this value must hold something,
					// so that the user can ignore this
					// field, and not get pestered all
					// the time
					m_deviceName = " ";
				}
			}
			else if( keyword == "backup_path" ) {
				iss >> std::ws;
				std::getline(iss, m_path);
				if( (m_path.size() == 0) || !(CheckPath(m_path)))
					BuildDefaultPath();
			}
			else if( keyword == "prompt_backup_label" ) {
				int flag;
				iss >> flag;
				m_promptBackupLabel = flag;
			}
			else if( keyword == "auto_select_all" ) {
				int flag;
				iss >> flag;
				m_autoSelectAll = flag;
			}
		}
	}

	m_loaded = true;
}

/// Saves current device's config, overwriting or creating a config file
bool ConfigFile::Save()
{
	using namespace std;

	if( !CheckPath(m_path, &m_last_error) )
		return false;

	ofstream out(m_filename.c_str(), std::ios::out | std::ios::binary);
	if( !out ) {
		m_last_error = "Unable to open " + m_filename + " for writing.";
		return false;
	}

	out << "backup_list" << endl;
	for( DBListType::iterator i = m_backupList.begin(); i != m_backupList.end(); ++i ) {
		out << " " << *i << endl;
	}

	out << "restore_list" << endl;
	for( DBListType::iterator i = m_restoreList.begin(); i != m_restoreList.end(); ++i ) {
		out << " " << *i << endl;
	}

	if( m_deviceName.size() ) {
		out << "device_name " << m_deviceName << endl;
	}

	if( m_path.size() ) {
		out << "backup_path " << m_path << endl;
	}

	out << "prompt_backup_label " << (m_promptBackupLabel ? 1 : 0) << endl;
	out << "auto_select_all " << (m_autoSelectAll ? 1 : 0) << endl;

	if( !out ) {
		m_last_error = "Error during write.  Config may be incomplete.";
		return false;
	}
	return true;
}

/// Compares a given databasedatabase from a real device with the
/// current config.  If not yet configured, initialize with valid
/// defaults.
void ConfigFile::Enlighten(const Barry::DatabaseDatabase &db)
{
	if( !m_loaded ) {
		// if not fully loaded, we use db as our default list
		// our defaults are: backup everything, restore everything
		// except email

		m_backupList.clear();
		m_restoreList.clear();

		Barry::DatabaseDatabase::DatabaseArrayType::const_iterator i =
			db.Databases.begin();
		for( ; i != db.Databases.end(); ++i ) {
			// backup everything
			m_backupList.push_back(i->Name);

			// restore everything except email (which could take ages)
			// and Handheld Agent (which seems write protected)
			if( i->Name != Barry::Message::GetDBName() &&
			    i->Name != "Handheld Agent" )
			{
				m_restoreList.push_back(i->Name);
			}
		}
	}
}

// fill list with all databases from dbdb
ConfigFile:: DBListType& ConfigFile::DBListType::operator=(const DatabaseDatabase &dbdb)
{
	// start empty
	clear();

	// copy over all DB names
	DatabaseDatabase::DatabaseArrayType::const_iterator
		i = dbdb.Databases.begin(), e = dbdb.Databases.end();
	for( ; i != e; ++i ) {
		push_back(i->Name);
	}

	return *this;
}

/// Sets list with new config
void ConfigFile::SetBackupList(const DBListType &list)
{
	m_backupList = list;
	m_loaded = true;
}

void ConfigFile::SetRestoreList(const DBListType &list)
{
	m_restoreList = list;
	m_loaded = true;
}

void ConfigFile::SetDeviceName(const std::string &name)
{
	if( name.size() )
		m_deviceName = name;
	else
		m_deviceName = " ";
}

void ConfigFile::SetBackupPath(const std::string &path)
{
	if( path.size() && CheckPath(path) )
		m_path = path;
	else
		BuildDefaultPath();
}

void ConfigFile::SetPromptBackupLabel(bool prompt)
{
	m_promptBackupLabel = prompt;
}

void ConfigFile::SetAutoSelectAll(bool asa)
{
	m_autoSelectAll = asa;
}

/// Checks that the path in path exists, and if not, creates it.
/// Returns false if unable to create path, true if ok.
bool ConfigFile::CheckPath(const std::string &path, std::string *perr)
{
	if( path.size() == 0 ) {
		if( perr )
			*perr = "path is empty!";
		return false;
	}

	if( access(path.c_str(), F_OK) == 0 )
		return true;

	std::string base;
	std::string::size_type slash = 0;
	while( (slash = path.find('/', slash + 1)) != std::string::npos ) {
		base = path.substr(0, slash);
		if( access(base.c_str(), F_OK) != 0 ) {
			if( mkdir(base.c_str(), 0755) == -1 ) {
				if( perr ) {
					*perr = "mkdir(" + base + ") failed: ";
					*perr += strerror(errno);
				}
				return false;
			}
		}
	}
	if( mkdir(path.c_str(), 0755) == -1 ) {
		if( perr ) {
			*perr = "last mkdir(" + path + ") failed: ";
			*perr += strerror(errno);
		}
		return false;
	}
	return true;
}



//////////////////////////////////////////////////////////////////////////////
// GlobalConfigFile class members

GlobalConfigFile::GlobalConfigFile()
	: m_loaded(false)
	, m_verboseLogging(false)
{
	BuildFilename();
	Load();
}

GlobalConfigFile::GlobalConfigFile(const std::string &appname)
	: m_loaded(false)
	, m_appname(appname)
	, m_verboseLogging(false)
{
	// there can be no spaces in the appname
	if( m_appname.find(' ') != std::string::npos )
		throw std::logic_error("App name must have no spaces.");

	BuildFilename();
	Load();
}

GlobalConfigFile::~GlobalConfigFile()
{
}

void GlobalConfigFile::BuildFilename()
{
	struct passwd *pw = getpwuid(getuid());
	if( !pw )
		throw ConfigFileError("BuildFilename: getpwuid failed", errno);

	m_filename = pw->pw_dir;
	m_filename += "/.barry/config";

	// build the global path too, since this never changes
	m_path = pw->pw_dir;
	m_path += "/.barry";
}

void GlobalConfigFile::Clear()
{
	m_loaded = false;
	m_lastDevice = 0;
}

void GlobalConfigFile::Load()
{
	// start fresh
	Clear();

	// open input file
	std::ifstream in(m_filename.c_str(), std::ios::in | std::ios::binary);
	if( !in )
		return;

	std::string line;

	while( std::getline(in, line) ) {
		std::string keyword;
		std::istringstream iss(line);
		iss >> keyword;

		if( keyword == "last_device" ) {
			iss >> std::ws;
			m_lastDevice.Clear();
			iss >> m_lastDevice;
		}
		else if( keyword == "verbose_logging" ) {
			int flag = 0;
			iss >> flag;
			m_verboseLogging = flag;
		}
		else {
			// store any other keys as app keys
			if( keyword.substr(0, 2) == "X-" ) {
				iss >> std::ws;
				line.clear();
				std::getline(iss, line);
				m_keymap[keyword] = line;
			}
		}
	}

	m_loaded = true;
}

/// Save the current global config, overwriting or creating as needed
bool GlobalConfigFile::Save()
{
	if( !ConfigFile::CheckPath(m_path, &m_last_error) )
		return false;

	std::ofstream out(m_filename.c_str(), std::ios::out | std::ios::binary);
	if( !out ) {
		m_last_error = "Unable to open " + m_filename + " for writing.";
		return false;
	}

	if( !(m_lastDevice == 0) ) {
		out << "last_device " << m_lastDevice.Str() << std::endl;
	}

	out << "verbose_logging " << (m_verboseLogging ? 1 : 0) << std::endl;

	// store all app keys
	keymap_type::const_iterator ci = m_keymap.begin();
	for( ; ci != m_keymap.end(); ++ci ) {
		out << ci->first << " " << ci->second << std::endl;
	}

	if( !out ) {
		m_last_error = "Error during write.  Config may be incomplete.";
		return false;
	}
	return true;
}

void GlobalConfigFile::SetKey(const std::string &key, const std::string &value)
{
	if( !m_appname.size() )
		throw std::logic_error("Cannot use SetKey() without specifying an appname in the constructor.");

	if( value.find_first_of("\n\r") != std::string::npos )
		throw std::logic_error("SetKey values may not contain newline characters.");

	std::string fullkey = "X-" + m_appname + "-" + key;
	m_keymap[fullkey] = value;
}

std::string GlobalConfigFile::GetKey(const std::string &key,
				     const std::string &default_value) const
{
	if( !m_appname.size() )
		throw std::logic_error("Cannot use SetKey() without specifying an appname in the constructor.");

	std::string fullkey = "X-" + m_appname + "-" + key;
	keymap_type::const_iterator ci = m_keymap.find(fullkey);
	if( ci == m_keymap.end() )
		return default_value;
	return ci->second;
}

void GlobalConfigFile::SetLastDevice(const Barry::Pin &pin)
{
	m_lastDevice = pin;
}

void GlobalConfigFile::SetVerboseLogging(bool verbose)
{
	m_verboseLogging = verbose;
}


} // namespace Barry

