/**
 * @file dp_codinfo.cc
 * @author Nicolas VIVIEN
 * @date 2009-08-01
 *
 * @note CopyRight Nicolas VIVIEN
 *
 * @brief COD debug file parser
 *   RIM's JDE generates several files when you build a COD application.
 *   Indeed, with the COD files for the device, we have a ".debug" file.
 *   This file is usefull to debug an application from JVM.
 *   This tool is a parser to understand these ".debug" files.
 *
 * @par Modifications
 *   - 2009/08/01 : N. VIVIEN
 *     - First release
 *
 * @par Licences
 *   Copyright (C) 2009-2010, Nicolas VIVIEN
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *
 *   See the GNU General Public License in the COPYING file at the
 *   root directory of this project for more details.
 */


#include <fstream>
#include <iomanip>

#include <sys/types.h>
#include <dirent.h>
#include <string.h>

#include "dp_parser.h"
#include "dp_codinfo.h"
#include "ios_state.h"
#include "debug.h"


#define COD_DEBUG_APPNAME_HEADERFIELD		0x0
#define COD_DEBUG_UNIQUEID_HEADERFIELD		0x8

#define COD_DEBUG_NONE_FIELD			0x0
#define COD_DEBUG_BOOLEAN_FIELD			0x1
#define COD_DEBUG_BYTE_FIELD			0x2
#define COD_DEBUG_CHAR_FIELD			0x3
#define COD_DEBUG_SHORT_FIELD			0x4
#define COD_DEBUG_INT_FIELD			0x5
#define COD_DEBUG_LONG_FIELD			0x6
#define COD_DEBUG_CLASS_FIELD			0x7
#define COD_DEBUG_ARRAY_FIELD			0x8
#define COD_DEBUG_VOID_FIELD			0xA
#define COD_DEBUG_DOUBLE_FIELD			0xC


using namespace std;


