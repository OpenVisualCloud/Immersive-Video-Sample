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
//! \file:   FormAllocator.h
//! \brief:  Allocate Atom class.
//! \detail: Used for basic atom allocator and std lib operation.
//!
//! Created on October 14, 2019, 13:39 PM
//!
#ifndef FORMALLOCATOR_H
#define FORMALLOCATOR_H

#include <iostream>
#include <list>
#include <map>
#include <memory>
#include <set>
#include <sstream>
#include <string>
#include <vector>
#include "../include/Common.h"

VCD_MP4_BEGIN

class FormAllocator
{
public:

    //!
    //! \brief Constructor
    //!
    FormAllocator() {};

    //!
    //! \brief Destructor
    //!
    virtual ~FormAllocator() {};

    //!
    //! \brief    allocator function
    //!
    //! \param    [in] size_t
    //!           number
    //! \param    [in] size_t
    //!           size
    //!
    //! \return   void*
    //!
    virtual void* allocate(size_t n, size_t size) = 0;

    //!
    //! \brief    deallocate function
    //!
    //! \param    [in] void*
    //!           pointer
    //!
    //! \return   void
    //!
    virtual void deallocate(void* ptr) = 0;
};

FormAllocator* GetDefaultAllocator();
FormAllocator* GetFormAllocator();

template <typename T>
class Allocator : public std::allocator<T>
{
public:
    template <typename U>
    struct rebind
    {
        typedef Allocator<U> other;
    };

    T* allocate(size_t n, const void* hint = 0)
    {
        (void) hint;
        return static_cast<T*>(GetFormAllocator()->allocate(n, sizeof(T)));
    }

    void deallocate(T* p, size_t n)
    {
        (void) n;
        return GetFormAllocator()->deallocate(p);
    }

    //!
    //! \brief Constructor
    //!
    Allocator() : std::allocator<T>()
    {
    }
    Allocator(const Allocator& a) : std::allocator<T>(a)
    {
    }
    template <class U>
    Allocator(const Allocator<U>& a) : std::allocator<T>(a)
    {
    }

    Allocator& operator=(const Allocator&) = default;

    //!
    //! \brief Destructor
    //!
    ~Allocator()
    {
    }
};

template <typename T>
class FormDelete : public std::default_delete<T>
{
public:
    void operator()(T* ptr) const
    {
        ptr->~T();
        free(ptr);
    }
};

template <class T>
std::shared_ptr<T> MakeShared()
{
    return std::allocate_shared<T>(std::allocator<T>());
}

template <class T, class... Args>
std::shared_ptr<T> MakeShared(Args&&... args)
{
    return std::allocate_shared<T>(std::allocator<T>(), std::forward<Args>(args)...);
}

template <typename T, typename Parent = T>
using UniquePtr = std::unique_ptr<T, FormDelete<Parent>>;

template <typename T, typename Parent, typename... Args>
UniquePtr<T, Parent> MakeUnique(Args&&... args)
{
    return UniquePtr<T, Parent>(new T(std::forward<Args>(args)...));
}

class Exception : public std::exception
{
public:

    //!
    //! \brief Constructor
    //!
    Exception()
        : std::exception()
        , m_data(NULL)
    {
    }

    //!
    //! \brief Destructor
    //!
    Exception(const char* msg)
        : std::exception()
        , m_data(msg)
    {
    }
    virtual const char* what() const noexcept
    {
        return m_data;
    }
    virtual ~Exception()
    {
    }

private:
    const char* m_data; //!< message log
};

VCD_MP4_END;
#endif  /* FORMALLOCATOR_H */
