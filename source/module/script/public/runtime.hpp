/* *** ** *** ** *** ** *** *
 * Part of rnjin            *
 * (c) Rajin Shankar, 2019  *
 *        rajinshankar.com  *
 * *** ** *** ** *** ** *** */

#pragma once

#include "script.hpp"

#include "core/public/core.hpp"
#include "log/public/log.hpp"

namespace rnjin
{
    namespace script
    {
        extern log::source runtime_log;

        // Constants
        // Note: maybe make total opcodes dynamic per-runtime?
        constexpr size_t opcode_count                 = ( 1 << ( sizeof( opcode ) * 8 ) );
        constexpr size_t opcodes_reserved_for_context = 32;
        constexpr size_t opcodes_free_for_runtime     = opcode_count - opcodes_reserved_for_context;

        // Forward declarations
        class execution_context;

        namespace internal
        {
            // We need a way to separate two parameter packs... this seems to work but I'd guess there's a better way
            // Passed in function_binding->invoke(...) as __
            extern int __separator[1];

            // TODO: Maybe replace this with dynamically allocated? Or initial size + grow/shrink operations
            constexpr size_t stack_size     = 2048;
            constexpr size_t max_stack_vars = 512;

            // Basic binding structure
            struct binding_base
            {
                public:
                enum class type
                {
                    static_function,
                    instance_method,
                    context_method
                };

                virtual void invoke( execution_context* context ) = 0;

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

            // Static function call wrapper struct.
            // Construct with get_binding(...) for return/parameter type inference from provided function
            template <typename return_type, typename... input_types>
            struct function_binding : binding_base
            {
                public:
                // Function pointer type
                typedef return_type ( *function_type )( input_types... );

                // Create a new function wrapper (return and parameter types cannot be inferred here)
                function_binding( function_type function ) : function( function )
                {
                    params_size = sizeof...( input_types );
                    call_type   = type::static_function;
                }

                // Non-returning invoke for implenting binding_base interface
                void invoke( execution_context* context )
                {
                    _invoke( context );
                }

                // Intial public call, use parameter types from function itself
                return_type _invoke( execution_context* context )
                {
                    return _invoke<input_types...>( context );
                }

                private:
                // Function pointer
                function_type function;

                // Initial internal call, only specify types to get
                template <typename first, typename... rest>
                return_type _invoke( execution_context* context )
                {
                    return _invoke<rest..., 1, first>( context, __separator, context->read<first>() );
                }

                // Base case: no remaining values to get
                template <int _, typename... param_types>
                return_type _invoke( execution_context* context, int __[_], param_types... params )
                {
                    return function( params... );
                }

                // Base case: one remaining type_info to get
                template <typename first, int _, typename... param_types>
                return_type _invoke( execution_context* context, int __[_], param_types... params )
                {
                    return function( context, params..., context->read<first>() );
                }

                // General case: get one type_info and put it into params
                template <typename first, typename... rest, int _, typename... param_types>
                return_type _invoke( execution_context* context, int __[_], param_types... params )
                {
                    return _invoke<rest..., _, first, param_types...>( context, __, params..., context->read<first>() );
                }
            };

            // Specialization for functions with no parameters
            template <typename return_type>
            struct function_binding<return_type> : binding_base
            {
                public:
                typedef return_type ( *function_type )();

                function_binding( function_type function ) : function( function )
                {
                    params_size = 0;
                    call_type   = type::static_function;
                }

                void invoke( execution_context* context )
                {
                    function();
                }
                return_type _invoke( execution_context* context )
                {
                    return function();
                }

                private:
                function_type function;
            };

            // Method call struct bound to a particular object instance
            // Construct with get_binding(...) for return/parameter/object type inference from provided method and instance
            template <typename object_type, typename return_type, typename... input_types>
            struct method_binding : binding_base
            {
                public:
                // Function pointer type
                typedef return_type ( object_type::*method_type )( input_types... );

                method_binding( object_type* object_reference, method_type method ) : object_reference( object_reference ), method( method )
                {
                    params_size = sizeof...( input_types );
                    call_type   = type::instance_method;
                }

                void invoke( execution_context* context )
                {
                    _invoke( context );
                }

                return_type _invoke( execution_context* context )
                {
                    return _invoke<input_types...>( context );
                }

                private:
                object_type* object_reference;
                method_type method;

                // Initial internal call, only specify types to get
                template <typename first, typename... rest>
                return_type _invoke( execution_context* context )
                {
                    return _invoke<rest..., 1, first>( context, __separator, context->read<first>() );
                }

                // Base case: no remaining values to get
                template <int _, typename... param_types>
                return_type _invoke( execution_context* context, int __[_], param_types... params )
                {
                    return ( object_reference->*method )( params... );
                }

                // Base case: one remaining type_info to get
                template <typename first, int _, typename... param_types>
                return_type _invoke( execution_context* context, int __[_], param_types... params )
                {
                    return ( object_reference->*method )( params..., context->read<first>() );
                }

