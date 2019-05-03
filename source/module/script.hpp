/* *** ** *** ** *** ** *** *
 * Part of rnjin            *
 * (c) Rajin Shankar, 2019  *
 *        rajinshankar.com  *
 * *** ** *** ** *** ** *** */

#pragma once
#include "core.hpp"
#include "log.hpp"

namespace rnjin
{
    namespace script
    {
        extern log::source script_log;
        typedef unsigned char byte;
        typedef unsigned char opcode;
        constexpr size_t opcode_count                 = ( 1 << ( sizeof( opcode ) * 8 ) );
        constexpr size_t opcodes_reserved_for_context = 32;
        constexpr size_t opcodes_free_for_runtime     = opcode_count - opcodes_reserved_for_context;

        // Read data from an arbitrary pointer as type T
        // (used to extract parameters from bytecode)
        template <typename T>
        T get_data( const byte* data )
        {
            return *( (T*)data );
        }

        // We need a way to separate two parameter packs... this seems to work but I'd guess there's a better way
        // Passed in _function_binding->invoke(...) as __
        extern int _separator[1];

        struct _binding_base
        {
            public:
            enum type
            {
                static_function,
                instance_method,
                context_method
            };

            virtual void invoke( const byte* data ) = 0;

            size_t get_params_size()
            {
                return params_size;
            }
            const type get_type()
            {
                return call_type;
            }

            protected:
            type call_type;
            size_t params_size;
        };

        // Static function call wrapper struct, construct with script::bind(...) for return/parameter type inference from provided function
        template <typename return_type, typename... input_types>
        struct _function_binding : _binding_base
        {
            public:
            // Function pointer type
            typedef return_type ( *function_type )( input_types... );

            // Create a new function wrapper (return and parameter types cannot be inferred here)
            _function_binding( function_type function ) : function( function )
            {
                params_size = sizeof...( input_types );
                call_type   = type::static_function;
            }

            // Non-returning invoke for implenting _binding_base interface
            void invoke( const byte* data )
            {
                _invoke( data );
            }

            // Intial public call, use parameter types from function itself
            return_type _invoke( const byte* data )
            {
                return _invoke<input_types...>( data );
            }

            private:
            // Function pointer
            function_type function;

            // Initial internal call, only specify types to get
            template <typename first, typename... rest>
            return_type _invoke( const byte* data )
            {
                return _invoke<rest..., 1, first>( data + sizeof( first ), _separator, get_data<first>( data ) );
            }

            // Base case: no remaining values to get
            template <int _, typename... param_types>
            return_type _invoke( const byte* data, int __[_], param_types... params )
            {
                return function( params... );
            }

            // Base case: one remaining type_info to get
            template <typename first, int _, typename... param_types>
            return_type _invoke( const byte* data, int __[_], param_types... params )
            {
                return function( params..., get_data<first>( data ) );
            }

            // General case: get one type_info and put it into params
            template <typename first, typename... rest, int _, typename... param_types>
            return_type _invoke( const byte* data, int __[_], param_types... params )
            {
                return _invoke<rest..., _, first, param_types...>( data + sizeof( first ), __, params..., get_data<first>( data ) );
            }
        };

        // Specialization for functions with no parameters
        template <typename return_type>
        struct _function_binding<return_type> : _binding_base
        {
            public:
            typedef return_type ( *function_type )();

            _function_binding( function_type function ) : function( function )
            {
                params_size = 0;
                call_type   = type::static_function;
            }

            void invoke( const byte* data )
            {
                function();
            }
            return_type _invoke( const byte* data )
            {
                return function();
            }

            private:
            function_type function;
        };

        // Method call struct bound to a particular object instance
        // Construct with script::bind(...) for return/parameter/object type inference from provided method and instance
        template <typename object_type, typename return_type, typename... input_types>
        struct _method_binding : _binding_base
        {
            public:
            // Function pointer type
            typedef return_type ( object_type::*method_type )( input_types... );

            _method_binding( object_type* object_reference, method_type method ) : object_reference( object_reference ), method( method )
            {
                params_size = sizeof...( input_types );
                call_type   = type::instance_method;
            }

            void invoke( const byte* data )
            {
                _invoke( data );
            }

            return_type _invoke( const byte* data )
            {
                _invoke<input_types...>( data );
            }

            private:
            object_type* object_reference;
            method_type method;

            // Initial internal call, only specify types to get
            template <typename first, typename... rest>
            return_type _invoke( const byte* data )
            {
                return _invoke<rest..., 1, first>( data + sizeof( first ), _separator, get_data<first>( data ) );
            }

            // Base case: no remaining values to get
            template <int _, typename... param_types>
            return_type _invoke( const byte* data, int __[_], param_types... params )
            {
                return ( object_reference->*method )( params... );
            }

            // Base case: one remaining type_info to get
            template <typename first, int _, typename... param_types>
            return_type _invoke( const byte* data, int __[_], param_types... params )
            {
                return ( object_reference->*method )( params..., get_data<first>( data ) );
            }

