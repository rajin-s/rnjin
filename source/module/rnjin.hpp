/* *** ** *** ** *** ** *** *
 * Part of rnjin            *
 * (c) Rajin Shankar, 2019  *
 *        rajinshankar.com  *
 * *** ** *** ** *** ** *** */

#pragma once

/* -------------------------------------------------------------------------- */
/*                         Module / Include Utilities                         */
/* -------------------------------------------------------------------------- */

// clang-format off

#define __make_string( s ) #s

#define rnjin_module( module_name ) __make_string( module_name/module_name.hpp )
#define rnjin_module_subset( module_name, subset_name ) __make_string( module_name/subset_name.hpp )
#define rnjin_module_public( module_name, header_name ) __make_string( module_name/public/header_name.hpp )
#define internal_header( header_name ) __make_string( header_name.hpp )
#define standard_library( header_name ) <header_name>

// clang-format on