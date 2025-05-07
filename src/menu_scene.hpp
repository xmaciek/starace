#pragma once

#include "model.hpp"
#include "update_context.hpp"
#include "render_context.hpp"
#include "space_dust.hpp"

class MenuScene {
    Texture m_uiAtlasTexture{};
    math::vec2 m_uiAtlasExtent{};
    math::vec4 m_uiSlice{};
    Model* m_model{};
    SpaceDust m_spaceDust{};

public:
    struct CreateInfo {
        Texture uiAtlasTexture{};
        math::vec2 uiAtlasExtent{};
        math::vec4 uiSlice{};
    };
    MenuScene() = default;
    MenuScene( const CreateInfo& );

    inline void setModel( Model* m ) { m_model = m; }
    void update( const UpdateContext& );
    void render( RenderContext ) const;

};
