/* *** ** *** ** *** ** *** *
 * Part of rnjin            *
 * (c) Rajin Shankar, 2019  *
 *        rajinshankar.com  *
 * *** ** *** ** *** ** *** */

#pragma once

#include "macro.hpp"
#include "containers.hpp"
#include "event.hpp"

namespace rnjin::core
{
    // Forward declarations
    class object
    {
        public: // methods
        object();
        ~object();

        // Create a new event handler for the target event
        // and store it as an owned event handlers of this object
        template <typename O, typename... As>
        void handle_event( event<As...>& target_event, void ( O::*method )( As... ) )
        {
            event_handler<O, As...>* new_event_handler = new event_handler<O, As...>( target_event, (O*) this, method );
            owned_event_handlers.push_back( new_event_handler );

            // Clean up any invalid handler pointers whenever a new one is added
            // note: this could happen any time, but this seems to make sense
            delete_invalid_event_handlers();
        }

        // Go through all event handlers and free ones that don't have a
        // valid target event (ie handlers for destroyed events)
        void delete_invalid_event_handlers();

        group
        {
            public: // accessors
            event<>& on_destroyed get_mutable_value( destroyed );

            private: // members
            event<> destroyed{ "Destroyed" };
        }
        lifetime;

        private: // members
        list<event_handler_base*> owned_event_handlers;
    };
} // namespace rnjin::core

// foo = event<>("Foo")
// bar = event<int>("Bar")
// baz = event<transform&>("Baz")
// this->handle_event(foo, &this_class::foo_handler)
