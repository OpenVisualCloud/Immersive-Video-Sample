/*
 * Copyright (c) 2018, Intel Corporation
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
 * File:   Singleton.h
 * Author: Zhang, Andrew
 *
 * Created on January 16, 2019, 1:33 PM
 */

#ifndef SINGLETON_H
#define SINGLETON_H
#include "ns_def.h"
#include <pthread.h>
#include "Threadable.h"

VCD_NS_BEGIN

template<class T>
class Singleton
{
private:
    static T* _instance;
    Singleton();
    static ThreadLock lc;
    
public:
    static T* GetInstance();
    static void DestroyInstance();
        
};

template<class T>
T* Singleton<T>::_instance = NULL;

template<class T>
ThreadLock Singleton<T>::lc;

template<class T>
T* Singleton<T>::GetInstance()
{
    if(_instance == NULL){
        lc.lock();
        if(_instance == NULL){
            _instance = new T;
        }
        lc.unlock();
    }
    return _instance;
}

template<class T>
void Singleton<T>::DestroyInstance()
{
    if(_instance != NULL){
        delete _instance;
        _instance = NULL;
    }
    
}

VCD_NS_END;
#endif /* SINGLETON_H */

