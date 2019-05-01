/* *** ** *** ** *** ** *** *
 * Part of rnjin            *
 * (c) Rajin Shankar, 2019  *
 *        rajinshankar.com  *
 * *** ** *** ** *** ** *** */

#include "core.hpp"

namespace rnjin
{
    namespace script
    {
        // Read data from an arbitrary pointer as type T
        // (used to extract parameters from bytecode)
        template <typename T>
        T get_data( const char* data )
        {
            return *( (T*)data );
        }

        // We need a way to separate two parameter packs... this seems to work but I'd guess there's a better way
        // Passed in _function_call::invoke(...) as __
        int _separator[1] = { 1 };

        // Function call wrapper struct, construct with script::function_call(...) for return/parameter type inference from provided function
        template <typename return_type, typename... input_types>
        struct _function_call
        {
            public:
            // Function pointer type
            typedef return_type ( *function_type )( input_types... );

            // Create a new function wrapper (return and parameter types cannot be inferred here)
            _function_call( function_type function ) : function( function ) {}

            // Intial public call, use parameter types from function itself
            return_type invoke( const char* data )
            {
                return invoke<input_types...>( data );
            }

            private:
            // Function pointer
            function_type function;

            // Initial internal call, only specify types to get
            template <typename first, typename... rest>
            return_type invoke( const char* data )
            {
                return invoke<rest..., 1, first>( data + sizeof( first ), _separator, get_data<first>( data ) );
            }

            // Base case: no remaining values to get
            template <int _, typename... param_types>
            return_type invoke( const char* data, int __[_], param_types... params )
            {
                return function( params... );
            }

            // Base case: one remaining value to get
            template <typename first, int _, typename... param_types>
            return_type invoke( const char* data, int __[_], param_types... params )
            {
                return function( params..., get_data<first>( data ) );
            }

            // General case: get one value and put it into params
            template <typename first, typename... rest, int _, typename... param_types>
            return_type invoke( const char* data, int __[_], param_types... params )
            {
                return invoke<rest..., _, first, param_types...>( data + sizeof( first ), __, params..., get_data<first>( data ) );
            }
        };

        // Specialization for functions with no parameters
        template <typename return_type>
        struct _function_call<return_type>
        {
            public:
            typedef return_type ( *function_type )();

            _function_call( function_type function ) : function( function ) {}

            return_type invoke( const char* data )
            {
                return function();
            }

            private:
            function_type function;
        };

        // Constructor function since template parameter deduction doesn't happen for actual constructors
        template <typename return_type, typename... param_types>
        _function_call<return_type, param_types...> function_call( return_type ( *function )( param_types... ) )
        {
            return _function_call<return_type, param_types...>( function );
        }

        // Data structures
        struct instruction_binding
        {
        };
        class script_runtime
        {
            public:
            instruction_binding bindings[0xFF];

            script_runtime( dictionary<char, instruction_binding> binds )
            {
                for ( auto k : binds )
                {
                    bindings[k.first] = k.second;
                }
            }
            ~script_runtime() {}

            private:
        };
        class compiled_script
        {
            public:
            const list<unsigned char> data;

            compiled_script( const list<unsigned char> source_data ) : data( source_data ) {}
            const string as_string();

            private:
        };
        class execution_context
        {
            private:
            int program_counter;
            bool valid;

            compiled_script& script;
            script_runtime& runtime;
        };
    } // namespace script
} // namespace rnjin