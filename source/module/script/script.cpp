/* *** ** *** ** *** ** *** *
 * Part of rnjin            *
 * (c) Rajin Shankar, 2019  *
 *        rajinshankar.com  *
 * *** ** *** ** *** ** *** */

#include "script.hpp"
#include <fstream>
#include <sstream>


namespace rnjin
{
    namespace script
    {
        // Output messages
        text( bad_script_file, "Failed to open script file '\1'!" );
        text( unbound_opcode, "Unbound opcode \1!" );
        text( bad_opcode, "Bad opcode \1!" );

        // runtime
        runtime::runtime() {}
        runtime::~runtime()
        {
            for ( int i = 0; i < opcode_count; i++ )
            {
                if ( bindings[i] != nullptr )
                {
                    delete bindings[i];
                }
            }
        }
        runtime::runtime( opcode_map operations )
        {
            set_bindings( operations );
        }
        void runtime::set_bindings( opcode_map operations )
        {
            for ( auto k : operations )
            {
                bindings[k.first - opcodes_reserved_for_context] = k.second;
            }
        }
        binding* runtime::get_binding( byte opcode )
        {
            return bindings[opcode - opcodes_reserved_for_context];
        }

        // compiled_script
        compiled_script::compiled_script( const string file_path )
        {
            std::ifstream file( file_path, std::ios::binary );
            if ( file.is_open() )
            {
                file.seekg( 0, std::ios::end );
                std::streampos size = file.tellg();
                file.seekg( 0, std::ios::beg );

                data.resize( size );
                file.read( (char*)&data[0], size );

                // script_log.printf( "'\1': size=\2", { file_path, s( size ) } );
                // script_log.print( as_string() );
            }
            else
            {
                script_log.printf( get_text( bad_script_file ), { file_path } );
            }
        }

        const string compiled_script::as_string()
        {
            std::stringstream ss;
            for ( const byte b : data )
            {
                ss << (char)b;
            }
            return ss.str();
        }

        // execution_context
        execution_context::execution_context( compiled_script& source_script, runtime& source_runtime ) : source_script( source_script ), source_runtime( source_runtime )
        {
            program_counter = 0;
        }
        bool execution_context::valid()
        {
            return ( &source_script && &source_runtime );
        }
        void execution_context::execute()
        {
            byte opcode = source_script.data[program_counter];
            program_counter += sizeof( byte );

            // script_log << "op: " << s( opcode ) << " ";

            if ( opcode < opcodes_reserved_for_context )
            {
                context_binding* context_operation = context_bindings[opcode];
                context_operation->invoke( this );
            }
            else
            {
                binding* operation = source_runtime.get_binding( opcode );
                if ( operation == nullptr )
                {
                    script_log.printf( get_text( unbound_opcode ), { s( opcode ) } );
                }
                else
                {
                    operation->invoke( this );
                }
            }
        }

