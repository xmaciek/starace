#pragma once

#include <engine/update_context.hpp>
#include <engine/render_context.hpp>

#include <glm/vec3.hpp>
#include <glm/vec4.hpp>

#include <vector>
#include <memory_resource>

class SpaceDust
{
    std::pmr::vector<glm::vec4> m_particles{};

    glm::vec3 m_center{};
    glm::vec3 m_velocity{};
    float m_lineWidth = 2.0f;
    float m_range = 1.5f;

public:
    ~SpaceDust() noexcept = default;
    SpaceDust() noexcept;

    void setRange( float );
    void setLineWidth( float );
    void setVelocity( const glm::vec3& );
    void setCenter( const glm::vec3& );

    void update( const UpdateContext& );
    void render( RenderContext ) const;

};
