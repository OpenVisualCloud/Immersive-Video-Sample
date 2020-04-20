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
//! \file:   Fraction.h
//! \brief:  Fraction class definition
//! \detail: Define the basic class to represent rational number
//!

#ifndef _FRACTION_H_
#define _FRACTION_H_

#include <cassert>
#include <cstdint>
#include <cstdlib>
#include <iostream>
#include <stdexcept>
#include "../include/Common.h"

using namespace std;

VCD_MP4_BEGIN

template <typename T>
T GetGCD(T first, T second)
{
    T third;
    while (first)
    {
        third = first;
        first = second % first;
        second = third;
    }

    return second;
}

template <typename T>
T GetLCM(T first, T second)
{
    return first / GetGCD(first, second) * second;
}

struct InvalidFraction
{
};

template <typename T>
struct Fraction
{
public:
    typedef T value;

    Fraction();
    Fraction(T aNum, T aDen);
    Fraction(InvalidFraction);
    ~Fraction();

    Fraction<T> GetMinimum() const;
    Fraction<T> per1() const;

    template <typename U>
    U cast() const;

    double asDouble() const;

    T m_num, m_den;
};

typedef Fraction<uint64_t> FractU64;
typedef Fraction<int64_t> FractS64;

template <typename T>
Fraction<T>::Fraction()
    : m_num(0)
    , m_den(1)
{
}

#if defined(_MSC_VER)
#pragma warning(push)
#pragma warning(disable : 4146)
#endif

template <typename T>
Fraction<T>::Fraction(T aNum, T aDen)
    : m_num(aNum)
    , m_den(aDen)
{
    if (m_den < 0)
    {
        m_den = -m_den;
        m_num = -m_num;
    }
    assert(m_den != 0);
}

#if defined(_MSC_VER)
#pragma warning(pop)
#endif

template <typename T>
template <typename U>
U Fraction<T>::cast() const
{
    return U(static_cast<typename U::value>(m_num), static_cast<typename U::value>(m_den));
}

template <typename T>
Fraction<T>::Fraction(InvalidFraction)
    : m_num(0)
    , m_den(0)
{
}

template <typename T>
Fraction<T>::~Fraction()
{
}

template <typename T>
double Fraction<T>::asDouble() const
{
    return double(m_num) / m_den;
}

template <typename T>
void shareDenominator(Fraction<T>& x, Fraction<T>& y)
{
    if (x.m_den != y.m_den)
    {
        T cm   = GetLCM(x.m_den, y.m_den);
        T mulX = cm / x.m_den;
        T mulY = cm / y.m_den;

        x.m_num *= mulX;
        x.m_den *= mulX;

        y.m_num *= mulY;
        y.m_den *= mulY;
        assert(x.m_den == y.m_den);
    }
}

template <typename Iterator>
void shareDenominators(Iterator begin, Iterator end)
{
    if (distance(begin, end) >= 2)
    {
        Iterator it = begin;
        auto cm     = (**it).m_den;
        ++it;
        for (; it != end; ++it)
        {
            cm = GetLCM(cm, (**it).m_den);
        }

        for (it = begin; it != end; ++it)
        {
            auto mul = cm / (**it).m_den;
            (**it).m_num *= mul;
            (**it).m_den *= mul;
        }
    }
}

template <typename T>
Fraction<T>& operator+=(Fraction<T>& self, Fraction<T> other)
{
    shareDenominator(self, other);
    self.m_num += other.m_num;
    return self;
}

template <typename T>
Fraction<T>& operator-=(Fraction<T>& self, Fraction<T> other)
{
    shareDenominator(self, other);
    self.m_num -= other.m_num;
    return self;
}

template <typename T>
bool operator==(Fraction<T> x, Fraction<T> y)
{
    if (x.m_den == y.m_den)
    {
        return x.m_num == y.m_num;
    }
    else
    {
        shareDenominator(x, y);
        return x.m_num == y.m_num;
    }
}

template <typename T>
bool operator<=(Fraction<T> x, Fraction<T> y)
{
    if (x.m_den == y.m_den)
    {
        return x.m_num <= y.m_num;
    }
    else
    {
        shareDenominator(x, y);
        return x.m_num <= y.m_num;
    }
}

template <typename T>
bool operator>=(Fraction<T> x, Fraction<T> y)
{
    if (x.m_den == y.m_den)
    {
        return x.m_num >= y.m_num;
    }
    else
    {
        shareDenominator(x, y);
        return x.m_num >= y.m_num;
    }
}

template <typename T>
bool operator!=(Fraction<T> x, Fraction<T> y)
{
    if (x.m_den == y.m_den)
    {
        return x.m_num != y.m_num;
    }
    else
    {
        shareDenominator(x, y);
        return x.m_num != y.m_num;
    }
}

template <typename T>
bool operator<(Fraction<T> x, Fraction<T> y)
{
    if (x.m_den == y.m_den)
    {
        return x.m_num < y.m_num;
    }
    else
    {
        shareDenominator(x, y);
        return x.m_num < y.m_num;
    }
}

template <typename T>
bool operator>(Fraction<T> x, Fraction<T> y)
{
    if (x.m_den == y.m_den)
    {
        return x.m_num > y.m_num;
    }
    else
    {
        shareDenominator(x, y);
        return x.m_num > y.m_num;
    }
}

template <typename T>
Fraction<T> operator*(Fraction<T> x, Fraction<T> y)
{
    return Fraction<T>(x.m_num * y.m_num, x.m_den * y.m_den).GetMinimum();
}

template <typename T>
Fraction<T> operator/(Fraction<T> x, Fraction<T> y)
{
    return x * y.per1().GetMinimum();
}

template <typename T>
Fraction<T> operator+(Fraction<T> x, Fraction<T> y)
{
    shareDenominator(x, y);
    return Fraction<T>(x.m_num + y.m_num, x.m_den).GetMinimum();
}

template <typename T>
Fraction<T> operator-(Fraction<T> x, Fraction<T> y)
{
    shareDenominator(x, y);
    return Fraction<T>(x.m_num - y.m_num, x.m_den).GetMinimum();
}

template <typename T>
Fraction<T> operator-(Fraction<T> x)
{
    return Fraction<T>(-x.m_num, x.m_den);
}

template <typename T>
Fraction<T> Fraction<T>::GetMinimum() const
{
    Fraction r(*this);
    if (r.m_num == 0)
    {
        return Fraction<T>(0, 1);
    }
    else
    {
        T cd = GetGCD(m_num, m_den);
        return Fraction<T>(m_num / cd, m_den / cd);
    }
}

template <typename T>
Fraction<T> Fraction<T>::per1() const
{
    return Fraction<T>(m_den, m_num);
}

template <typename T>
ostream& operator<<(ostream& stream, Fraction<T> x)
{
    return stream << x.m_num << "/" << x.m_den;
}

VCD_MP4_END;

#endif  // _FRACTION_H_
