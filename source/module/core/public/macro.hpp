/* *** ** *** ** *** ** *** *
 * Part of rnjin            *
 * (c) Rajin Shankar, 2019  *
 *        rajinshankar.com  *
 * *** ** *** ** *** ** *** */

/*

    TODO

    Remove this

*/

#pragma once
#include <rnjin.hpp>

// Introspection utilities
#define __STR2( x ) #x
#define __STR( x ) __STR2( x )

#define __raw_function_string __FUNCTION__
#define __raw_file_string __FILE__
#define __raw_line_string __STR( __LINE__ )