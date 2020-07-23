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

//! \file:   DownloadManager.h
//! \brief:
//! \detail:
//!
//! Created on May 28, 2019, 2:39 PM
//!

#ifndef _DOWNLOADMANAGER_H
#define _DOWNLOADMANAGER_H

#include "general.h"
#include <mutex>

typedef bool (*enum_dir_item)(void *cbck, std::string item_name, std::string item_path);

VCD_OMAF_BEGIN

class DownloadManager {
public:
    DownloadManager();
    virtual ~DownloadManager();

public:
    //!
    //! \brief  Delete a cached file from cache.
    //!
    int DeleteCacheFile(std::string url);

    //!
    //! \brief  Delete a all cached files from cache.
    //!
    void CleanCache();

    //!
    //! \brief  Delete a all cached files from cache with condition that
    //!         the file time is less than ...
    //!
    void DeleteCacheByTime( uint64_t interval );

    //!
    //! \brief  Delete a all cached files from cache with condition that
    //!         the total cache size is large thanm MaxCacheSize
    //!
    void DeleteCacheBySize( );

    //!
    //! \brief  Get a Cache file name
    //!
    std::string AssignCacheFileName();

    //!
    //! \brief  Get a downloading bit rate
    //!
    int GetImmediateBitrate();

    //!
    //! \brief  Get an average downloading bit rate
    //!
    int GetAverageBitrate();

    //!
    //! \brief  Get/Set methods for properties
    //!
    void        SetMaxCacheSize(uint64_t size)          { mMaxCacheSize = size;        };
    uint64_t    GetMaxCacheSize()                       { return mMaxCacheSize;        };
    void        SetStartTime(uint64_t size)             { mStartTime = size;           };
    uint64_t    GetStartTime()                          { return mStartTime;           };
    uint64_t    GetDownloadBytes()                      { return mDownloadedBytes;     };
    std::string GetCacheFolder()                        { return mCacheDir;            };
    int         SetCacheFolder( std::string cache_dir );
    void        SetFilePrefix(std::string prefix)       { mFilePrefix = prefix;        };
    std::string GetFilePrefix()                         { return mFilePrefix;          };
    bool        UseCache()                              { return mUseCache;            };
    void        SetUseCache(bool bCache)                { mUseCache = bCache;          };

private:

    //!
    //! \brief  the implementation to delete all files
    //!
    void delete_all_cached_files( std::string directory );

    //!
    //! \brief  the implementation to get cache size
    //!
    uint64_t CacheGetSize( std::string directory );

    //!
    //! \brief callback to delete file
    //!
    //static bool delete_cache_files( void *cbck,
    //                         std::string item_name,
    //                         std::string item_path );

    //!
    //! \brief  callback to get size of each file
    //!
    //static bool GatherCacheSize( void *cbck,
    //                      std::string item_name,
     //                     std::string item_path );

    //!
    //! \brief  enumerate all files in a directory
    //!
    int enum_directory( const char *dir,
                        bool enum_directory,
                        enum_dir_item enum_dir_fct,
                        void *cbck,
                        const char *filter = NULL);

    //!
    //! \brief  generate a random string for assigning cache file name
    //!
    std::string GetRandomString(int size);

private:
    int                            mDownloadedBytes;    //<! the total downloaded bytes
    int                            mDownloadedFiles;    //<! the total downloaded files
    std::string                    mCacheDir;           //<! the directory of the cache file
    std::string                    mFilePrefix;         //<! the prefix for each cached file
    std::mutex                     mMutex;              //<! for synchronization
    uint64_t                       mStartTime;          //<! the start time to caching in this process
    uint64_t                       mMaxCacheSize;       //<! the threshold of total cache file size
    bool                           mUseCache;           //<! the flag to indicate whether using file caching
    int32_t                        m_count;             //<! count for random file name
    std::mutex                     mCacheMtx;                //<! mutex for cache clear
};

typedef VCD::VRVideo::Singleton<DownloadManager> DOWNLOADMANAGER;    //<! singleton of DownloadManager

VCD_OMAF_END;

#endif /* DOWNLOADMANAGER_H */

