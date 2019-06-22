/* *** ** *** ** *** ** *** *
 * Part of rnjin            *
 * (c) Rajin Shankar, 2019  *
 *        rajinshankar.com  *
 * *** ** *** ** *** ** *** */

#pragma once

// Common macros (these aren't that great...)
#define s( value ) std::to_string( value )
#define text( name, value ) const string message__##name = value
#define get_text( name ) message__##name

// C++ language extensions
#define foreach(iteration) for (const auto& iteration)
#define let const auto