#include "renderer.hpp"

#include "render_pipeline.hpp"


#include <GL/gl.h>
#include <SDL/SDL.h>
#include <glm/gtc/type_ptr.hpp>

struct ScopeEnable {
    uint32_t m_x;
    ScopeEnable( uint32_t x )
    : m_x{ x }
    {
        glEnable( x );
    }
    ~ScopeEnable()
    {
        glDisable( m_x );
    }
};

Renderer::Renderer()
{
    SDL_GL_SetAttribute( SDL_GL_ACCUM_ALPHA_SIZE, 0 );
    SDL_GL_SetAttribute( SDL_GL_ACCUM_BLUE_SIZE, 0 );
    SDL_GL_SetAttribute( SDL_GL_ACCUM_GREEN_SIZE, 0 );
    SDL_GL_SetAttribute( SDL_GL_ACCUM_RED_SIZE, 0 );
    SDL_GL_SetAttribute( SDL_GL_ALPHA_SIZE, 8 );
    SDL_GL_SetAttribute( SDL_GL_BLUE_SIZE, 8 );
    SDL_GL_SetAttribute( SDL_GL_BUFFER_SIZE, 32 );
    SDL_GL_SetAttribute( SDL_GL_DEPTH_SIZE, 16 );
    SDL_GL_SetAttribute( SDL_GL_GREEN_SIZE, 8 );
    SDL_GL_SetAttribute( SDL_GL_MULTISAMPLEBUFFERS, 1 );
    SDL_GL_SetAttribute( SDL_GL_MULTISAMPLESAMPLES, 2 );
    SDL_GL_SetAttribute( SDL_GL_RED_SIZE, 8 );

    glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
    glEnable( GL_CULL_FACE );
    glFrontFace( GL_CCW );

    const glm::vec4 lightAmbient{ 0.5f, 0.5f, 0.5f, 1.0f };
    const glm::vec4 lightDiffuse{ 0.8f, 0.8f, 0.8f, 1.0f };
    const glm::vec4 lightPosition{ 0, 1, 1, 1 };
    glMaterialfv( GL_FRONT, GL_AMBIENT, glm::value_ptr( lightAmbient ) );
    glMaterialfv( GL_FRONT, GL_DIFFUSE, glm::value_ptr( lightDiffuse ) );
    glLightfv( GL_LIGHT0, GL_AMBIENT, glm::value_ptr( lightAmbient ) );
    glLightfv( GL_LIGHT0, GL_DIFFUSE, glm::value_ptr( lightDiffuse ) );
    glLightfv( GL_LIGHT0, GL_POSITION, glm::value_ptr( lightPosition ) );
    glEnable( GL_LIGHT0 );
}

std::pmr::memory_resource* Renderer::allocator()
{
    return std::pmr::get_default_resource();
}

void Renderer::setViewportSize( uint32_t w, uint32_t h )
{
    glViewport( 0, 0, w, h );
}

void Renderer::clear()
{
    glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
}

void Renderer::present()
{
    SDL_GL_SwapBuffers();
}

