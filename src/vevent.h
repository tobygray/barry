///
/// \file	vevent.h
///		Conversion routines for vevents (VCALENDAR, etc)
///

/*
    Copyright (C) 2006-2012, Net Direct Inc. (http://www.netdirect.ca/)

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

#ifndef __BARRY_SYNC_VEVENT_H__
#define __BARRY_SYNC_VEVENT_H__

#include "dll.h"
#include "vbase.h"
#include "vformat.h"
#include "r_calendar.h"
#include <stdint.h>
#include <string>
#include <map>

namespace Barry { namespace Sync {

//
// vCalendar
//
/// Class for converting between RFC 2445 iCalendar data format,
/// and the Barry::Calendar class.
///
class BXEXPORT vCalendar : public vBase
{
	typedef std::map<std::string, std::string>	ArgMapType;

	// external reference
	vTimeConverter &m_vtc;

	// data to pass to external requests
	char *m_gCalData;	// dynamic memory returned by vformat()... can
				// be used directly by the plugin, without
				// overmuch allocation and freeing (see Extract())
	std::string m_vCalData;	// copy of m_gCalData, for C++ use
	Barry::Calendar m_BarryCal;

	static const char *WeekDays[7];

	void CheckUnsupportedArg(const ArgMapType &args,
		const std::string &name);
	std::vector<std::string> SplitBYDAY(const std::string &ByDay);
	uint16_t GetMonthWeekNumFromBYDAY(const std::string& ByDay);
	uint16_t GetWeekDayIndexFromBYDAY(const std::string& ByDay);
	uint16_t GetDayOfMonthFromBYMONTHDAY(const ArgMapType &args,
					int month_override = -1);

protected:
	void RecurToVCal();
	void RecurToBarryCal(vAttr& rrule, time_t starttime);

	static uint16_t GetWeekDayIndex(const char *dayname);
	bool HasMultipleVEvents() const;

public:
	explicit vCalendar(vTimeConverter &vtc);
	~vCalendar();

	const std::string&	ToVCal(const Barry::Calendar &cal);
	const Barry::Calendar&	ToBarry(const char *vcal, uint32_t RecordId);

	const std::string&	GetVCal() const { return m_vCalData; }
	const Barry::Calendar&	GetBarryCal() const { return m_BarryCal; }

	char* ExtractVCal();

	void Clear();

	// This is the v-name of the innermost BEGIN/END block
	static const char* GetVName() { return "VEVENT"; }
};

}} // namespace Barry::Sync

#endif

