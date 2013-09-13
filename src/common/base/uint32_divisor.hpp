// Copyright (c) 2010, Tencent Inc.
// All rights reserved.
//
// Author: CHEN Feng <phongchen@tencent.com>

#ifndef COMMON_BASE_UINT32_DIVISOR_HPP
#define COMMON_BASE_UINT32_DIVISOR_HPP

/// @author phongchen <phongchen@tencent.com>
/// @date Dec 5, 2010

#include <assert.h>
#include <stdexcept>

#include "common/base/stdint.h"

/// optimized unsigned 32 bit integer divide for invariant divisors.
/// majorly using multiply and shift to instead of CPU div instruction.
/// performance:
/// 4 times faster than div instruction for small (< 10000) divisors.
/// 3 times faster than div instruction for midium (10000, 10000000) divisors.
/// 2 times faster than div instruction for large (> 10000000) divisors.
/// @sa
/// [1] http://www.hackersdelight.org/divcMore.pdf
/// [2] http://support.amd.com/us/Processor_TechDocs/22007.pdf
/// @note dividend can't be 64 bits
class UInt32Divisor
{
    // objects of this class are safe to copy and assign.
public:
    enum Algorithm
    {
        MULTIPLE_SHIFT,
        MULTIPLE_ADD_SHIFT,
        SHIFT,
        COMPARE,
        INVALID = -1
    };
public:
    UInt32Divisor() :
        m_divisor(0),
        m_algorithm(INVALID),
        m_multiplier(0),
        m_shift(0)
    {
    }

    explicit UInt32Divisor(uint32_t divisor)
    {
        if (!SetValue(divisor))
            throw std::runtime_error("can't find mul and shift");
    }

    /// set new divisor, then update internal state
    /// @return whether success
    /// @return true success
    /// @false invalid divisor, object keep unchanged
    bool SetValue(uint32_t divisor)
    {
        if (DetectAlgorithm(divisor, &m_algorithm, &m_multiplier, &m_shift))
        {
            m_divisor = divisor;
            return true;
        }
        return false;
    }

    /// do divide operation
    /// @return the quotient
    uint32_t Divide(uint32_t dividend) const
    {
        switch (m_algorithm)
        {
        case MULTIPLE_SHIFT:
            {
                uint64_t product = (uint64_t) dividend * (uint64_t) m_multiplier;
                uint32_t high = product >> 32;
                return high >> m_shift;
            }
        case MULTIPLE_ADD_SHIFT:
            {
                uint64_t product = (uint64_t) dividend * (uint64_t) m_multiplier;
                uint32_t high = product >> 32;
                uint32_t low = (uint32_t) product;
                if (low + m_multiplier < low) // check overflow
                    ++high;
                return high >> m_shift;
            }
        case SHIFT:
            return dividend >> m_shift;
        case COMPARE:
            return dividend >= m_divisor;
        case INVALID:
            assert(!"Invalid divisor");
        }

        // never reachable
        return 0;
    }

    /// do divide operation, obtain both quotient and reminder
    /// @return quotient
    uint32_t Divide(uint32_t dividend, uint32_t* remainder) const
    {
        uint32_t quotient = Divide(dividend);
        *remainder = dividend - quotient * m_divisor;
        return quotient;
    }

    /// Mudulu operation
    /// @return remainder
    uint32_t Modulu(uint32_t dividend) const
    {
        return dividend - Divide(dividend) * m_divisor;
    }

public: // attributes
    uint32_t GetValue() const
    {
        return m_divisor;
    }

    // just for test
    Algorithm GetAlgorithm() const
    {
        return m_algorithm;
    }

    // just for test
    uint32_t GetMultiplier() const
    {
        return m_multiplier;
    }

    // just for test
    uint32_t GetShift() const
    {
        return m_shift;
    }

private:
    uint32_t m_divisor;
    Algorithm m_algorithm;
    uint32_t m_multiplier;
    uint32_t m_shift;

private:
    static uint32_t log2(uint32_t i)
    {
        uint32_t t = 0;
        i = i >> 1;
        while (i)
        {
            i = i >> 1;
            t++;
        }
        return t;
    }

    /// detect algorithm and corresponding parameters.
    /// the following code is from:
    /// [2] AMD Athlon Processor x86 Code Optimization Guide, page 144,
    /// and made some necessary modification.
    /// @param divisor divisor to be handled
    /// @param multiply multiply factor
    /// @param shift shift factor (0..63)
    /// @retval true if a valid mul and shift value are found.
    static bool DetectAlgorithm(
        uint32_t divisor,
        Algorithm* algorithm,
        uint32_t* multiplier,
        uint32_t* shift
        )
    {
        if (divisor == 0)
            return false;

        if (divisor >= 0x80000000) {
            *algorithm = COMPARE;
            return true;
        }

        /* Reduce divisor until it becomes odd */
        uint32_t lowest_zero_bits = 0;
        uint32_t t = divisor;
        while (!(t & 1))
        {
            t >>= 1;
            lowest_zero_bits++;
        }

        // is power of 2
        if (t == 1)
        {
            *algorithm = SHIFT;
            *multiplier = 1;
            *shift = lowest_zero_bits;
            return true;
        }

        /* Generate multiplier, shift for algorithm 0. Based on: Granlund, T.;
         * Montgomery,
         * P.L.: "Division by Invariant Integers using Multiplication".
         * SIGPLAN Notices, Vol. 29, June 1994, page 61.
         * */

        uint32_t l = log2(t) + 1;
        uint64_t j = ((0xffffffffULL) % (uint64_t) t);
        uint64_t k = (1ULL << (32 + l)) / (uint64_t) (0xffffffff - j);
        uint64_t m_low = (1ULL << (32 + l)) / t;
        uint64_t m_high = ((1ULL << (32 + l)) + k) / t;

        while (((m_low >> 1) < (m_high >> 1)) && (l > 0))
        {
            m_low = m_low >> 1;
            m_high = m_high >> 1;
            --l;
        }

        if ((m_high >> 32) == 0)
        {
            *multiplier = (uint32_t) m_high;
            *shift = l;
            *algorithm = MULTIPLE_SHIFT;
        }
        else
        {
            /* Generate multiplier, shift for algorithm 1. Based on: Magenheimer,
             * D.J.; et al:
             * "Integer Multiplication and Division on the HP Precision Architecture".
             * IEEE Transactions on Computers, Vol 37, No. 8, August 1988, page 980.
             * */
            *shift = log2(t);
            m_low = (1ULL << (32 + *shift)) / (uint64_t) t;
            uint32_t r = (uint32_t) ((1ULL << (32 + *shift)) % (uint64_t) t);
            *multiplier = (r < ((t>>1)+1)) ? (uint32_t) m_low : (uint32_t) m_low + 1;
            *algorithm = MULTIPLE_ADD_SHIFT;
        }

        /* Reduce multiplier for either algorithm to smallest possible */
        while (!(*multiplier & 1))
        {
            *multiplier >>= 1;
            --*shift;
        }

        /* Adjust multiplier for reduction of even divisors */
        *shift += lowest_zero_bits;
        return true;
    }
};

#endif // COMMON_BASE_UINT32_DIVISOR_HPP
