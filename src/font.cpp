#include "font.hpp"

#include "render_pipeline.hpp"

#include <glm/gtc/matrix_transform.hpp>

#include <algorithm>

Font::Font( std::string_view fontname, uint32_t h )
: m_name( fontname )
, m_textures( 128 )
, m_charData( 128 )
, m_height( h )
{
    TTF_Font* font = TTF_OpenFont( m_name.c_str(), h );

    for ( uint8_t i = 0; i < 128; i++ ) {
        makeDlist( font, i );
    }
    TTF_CloseFont( font );
}

Font::~Font()
{
    for ( auto it : m_textures ) {
        destroyTexture( it );
    }
}

static uint32_t pow2( uint32_t a )
{
    uint32_t r = 4;
    while ( r < a ) {
        r *= 2;
    }
    return r;
}

void Font::makeDlist( TTF_Font* font, uint32_t ch )
{
    SDL_Color col = { 255, 255, 255, 255 };
    SDL_Surface* tmp = TTF_RenderGlyph_Blended( font, static_cast<Uint16>( ch ), col );

    int32_t minX = 0;
    int32_t maxX = 0;
    int32_t minY = 0;
    int32_t maxY = 0;
    int32_t advance = 0;
    TTF_GlyphMetrics( font, ch, &minX, &maxX, &minY, &maxY, &advance );

    uint32_t optW = pow2( tmp->w );
    uint32_t optH = pow2( m_height );
    SDL_Rect rect;
    rect.x = minX;
    rect.y = m_height - maxY;

    SDL_Surface* expanded_data = SDL_CreateRGBSurface( tmp->flags, optW, optH, 32, 0xff000000, 0x00ff0000, 0x0000ff00, 0x000000ff );
    SDL_BlitSurface( tmp, nullptr, expanded_data, &rect );
    SDL_FreeSurface( tmp );

    std::vector<uint32_t> pixels( optH * optW );
    const uint32_t* pix = reinterpret_cast<const uint32_t*>( expanded_data->pixels );
    const uint32_t* pixEnd = pix;
    std::advance( pixEnd, optH * optW );
    std::transform( pix, pixEnd, pixels.begin(), []( uint32_t pix ) {
        return ( pix & 0xffffff00u ) | ( ( pix & 0xff00u ) >> 8 );
    } );

    m_textures[ ch ] = Renderer::instance()->createTexture( optW, optH, TextureFormat::eRGBA, reinterpret_cast<const uint8_t*>( pixels.data() ) );

    SDL_FreeSurface( expanded_data );
    m_middlePoint = optH / 2;
    m_charData[ ch ] = glm::vec3{ optW, optH, advance };
}

uint32_t Font::textLength( std::string_view text )
{
    uint32_t length = 0;
    for ( char i : text ) {
        length += m_charData[ static_cast<size_t>( i ) ].z;
    }
    return length;
}

uint32_t Font::height() const
{
    return m_height;
}

uint32_t Font::middlePoint() const
{
    return m_middlePoint;
}

void Font::renderText( RenderContext rctx, const glm::vec4& color, double x, double y, std::string_view text )
{
    rctx.model = glm::translate( rctx.model, glm::vec3{ x, y, 0.0 } );
    for ( char ch : text ) {
        PushBuffer<Pipeline::eGuiTextureColor1> pushBuffer{ rctx.renderer->allocator() };
        pushBuffer.m_texture = m_textures[ ch ];
        PushConstant<Pipeline::eGuiTextureColor1> pushConstant{};
        pushConstant.m_model = rctx.model;
        pushConstant.m_view = rctx.view;
        pushConstant.m_projection = rctx.projection;
        pushConstant.m_color = color;
        pushConstant.m_vertices[ 0 ] = glm::vec2{ 0.0f, m_charData[ ch ].y };
        pushConstant.m_vertices[ 1 ] = glm::vec2{ 0.0f, 0.0f };
        pushConstant.m_vertices[ 2 ] = glm::vec2{ m_charData[ ch ].x, 0.0f };
        pushConstant.m_vertices[ 3 ] = glm::vec2{ m_charData[ ch ].x, m_charData[ ch ].y };
        pushConstant.m_uv[ 0 ] = glm::vec2{ 0, 0 };
        pushConstant.m_uv[ 1 ] = glm::vec2{ 0, 1 };
        pushConstant.m_uv[ 2 ] = glm::vec2{ 1, 1 };
        pushConstant.m_uv[ 3 ] = glm::vec2{ 1, 0 };
        rctx.renderer->push( &pushBuffer, &pushConstant );
        rctx.model = glm::translate( rctx.model, glm::vec3{ m_charData[ ch ].z, 0.0f, 0.0f } );
    }
}
