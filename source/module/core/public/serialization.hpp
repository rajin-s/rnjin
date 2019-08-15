/* *** ** *** ** *** ** *** *
 * Part of rnjin            *
 * (c) Rajin Shankar, 2019  *
 *        rajinshankar.com  *
 * *** ** *** ** *** ** *** */

#pragma once

#include "containers.hpp"
#include "macro.hpp"


namespace rnjin
{
    namespace core
    {
        template <typename T>
        class serializer
        {
            list<byte> serialize( const T& target ) {}
            void deserialize( const list<byte>& data, T& target ) {}
        };
    } // namespace core
} // namespace rnjin