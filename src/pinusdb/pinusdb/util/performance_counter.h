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

#ifndef __PINUSDB_PERFORMANCE_COUNTER_H__
#define __PINUSDB_PERFORMANCE_COUNTER_H__

#define BEGIN_QUERY_PERFORMANCE  \
LARGE_INTEGER __performanceStartTime_; \
BOOL __performanceRet_ = QueryPerformanceCounter(&__performanceStartTime_)

#define END_QUERY_PERFORMANCE(thresholdTime, msg)  do { \
  LARGE_INTEGER __performanceEndTime_;  \
  LARGE_INTEGER __frequency_;  \
  if (__performanceRet_ == FALSE)  \
    break;  \
  if (QueryPerformanceCounter(&__performanceEndTime_) == FALSE)  \
    break;  \
  if (QueryPerformanceFrequency(&__frequency_) == FALSE)  \
    break;  \
  double __elapsed_ = ((__performanceEndTime_.QuadPart - __performanceStartTime_.QuadPart) /  (double)__frequency_.QuadPart) * 1000; \
  if (__elapsed_ > thresholdTime)  \
    LOG_DEBUG(msg, __elapsed_);\
} while(false)


#endif
