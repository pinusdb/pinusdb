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

class DateTime
{
public:
  const static int64_t MicrosecondPerMillisecond = 1000l;
  const static int64_t MicrosecondPerSecond = MicrosecondPerMillisecond * 1000l;
  const static int64_t MicrosecondPerMinute = MicrosecondPerSecond * 60l;
  const static int64_t MicrosecondPerHour = MicrosecondPerMinute * 60l;
  const static int64_t MicrosecondPerDay = MicrosecondPerHour * 24l;

  const static int32_t DaysPerYear = 365;
  const static int32_t DaysPer4Years = DaysPerYear * 4 + 1;       // 1461
  const static int32_t DaysPer100Years = DaysPer4Years * 25 - 1;  // 3556524
  const static int32_t DaysPer400Years = DaysPer100Years * 4 + 1; // 146097

  const static int32_t DaysTo1601 = DaysPer400Years * 4; // 584388
  const static int32_t DaysTo1970 = DaysPer400Years * 4 + DaysPer100Years * 3 + DaysPer4Years * 17 + DaysPerYear; // 719,162 
  const static int32_t DaysTo3000 = DaysPer400Years * 7 + DaysPer100Years * 2 - 366;

  const static int64_t MinDay = 0;
  const static int64_t MaxDay = (DaysTo3000 - DaysTo1970);

  const static int64_t MinMicrosecond = 0;
  const static int64_t MaxMicrosecond = MaxDay * MicrosecondPerDay - 1;

  const static int32_t MinYear = 1970;
  const static int32_t MaxYear = 2999;

public:
  DateTime();
  DateTime(int64_t microseconds);

  DateTime(int year, int month, int day);
  DateTime(int year, int month, int day, int tzMinute);

  bool Parse(const char* pStr);
  bool Parse(const char* pStr, size_t len);

  bool IsValid() const { return microseconds_ >= 0; }

  static bool Parse(const char* pStr, size_t len, int64_t* pMicroseconds);
  static bool ParseDate(const char* pStr, size_t len, int32_t* pDayCode);
  static int64_t GetSysTimeZone() { return sysTimeZone_; }

  int64_t TotalMicrosecond() const { return microseconds_; }

  DateTime AddDays(int days) const;
  int GetDayForWeek() const;

  static int64_t NowMicrosecond();
  static int32_t NowDayCode();
  static uint64_t NowTickCount();

  static bool GetMicrosecondByTimeUnit(const char* pUnit, size_t unitLen, int64_t* pMicrosecond);

  static void InitTimeZone();

  void GetDatePart(int* pYear, int* pMonth, int* pDay) const;
  std::string GetDateStr() const;

private:
  static int64_t sysTimeZone_;
  int64_t microseconds_;
};