                // General case: get one type_info and put it into params
                template <typename first, typename... rest, int _, typename... param_types>
                return_type _invoke( execution_context* context, int __[_], param_types... params )
                {
                    return _invoke<rest..., _, first, param_types...>( context, __, params..., context->read<first>() );
                }
            };

            // Specialization for instance methods with no parameters
            template <typename object_type, typename return_type>
            struct method_binding<object_type, return_type> : binding_base
            {
                public:
                // Function pointer type
                typedef return_type ( object_type::*method_type )();

                method_binding( object_type* object_reference, method_type method ) : object_reference( object_reference ), method( method )
                {
                    params_size = 0;
                    call_type   = type::instance_method;
                }

                void invoke( execution_context* context )
                {
                    ( object_reference->*method )();
                }

                private:
                object_type* object_reference;
                method_type method;
            };

            struct context_binding_base : binding_base
            {
                virtual void invoke( execution_context* context ) = 0;
            };

            // Like a method binding that will be executed against the provided execution context
            // Construct with get_context_binding(...) for return/parameter type inference from provided method
            template <typename return_type, typename... input_types>
            struct context_binding : context_binding_base
            {
                public:
                // Function pointer type
                typedef return_type ( execution_context::*method_type )( input_types... );

                context_binding( method_type method ) : method( method )
                {
                    params_size = sizeof...( input_types );
                    call_type   = type::instance_method;
                }

                void invoke( execution_context* context )
                {
                    _invoke( context );
                }

                return_type _invoke( execution_context* context )
                {
                    _invoke<input_types...>( context );
                }

                private:
                method_type method;

                // Initial internal call, only specify types to get
                template <typename first, typename... rest>
                return_type _invoke( execution_context* context )
                {
                    return _invoke<rest..., 1, first>( context, __separator, context->read<first>() );
                }

                // Base case: no remaining values to get
                template <int _, typename... param_types>
                return_type _invoke( execution_context* context, int __[_], param_types... params )
                {
                    return ( context->*method )( params... );
                }

                // Base case: one remaining type_info to get
                template <typename first, int _, typename... param_types>
                return_type _invoke( execution_context* context, int __[_], param_types... params )
                {
                    return ( context->*method )( params..., context->read<first>() );
                }

                // General case: get one type_info and put it into params
                template <typename first, typename... rest, int _, typename... param_types>
                return_type _invoke( execution_context* context, int __[_], param_types... params )
                {
                    return _invoke<rest..., _, first, param_types...>( context, __, params..., context->read<first>() );
                }
            };

            // Specialization for context methods with no parameters
            template <typename return_type>
            struct context_binding<return_type> : context_binding_base
            {
                public:
                // Function pointer type
                typedef return_type ( execution_context::*method_type )();

                context_binding( method_type method ) : method( method )
                {
                    params_size = 0;
                    call_type   = type::context_method;
                }

                void invoke( execution_context* context )
                {
                    ( context->*method )();
                }

                private:
                method_type method;
            };

            template <typename return_type, typename... param_types>
            // Constructor function since template parameter deduction doesn't happen for actual constructors
            // Also allocates memory for a new binding !
            function_binding<return_type, param_types...>* get_binding( return_type ( *function )( param_types... ) )
            {
                return new function_binding<return_type, param_types...>( function );
            }

            template <typename object_type, typename return_type, typename... param_types>
            // Constructor function for bound method calls
            // Also allocates memory for a new binding !
            method_binding<object_type, return_type, param_types...>* get_binding( return_type ( object_type::*method )( param_types... ), object_type* object_reference )
            {
                return new method_binding<object_type, return_type, param_types...>( object_reference, method );
            }

            template <typename return_type, typename... param_types>
            // Constructor function for unbound method calls that operate on an execution context
            // Also allocates memory for a new binding !
            context_binding<return_type, param_types...>* get_context_binding( return_type ( execution_context::*method )( param_types... ) )
            {
                return new context_binding<return_type, param_types...>( method );
            }
        } // namespace internal

        // Type aliases
        typedef internal::binding_base binding;
        typedef internal::context_binding_base context_binding;
        typedef dictionary<opcode, binding*> opcode_map;
        typedef unsigned int instruction_pointer;

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
                    runtime_log.printf( "Can't bind to reserved opcode \1!", { s( instruction ) } );
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
                    runtime_log.printf( "Can't bind to reserved opcode \1!", { s( instruction ) } );
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

        // A compiled script's bytecode
        class compiled_script
        {
            public:
            // Script bytecode (little-endian)
            list<byte> data;

            compiled_script( const string file_path );
            const string as_string();
        };

        // Dynamic runtime data
        typedef short int var_pointer;
        struct any
        {
            public:
            enum class type
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
            // Single byte specifying type of a variable according to any::type enum.
            typedef unsigned char type_info;

            execution_context( compiled_script& script, runtime& runtime );

            // Check if the script and runtime are both valid.
            bool valid();

