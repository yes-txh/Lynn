// Copyright (c) 2010, Tencent Inc.
// All rights reserved.
//
// Author: CHEN Feng <phongchen@tencent.com>

#ifndef COMMON_SYSTEM_TIME_STOPWATCH_HPP
#define COMMON_SYSTEM_TIME_STOPWATCH_HPP

#include "common/base/stdint.h"

/// 秒表类，用于计时
class Stopwatch
{
public:
    /// @param auto_start 是否构造时自动开始
    explicit Stopwatch(bool auto_start = true);

    /// 开始计时，已经累计时间继续累加
    void Start();

    /// 停止计时，累计时间不再增长
    void Stop();

    /// 计时复位为 0
    void Restart();

    /// 返回累积的秒数
    double ElapsedSeconds() const;

    /// 返回累积的微秒数
    int64_t ElapsedMicroSeconds() const;

    /// 返回累积的毫秒数
    int64_t ElapsedMilliSeconds() const;

    /// 是否在计时
    bool IsRunning() const;

    /// 停止计时，计时复位为 0。
    void Reset();
private:
    Stopwatch(const Stopwatch&);
    Stopwatch& operator=(const Stopwatch&);
private:
    int64_t LastInterval() const;
private:
    bool m_is_running;
    int64_t m_cumulated_time; ///< 积累的累计时间
    int64_t m_start_time; ///< 上次启动的时间
};

#endif // COMMON_SYSTEM_TIME_STOPWATCH_HPP