            // General case: get one type_info and put it into params
            template <typename first, typename... rest, int _, typename... param_types>
            return_type _invoke( const byte* data, int __[_], param_types... params )
            {
                return _invoke<rest..., _, first, param_types...>( data + sizeof( first ), __, params..., get_data<first>( data ) );
            }
        };

        template <typename object_type, typename return_type>
        struct _method_binding<object_type, return_type> : _binding_base
        {
            public:
            // Function pointer type
            typedef return_type ( object_type::*method_type )();

            _method_binding( object_type* object_reference, method_type method ) : object_reference( object_reference ), method( method )
            {
                params_size = 0;
                call_type   = type::instance_method;
            }

            void invoke( const byte* data )
            {
                ( object_reference->*method )();
            }

            private:
            object_type* object_reference;
            method_type method;
        };

        class execution_context;

        struct _context_binding_base : _binding_base
        {
            virtual void invoke( execution_context* context, const byte* data ) = 0;
        };

        // Method call struct not bound to a particular object instance, can be invoked provided an execution context
        // Construct with script::bind_context(...) for return/parameter type inference from provided method
        template <typename return_type, typename... input_types>
        struct _context_binding : _context_binding_base
        {
            public:
            // Function pointer type
            typedef return_type ( execution_context::*method_type )( input_types... );

            _context_binding( method_type method ) : method( method )
            {
                params_size = sizeof...( input_types );
                call_type   = type::instance_method;
            }

            void invoke( const byte* data )
            {
                script_log.print( "Can't call context method without context!" );
            }

            void invoke( execution_context* context, const byte* data )
            {
                _invoke( context, data );
            }

            return_type _invoke( execution_context* context, const byte* data )
            {
                _invoke<input_types...>( context, data );
            }

            private:
            method_type method;

            // Initial internal call, only specify types to get
            template <typename first, typename... rest>
            return_type _invoke( execution_context* context, const byte* data )
            {
                return _invoke<rest..., 1, first>( data + sizeof( first ), context, _separator, get_data<first>( data ) );
            }

            // Base case: no remaining values to get
            template <int _, typename... param_types>
            return_type _invoke( execution_context* context, const byte* data, int __[_], param_types... params )
            {
                return ( context->*method )( params... );
            }

            // Base case: one remaining type_info to get
            template <typename first, int _, typename... param_types>
            return_type _invoke( execution_context* context, const byte* data, int __[_], param_types... params )
            {
                return ( context->*method )( params..., get_data<first>( data ) );
            }

            // General case: get one type_info and put it into params
            template <typename first, typename... rest, int _, typename... param_types>
            return_type _invoke( execution_context* context, const byte* data, int __[_], param_types... params )
            {
                return _invoke<rest..., _, first, param_types...>( data + sizeof( first ), context, __, params..., get_data<first>( data ) );
            }
        };

        template <typename return_type>
        struct _context_binding<return_type> : _context_binding_base
        {
            public:
            // Function pointer type
            typedef return_type ( execution_context::*method_type )();

            _context_binding( method_type method ) : method( method )
            {
                params_size = 0;
                call_type   = type::context_method;
            }

            void invoke( const byte* data )
            {
                script_log.print( "Can't call context method without context!" );
            }

            void invoke( execution_context* context, const byte* data )
            {
                ( context->*method )();
            }

            private:
            method_type method;
        };


        typedef _binding_base binding;
        typedef _context_binding_base context_binding;
        typedef dictionary<opcode, binding*> opcode_map;
        typedef unsigned int instruction_pointer;

        // Data structures

        // Bindings to static and instance methods, shared between many execution contexts.
        // Use bind( instr, function ) or bind( instr, method, instance ) to add bindings.
        // Note: bindings 0x00 - 0x1F are reserved for context methods, so don't bind to these.
        class runtime
        {
            public:
            // Create a runtime from a byte->binding map
            runtime();
            runtime( opcode_map operations );
            ~runtime();

            void set_bindings( opcode_map operations );

            template <typename return_type, typename... param_types>
            void bind( opcode instruction, return_type ( *function )( param_types... ) )
            {
                if ( instruction < opcodes_reserved_for_context )
                {
                    script_log.printf( "Can't bind to reserved opcode \1!", { s( instruction ) } );
                }
                else
                {
                    bindings[instruction - opcodes_reserved_for_context] = _get_binding( function );
                }
            }

            template <typename object_type, typename return_type, typename... param_types>
            void bind( opcode instruction, return_type ( *function )( param_types... ), object_type* object_reference )
            {
                if ( instruction < opcodes_reserved_for_context )
                {
                    script_log.printf( "Can't bind to reserved opcode \1!", { s( instruction ) } );
                }
                else
                {
                    bindings[instruction - opcodes_reserved_for_context] = _get_binding( function, object_reference );
                }
            }

            binding* get_binding( byte opcode );

            private:
            // Bindings array with entries for every possible opcode (index)
            //   Note: for large opcodes this will get very large...
            binding* bindings[opcodes_free_for_runtime] = { nullptr };
        };

        class compiled_script
        {
            public:
            list<byte> data;

            compiled_script( const string file_path );
            const string as_string();
        };

