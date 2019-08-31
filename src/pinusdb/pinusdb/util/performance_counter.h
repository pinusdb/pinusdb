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
