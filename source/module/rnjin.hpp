/* *** ** *** ** *** ** *** *
 * Part of rnjin            *
 * (c) Rajin Shankar, 2019  *
 *        rajinshankar.com  *
 * *** ** *** ** *** ** *** */

// clang-format off
#pragma once

#define __make_string( s ) #s

#define rnjin_module( module_name ) __make_string( module_name/module_name.hpp )
#define rnjin_module_header( module_name, header_name ) __make_string( module_name/public/header_name.hpp )
#define internal_header( header_name ) __make_string( header_name.hpp )

// clang-format on