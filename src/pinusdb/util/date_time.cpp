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

#include "util/date_time.h"
#include "util/string_tool.h"

#include <time.h>
#include <string.h>
#ifdef _WIN32
#include <Windows.h>
#endif

  ///////////////////////////////////////////////////////////////

#define IS_NUMBER(ch) ((ch) >= '0' && (ch) <= '9')
#define NOT_NUMBER(ch) (!IS_NUMBER(ch))
#define NUMBER_VAL(ch) ((ch) - '0')

#define GET_TWO_NUM(pCur, pEnd, Ret) do {    \
  if (pCur >= pEnd)                          \
    return false;                            \
                                             \
  if (IS_NUMBER(*pCur)) {                    \
    Ret = NUMBER_VAL(*pCur);                 \
    pCur++;                                  \
  }                                          \
  else                                       \
    return false;                            \
                                             \
  if (pCur < pEnd && IS_NUMBER(*pCur)) {     \
    Ret = Ret * 10 + NUMBER_VAL(*pCur);      \
    pCur++;                                  \
  }                                          \
} while(false)

  //Nonzero if YEAR is a leap year (every 4 years,
  // except every 100th isn't, and every 400th is).
#define IS_LEAP_YEAR(year) ((year) % 4 == 0 && ((year) % 100 != 0 || (year) % 400 == 0))

const int DaysToMonth365[13] = { 0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334, 365 };
const int DaysToMonth366[13] = { 0, 31, 60, 91, 121, 152, 182, 213, 244, 274, 305, 335, 366 };

int64_t DateTime::sysTimeZone_ = 0;

int32_t DateToDayCode(int year, int month, int day)
{
  if (year >= DateTime::MinYear && year <= DateTime::MaxYear && month >= 1 && month <= 12)
  {
    const int* pDays = IS_LEAP_YEAR(year) ? DaysToMonth366 : DaysToMonth365;
    if (day >= 1 && day <= pDays[month] - pDays[month - 1])
    {
      int32_t y = year - 1;
      int32_t n = y * 365 + y / 4 - y / 100 + y / 400 + pDays[month - 1] + day - 1;
      return (n - DateTime::DaysTo1970);
    }
  }

  return -1;
}

int64_t DateToMicroseconds(int year, int month, int day)
{
  int32_t dayCode = DateToDayCode(year, month, day);
  if (dayCode < 0)
    return dayCode;

  return DateTime::MicrosecondPerDay * dayCode;
}

int64_t TimeToMicroseconds(int hour, int minute, int second)
{
  if (hour >= 0 && hour < 24 && minute >= 0 && minute < 60 && second >= 0 && second < 60)
  {
    return (hour * DateTime::MicrosecondPerHour + minute * DateTime::MicrosecondPerMinute
      + second * DateTime::MicrosecondPerSecond);
  }

  return -1;
}

/////////////////////////////////////////////////////////////////////

DateTime::DateTime()
{
  microseconds_ = 0;
}

DateTime::DateTime(int64_t microseconds)
{
  microseconds_ = microseconds;
}

DateTime::DateTime(int year, int month, int day)
{
  microseconds_ = DateToMicroseconds(year, month, day);
  if (microseconds_ >= 0)
  {
    microseconds_ += sysTimeZone_;
  }
}

DateTime::DateTime(int year, int month, int day, int tzMinute)
{
  microseconds_ = DateToMicroseconds(year, month, day);
  if (microseconds_ >= 0)
  {
    microseconds_ += (tzMinute * DateTime::MicrosecondPerMinute);
  }
}

bool DateTime::Parse(const char* pStr)
{
  return Parse(pStr, strlen(pStr));
}

bool DateTime::Parse(const char* pStr, size_t len)
{
  microseconds_ = -1;
  return Parse(pStr, len, &microseconds_);
}

