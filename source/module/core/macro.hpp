/* *** ** *** ** *** ** *** *
* Part of rnjin            *
* (c) Rajin Shankar, 2019  *
*        rajinshankar.com  *
* *** ** *** ** *** ** *** */

/*  Common macros
 */

#define text(name, value) const string message__##name = value
#define get_text(name) message__##name