#pragma once

#include "model.hpp"
#include "update_context.hpp"
#include "render_context.hpp"
#include "space_dust.hpp"

#include <ui/sprite.hpp>

class MenuScene {
    ui::Sprite m_background{};
    Model* m_model{};
    PipelineSlot m_pipeline{};
    SpaceDust m_spaceDust{};

public:
    struct CreateInfo {
        ui::Sprite background{};
        PipelineSlot pipeline{};
    };
    MenuScene() = default;
    MenuScene( const CreateInfo& );

    inline void setModel( Model* m ) { m_model = m; }
    void update( const UpdateContext& );
    void render( Renderer* renderer, math::vec2 viewport );

};
