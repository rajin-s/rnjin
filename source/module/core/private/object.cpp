/* *** ** *** ** *** ** *** *
 * Part of rnjin            *
 * (c) Rajin Shankar, 2019  *
 *        rajinshankar.com  *
 * *** ** *** ** *** ** *** */

#include "object.hpp"

namespace rnjin::core
{
    object::object() {}
    object::~object()
    {
        // Notify others that this object is being destroyed
        lifetime.on_destroyed().send();

        // Free memory associated with this object's
        // event handlers, calling their destructor to
        // disassociate them from their target events
        for ( event_handler_base*& handler_pointer : owned_event_handlers )
        {
            delete handler_pointer;
            // handler_pointer = nullptr;
        }
    }

    void object::delete_invalid_event_handlers()
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
} // namespace rnjin::core