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

/*
 * File:   DownloadManager.cpp
 * Author: media
 *
 * Created on May 28, 2019, 2:39 PM
 */

#include "DownloadManager.h"

#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <map>

#define MAX_PATH_COUNT 1024

VCD_OMAF_BEGIN

typedef struct GatherStruct {
  int size;
  std::string prefix;
} GatherStruct;

DownloadManager::DownloadManager() {
  mDownloadedBytes = 0;
  mDownloadedFiles = 0;
  mCacheDir = "";
  mMaxCacheSize = 200000000;
  mStartTime = 0;
  mFilePrefix = "";
  // start count from 1 because 0 and 1 would generate
  // same random file name, so ignore 0
  m_count = 1;
  mUseCache = false;
}

DownloadManager::~DownloadManager() { }

int DownloadManager::DeleteCacheFile(std::string url) {
  if (remove(url.c_str())) {
    OMAF_LOG(LOG_WARNING, "Failed to delete file in cache for url: %s ! Be cautious cache may exceed the storage limitation!\n", url.c_str());
    return ERROR_INVALID;
  }
  return ERROR_NONE;
}

int DownloadManager::SetCacheFolder(std::string cache_dir) {
  mCacheDir = cache_dir;
  if ((access(mCacheDir.c_str(), 2)) != -1) {
    DeleteCacheBySize();
    return ERROR_NONE;
  }

  if (-1 == mkdir(mCacheDir.c_str(), 0755)) return ERROR_INVALID;

  return ERROR_NONE;
}

std::string DownloadManager::AssignCacheFileName() {
  std::lock_guard<std::mutex> lock(mMutex);
  std::string file_name = GetRandomString(32);
  return file_name;
}

/// get download bit rate
int DownloadManager::GetImmediateBitrate() { return 0; }

int DownloadManager::GetAverageBitrate() { return 0; }

void DownloadManager::CleanCache() { delete_all_cached_files(mCacheDir.c_str()); }

void DownloadManager::DeleteCacheByTime(uint64_t interval) {
  if (mCacheMtx.try_lock()) {
    uint64_t out_size = CacheGetSize(mCacheDir.c_str());
    if (out_size < mMaxCacheSize) {
      mCacheMtx.unlock();
      return;
    }

    auto cachePath = GetCacheFolder();
    DIR *dir = opendir(cachePath.c_str());
    if (!dir) {
      OMAF_LOG(LOG_WARNING, "Failed to open cache folder! Be cautious cache may exceed the storage limitation!\n");
      mCacheMtx.unlock();
      return;
    }

    dirent *ent = readdir(dir);
    int32_t removeCnt = 0;
    while (ent && ((uint32_t)removeCnt) < out_size / 2) {
      struct stat st;
      if (stat(ent->d_name, &st)) {
        OMAF_LOG(LOG_WARNING, "Failed to get cache file time info! Be cautious cache may exceed the storage limitation!\n");
        mCacheMtx.unlock();
        return;
      }
      if ((uint64_t)(st.st_mtim.tv_nsec) < GetStartTime() + interval) {
        if (remove(ent->d_name))
          OMAF_LOG(LOG_WARNING, "Failed to delete file in cache ! Be cautious cache may exceed the storage limitation!\n");
      }

      ent = readdir(dir);
    }
    closedir(dir);

    mCacheMtx.unlock();
  }
  return;
}

void DownloadManager::DeleteCacheBySize() {
  uint64_t out_size = CacheGetSize(mCacheDir.c_str());
  if (out_size >= mMaxCacheSize) {
    delete_all_cached_files(mCacheDir.c_str());
  }
}

static bool delete_cache_files(void *cbck, std::string item_name, std::string itemPath) {
  std::string *startPattern;
  uint32_t sz;
  if (NULL == cbck) return false;
  if (0 == item_name.size()) return false;
  if (0 == itemPath.size()) return false;

  startPattern = (std::string *)cbck;
  sz = (uint32_t)strlen(startPattern->c_str());
  if (!strncmp(startPattern->c_str(), item_name.c_str(), sz)) {
    if (remove(itemPath.c_str()) == 0) return true;
  }

  return false;
}

void DownloadManager::delete_all_cached_files(std::string directory) {
  enum_directory(directory.c_str(), false, delete_cache_files, (void *)&mFilePrefix, NULL);
}

static bool GatherCacheSize(void *cbck, std::string item_name, std::string itemPath) {
  GatherStruct *out = (GatherStruct *)cbck;
  if (!strncmp(out->prefix.c_str(), item_name.c_str(), out->prefix.size())) {
    struct stat statbuf;
    stat(itemPath.c_str(), &statbuf);
    out->size += statbuf.st_size;
    return true;
  }
  return false;
}

uint64_t DownloadManager::CacheGetSize(std::string directory) {
  GatherStruct gat;
  gat.prefix = mFilePrefix;
  gat.size = 0;

  enum_directory(directory.c_str(), false, GatherCacheSize, (void *)&gat, NULL);
  return gat.size;
}

int DownloadManager::enum_directory(const char *dir, bool enum_directory, enum_dir_item enumDirFct, void *cbck,
                                    const char *filter) {
  unsigned char itemPath[MAX_PATH_COUNT];
  unsigned char path[MAX_PATH_COUNT], *file;
  DIR *currentDir;
  struct dirent *currentFile;
  struct stat st;

  if (!dir || !enumDirFct) return ERROR_INVALID;

  strncpy((char *)path, dir, strlen(dir) + 1);
  if (path[strlen((const char *)path) - 1] != '\\') strncat((char *)path, "\\", strlen("\\"));

  currentDir = opendir((char *)path);
  if (currentDir == NULL) return ERROR_INVALID;

  currentFile = readdir(currentDir);
  while (currentFile) {
    if (!strcmp(currentFile->d_name, "..")) goto end;
    if (currentFile->d_name[0] == '.') goto end;

    if (filter) {
      char *separate = strrchr(currentFile->d_name, '.');
      if (!separate) goto end;
      if (!strstr(filter, separate + 1)) goto end;
    }

    strncpy((char *)itemPath, (const char *)path, strlen((const char *)path) + 1);
    if (strlen(currentFile->d_name) < 1024 - strlen((const char *)itemPath))
      strncat((char *)itemPath, currentFile->d_name, sizeof(currentFile->d_name));
    if (stat((const char *)itemPath, &st) != 0) goto end;
    if (enum_directory && ((st.st_mode & S_IFMT) != S_IFDIR)) {
      goto end;
    }

    if (!enum_directory && ((st.st_mode & S_IFMT) == S_IFDIR)) goto end;
    file = (unsigned char *)currentFile->d_name;

    if (enumDirFct(cbck, (char *)file, (char *)itemPath)) {
      break;
    }
  end:
    currentFile = readdir(currentDir);
  }

  return ERROR_NONE;
}

std::string DownloadManager::GetRandomString(int size) {
  std::string str = "";
  const char CCH[] = "_0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ_";

  // generate seed for random number with count
  srand(m_count % INT32_MAX);

  for (int i = 0; i < size; ++i) {
    int x = rand() / (RAND_MAX / (sizeof(CCH) - 1));
    str += CCH[x];
  }
  str = mFilePrefix + "_" + str;

  m_count++;
  if (m_count == 0) m_count++;

  return str;
}

VCD_OMAF_END