            // Execute the current instruction.
            void execute();

            void jump( int offset );
            void jump_to( unsigned int position );

            void jump_if( int offset, any value );
            void jump_to_if( unsigned int position, any value );

            void create_var( any initial_value );
            void delete_var( type_info type );
            void set_var( var_pointer var, any value );
            void set_var_array( void );
            void set_var_offset( unsigned char offset );

            void create_static( any initial_value );
            void delete_static( type_info type );
            void set_static( var_pointer var, any value );
            void set_static_array( void );
            void set_static_offset( unsigned char offset );

            void set_or( var_pointer var, any a, any b );
            void set_and( var_pointer var, any a, any b );
            void set_not( var_pointer var, any a );

            void set_sum( var_pointer var, any a, any b );
            void set_difference( var_pointer var, any a, any b );
            void set_product( var_pointer var, any a, any b );
            void set_quotient( var_pointer var, any a, any b );
            void set_remainder( var_pointer var, any a, any b );
            void set_exponent( var_pointer var, any a, any b );

            void set_dot( var_pointer var, any a, any b );
            void set_cross( var_pointer var, any a, any b );
            void set_norm( var_pointer var, any a, any b );

            void increment( var_pointer var );
            void decrement( var_pointer var );

            void print_value( any value );
            void print_absolute( any value );

            // Read a value of type T at the current instruction pointer, and advance it forward.
            template <typename T>
            T read()
            {
                type_info type = *( (type_info*) ( &source_script.data[program_counter] ) );
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

            // Read a lone type_info with no attached value.
            template <>
            type_info read<type_info>()
            {
                return read_value<type_info>();
            }

            // Read a lone var_ptr with no attached type
            template <>
            var_pointer read<var_pointer>()
            {
                return read_value<var_pointer>();
            }

            // Fill in an any struct based on the read type_info.
            // Note: always returns a value (will do pointer lookup internally)
            template <>
            any read<any>()
            {
                type_info type = *( (type_info*) ( &source_script.data[program_counter] ) );
                program_counter += sizeof( type_info );
                any result;

                switch ( (any::type) type )
                {
                    case any::type::byte_val:
                        result.var_type      = any::type::byte_val;
                        result.value.as_byte = read_value<byte>();
                        break;

                    case any::type::byte_ptr:
                        result.var_type      = any::type::byte_val;
                        result.value.as_byte = read_pointer<byte>();
                        break;

                    case any::type::int_val:
                        result.var_type     = any::type::int_val;
                        result.value.as_int = read_value<int>();
                        break;

                    case any::type::int_ptr:
                        result.var_type     = any::type::int_val;
                        result.value.as_int = read_pointer<int>();
                        break;

                    case any::type::float_val:
                        result.var_type       = any::type::float_val;
                        result.value.as_float = read_value<float>();
                        break;

                    case any::type::float_ptr:
                        result.var_type       = any::type::float_val;
                        result.value.as_float = read_pointer<float>();
                        break;

                    case any::type::double_val:
                        result.var_type        = any::type::double_val;
                        result.value.as_double = read_value<double>();
                        break;

                    case any::type::double_ptr:
                        result.var_type        = any::type::double_val;
                        result.value.as_double = read_pointer<double>();
                        break;

                    default:
                        break;
                }

                return result;
            }

            private:
            // Read a value from the source bytecode at the program counter, advancing it forward.
            template <typename T>
            T read_value()
            {
                T value = *( (T*) ( &source_script.data[program_counter] ) );
                program_counter += sizeof( T );
                return value;
            }

            // Read a variable number from the source bytecode at the program counter, advancing it forward.
            // Look up the variable in the context's variable stack.
            template <typename T>
            T read_pointer()
            {
                var_pointer var = *( (var_pointer*) ( &source_script.data[program_counter] ) );
                program_counter += sizeof( var_pointer );
                return lookup<T>( var );
            }

            static context_binding* context_bindings[opcodes_reserved_for_context];

            instruction_pointer program_counter;
            compiled_script& source_script;
            runtime& source_runtime;

            // Variables
            byte stack[internal::stack_size];
            byte* stack_top;

            void* vars[internal::max_stack_vars];
            unsigned int next_var;

            template <typename T>
            T lookup( var_pointer var )
            {
                T* pointer = (T*) ( vars[var] );
                return *pointer;
            }

            template <typename T>
            void allocate_var( T initial_value )
            {
                vars[next_var] = (void*) stack_top;
                stack_top += sizeof( T );
                next_var += 1;
            }

            template <typename T>
            void free_var()
            {
                stack_top -= sizeof( T );
                next_var -= 1;
                vars[next_var] = nullptr;
            }

            template <typename T>
            void write_var( var_pointer var, T value )
            {
                T* pointer = (T*) ( vars[var] );
                *pointer   = value;
            }

            static internal::context_binding<void>* unbound_context_call;
            void print_bad_opcode();
            void do_nothing();
        };
    } // namespace script
} // namespace rnjin