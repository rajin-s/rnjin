/* *** ** *** ** *** ** *** *
 * Part of rnjin            *
 * (c) Rajin Shankar, 2019  *
 *        rajinshankar.com  *
 * *** ** *** ** *** ** *** */

#pragma once

#include "macro.hpp"
#include "containers.hpp"
#include "log.hpp"

namespace rnjin::core
{
    // Forward declaration of the event class
    template <typename... As>
    class event;

    // Base type of event handlers for external use
    // note: used so objects can store references to
    //       their owned event handlers without
    //       knowing their exact type
    class event_handler_base
    {
        public: // methods
        virtual ~event_handler_base() is_abstract{};
        virtual bool has_valid_event() is_abstract;
    };

    // Base type of event handlers as known by an event
    // note: used so the event can call the invoke method properly
    template <typename... As>
    class event_handler_args
    {
        public: // methods
        virtual ~event_handler_args() is_abstract{};

        virtual void invoke( As... args ) is_abstract;
        virtual void invalidate_event() is_abstract;
    };

    // Actual event handler type that stores an instance and an instance method
    // note: event handlers are invalidated with their target event is destroyed,
    //       but are not themselves destroyed (ie still take up memory)
    // note: invalidated event handlers owned by objects are destroyed periodically
    //       in event_receiver::delete_invalid_event_handlers()
    template <typename O, typename... As>
    class event_handler : public event_handler_base, public event_handler_args<As...>
    {
        private: // types
        using event_type  = event<As...>;
        using method_type = void ( O::* )( As... );

        public: // methods
        // Register this handler with the target event on creation
        event_handler( event_type& target_event, O* instance, method_type method ) : target_event( &target_event ), pass_member( instance ), pass_member( method )
        {
            target_event.add_handler( this );
        }
        // Remove this handler from the target event if it still exists
        ~event_handler()
        {
            if ( target_event != nullptr )
            {
                target_event->remove_handler( this );
            }
        }

        // Call the handlers method on its instance
        void invoke( As... args ) override
        {
            // instance->*method( args... );
            std::invoke( method, instance, args... );
        }

        // Clear the target event pointer so it isn't improperly dereferenced during destruction
        // note: called from the event's destructor (ie when a handler outlives its event)
        void invalidate_event()
        {
            target_event = nullptr;
        }

        // Check if this handler is still associated with an event
        // note: handlers without valid events can be freely deleted
        bool has_valid_event()
        {
            return target_event != nullptr;
        }

        private: // members
        event_type* target_event;
        O* instance;
        method_type method;
    };

    template <typename... As>
    class static_event_handler : public event_handler_base, public event_handler_args<As...>
    {
        private: // types
        using event_type    = event<As...>;
        using function_type = void( As... );

        public: // methods
        // Register this handler with the target event on creation
        static_event_handler( event_type& target_event, function_type function ) : target_event( &target_event ), pass_member( function )
        {
            target_event.add_handler( this );
        }
        // Remove this handler from the target event if it still exists
        ~static_event_handler()
        {
            if ( target_event != nullptr )
            {
                target_event->remove_handler( this );
            }
        }

        // Call the handlers method on its instance
        void invoke( As... args ) override
        {
            function( args... );
        }

        // Clear the target event pointer so it isn't improperly dereferenced during destruction
        // note: called from the event's destructor (ie when a handler outlives its event)
        void invalidate_event()
        {
            target_event = nullptr;
        }

        // Check if this handler is still associated with an event
        // note: handlers without valid events can be freely deleted
        bool has_valid_event()
        {
            return target_event != nullptr;
        }

        private: // members
        event_type* target_event;
        function_type function;
    };

    // An event source (ie publisher, delegate, etc.)
    template <typename... As>
    class event
    {
        private: // types
        using handler_type = event_handler_args<As...>;

        public: // methods
        event( const string& name ) : pass_member( name ) {}

        // Invalidate all handlers for this event when it is destroyed
        // note: invalidated handlers still reside in memory and need to be freed elsewhere
        ~event()
        {
            foreach ( handler_pointer : handler_pointers )
            {
                handler_pointer->invalidate_event();
            }
        }

        // Register a handler with this event
        // note: called from event_handler constructor
        void add_handler( handler_type* new_handler_pointer )
        {
            handler_pointers.insert( new_handler_pointer );
        }

        // Remove a handler from this event
        // note: called from event_handler destructor
        void remove_handler( handler_type* handler_pointer )
        {
            handler_pointers.erase( handler_pointer );
        }

        // Invoke all handlers associated with this event
        void send( As... args ) const
        {
            foreach ( handler_pointer : handler_pointers )
            {
                handler_pointer->invoke( args... );
            }
        }

        public: // accessors
        let& get_name get_value( name );

        private: // members
        string name;
        set<handler_type*> handler_pointers;
    };

    // A base helper class for types that will do lots of event handling
    class event_receiver
    {
        public: // methods
        event_receiver();
        ~event_receiver();

        // Create a new event handler for the target event
        // and store it as an owned event handlers of this event_receiver
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

        private: // members
        list<event_handler_base*> owned_event_handlers;
    };

    // A base helper class for types that need others to know about their lifetime
    class lifetime_events
    {
        public: // methods
        lifetime_events();
        ~lifetime_events();

        // events are accessible through the lifetime group
        // ie my_obj.lifetime.destroyed()
        group
        {
            public: // accessors
            let_mutable& destroyed get_mutable_value( destroyed_event );

            private: // members
            event<> destroyed_event{ "destroyed" };
        }
        lifetime;
    };
} // namespace rnjin::core