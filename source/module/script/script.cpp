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
            program_counter++;

            // script_log << "op: " << s( opcode ) << " ";

            if ( opcode < opcodes_reserved_for_context )
            {
                context_binding* context_operation = context_bindings[opcode];
                context_operation->invoke( this, &source_script.data[program_counter] );
                program_counter += context_operation->get_params_size();
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
                    operation->invoke( &source_script.data[program_counter] );
                    program_counter += operation->get_params_size();
                }
            }
        }
        void execution_context::jump( int offset )
        {
            program_counter += offset;
        }
        void execution_context::jump_to( unsigned int position )
        {
            program_counter = position;
        }
        void execution_context::jump_if( int offset )
        {
            return;
        }
        void execution_context::jump_to_if( unsigned int position )
        {
            return;
        }
        void execution_context::create_var( void )
        {
            return;
        }
        void execution_context::delete_var( void )
        {
            return;
        }
        void execution_context::set_var( void )
        {
            return;
        }
        void execution_context::set_var_array( void )
        {
            return;
        }
        void execution_context::set_var_offset( unsigned char offset )
        {
            return;
        }
        void execution_context::create_static( void )
        {
            return;
        }
        void execution_context::delete_static( void )
        {
            return;
        }
        void execution_context::set_static( void )
        {
            return;
        }
        void execution_context::set_static_array( void )
        {
            return;
        }
        void execution_context::set_static_offset( void )
        {
            return;
        }
        void execution_context::set_or( void )
        {
            return;
        }
        void execution_context::set_and( void )
        {
            return;
        }
        void execution_context::set_not( void )
        {
            return;
        }
        void execution_context::set_sum( void )
        {
            return;
        }
        void execution_context::set_difference( void )
        {
            return;
        }
        void execution_context::set_product( void )
        {
            return;
        }
        void execution_context::set_quotient( void )
        {
            return;
        }
        void execution_context::set_remainder( void )
        {
            return;
        }
        void execution_context::set_exponent( void )
        {
            return;
        }
        void execution_context::set_dot( void )
        {
            return;
        }
        void execution_context::set_cross( void )
        {
            return;
        }
        void execution_context::set_norm( void )
        {
            return;
        }
        void execution_context::increment( void )
        {
            return;
        }
        void execution_context::decrement( void )
        {
            return;
        }
        void execution_context::print_value( void )
        {
            return;
        }
        void execution_context::print_absolute( void )
        {
            return;
        }

        // Default call for opcodes that aren't otherwise bound
        void execution_context::print_bad_opcode()
        {
            script_log.printf( get_text( bad_opcode ), { s( source_script.data[program_counter - 1] ) } );
        }
        _context_binding<void>* execution_context::unbound_context_call = _get_context_binding( &execution_context::print_bad_opcode );

        // 32 opcodes reserved for context bindings
        context_binding* execution_context::context_bindings[opcodes_reserved_for_context] = {
            /* 0x00 */ unbound_context_call,
            /* 0x01 */ unbound_context_call,
            /* 0x02 */ unbound_context_call,
            /* 0x03 */ unbound_context_call,
            /* 0x04 */ unbound_context_call,
            /* 0x05 */ unbound_context_call,
            /* 0x06 */ unbound_context_call,
            /* 0x07 */ unbound_context_call,
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
            /* 0x1E */ unbound_context_call,
            /* 0x1F */ unbound_context_call,
        };

        // namespace variables
        int _separator[1] = { 1 };
        log::source script_log( "rnjin.script", log::write_to_file::immediately );
    } // namespace script
} // namespace rnjin