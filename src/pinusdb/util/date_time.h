/*
* Copyright (c) 2019 ChangSha JuSong Soft Inc. <service@pinusdb.cn>.
*
* This program is free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation; version 3 of the License.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.

* You should have received a copy of the GNU General Public License
* along with this program; If not, see <http://www.gnu.org/licenses>
*/

#pragma once

#include <stdint.h>
#include <string>

const int64_t MillisPerSecond = 1000;
const int64_t MillisPerMinute = MillisPerSecond * 60;
const int64_t MillisPerHour = MillisPerMinute * 60;
const int64_t MillisPerDay = MillisPerHour * 24;

// Number of days in a non-leap year
const int32_t DaysPerYear = 365;
const int32_t DaysPer4Years = DaysPerYear * 4 + 1;       // 1461
const int32_t DaysPer100Years = DaysPer4Years * 25 - 1;  // 36524
const int32_t DaysPer400Years = DaysPer100Years * 4 + 1; // 146097

const int32_t DaysTo1601 = DaysPer400Years * 4;  // 584388
const int32_t DaysTo1970 = DaysPer400Years * 4 + DaysPer100Years * 3 + DaysPer4Years * 17 + DaysPerYear; // 719,162

const int32_t DaysTo3000 = DaysPer400Years * 7 + DaysPer100Years * 2 - 366;

const int32_t MinDay = 0;
const int32_t MaxDay = (DaysTo3000 - DaysTo1970);

const int64_t MinMillis = 0;
const int64_t MaxMillis = MaxDay * MillisPerDay - 1;

const int32_t MinYear = 1970;
const int32_t MaxYear = 2999;

class DateTime
{
public:
  DateTime();
  DateTime(int64_t totalMillseconds);

  DateTime(int year, int month, int day);
  DateTime(int year, int month, int day, int tzMinute);

  bool Parse(const char* pStr);
  bool Parse(const char* pStr, size_t len);

  bool IsValid() const { return totalMilliseconds_ >= 0; }

  static bool Parse(const char* pStr, size_t len, int64_t* pMillseconds);
  static bool ParseDate(const char* pStr, size_t len, int32_t* pDayCode);
  static int64_t GetSysTimeZone() { return sysTimeZone_; }

  int64_t TotalMilliseconds() const;

  DateTime AddDays(int days) const;
  int GetDayForWeek() const;

  static int64_t NowMilliseconds();
  static int32_t NowDayCode();
  static uint64_t NowTickCount();

  static void InitTimeZone();

  void GetDatePart(int* pYear, int* pMonth, int* pDay) const;
  std::string GetDateStr() const;

private:
  static int64_t sysTimeZone_;
  int64_t totalMilliseconds_;
};

