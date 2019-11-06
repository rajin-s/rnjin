/* *** ** *** ** *** ** *** *
 * Part of rnjin            *
 * (c) Rajin Shankar, 2019  *
 *        rajinshankar.com  *
 * *** ** *** ** *** ** *** */

#pragma once

#include "core.hpp"
#include "file.hpp"

namespace rnjin::core
{
    class resource
    {
        public: // types
        using id = unique_id<resource>;

        public: // methods
        resource();
        ~resource();

        void save() const;   // Save a resource that has an associated file path
        void force_reload(); // Load a resource that has an associated file path

        // Handle saving/loading of sub-resources
        void save_to( io::file& file ) const; 
        void load_from( io::file& file );

        void set_path( const string& new_path ); // Set the resource file path

        public: // accessors
        inline let get_id get_value( resource_id );
        let& get_path get_value( file_path );
        let has_file get_value( not file_path.empty() );
        let has_references get_value( reference_count > 0 );

        protected:
        // Virtual methods do that nothing for a base resource type
        virtual void write_data( io::file& file ) const;
        virtual void read_data( io::file& file );

        private: // enums
        enum class subresource_type
        {
            internal = 'i',
            external = 'e'
        };

        private: // members
        string file_path;
        id resource_id;

        uint reference_count;
        void add_reference();
        void remove_reference();

        public: // static members
        static group
        {
            public: // accessors
            let_mutable& resource_no_longer_referenced get_mutable_value( resource_no_longer_referenced_event );

            private: // members
            event<const resource&> resource_no_longer_referenced_event{ "last reference removed" };
        }
        events;

        // A reference that calls add/remove_reference on a given resource type when it is created/copied/destroyed
        public: // reference type
        template <typename T>
        class reference
        {
            private: // methods
            reference( T& target ) : pass_member( target )
            {
                target.add_reference();
            }

            public: // methods
            reference( reference& other ) : pass_member( other.target )
            {
                target.add_reference();
            }
            ~reference()
            {
                target.remove_reference();
            }

            // Get a mutable reference to the target resource
            inline T& get_mutable()
            {
                return target;
            }

            // Allow implicit conversion from a reference type to a const reference to the target resource
            inline operator const T&() const
            {
                return target;
            }

            private:
            T& target;
        };
    };
} // namespace rnjin::core