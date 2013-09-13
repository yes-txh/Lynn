// Copyright (c) 2010, Tencent Inc.
// All rights reserved.
//
// Author: CHEN Feng <phongchen@tencent.com>

#ifndef COMMON_SYSTEM_TIME_STOPWATCH_HPP
#define COMMON_SYSTEM_TIME_STOPWATCH_HPP

#include "common/base/stdint.h"

/// ����࣬���ڼ�ʱ
class Stopwatch
{
public:
    /// @param auto_start �Ƿ���ʱ�Զ���ʼ
    explicit Stopwatch(bool auto_start = true);

    /// ��ʼ��ʱ���Ѿ��ۼ�ʱ������ۼ�
    void Start();

    /// ֹͣ��ʱ���ۼ�ʱ�䲻������
    void Stop();

    /// ��ʱ��λΪ 0
    void Restart();

    /// �����ۻ�������
    double ElapsedSeconds() const;

    /// �����ۻ���΢����
    int64_t ElapsedMicroSeconds() const;

    /// �����ۻ��ĺ�����
    int64_t ElapsedMilliSeconds() const;

    /// �Ƿ��ڼ�ʱ
    bool IsRunning() const;

    /// ֹͣ��ʱ����ʱ��λΪ 0��
    void Reset();
private:
    Stopwatch(const Stopwatch&);
    Stopwatch& operator=(const Stopwatch&);
private:
    int64_t LastInterval() const;
private:
    bool m_is_running;
    int64_t m_cumulated_time; ///< ���۵��ۼ�ʱ��
    int64_t m_start_time; ///< �ϴ�������ʱ��
};

#endif // COMMON_SYSTEM_TIME_STOPWATCH_HPP