bool DateTime::Parse(const char* pStr, size_t len, int64_t* pMicroseconds)
{
  if (pStr == nullptr || len <= 0 || pMicroseconds == nullptr)
    return false;

  int year = 0;
  int month = 0;
  int day = 0;

  int hour = 0;
  int minute = 0;
  int second = 0;
  int microsecond = 0;

  const char* pBg = pStr;
  const char* pEnd = (pStr + len);
  int64_t curTimeZone = sysTimeZone_;

  //跳过空格
  while (pBg < pEnd && *pBg == ' ') { pBg++; }

  //获取年
  do{
    if ((pBg + 4) > pEnd)
      return false; //字符串长度不够

    if (NOT_NUMBER(pBg[0]) || NOT_NUMBER(pBg[1])
      || NOT_NUMBER(pBg[2]) || NOT_NUMBER(pBg[3]))
      return false;

    year = NUMBER_VAL(pBg[0]) * 1000 + NUMBER_VAL(pBg[1]) * 100
      + NUMBER_VAL(pBg[2]) * 10 + NUMBER_VAL(pBg[3]);

    pBg += 4;
  } while (false);

  //跳过 - 或 /
  do {
    if (pBg >= pEnd)
      return false;

    if (*pBg != '-' && *pBg != '/')
      return false;

    pBg++;
  } while (false);

  //获取月
  GET_TWO_NUM(pBg, pEnd, month);

  //跳过 - 或 /
  do {
    if (pBg >= pEnd)
      return false;

    if (*pBg != '-' && *pBg != '/')
      return false;

    pBg++;
  } while (false);

  //获取日，有可能一个字符，有可能两个字符
  GET_TWO_NUM(pBg, pEnd, day);

  //跳过空格
  while (pBg < pEnd && *pBg == ' ') { pBg++; }

  if (pBg < pEnd && *pBg != '\0')
  {
    //获取时
    GET_TWO_NUM(pBg, pEnd, hour);

    //跳过 :
    {
      if (pBg >= pEnd)
        return false;

      if (*pBg != ':')
        return false;

      pBg++;
    }

    //获取分
    GET_TWO_NUM(pBg, pEnd, minute);

    //跳过 :
    do {
      if (pBg >= pEnd)
        return false;

      if (*pBg != ':')
        return false;

      pBg++;
    } while (false);

    //获取秒
    GET_TWO_NUM(pBg, pEnd, second);

    //有微秒
    do {
      if (pBg < pEnd && *pBg == '.')
      {
        pBg++;

        if (pBg < pEnd && IS_NUMBER(*pBg))
        {
          microsecond = NUMBER_VAL(*pBg) * 100000;
          pBg++;
        }

        if (pBg < pEnd && IS_NUMBER(*pBg))
        {
          microsecond += NUMBER_VAL(*pBg) * 10000;
          pBg++;
        }

        if (pBg < pEnd && IS_NUMBER(*pBg))
        {
          microsecond += NUMBER_VAL(*pBg) * 1000;
          pBg++;
        }

        if (pBg < pEnd && IS_NUMBER(*pBg))
        {
          microsecond += NUMBER_VAL(*pBg) * 100;
          pBg++;
        }

        if (pBg < pEnd && IS_NUMBER(*pBg))
        {
          microsecond += NUMBER_VAL(*pBg) * 10;
          pBg++;
        }

        if (pBg < pEnd && IS_NUMBER(*pBg))
        {
          microsecond += NUMBER_VAL(*pBg);
          pBg++;
        }
      }
    } while (false);

    //跳过空格
    while (pBg < pEnd && *pBg == ' ') { pBg++; }
  }

  // timezone +08 or +0800 or -08 or -0800
  if (pBg < pEnd && *pBg != '\0')
  {
    int tzHour = 0;
    int tzMinute = 0;
    if (*pBg == '-')
      curTimeZone = 1;
    else if (*pBg == '+')
      curTimeZone = -1;
    else
      return false;

    pBg++;

    //+08 or +0800
    if ((pBg + 1) >= pEnd)
      return false;

    if (!IS_NUMBER(pBg[0]) || !IS_NUMBER(pBg[1]))
      return false;

    tzHour = (NUMBER_VAL(pBg[0]) * 10 + NUMBER_VAL(pBg[1]));

    pBg += 2;
    if ((pBg + 1) < pEnd && pBg[0] != ' ')
    {
      if (!IS_NUMBER(pBg[0]) || !IS_NUMBER(pBg[1]))
        return false;

      tzMinute = (NUMBER_VAL(pBg[0]) * 10 + NUMBER_VAL(pBg[1]));
    }

    pBg += 2;

    //跳过空格
    while (pBg < pEnd && *pBg == ' ') { pBg++; }

    curTimeZone *= ((tzHour * DateTime::MicrosecondPerHour) + tzMinute * DateTime::MicrosecondPerMinute);
  }

  //string is end
  if (pBg < pEnd && *pBg != '\0')
    return false; // not end

  if (microsecond < 0 || microsecond >= DateTime::MicrosecondPerSecond)
    return false;

  int64_t msForDate = DateToMicroseconds(year, month, day);
  int64_t msForTime = TimeToMicroseconds(hour, minute, second);

  if (msForDate < 0 || msForTime < 0)
    return false;

  *pMicroseconds = msForDate + msForTime + microsecond + curTimeZone;
  return true;
}

bool DateTime::ParseDate(const char* pStr, size_t len, int32_t* pDayCode)
{
  if (pStr == nullptr || len <= 0 || pDayCode == nullptr)
    return false;

  int year = 0;
  int month = 0;
  int day = 0;

  const char* pBg = pStr;
  const char* pEnd = (pStr + len);

  //跳过空格
  while (pBg < pEnd && *pBg == ' ') { pBg++; }

  //获取年
  do {
    if ((pBg + 4) > pEnd)
      return false; //字符串长度不够

    if (NOT_NUMBER(pBg[0]) || NOT_NUMBER(pBg[1])
      || NOT_NUMBER(pBg[2]) || NOT_NUMBER(pBg[3]))
      return false;

    year = NUMBER_VAL(pBg[0]) * 1000 + NUMBER_VAL(pBg[1]) * 100
      + NUMBER_VAL(pBg[2]) * 10 + NUMBER_VAL(pBg[3]);

    pBg += 4;
  } while (false);

  //跳过 - 或 /
  do {
    if (pBg >= pEnd)
      return false;

    if (*pBg != '-' && *pBg != '/')
      return false;

    pBg++;
  } while (false);

  //获取月
  GET_TWO_NUM(pBg, pEnd, month);

  //跳过 - 或 /
  do {
    if (pBg >= pEnd)
      return false;

    if (*pBg != '-' && *pBg != '/')
      return false;

    pBg++;
  } while (false);

  //获取日，有可能一个字符，有可能两个字符
  GET_TWO_NUM(pBg, pEnd, day);

  //跳过空格
  while (pBg < pEnd && *pBg == ' ') { pBg++; }

  if (pBg != pEnd)
    return false;

  *pDayCode = DateToDayCode(year, month, day);
  return true;

}

