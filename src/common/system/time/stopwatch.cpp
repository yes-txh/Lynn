// Copyright (c) 2010, Tencent Inc.
// All rights reserved.
//
// Author: CHEN Feng <phongchen@tencent.com>

#include "common/system/time/stopwatch.hpp"
#include "common/system/time/timestamp.hpp"

Stopwatch::Stopwatch(bool auto_start):
    m_is_running(auto_start),
    m_cumulated_time(0),
    m_start_time(0)
{
    if (auto_start)
        m_start_time = GetTimeStampInUs();
}

void Stopwatch::Start()
{
    if (!m_is_running)
    {
        m_is_running = true;
        m_start_time = GetTimeStampInUs();
    }
}

void Stopwatch::Stop()
{
    if (m_is_running)
    {
        m_is_running = false;
        int64_t now = GetTimeStampInUs();
        m_cumulated_time += now - m_start_time;
    }
}

void Stopwatch::Restart()
{
    m_cumulated_time = 0;
    m_start_time = GetTimeStampInUs();
    m_is_running = true;
}

void Stopwatch::Reset()
{
    m_cumulated_time = 0;
    m_start_time = 0;
    m_is_running = false;
}

bool Stopwatch::IsRunning() const
{
    return m_is_running;
}

int64_t Stopwatch::LastInterval() const
{
    return GetTimeStampInUs() - m_start_time;
}

int64_t Stopwatch::ElapsedMicroSeconds() const
{
    if (IsRunning())
        return m_cumulated_time + LastInterval();
    else
        return m_cumulated_time;
}

int64_t Stopwatch::ElapsedMilliSeconds() const
{
    return ElapsedMicroSeconds() / 1000;
}

double Stopwatch::ElapsedSeconds() const
{
    return ElapsedMicroSeconds() / 1000000.0;
}