namespace Barry {

namespace JDG {


// Public API
//------------

#define DEBUG_FILE_EXT		".debug"


void SearchDebugFile(DebugFileList &list)
{
	DIR *path;
	struct dirent *entry;

	path = opendir(".");

	while( (entry = readdir(path)) ) {
		int offset;

		if (strlen(entry->d_name) < strlen(DEBUG_FILE_EXT))
			continue;

		offset = strlen(entry->d_name) - strlen(DEBUG_FILE_EXT);

		if (!strcmp(entry->d_name + offset, DEBUG_FILE_EXT)) {
			ifstream file(entry->d_name);

			CodInfo info;

			// Parse header section
			info.ParseHeaderSection(file);

			// Add element to list
			list.AddElement(info.GetUniqueId(), info.GetAppName(), entry->d_name);
		}
	}

	closedir(path);
}


bool LoadDebugInfo(const DebugFileList &list, const char *filename, CodInfo &info)
{
	if (filename == NULL)
		return false;

	DebugFileList::const_iterator b = list.begin();

	for( ; b != list.end(); b++ ) {
		const DebugFileEntry &entry = (*b);

		if( entry.fileName == filename ) {
			info.LoadDebugFile(filename);
			return true;
		}
	}

	return false;
}


bool LoadDebugInfo(const DebugFileList &list, const uint32_t uniqueId, const std::string module, CodInfo &info)
{
	DebugFileList::const_iterator b = list.begin();

	for( ; b != list.end(); b++ ) {
		const DebugFileEntry &entry = (*b);

		if ((entry.uniqueId == uniqueId) && (entry.appName == module)) {
			info.LoadDebugFile(entry.fileName.c_str());
			return true;
		}
	}

	return false;
}


// DebugFileList class
//------------------------

void DebugFileList::AddElement(uint32_t uniqueid,
				const std::string &appname,
				const std::string &filename)
{
	DebugFileEntry entry;

	entry.uniqueId = uniqueid;
	entry.appName = appname;
	entry.fileName = filename;

	push_back(entry);
}


void DebugFileList::Dump(std::ostream &os) const
{
	ios_format_state state(os);

	const_iterator i = begin(), e = end();

	os << "  UniqueID  " << "|";
	os << "        Module Name       " << "|";
	os << "         File Name        " << endl;

	os << "------------+";
	os << "--------------------------+";
	os << "--------------------------";
	os << endl;

	for( ; i != e; ++i ) {
		(*i).Dump(os);
	}
}


void DebugFileEntry::Dump(std::ostream &os) const
{
	ios_format_state state(os);

	os << " 0x" << setfill('0') << setw(8) << hex << uniqueId << " |";
	os << " " << appName << setfill(' ') << setw(24) << " |";
	os << " " << fileName << endl;
}


// ClassList class
//---------------------------


void ClassList::CreateDefaultEntries()
{
	ClassEntry entry;

	// 1
	entry.classPath = "com.rim.resources";
	entry.className = "net_rim_rimsecuridlibRIMResources";
	push_back(entry);

	// 2
	entry.classPath = "net.rim.device.cldc.impl.softtoken.rimsecuridlib";
	entry.className = "RimSecurIDLib";
	push_back(entry);

	// 3
	entry.classPath = "net.rim.device.cldc.impl.softtoken.rimsecuridlib";
	entry.className = "RimDatabaseFullException";
	push_back(entry);

	// 4
	entry.classPath = "net.rim.device.cldc.impl.softtoken.rimsecuridlib";
	entry.className = "RimDecryptFailException";
	push_back(entry);

	// 5
	entry.classPath = "net.rim.device.cldc.impl.softtoken.rimsecuridlib";
	entry.className = "RimDuplicateNameException";
	push_back(entry);

	// 6
	entry.classPath = "net.rim.device.cldc.impl.softtoken.rimsecuridlib";
	entry.className = "RimDuplicateTokenException";
	push_back(entry);

	// 7
	entry.classPath = "net.rim.device.cldc.impl.softtoken.rimsecuridlib";
	entry.className = "RimInvalidParamException";
	push_back(entry);

	// 8
	entry.classPath = "net.rim.device.cldc.impl.softtoken.rimsecuridlib";
	entry.className = "RimSecurIDLib";
	push_back(entry);

	// 9
	entry.classPath = "net.rim.device.cldc.impl.softtoken.rimsecuridlib";
	entry.className = "RimWrongDeviceIDException";
	push_back(entry);

	// 10
	entry.classPath = "net.rim.device.cldc.impl.softtoken.rimsecuridlib";
	entry.className = "RimWrongFormFactorException";
	push_back(entry);
}


// CodInfo class
//------------------------

bool CodInfo::LoadDebugFile(const char *filename)
{
	uint32_t field;

	if (filename == NULL)
		return false;

	ifstream file(filename);

	// Parse header file
	ParseHeaderSection(file);

	// Parse type area zone
	ParseTypeSection(file);

	// FIXME : ???
	field = ParseInteger(file); // Read 0x0
	field = ParseInteger(file); // Read 0x1

	// FIXME : ???
	field = ParseInteger(file); // Read 0x0
	field = ParseInteger(file); // Read 0x0 or 0xA

	if (field == 0xA) {
		// Parse ressource area zone
		ParseResourceSection(file);
	}

	return true;
}


uint32_t CodInfo::GetUniqueId()
{
	return uniqueId;
}


string CodInfo::GetAppName()
{
	return appName;
}


// Private API - Section parsing
//-------------------------------

void CodInfo::ParseHeaderSection(istream &input)
{
	uint32_t type;

	type = ParseNextHeaderField(input);

	if (type != COD_DEBUG_UNIQUEID_HEADERFIELD)
		return;

	type = ParseNextHeaderField(input);

	if (type != COD_DEBUG_APPNAME_HEADERFIELD)
		return;
}


void CodInfo::ParseTypeSection(istream &input)
{
	uint32_t type;
	uint32_t count;
	uint32_t nbr, check;

	// Read number of declared type content into this section
	nbr = ParseInteger(input);

	// Read each object
	count = 0;

	while (!input.eof()) {
		type = ParseNextTypeField(input);

		if (type == COD_DEBUG_NONE_FIELD)
			break;

		count++;
	}

	// Read again number of declared type content into this section
	// We have to find the same value
	check = ParseInteger(input);

	// Checking...
	dout("Nbr = " << dec << nbr << " / Count = " << dec << count << " / check = " << check);
}


void CodInfo::ParseResourceSection(istream &input)
{
	uint32_t len;
	uint32_t type;
	uint32_t unknown01;
	uint32_t unknown02;
	uint32_t unknown03;
	uint32_t unknown04;
	uint32_t unknown05;
	uint32_t unknown06;
	uint32_t unknown07;

	string name;

	// type = 1
	for (int i=0; i<10; i++) {
		type = ParseInteger(input);

		len = ParseInteger(input);
		name = ParseString(input, len);

		unknown01 = ParseInteger(input);
		unknown02 = ParseInteger(input);
		unknown03 = ParseInteger(input);

		dout("JDGCodInfo::parseRessource"
			<< "\n  Name : " << name
			<< "\n  unknown01 : " << hex << unknown01
			<< "\n  unknown02 : " << hex << unknown02
			<< "\n  unknown03 : " << hex << unknown03);
	}

	// type = 2
	type = ParseInteger(input);

	len = ParseInteger(input);
	name = ParseString(input, len);

	unknown01 = ParseInteger(input);
	unknown02 = ParseInteger(input);
	unknown03 = ParseInteger(input);
	unknown04 = ParseInteger(input);
	unknown05 = ParseInteger(input);
	unknown06 = ParseInteger(input);
	unknown07 = ParseInteger(input);

	dout("JDGCodInfo::parseRessource"
		<< "\n  Name : " << name
		<< "\n  unknown01 : " << hex << unknown01
		<< "\n  unknown02 : " << hex << unknown02
		<< "\n  unknown03 : " << hex << unknown03
		<< "\n  unknown04 : " << hex << unknown04
		<< "\n  unknown05 : " << hex << unknown05
		<< "\n  unknown06 : " << hex << unknown06
		<< "\n  unknown07 : " << hex << unknown07);

	// type = 1
	type = ParseInteger(input);

	len = ParseInteger(input);
	name = ParseString(input, len);

	unknown01 = ParseInteger(input);
	unknown02 = ParseInteger(input);
	unknown03 = ParseInteger(input);
	unknown04 = ParseInteger(input);

	dout("JDGCodInfo::parseRessource"
		<< "\n  Name : " << name
		<< "\n  unknown01 : " << hex << unknown01
		<< "\n  unknown02 : " << hex << unknown02
		<< "\n  unknown03 : " << hex << unknown03
		<< "\n  unknown04 : " << hex << unknown04);

	// type = 0
	type = ParseInteger(input);

	len = ParseInteger(input);
	name = ParseString(input, len);

	unknown01 = ParseInteger(input);
	unknown02 = ParseInteger(input);
	unknown03 = ParseInteger(input);
	unknown04 = ParseInteger(input);
	unknown05 = ParseInteger(input);

	dout("JDGCodInfo::parseRessource"
		<< "\n  Name : " << name
		<< "\n  unknown01 : " << hex << unknown01
		<< "\n  unknown02 : " << hex << unknown02
		<< "\n  unknown03 : " << hex << unknown03
		<< "\n  unknown04 : " << hex << unknown04
		<< "\n  unknown05 : " << hex << unknown05);
}



// Private API - Field parsing
//-------------------------------


uint32_t CodInfo::ParseNextHeaderField(istream &input)
{
	uint32_t type = ParseInteger(input);

	switch (type) {
	case COD_DEBUG_UNIQUEID_HEADERFIELD:
		ParseUniqueId(input);
		break;

	case COD_DEBUG_APPNAME_HEADERFIELD:
		ParseAppName(input);
		break;

	default:
		type = 0xFFFFFFFF;
	}

	return type;
}


uint32_t CodInfo::ParseNextTypeField(istream &input)
{
	uint32_t type = ParseInteger(input);

	switch (type) {
	case COD_DEBUG_NONE_FIELD:
		break;

	case COD_DEBUG_BOOLEAN_FIELD:
		ParseBoolean(input);
		break;

	case COD_DEBUG_BYTE_FIELD:
		ParseByte(input);
		break;

	case COD_DEBUG_CHAR_FIELD:
		ParseChar(input);
		break;

	case COD_DEBUG_SHORT_FIELD:
		ParseShort(input);
		break;

	case COD_DEBUG_INT_FIELD:
		ParseInt(input);
		break;

	case COD_DEBUG_LONG_FIELD:
		ParseLong(input);
		break;

	case COD_DEBUG_CLASS_FIELD:
		ParseClass(input);
		break;

	case COD_DEBUG_ARRAY_FIELD:
		ParseArray(input);
		break;

	case COD_DEBUG_VOID_FIELD:
		ParseVoid(input);
		break;

	case COD_DEBUG_DOUBLE_FIELD:
		ParseDouble(input);
		break;

	default:
		dout("Type unknown ! " << hex << type);
		type = 0xFFFFFFFF;
	}

	return type;
}


void CodInfo::ParseUniqueId(istream &input)
{
	uniqueId = ParseInteger(input);
}


void CodInfo::ParseAppName(istream &input)
{
	uint32_t len = ParseInteger(input);

	appName = ParseString(input, len);
}


void CodInfo::ParseBoolean(istream &input)
{
	uint32_t len  = ParseInteger(input);

	string str = ParseString(input, len);

	dout("JDG::CodInfo::ParseBoolean\n  name : " << str);
}


void CodInfo::ParseByte(istream &input)
{
	uint32_t len  = ParseInteger(input);

	string str = ParseString(input, len);

	dout("JDG::CodInfo::ParseByte\n  name : " << str);
}


void CodInfo::ParseChar(istream &input)
{
	uint32_t len  = ParseInteger(input);

	string str = ParseString(input, len);

	dout("JDG::CodInfo::ParseChar\n  name : " << str);
}


void CodInfo::ParseShort(istream &input)
{
	uint32_t len  = ParseInteger(input);

	string str = ParseString(input, len);

	dout("JDG::CodInfo::ParseShort\n  name : " << str);
}


void CodInfo::ParseInt(istream &input)
{
	uint32_t len  = ParseInteger(input);

	string str = ParseString(input, len);

	dout("JDG::CodInfo::ParseInt\n  name : " << str);
}


void CodInfo::ParseLong(istream &input)
{
	uint32_t len  = ParseInteger(input);

	string str = ParseString(input, len);

	dout("JDG::CodInfo::ParseLong\n  name : " << str);
}


void CodInfo::ParseClass(istream &input)
{
	uint32_t len;

	ClassEntry object;

	dout("JDG::CodInfo::ParseClass");

	len  = ParseInteger(input);

	object.className = ParseString(input, len);

	object.type = ParseInteger(input);
	object.unknown02 = ParseInteger(input);
	object.unknown03 = ParseInteger(input);
	object.id = ParseInteger(input);

	len  = ParseInteger(input);

	if (len == 0)
		object.classPath = "com.barry." + appName;
	else if (len != 0xFFFFFF)
		object.classPath = ParseString(input, len);

	len  = ParseInteger(input);

	object.sourceFile = ParseString(input, len);

	object.unknown05 = ParseInteger(input);
	object.unknown06 = ParseInteger(input);
	object.unknown07 = ParseInteger(input);
	object.unknown08 = ParseInteger(input);

	classList.push_back(object);

	dout("\n  name : " << object.className
		<< "\n  path : " << object.classPath
		<< "\n  type : " << hex << object.type
		<< "\n  unknown02 : " << hex << object.unknown02
		<< "\n  unknown03 : " << hex << object.unknown03
		<< "\n  id : " << hex << object.id
		<< "\n  source file : " << object.sourceFile
		<< "\n  unknown05 : " << hex << object.unknown05
		<< "\n  unknown06 : " << hex << object.unknown06
		<< "\n  unknown07 : " << hex << object.unknown07
		<< "\n  unknown08 : " << hex << object.unknown08);
}


void CodInfo::ParseArray(istream &input)
{
	uint32_t len  = ParseInteger(input);

	string str = ParseString(input, len);

	dout("JDG::CodInfo::ParseArray\n  name : " << str);
}


void CodInfo::ParseVoid(istream &input)
{
	uint32_t len  = ParseInteger(input);

	string str = ParseString(input, len);

	dout("JDG::CodInfo::ParseVoid\n  name : " << str);
}


void CodInfo::ParseDouble(istream &input)
{
	uint32_t len  = ParseInteger(input);

	string str = ParseString(input, len);

	dout("JDG::CodInfo::ParseDouble\n  name : " << str);
}

/*
void CodInfo::ParseType2(istream &input) {
	uint32_t value;
	uint32_t len  = ParseInteger(input);

	string str = ParseString(input, len);

	dout("Type2 : " << str);

	value = ParseInteger(input);
	value = ParseInteger(input);
	value = ParseInteger(input);
	value = ParseInteger(input);
	value = ParseInteger(input);
	value = ParseInteger(input);
}
*/
} // namespace JDG

} // namespace Barry

