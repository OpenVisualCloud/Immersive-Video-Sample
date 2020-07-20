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
//! \file:   Threadable.h
//! \brief:
//! \detail:
//!
//!
//!
//!
//! Created on May 29, 2019, 9:53 AM
//!

#ifndef THREADABLE_H
#define THREADABLE_H

#include "ns_def.h"

extern "C" {
#include <pthread.h>
}
#include <string>
#include "errno.h"
#include <cstring>
#include <cstdlib>

using namespace std;

VCD_NS_BEGIN

class ThreadLock {
 public:
  ThreadLock() { m_mutex = PTHREAD_MUTEX_INITIALIZER; };
  ~ThreadLock() { pthread_mutex_destroy(&m_mutex); };

  void lock() { pthread_mutex_lock(&m_mutex); }

  void unlock() { pthread_mutex_unlock(&m_mutex); }

 private:
  pthread_mutex_t m_mutex;
};

class ScopeLock {
 public:
  ScopeLock(ThreadLock& lock) : mLock(lock) { mLock.lock(); }

  ~ScopeLock() { mLock.unlock(); }

 private:
  ThreadLock& mLock;
};

//!
//!   Signals a problem with the thread handling.
//!

class CThreadException : public std::exception {
 public:
  //!
  //! \brief Construct a SocketException with a explanatory message.
  //! \param message explanatory message
  //! \param bSysMsg true if system message (from strerror(errno))
  //!   should be postfixed to the user provided message
  //!
  CThreadException(const string& message, bool bSysMsg = false) throw() {
    if (bSysMsg) {
      m_sMsg.append(": ");
      m_sMsg.append(strerror(errno));
    }
  };

  //!
  //! Destructor.
  //! Virtual to allow for subclassing.
  //!
  virtual ~CThreadException() throw(){};

  //!
  //! \brief Returns a pointer to the (constant) error description.
  //! \return A pointer to a \c const \c char*. The underlying memory
  //!          is in posession of the \c Exception object. Callers \a must
  //!          not attempt to free the memory.
  //!
  virtual const char* what() const throw() { return m_sMsg.c_str(); }

 protected:
  std::string m_sMsg;  //<! Error message.
};

//!
//!  Abstract class for Thread management
//!
class Threadable {
 public:
  //!
  //! \brief  Default Constructor for thread
  //!
  Threadable(){};

  //!
  //! \brief  virtual destructor
  //!
  virtual ~Threadable(){};

  //!
  //! \brief  Thread functionality Pure virtual function  , it will be re implemented in derived classes
  //!
  virtual void Run() = 0;

  //!
  //! \brief  Function to start thread.
  //! \param  [in] detached
  //!         if user need the thread detached at the creation or not
  //!
  void StartThread(bool detached = false)  // throw(CThreadException)
  {
    CreateThread(detached);
  };

  //!
  //! \brief  Function to join thread.
  //!
  void Join()  // throw(CThreadException)
  {
    if (!m_Tid) return;

    int rc = pthread_join(m_Tid, NULL);

    // won't throw exception if joined successfully or thread already exited
    if (rc != 0 && rc != EINVAL) {
      throw CThreadException("Error in thread join.... (pthread_join())", true);
    }
  };

 private:
  //!
  //! \brief  private Function to create thread.
  //!
  void CreateThread(bool detached)  // throw(CThreadException)
  {
    int rc = 0;
    if (detached) {
      pthread_attr_t attr;
      pthread_attr_init(&attr);
      pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
      rc = pthread_create(&m_Tid, &attr, ThreadFunc, this);
    } else {
      rc = pthread_create(&m_Tid, NULL, ThreadFunc, this);
    }

    if (rc != 0) {
      throw CThreadException("Error in thread creation... (pthread_create())", true);
    }
  }

  //!
  //! \brief Call back Function Passing to pthread create API
  //!
  static void* ThreadFunc(void* pTr) {
    Threadable* pThis = static_cast<Threadable*>(pTr);
    pThis->Run();
    pThis->m_Tid = 0;
    pthread_exit(0);
  };

  //!
  //!  \brief Internal pthread ID..
  //!
  pthread_t m_Tid = 0;
};

VCD_NS_END;

#endif /* THREADABLE_H */
