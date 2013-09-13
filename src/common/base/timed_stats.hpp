// Copyright 2011, Tencent Inc.
// Author: Xiaokang Liu (hsiaokangliu@tencent.com)

#ifndef COMMON_BASE_TIMED_STATS_HPP
#define COMMON_BASE_TIMED_STATS_HPP

#include <iostream>
#include <vector>
#include "common/base/stdint.h"
#include "common/system/concurrency/mutex.hpp"
#include "common/system/time/timestamp.hpp"

template <typename T>
class BaseTimedBuckets
{
public:
    BaseTimedBuckets() {}
    virtual ~BaseTimedBuckets() {}

    virtual void OnTick(int tick_count, int count, T min, T max, T sum) = 0;
    // 上层的watcher，当前数据发生变化时，watcher进行相应的处理
    void AddWatcher(BaseTimedBuckets<T>* watcher)
    {
        m_watchers.push_back(watcher);
    }
protected:
    std::vector<BaseTimedBuckets<T>*> m_watchers;
};

template <typename T, int NumUnits>
class TimedBuckets : public BaseTimedBuckets<T>
{
public:
    TimedBuckets() : m_last_tick_count(-1)
    {
        for (int i = 0; i < NumUnits; i++)
        {
            m_count[i] = 0;
            m_min[i] = 0;
            m_max[i] = 0;
            m_sum[i] = 0;
        }
    }
    virtual ~TimedBuckets() {}

    void OnTick(int tick_count, int event_count, T min, T max, T sum)
    {
        NotifyWatchers(tick_count / NumUnits, event_count, min, max, sum);
        int offset = tick_count % NumUnits;
        if (m_last_tick_count != tick_count)
        {
            int diff = tick_count - m_last_tick_count;
            if (diff >= NumUnits)
            {
                for (int i = 0; i < NumUnits; i++)
                {
                    m_count[i] = 0;
                    m_min[i] = 0;
                    m_max[i] = 0;
                    m_sum[i] = 0;
                }
            }
            else
            {
                while (++m_last_tick_count != tick_count)
                {
                    int pos = m_last_tick_count % NumUnits;
                    m_count[pos] = 0;
                    m_min[pos] = 0;
                    m_max[pos] = 0;
                    m_sum[pos] = 0;
                }
            }
            m_last_tick_count = tick_count;
            m_count[offset] = event_count;
            m_min[offset] = min;
            m_max[offset] = max;
            m_sum[offset] = sum;
        }
        else
        {
            m_count[offset] += event_count;
            if (m_min[offset] > min)
                m_min[offset] = min;
            if (m_max[offset] < max)
                m_max[offset] = max;
            m_sum[offset] += sum;
        }
    }

    void Print(std::ostream& out)
    {
        for (int i = 0; i < NumUnits; i++)
        {
            out << i << ":\t"
                << "count: " << m_count[i] << "\t"
                << "min: " << m_min[i] << "\t"
                << "max: " << m_max[i] << "\t"
                << "sum: " << m_sum[i] << "\n";
        }
    }

private:
    void NotifyWatchers(int tick_count, int count, T min, T max, T sum)
    {
        size_t length = this->m_watchers.size();
        for (size_t i = 0; i < length; i++)
        {
            this->m_watchers[i]->OnTick(tick_count, count, min, max, sum);
        }
    }
    // NumUnits个Buckets，每个bucket内的事件数
    int m_count[NumUnits];
    // 每个bucket最小事件的值
    T m_min[NumUnits];
    // 每个bucket最大事件的值
    T m_max[NumUnits];
    // 每个bucket所有事件值的总和
    T m_sum[NumUnits];
    // 上一次触发步数
    int m_last_tick_count;
};

template <typename T>
class TimedStats
{
public:
    TimedStats()
    {
        m_start_time = -1;
        m_current_time = -1;
        m_hours_buckets.AddWatcher(&m_days_buckets);
        m_mins_buckets.AddWatcher(&m_hours_buckets);
        m_secs_buckets.AddWatcher(&m_mins_buckets);
        m_count = 0;
        m_min = 0;
        m_max = 0;
        m_sum = 0;
    }
    virtual ~TimedStats() {}

    void Print(std::ostream& out)
    {
        MutexLocker locker(&m_mutex);
        out << "++++++++++seconds:+++++++\n";
        m_secs_buckets.Print(out);
        out << "++++++++++minutes:+++++++\n";
        m_mins_buckets.Print(out);
        out << "++++++++++hours:+++++++++\n";
        m_hours_buckets.Print(out);
        out << "++++++++++days:++++++++++\n";
        m_days_buckets.Print(out);
    }

    void AddCount(int64_t time = -1)
    {
        int64_t timestamp = (time != -1) ? time : GetTimeStampInMs() / 1000;
        MutexLocker locker(&m_mutex);
        if (m_start_time < 0) // first count
        {
            m_start_time = timestamp;
            m_current_time = timestamp;
            m_count = 1;
        }
        else if (timestamp > m_current_time)
        {
            int elapse_time = static_cast<int>(m_current_time - m_start_time);
            m_secs_buckets.OnTick(elapse_time, m_count, 0, 0, 0);
            m_count = 1;
            m_current_time = timestamp;
        }
        else
        {
            m_count++;
        }
    }

    void AddValue(T value, int64_t time = -1)
    {
        int64_t timestamp = (time != -1) ? time : GetTimeStampInMs() / 1000;
        MutexLocker locker(&m_mutex);
        if (m_start_time < 0) // first value
        {
            m_start_time = timestamp;
            m_current_time = timestamp;
            m_count = 1;
            m_min = value;
            m_max = value;
            m_sum = value;
        }
        else if (timestamp > m_current_time)
        {
            int elapse_time = static_cast<int>(m_current_time - m_start_time);
            m_secs_buckets.OnTick(elapse_time, m_count, m_min, m_max, m_sum);
            m_count = 1;
            m_min = value;
            m_max = value;
            m_sum = value;
            m_current_time = timestamp;
        }
        else
        {
            m_count++;
            if (m_min > value)
                m_min = value;
            if (m_max < value)
                m_max = value;
            m_sum += value;
        }
    }

protected:
    SimpleMutex m_mutex;
    TimedBuckets<T, 7>  m_days_buckets;
    TimedBuckets<T, 24> m_hours_buckets;
    TimedBuckets<T, 60> m_mins_buckets;
    TimedBuckets<T, 60> m_secs_buckets;
    T m_count;
    T m_min;
    T m_max;
    T m_sum;
    int64_t m_start_time;
    int64_t m_current_time;
};

#endif // COMMON_BASE_TIMED_STATS_HPP
