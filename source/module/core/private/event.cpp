/* *** ** *** ** *** ** *** *
 * Part of rnjin            *
 * (c) Rajin Shankar, 2019  *
 *        rajinshankar.com  *
 * *** ** *** ** *** ** *** */

#include "event.hpp"

namespace rnjin::core
{
    event_receiver::event_receiver() {}
    event_receiver::~event_receiver()
    {
        // Free memory associated with this event_receiver's event handlers
        // note: event_handler destructor is called to remove association with target event
        for ( event_handler_base*& handler_pointer : owned_event_handlers )
        {
            delete handler_pointer;
            // handler_pointer = nullptr;
        }
    }
    // Free memory associated with this event_receiver's invalidated event handlers
    void event_receiver::delete_invalid_event_handlers()
    {
        for ( int i = 0; i < owned_event_handlers.size(); i++ )
        {
            event_handler_base* handler = owned_event_handlers[i];

            // Do a quick removal of invalidated events
            if ( not handler->has_valid_event() )
            {
                // Move last element into the invalid element's slot
                owned_event_handlers[i] = owned_event_handlers.back();

                // Remove the extra copy of the last element
                owned_event_handlers.pop_back();

                // Delete the invalidated handler
                delete handler;

                // Decrement the iterator so we re-visit the same index,
                // as it now holds a new value
                i--;
            }
        }
    }

    lifetime_events::lifetime_events() {}
    lifetime_events::~lifetime_events()
    {
        lifetime.destroyed().send();
    }
} // namespace rnjin::core