void Renderer::push( void* buffer, void* constant )
{
    switch ( *reinterpret_cast<Pipeline*>( buffer ) ) {
    case Pipeline::eLine3dStripColor: {
        auto* pushBuffer = reinterpret_cast<PushBuffer<Pipeline::eLine3dStripColor>*>( buffer );
        auto* pushConstant = reinterpret_cast<PushConstant<Pipeline::eLine3dStripColor>*>( constant );

        ScopeEnable blend( GL_BLEND );
        ScopeEnable depthTest( GL_DEPTH_TEST );

        glPushMatrix();
        glMatrixMode( GL_PROJECTION );
        glLoadMatrixf( glm::value_ptr( pushConstant->m_projection ) );
        glMatrixMode( GL_MODELVIEW );
        glLoadMatrixf( glm::value_ptr( pushConstant->m_view * pushConstant->m_model ) );

        glLineWidth( pushBuffer->m_lineWidth );
        glBegin( GL_LINE_STRIP );
        for ( size_t i = 0; i < pushBuffer->m_vertices.size(); ++i ) {
            glColor4fv( glm::value_ptr( pushBuffer->m_colors[ i ] ) );
            glVertex3fv( glm::value_ptr( pushBuffer->m_vertices[ i ] ) );
        }
        glEnd();
        glPopMatrix();
    } break;

    case Pipeline::eTriangleFan2dTextureColor: {
        auto* pushBuffer = reinterpret_cast<PushBuffer<Pipeline::eTriangleFan2dTextureColor>*>( buffer );
        auto* pushConstant = reinterpret_cast<PushConstant<Pipeline::eTriangleFan2dTextureColor>*>( constant );

        ScopeEnable blend( GL_BLEND );
        ScopeEnable texture2d( GL_TEXTURE_2D );

        glPushMatrix();
        glMatrixMode( GL_PROJECTION );
        glLoadMatrixf( glm::value_ptr( pushConstant->m_projection ) );
        glMatrixMode( GL_MODELVIEW );
        glLoadMatrixf( glm::value_ptr( pushConstant->m_view * pushConstant->m_model ) );

        glBindTexture( GL_TEXTURE_2D, pushBuffer->m_texture );
        glBegin( GL_TRIANGLE_FAN );
        for ( size_t i = 0; i < pushBuffer->m_vertices.size(); ++i ) {
            glColor4fv( glm::value_ptr( pushBuffer->m_colors[ i ] ) );
            glTexCoord2fv( glm::value_ptr( pushBuffer->m_uv[ i ] ) );
            glVertex2fv( glm::value_ptr( pushBuffer->m_vertices[ i ] ) );
        }
        glEnd();
        glPopMatrix();

    } break;

    case Pipeline::eGuiTextureColor1: {
        auto* pushBuffer = reinterpret_cast<PushBuffer<Pipeline::eGuiTextureColor1>*>( buffer );
        auto* pushConstant = reinterpret_cast<PushConstant<Pipeline::eGuiTextureColor1>*>( constant );

        ScopeEnable blend( GL_BLEND );
        ScopeEnable texture2d( GL_TEXTURE_2D );

        glPushMatrix();
        glMatrixMode( GL_PROJECTION );
        glLoadMatrixf( glm::value_ptr( pushConstant->m_projection ) );
        glMatrixMode( GL_MODELVIEW );
        glLoadMatrixf( glm::value_ptr( pushConstant->m_view * pushConstant->m_model ) );

        glBindTexture( GL_TEXTURE_2D, pushBuffer->m_texture );
        glBegin( GL_TRIANGLE_FAN );
        glColor4fv( glm::value_ptr( pushConstant->m_color ) );
        for ( size_t i = 0; i < pushConstant->m_vertices.size(); ++i ) {
            glTexCoord2fv( glm::value_ptr( pushConstant->m_uv[ i ] ) );
            glVertex2fv( glm::value_ptr( pushConstant->m_vertices[ i ] ) );
        }
        glEnd();
        glPopMatrix();
    } break;

    case Pipeline::eTriangleFan3dTexture: {
        auto* pushBuffer = reinterpret_cast<PushBuffer<Pipeline::eTriangleFan3dTexture>*>( buffer );
        auto* pushConstant = reinterpret_cast<PushConstant<Pipeline::eTriangleFan3dTexture>*>( constant );

        ScopeEnable blend( GL_BLEND );
        ScopeEnable depthTest( GL_DEPTH_TEST );
        ScopeEnable texture2d( GL_TEXTURE_2D );

        glPushMatrix();
        glMatrixMode( GL_PROJECTION );
        glLoadMatrixf( glm::value_ptr( pushConstant->m_projection ) );
        glMatrixMode( GL_MODELVIEW );
        glLoadMatrixf( glm::value_ptr( pushConstant->m_view * pushConstant->m_model ) );

        glBindTexture( GL_TEXTURE_2D, pushBuffer->m_texture );
        glBegin( GL_TRIANGLE_FAN );
        glColor4f( 1, 1, 1, 1 );
        for ( size_t i = 0; i < pushBuffer->m_vertices.size(); ++i ) {
            glTexCoord2fv( glm::value_ptr( pushBuffer->m_uv[ i ] ) );
            glVertex3fv( glm::value_ptr( pushBuffer->m_vertices[ i ] ) );
        }
        glEnd();
        glPopMatrix();
    } break;

    case Pipeline::eTriangleFan3dColor: {
        auto* pushBuffer = reinterpret_cast<PushBuffer<Pipeline::eTriangleFan3dColor>*>( buffer );
        auto* pushConstant = reinterpret_cast<PushConstant<Pipeline::eTriangleFan3dColor>*>( constant );

        ScopeEnable blend( GL_BLEND );
        ScopeEnable depthTest( GL_DEPTH_TEST );

        glPushMatrix();
        glMatrixMode( GL_PROJECTION );
        glLoadMatrixf( glm::value_ptr( pushConstant->m_projection ) );
        glMatrixMode( GL_MODELVIEW );
        glLoadMatrixf( glm::value_ptr( pushConstant->m_view * pushConstant->m_model ) );

        glBegin( GL_TRIANGLE_FAN );
        for ( size_t i = 0; i < pushBuffer->m_vertices.size(); ++i ) {
            glColor4fv( glm::value_ptr( pushBuffer->m_colors[ i ] ) );
            glVertex3fv( glm::value_ptr( pushBuffer->m_vertices[ i ] ) );
        }
        glEnd();
        glPopMatrix();
    } break;

    case Pipeline::eLine3dColor1: {
        auto* pushBuffer = reinterpret_cast<PushBuffer<Pipeline::eLine3dColor1>*>( buffer );
        auto* pushConstant = reinterpret_cast<PushConstant<Pipeline::eLine3dColor1>*>( constant );

        ScopeEnable blend( GL_BLEND );
        ScopeEnable depthTest( GL_DEPTH_TEST );

        glPushMatrix();
        glMatrixMode( GL_PROJECTION );
        glLoadMatrixf( glm::value_ptr( pushConstant->m_projection ) );
        glMatrixMode( GL_MODELVIEW );
        glLoadMatrixf( glm::value_ptr( pushConstant->m_view * pushConstant->m_model ) );

        glLineWidth( pushBuffer->m_lineWidth );
        glBegin( GL_LINES );
        glColor4fv( glm::value_ptr( pushConstant->m_color ) );
        for ( const glm::vec3& it : pushBuffer->m_vertices ) {
            glVertex3fv( glm::value_ptr( it ) );
        }
        glEnd();
        glPopMatrix();
    } break;

    case Pipeline::eTriangle3dTextureNormal: {
        auto* pushBuffer = reinterpret_cast<PushBuffer<Pipeline::eTriangle3dTextureNormal>*>( buffer );
        auto* pushConstant = reinterpret_cast<PushConstant<Pipeline::eTriangle3dTextureNormal>*>( constant );

        ScopeEnable depthTest( GL_DEPTH_TEST );
        ScopeEnable lightning( GL_LIGHTING );
        ScopeEnable texture2d( GL_TEXTURE_2D );

        glPushMatrix();
        glMatrixMode( GL_PROJECTION );
        glLoadMatrixf( glm::value_ptr( pushConstant->m_projection ) );
        glMatrixMode( GL_MODELVIEW );
        glLoadMatrixf( glm::value_ptr( pushConstant->m_view * pushConstant->m_model ) );

        glBindTexture( GL_TEXTURE_2D, pushBuffer->m_texture );
        glBegin( GL_TRIANGLES );
        glColor4f( 1, 1, 1, 1 );
        for ( size_t i = 0; i < pushBuffer->m_vertices.size(); ++i ) {
            glNormal3fv( glm::value_ptr( pushBuffer->m_normal[ i ] ) );
            glTexCoord2fv( glm::value_ptr( pushBuffer->m_uv[ i ] ) );
            glVertex3fv( glm::value_ptr( pushBuffer->m_vertices[ i ] ) );
        }
        glEnd();
        glPopMatrix();
    } break;
    }
}
