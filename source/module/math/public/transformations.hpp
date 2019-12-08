/* *** ** *** ** *** ** *** *
 * Part of rnjin            *
 * (c) Rajin Shankar, 2019  *
 *        rajinshankar.com  *
 * *** ** *** ** *** ** *** */

#pragma once
#include <rnjin.hpp>

#include "vectors.hpp"

namespace rnjin::core::math
{

    /* -------------------------------------------------------------------------- */
    /*                           Affine Transformations                           */
    /* -------------------------------------------------------------------------- */

    class transform
    {
        public: // methods
        transform();
        ~transform();

        void set_position( float3 new_position );
        void set_rotation( float3 new_euler_angles );
        void set_scale( float3 new_scale );

        void translate( float3 offset );
        void scale( float3 amount );
        void rotate( float3 amount );

        void set_uniform_scale( float new_scale );
        void uniform_scale( float amount );

        public: // accessors
        let get_matrix get_value( matrix );
        let get_inverse_matrix get_value( inverse_matrix );

        let has_zero_scale get_value( ( scale_amount.x * scale_amount.y * scale_amount.z ) == 0 );

        private: // members
        float3 translation_amount;

        float3 rotation_degrees;
        float3 rotation_sin;
        float3 rotation_cos;

        float3 scale_amount;

        float4x4 matrix;
        float4x4 inverse_matrix;
    };

    /* -------------------------------------------------------------------------- */
    /*                         Projection Transformations                         */
    /* -------------------------------------------------------------------------- */

    class projection_base
    {
        public:
        virtual float4x4 get_matrix() const pure_virtual;
    };

    class perspective_projection : public projection_base
    {

        public: // methods
        perspective_projection();
        perspective_projection( float vertical_field_of_view, float near_plane_distance, float far_plane_distance );
        perspective_projection( float aspect_ratio, float field_of_view, float near_plane_distance, float far_plane_distance );

        ~perspective_projection();

        public: // inherited
        inline float4x4 get_matrix() const override
        {
            return matrix;
        }

        public: // accessors
        let get_aspect_ratio get_value( aspect_ratio );

        let get_vertical_field_of_view get_value( vertical_field_of_view );
        let get_near_plane_distance get_value( near_plane_distance );
        let get_far_plane_distance get_value( far_plane_distance );

        void set_field_of_view( float new_angle );
        void set_near_plane_distance( float new_near_plane_distance );
        void set_far_plane_distance( float new_far_plane_distance );

        void set_aspect_ratio( float new_ratio );

        private: // methods
        void update_matrix();

        private: // members
        float aspect_ratio;
        float vertical_field_of_view;

        float near_plane_distance;
        float far_plane_distance;

        float4x4 matrix;
    };

    class orthographic_projection : public projection_base
    {
        public: // methods
        orthographic_projection();
        orthographic_projection( float height );
        orthographic_projection( float height, float near_plane_distance, float far_plane_distance );
        orthographic_projection( float aspect_ratio, float height, float near_plane_distance, float far_plane_distance );

        ~orthographic_projection();

        public: // inherited
        inline float4x4 get_matrix() const override
        {
            return matrix;
        }

        public: // accessors
        let get_aspect_ratio get_value( aspect_ratio );

        let get_size get_value( world_size );
        let get_width get_value( world_size.x );
        let get_height get_value( world_size.y );

        void set_height( float new_height );
        void set_width( float new_width );
        void set_near_plane_distance( float new_near_plane_distance );
        void set_far_plane_distance( float new_far_plane_distance );

        void set_aspect_ratio( float new_ratio );

        private: // methods
        void update_matrix();

        private: // members
        float aspect_ratio;
        float2 world_size;

        float near_plane_distance;
        float far_plane_distance;

        float4x4 matrix;
    };
} // namespace rnjin::core::math