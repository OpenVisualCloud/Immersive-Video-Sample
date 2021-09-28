/*
 * Copyright (c) 2019, Intel Corporation
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * * Redistributions of source code must retain the above copyright notice, this
 *   list of conditions and the following disclaimer.
 * * Redistributions in binary form must reproduce the above copyright notice,
 *   this list of conditions and the following disclaimer in the documentation
 *   and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.

 */

//!

//! \file:   DataLog.h
//! \brief:  the class for Data log
//! \detail: it's class to describe data log
//!
//! Created on May 21, 2020, 1:18 PM
//!

#ifndef _DATALOG_H_
#define _DATALOG_H_

#include "ns_def.h"
#include <stdint.h>
#include "Singleton.h"
#include <fstream>

VCD_NS_BEGIN

class DataLog {
public:
    //!
    //! \brief  construct
    //!
    DataLog(){
        //switch log
        switch_start_time_ = 0;
        switch_end_time_ = 0;
        total_switch_times_ = 0;
        avg_switch_time_ = 0;
        max_switch_time_ = 0;
        min_switch_time_ = __LONG_MAX__;
        log_file_.open("perf.log");
        log_file_ << "-=-=-=-=-=-=-M2HQ performance-=-=-=-=-=-=-" << endl;
        // e2e latency log
        avg_e2elatency_time_ = 0;
        max_e2elatency_time_ = 0;
        min_e2elatency_time_ = __LONG_MAX__;
        total_record_latency_times_ = 0;
    };
    //!
    //! \brief  de-construct
    //!
    ~DataLog() {
        log_file_.close();
    }
    //!
    //! \brief  set switch start time
    //!
    void SetSwitchStartTime(uint64_t start_time) {
        switch_start_time_ = start_time;
    }
    //!
    //! \brief  set switch end time
    //!
    void SetSwitchEndTime(uint64_t end_time) {
        switch_end_time_ = end_time;
        uint64_t single_switch_time = switch_end_time_ - switch_start_time_;
        PrintSingleSwitchTimeInLog();
        PrintSingleSwitchTimeInFile();
        avg_switch_time_ = (avg_switch_time_ * total_switch_times_ + single_switch_time) / (total_switch_times_ + 1);
        total_switch_times_++;
        if (single_switch_time > max_switch_time_) max_switch_time_ = single_switch_time;
        if (single_switch_time < min_switch_time_) min_switch_time_ = single_switch_time;
    }
    //!
    //! \brief  get total switch times
    //!
    uint32_t GetTotalSwitchTimes() { return total_switch_times_; }
    //!
    //! \brief  get average switch time
    //!
    uint64_t GetAVGSwitchTime() { return avg_switch_time_; }
    //!
    //! \brief  get max switch time
    //!
    uint64_t GetMaxSwitchTime() { return max_switch_time_; }
    //!
    //! \brief  get min switch time
    //!
    uint64_t GetMinSwitchTime() { return min_switch_time_; }
    //!
    //! \brief  print single switch time in log
    //!
    void PrintSingleSwitchTimeInLog() {
        static uint32_t cnt_log = 1;
        LOG(INFO) << "[" << cnt_log++ << "] single switch time : " << switch_end_time_ - switch_start_time_ << " ms " << endl;
    }
    //!
    //! \brief  print single switch time in file
    //!
    void PrintSingleSwitchTimeInFile() {
        static uint32_t cnt_file = 1;
        log_file_ << "[" << cnt_file++ << "] single switch time : " << switch_end_time_ - switch_start_time_ << " ms " << endl;
    }
    //!
    //! \brief  print switch performance in log
    //!
    void PrintSwitchPerformanceInLog() {
        LOG(INFO) << "--------------------------------------------" << endl
                  << "switch total times : " << total_switch_times_ << endl
                  << "average switch latency : " << avg_switch_time_ << " ms" << endl
                  << "max switch latency : " << max_switch_time_ << " ms" << endl
                  << "min switch latency : " << min_switch_time_ << " ms" << endl
                  << "--------------------------------------------" << endl;
    }
    //!
    //! \brief  print switch performance in file
    //!
    void PrintSwitchPerformanceInFile() {
        log_file_ << "--------------------------------------------" << endl
                  << "switch total times : " << total_switch_times_ << endl
                  << "average switch latency : " << avg_switch_time_ << " ms" << endl
                  << "max switch latency : " << max_switch_time_ << " ms" << endl
                  << "min switch latency : " << min_switch_time_ << " ms" << endl
                  << "--------------------------------------------" << endl;
    }
    //!
    //! \brief  set single e2e latency and update some characteristics
    //!
    void SetSingleE2ELatency(uint64_t time) {
        if (avg_e2elatency_time_ * total_record_latency_times_ > UINT64_MAX) {
            LOG(WARNING) << "Too large for the latency profiling! Re-collect the latency data!" << endl;
            total_record_latency_times_ = 0;
            max_e2elatency_time_ = 0;
            min_e2elatency_time_ = __LONG_MAX__;
            avg_e2elatency_time_ = 0;
        }
        if (time > max_e2elatency_time_) max_e2elatency_time_ = time;
        if (time < min_e2elatency_time_) min_e2elatency_time_ = time;
        avg_e2elatency_time_ = (avg_e2elatency_time_ * total_record_latency_times_ + time) / (total_record_latency_times_ + 1);
        total_record_latency_times_++;
    }
    //!
    //! \brief  print latency performance in log
    //!
    void PrintE2ELatencyPerformanceInLog() {
        LOG(INFO) << "-=-=-=-=-=-=-E2E latency performance-=-=-=-=-=-=-" << endl
                  << "--------------------------------------------" << endl
                  << "average e2e latency : " << avg_e2elatency_time_ << " ms" << endl
                  << "max e2e latency : " << max_e2elatency_time_ << " ms" << endl
                  << "min e2e latency : " << min_e2elatency_time_ << " ms" << endl
                  << "--------------------------------------------" << endl;
    }
    //!
    //! \brief  print latency performance in file
    //!
    void PrintE2ELatencyPerformanceInFile() {
        log_file_ << "-=-=-=-=-=-=-E2E latency performance-=-=-=-=-=-=-" << endl
                  << "--------------------------------------------" << endl
                  << "average switch latency : " << avg_e2elatency_time_ << " ms" << endl
                  << "max switch latency : " << max_e2elatency_time_ << " ms" << endl
                  << "min switch latency : " << min_e2elatency_time_ << " ms" << endl
                  << "--------------------------------------------" << endl;
    }

private:
    DataLog& operator=(const DataLog& other) { return *this; };
    DataLog(const DataLog& other) { /* do not create copies */ };

private:
    uint64_t switch_start_time_; //<! switch start time
    uint64_t switch_end_time_; //<! switch end time

    uint32_t total_switch_times_; //<! total switch times
    uint64_t avg_switch_time_; //<! average switch time

    uint64_t max_switch_time_; //<! max switch time
    uint64_t min_switch_time_; //<! min switch time

    uint64_t avg_e2elatency_time_;
    uint64_t max_e2elatency_time_;
    uint64_t min_e2elatency_time_;
    uint64_t total_record_latency_times_;

    ofstream log_file_; //<! log file

};

typedef VCD::VRVideo::Singleton<DataLog> DATALOG;    //<! singleton of DataLog

VCD_NS_END;
#endif /* _DATALOG_H_ */