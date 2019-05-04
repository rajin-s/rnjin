/* *** ** *** ** *** ** *** *
 * Part of rnjin            *
 * (c) Rajin Shankar, 2019  *
 *        rajinshankar.com  *
 * *** ** *** ** *** ** *** */

// Aliases for STL types
#define string std::string
#define list std::vector
#define dictionary std::unordered_map

// Common macros
#define s( value ) std::to_string( value )
#define text( name, value ) const string message__##name = value
#define get_text( name ) message__##name