        typedef short int var_pointer;
        struct any
        {
            public:
            enum type
            {
                byte_val   = 0x00,
                byte_ptr   = 0x01,
                int_val    = 0x02,
                int_ptr    = 0x03,
                float_val  = 0x04,
                float_ptr  = 0x05,
                double_val = 0x06,
                double_ptr = 0x07,
                float2_val = 0x08,
                float2_ptr = 0x09,
                float3_val = 0x0A,
                float3_ptr = 0x0B,
                float4_val = 0x0C,
                float4_ptr = 0x0D,
                string_val = 0x0E,
                string_ptr = 0x0F,
            };

            type var_type;
            union {
                var_pointer as_pointer;
                byte as_byte;
                int as_int;
                float as_float;
                double as_double;
                // TODO: Add others
            } value;
        };
        typedef any any_ptr;
        typedef any numeric;
        typedef any numeric_ptr;
        typedef any scalar;
        typedef any scalar_ptr;


        class execution_context
        {
            public:
            execution_context( compiled_script& script, runtime& runtime );

            // Check if the script and runtime are both valid
            bool valid();
            void execute();

            void jump( int offset );
            void jump_to( unsigned int position );

            // Additional parameters are manually extracted
            void jump_if( int offset );
            void jump_to_if( unsigned int position );

            void create_var( void );
            void delete_var( void );
            void set_var( void );
            void set_var_array( void );
            void set_var_offset( unsigned char offset );

            void create_static( void );
            void delete_static( void );
            void set_static( void );
            void set_static_array( void );
            void set_static_offset( void );

            void set_or( void );
            void set_and( void );
            void set_not( void );

            void set_sum( void );
            void set_difference( void );
            void set_product( void );
            void set_quotient( void );
            void set_remainder( void );
            void set_exponent( void );

            void set_dot( void );
            void set_cross( void );
            void set_norm( void );

            void increment( void );
            void decrement( void );

            void print_value( void );
            void print_absolute( void );

            template <typename T>
            T read()
            {
                type_info type = *( (type_info*)( &source_script.data[program_counter] ) );
                program_counter += sizeof( type_info );

                if ( type & 0x01 )
                {
                    return read_pointer<T>();
                }
                else
                {
                    return read_value<T>();
                }
            }

            private:
            typedef unsigned char type_info;

            template <typename T>
            T read_value()
            {
                T value = *( (T*)( &source_script.data[program_counter] ) );
                program_counter += sizeof( T );
                return value;
            }

            template <typename T>
            T read_pointer()
            {
                var_pointer var = *( (var_pointer*)( &source_script.data[program_counter] ) );
                program_counter += sizeof( var_pointer );
                return lookup<T>( var );
            }

            template <>
            any read<any>()
            {
                type_info type = *( (type_info*)( &source_script.data[program_counter] ) );
                program_counter += sizeof( type_info );
                any result;
                result.var_type = (any::type)type;

                switch ( result.var_type )
                {
                    case any::type::byte_val:
                        result.value.as_byte = read_value<byte>();
                        break;
                    case any::type::byte_ptr:
                        result.value.as_byte = read_pointer<byte>();
                        break;
                    case any::type::int_val:
                        result.value.as_int = read_value<int>();
                        break;
                    case any::type::int_ptr:
                        result.value.as_int = read_pointer<int>();
                        break;
                    case any::type::float_val:
                        result.value.as_float = read_value<float>();
                        break;
                    case any::type::float_ptr:
                        result.value.as_float = read_pointer<float>();
                        break;
                    case any::type::double_val:
                        result.value.as_double = read_value<double>();
                        break;
                    case any::type::double_ptr:
                        result.value.as_double = read_pointer<double>();
                        break;
                    default:
                        break;
                }

                return result;
            }

            static context_binding* context_bindings[opcodes_reserved_for_context];

            instruction_pointer program_counter;
            compiled_script& source_script;
            runtime& source_runtime;

            template <typename T>
            T lookup( var_pointer var )
            {
                return T();
            }

            static _context_binding<void>* unbound_context_call;
            void print_bad_opcode();
        };

        template <typename return_type, typename... param_types>
        // Constructor function since template parameter deduction doesn't happen for actual constructors
        // Also allocates memory for a new binding !
        _function_binding<return_type, param_types...>* _get_binding( return_type ( *function )( param_types... ) )
        {
            return new _function_binding<return_type, param_types...>( function );
        }

        template <typename object_type, typename return_type, typename... param_types>
        // Constructor function for bound method calls
        // Also allocates memory for a new binding !
        _method_binding<object_type, return_type, param_types...>* _get_binding( return_type ( object_type::*method )( param_types... ), object_type* object_reference )
        {
            return new _method_binding<object_type, return_type, param_types...>( object_reference, method );
        }

        template <typename return_type, typename... param_types>
        // Constructor function for unbound method calls that operate on an execution context
        // Also allocates memory for a new binding !
        _context_binding<return_type, param_types...>* _get_context_binding( return_type ( execution_context::*method )( param_types... ) )
        {
            return new _context_binding<return_type, param_types...>( method );
        }
    } // namespace script
} // namespace rnjin