        // Execution context bindings
        void execution_context::jump( int offset )
        {
            program_counter += offset;
        }
        void execution_context::jump_to( unsigned int position )
        {
            program_counter = position;
        }
        void execution_context::jump_if( int offset, any value )
        {
            return;
        }
        void execution_context::jump_to_if( unsigned int position, any value )
        {
            return;
        }
        void execution_context::create_var( any initial_value )
        {
            switch ( initial_value.var_type )
            {
                case any::type::byte_val:
                    allocate_var<byte>( initial_value.value.as_byte );
                    break;

                case any::type::int_val:
                    allocate_var<int>( initial_value.value.as_int );
                    break;

                case any::type::float_val:
                    allocate_var<float>( initial_value.value.as_float );
                    break;

                case any::type::double_val:
                    allocate_var<double>( initial_value.value.as_double );
                    break;

                default:
                    break;
            }
        }
        void execution_context::delete_var( type_info type )
        {
            switch ( type )
            {
                case any::type::byte_val:
                    free_var<byte>();
                    break;

                case any::type::int_val:
                    free_var<int>();
                    break;

                case any::type::float_val:
                    free_var<float>();
                    break;

                case any::type::double_val:
                    free_var<double>();
                    break;

                default:
                    break;
            }
        }
        void execution_context::set_var( var_pointer var, any value )
        {
            switch ( value.var_type )
            {
                case any::type::byte_ptr:
                case any::type::byte_val:
                    write_var( var, value.value.as_byte );
                    break;

                case any::type::int_ptr:
                case any::type::int_val:
                    write_var( var, value.value.as_int );
                    break;

                case any::type::float_ptr:
                case any::type::float_val:
                    write_var( var, value.value.as_float );
                    break;

                case any::type::double_ptr:
                case any::type::double_val:
                    write_var( var, value.value.as_double );
                    break;

                default:
                    break;
            }
        }
        void execution_context::set_var_array( void )
        {
            return;
        }
        void execution_context::set_var_offset( unsigned char offset )
        {
            return;
        }
        void execution_context::create_static( any initial_value )
        {
            return;
        }
        void execution_context::delete_static( type_info type )
        {
            return;
        }
        void execution_context::set_static( var_pointer var, any value )
        {
            return;
        }
        void execution_context::set_static_array( void )
        {
            return;
        }
        void execution_context::set_static_offset( unsigned char offset )
        {
            return;
        }
        void execution_context::set_or( var_pointer var, any a, any b )
        {
            return;
        }
        void execution_context::set_and( var_pointer var, any a, any b )
        {
            return;
        }
        void execution_context::set_not( var_pointer var, any a )
        {
            return;
        }
        void execution_context::set_sum( var_pointer var, any a, any b )
        {
            return;
        }
        void execution_context::set_difference( var_pointer var, any a, any b )
        {
            return;
        }
        void execution_context::set_product( var_pointer var, any a, any b )
        {
            return;
        }
        void execution_context::set_quotient( var_pointer var, any a, any b )
        {
            return;
        }
        void execution_context::set_remainder( var_pointer var, any a, any b )
        {
            return;
        }
        void execution_context::set_exponent( var_pointer var, any a, any b )
        {
            return;
        }
        void execution_context::set_dot( var_pointer var, any a, any b )
        {
            return;
        }
        void execution_context::set_cross( var_pointer var, any a, any b )
        {
            return;
        }
        void execution_context::set_norm( var_pointer var, any a, any b )
        {
            return;
        }
        void execution_context::increment( var_pointer var )
        {
            return;
        }
        void execution_context::decrement( var_pointer var )
        {
            return;
        }
        void execution_context::print_value( any value )
        {
            script_log << "(debug) ";
            switch ( value.var_type )
            {
                case any::type::byte_val:
                    script_log << s( value.value.as_byte );
                    break;

                case any::type::int_val:
                    script_log << s( value.value.as_int );
                    break;

                case any::type::float_val:
                    script_log << s( value.value.as_float );
                    break;

                case any::type::double_val:
                    script_log << s( value.value.as_double );
                    break;

                default:
                    break;
            }
            script_log << log::line();
        }
        void execution_context::print_absolute( any value )
        {
            return;
        }

        // Default call for opcodes that aren't otherwise bound
        _context_binding<void>* execution_context::unbound_context_call = _get_context_binding( &execution_context::print_bad_opcode );
        void execution_context::print_bad_opcode()
        {
            script_log.printf( get_text( bad_opcode ), { s( source_script.data[program_counter - 1] ) } );
        }

        // Null instruction
        void execution_context::do_nothing() {}

        // 32 opcodes reserved for context bindings
        context_binding* execution_context::context_bindings[opcodes_reserved_for_context] = {
            /* 0x00 */ _get_context_binding( &execution_context::do_nothing ),
            /* 0x01 */ _get_context_binding( &execution_context::jump ),
            /* 0x02 */ _get_context_binding( &execution_context::jump_to ),
            /* 0x03 */ unbound_context_call,
            /* 0x04 */ unbound_context_call,
            /* 0x05 */ _get_context_binding( &execution_context::create_var ),
            /* 0x06 */ _get_context_binding( &execution_context::delete_var ),
            /* 0x07 */ _get_context_binding( &execution_context::set_var ),
            /* 0x08 */ unbound_context_call,
            /* 0x09 */ unbound_context_call,
            /* 0x0A */ unbound_context_call,
            /* 0x0B */ unbound_context_call,
            /* 0x0C */ unbound_context_call,
            /* 0x0D */ unbound_context_call,
            /* 0x0E */ unbound_context_call,
            /* 0x0F */ unbound_context_call,
            /* 0x10 */ unbound_context_call,
            /* 0x11 */ unbound_context_call,
            /* 0x12 */ unbound_context_call,
            /* 0x13 */ unbound_context_call,
            /* 0x14 */ unbound_context_call,
            /* 0x15 */ unbound_context_call,
            /* 0x16 */ unbound_context_call,
            /* 0x17 */ unbound_context_call,
            /* 0x18 */ unbound_context_call,
            /* 0x19 */ unbound_context_call,
            /* 0x1A */ unbound_context_call,
            /* 0x1B */ unbound_context_call,
            /* 0x1C */ unbound_context_call,
            /* 0x1D */ unbound_context_call,
            /* 0x1E */ _get_context_binding( &execution_context::print_value ),
            /* 0x1F */ unbound_context_call,
        };

        // namespace variables
        int _separator[1] = { 1 };
        log::source script_log( "rnjin.script", log::write_to_file::immediately );
    } // namespace script
} // namespace rnjin