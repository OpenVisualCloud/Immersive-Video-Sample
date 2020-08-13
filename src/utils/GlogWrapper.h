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

 * 
 */
//!
//! \file:   GlogWrapper.h
//! \brief:  Internal structure definition for the library.
//! \detail: 
//!
//! Created on June 3, 2019, 9:31 AM
//!

#ifndef GLOGWRAPPER_H
#define GLOGWRAPPER_H

#include "glog/logging.h"
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>

#define VLOG_METHOD 10
#define VLOG_TRACE 20

class GlogWrapper {
 public:
  GlogWrapper(char* name, int32_t minLogLevel = google::INFO) {
    if (0 != access("./logfiles", 0)) {
      mkdir("./logfiles", 0755);
    }
    if (0 != access("./logfiles/INFO", 0)) {
      mkdir("./logfiles/INFO", 0755);
    }
    if (0 != access("./logfiles/WARNING", 0)) {
      mkdir("./logfiles/WARNING", 0755);
    }
    if (0 != access("./logfiles/ERROR", 0)) {
      mkdir("./logfiles/ERROR", 0755);
    }
    if (0 != access("./logfiles/FATAL", 0)) {
      mkdir("./logfiles/FATAL", 0755);
    }
    google::InitGoogleLogging(name);

    google::SetStderrLogging(google::ERROR);                           // output to terminal if log level > google::INFO
    google::SetLogDestination(google::INFO, "./logfiles/INFO/INFO_");  // set google::INFO log file prefix
    google::SetLogDestination(google::WARNING, "./logfiles/WARNING/WARNING_");  // set google::WARNING log file prefix
    google::SetLogDestination(google::ERROR, "./logfiles/ERROR/ERROR_");        // set google::ERROR log file prefix
    google::SetLogDestination(google::FATAL, "./logfiles/FATAL/FATAL_");        // set google::FATAL log file prefix
    google::SetLogFilenameExtension("log_");                                    // set the extension name of the log
    google::InstallFailureSignalHandler();

    FLAGS_colorlogtostderr = true;           // set the color of the log which will be output to terminal
    FLAGS_logbufsecs = 0;                    // buffer the log output. unit is second.
    FLAGS_max_log_size = 100;                // set the max size of log file to 100MB
    FLAGS_stop_logging_if_full_disk = true;  // stop logging if disk full
    FLAGS_minloglevel = minLogLevel;
  };
  ~GlogWrapper() { google::ShutdownGoogleLogging(); };

 private:
};

#endif /* GLOGWRAPPER_H */