DateTime DateTime::AddDays(int days) const
{
  return DateTime(microseconds_ + days * DateTime::MicrosecondPerDay);
}

int DateTime::GetDayForWeek() const
{
  int n = static_cast<int>(microseconds_ / DateTime::MicrosecondPerDay);

  return (n + 1) % 7;
}

int64_t DateTime::NowMicrosecond()
{
  time_t rawtime;
  time(&rawtime);

  return (rawtime * DateTime::MicrosecondPerSecond);
}

int32_t DateTime::NowDayCode()
{
  time_t rawtime;
  time(&rawtime);
  return static_cast<int32_t>(rawtime / (3600 * 24));
}

uint64_t DateTime::NowTickCount()
{
#ifdef _WIN32
  return GetTickCount64();
#else
  struct timespec ts;
  clock_gettime(CLOCK_MONOTONIC, &ts);
  return (((int64_t)(ts.tv_sec) * 1000) + (ts.tv_nsec / 1000000));
#endif
}

bool DateTime::GetMicrosecondByTimeUnit(const char* pUnit, size_t unitLen, int64_t* pMicrosecond)
{
  if (pUnit == nullptr || unitLen == 0)
    return false;

  int64_t microOffset = 0;
  if (StringTool::ComparyNoCase(pUnit, unitLen, "s", (sizeof("s") - 1)),
    StringTool::ComparyNoCase(pUnit, unitLen, "second", (sizeof("second") - 1)))
  {
    microOffset = DateTime::MicrosecondPerSecond;
  }
  else if (StringTool::ComparyNoCase(pUnit, unitLen, "m", (sizeof("m") - 1)),
    StringTool::ComparyNoCase(pUnit, unitLen, "minute", (sizeof("minute") - 1)))
  {
    microOffset = DateTime::MicrosecondPerMinute;
  }
  else if (StringTool::ComparyNoCase(pUnit, unitLen, "h", (sizeof("h") - 1)),
    StringTool::ComparyNoCase(pUnit, unitLen, "hour", (sizeof("hour") - 1)))
  {
    microOffset = DateTime::MicrosecondPerHour;
  }
  else if (StringTool::ComparyNoCase(pUnit, unitLen, "d", (sizeof("d") - 1)),
    StringTool::ComparyNoCase(pUnit, unitLen, "day", (sizeof("day") - 1)))
  {
    microOffset = DateTime::MicrosecondPerDay;
  }
  else
  {
    return false;
  }

  if (pMicrosecond != nullptr)
    *pMicrosecond = microOffset;

  return true;
}

void DateTime::InitTimeZone()
{
  time_t t1, t2;
  struct tm *tm_utc;
  if (time(&t1) < 0)
    return;

  t2 = t1;
  tm_utc = gmtime(&t2);
  t2 = mktime(tm_utc);

  sysTimeZone_ = -1 * ((t1 - t2) * DateTime::MicrosecondPerSecond);
}

void DateTime::GetDatePart(int* pYear, int* pMonth, int* pDay) const
{
  if (microseconds_ >= 0)
  {
    int n = static_cast<int>(microseconds_ / DateTime::MicrosecondPerDay) + DaysTo1970;
    int y400 = n / DaysPer400Years;
    n -= y400 * DaysPer400Years;
    int y100 = n / DaysPer100Years;
    if (y100 == 4) y100 = 3;
    n -= y100 * DaysPer100Years;
    int y4 = n / DaysPer4Years;
    n -= y4 * DaysPer4Years;
    int y1 = n / DaysPerYear;
    if (y1 == 4) y1 = 3;

    if (pYear != nullptr)
      *pYear = y400 * 400 + y100 * 100 + y4 * 4 + y1 + 1;

    n -= y1 * DaysPerYear;
    bool leapYear = y1 == 3 && (y4 != 24 || y100 == 3);
    const int* pDays = leapYear ? DaysToMonth366 : DaysToMonth365;
    int m = (n >> 5) + 1;
    while (n >= pDays[m]) m++;
    
    if (pMonth != nullptr)
      *pMonth = m;

    if (pDay != nullptr)
      *pDay = n - pDays[m - 1] + 1;
  }
}

std::string DateTime::GetDateStr() const
{
  char buf[32];
  int year = 0;
  int month = 0;
  int day = 0;

  GetDatePart(&year, &month, &day);
  sprintf(buf, "%04d-%02d-%02d", year, month, day);
  return buf;
}
