#include <renderer/renderer.hpp>

#include <renderer/buffer.hpp>
#include <renderer/pipeline.hpp>

#include <GL/gl.h>
#include <GL/glu.h>
#include <glm/gtc/type_ptr.hpp>

#include <cassert>
#include <map>
#include <utility>

static Renderer* g_instance = nullptr;

Renderer* Renderer::instance()
{
    return g_instance;
}

SDL_WindowFlags Renderer::windowFlag()
{
    return SDL_WINDOW_OPENGL;
}

static bool operator < ( const Buffer& lhs, const Buffer& rhs ) noexcept
{
    return lhs.m_id < rhs.m_id;
}


class RendererGL : public Renderer {
    SDL_Window* m_window = nullptr;
    SDL_GLContext m_contextInit{};
    SDL_GLContext m_contextGL{};
    std::pmr::map<Buffer, std::pmr::vector<float>> m_bufferMap{};
    std::pmr::map<Buffer, std::pmr::vector<glm::vec2>> m_bufferMap2{};
    std::pmr::map<Buffer, std::pmr::vector<glm::vec3>> m_bufferMap3{};
    std::pmr::map<Buffer, std::pmr::vector<glm::vec4>> m_bufferMap4{};

    void maybeDeleteBuffer( const Buffer& );

public:
    virtual ~RendererGL() override;
    RendererGL( SDL_Window* );

    virtual Buffer createBuffer( std::pmr::vector<float>&&, Buffer::Lifetime ) override;
    virtual Buffer createBuffer( std::pmr::vector<glm::vec2>&&, Buffer::Lifetime ) override;
    virtual Buffer createBuffer( std::pmr::vector<glm::vec3>&&, Buffer::Lifetime ) override;
    virtual Buffer createBuffer( std::pmr::vector<glm::vec4>&&, Buffer::Lifetime ) override;
    virtual std::pmr::memory_resource* allocator() override;
    virtual Texture createTexture( uint32_t w, uint32_t h, Texture::Format, bool, const uint8_t* ) override;
    virtual void beginFrame() override;
    virtual void clear() override;
    virtual void deleteBuffer( const Buffer& ) override;
    virtual void deleteTexture( Texture ) override;
    virtual void present() override;
    virtual void push( void* buffer, void* constant ) override;
    virtual void setViewportSize( uint32_t w, uint32_t h ) override;
    virtual void submit() override;
};


Renderer* Renderer::create( SDL_Window* window )
{
    return new RendererGL( window );
}


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

static int typeToInternalFormat( Texture::Format e )
{
    switch ( e ) {
    case Texture::Format::eRGB: return 3;
    case Texture::Format::eRGBA: return 4;
    default:
        assert( !"unhandled enum" );
    }
    return 0;
}

static int typeToFormat( Texture::Format e )
{
    switch ( e ) {
    case Texture::Format::eRGB: return GL_RGB;
    case Texture::Format::eRGBA: return GL_RGBA;
    default:
        assert( !"unhandled enum" );
    }
    return 0;
}

RendererGL::~RendererGL()
{
    g_instance = nullptr;
    SDL_GL_DeleteContext( m_contextGL );
    SDL_GL_DeleteContext( m_contextInit );
}

RendererGL::RendererGL( SDL_Window* window )
: m_window{ window }
{
    assert( !g_instance );
    g_instance = this;

    m_contextGL = SDL_GL_CreateContext( window );

    SDL_GL_SetSwapInterval( 1 );
    SDL_GL_SetAttribute( SDL_GL_DOUBLEBUFFER, 1 );
    SDL_GL_SetAttribute( SDL_GL_SHARE_WITH_CURRENT_CONTEXT, 1 );
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


    glClearColor( 0, 0, 0, 1 );
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

    m_contextInit = SDL_GL_CreateContext( window );
}

void RendererGL::beginFrame()
{
    SDL_GL_MakeCurrent( m_window, m_contextGL );
}

std::pmr::memory_resource* RendererGL::allocator()
{
    return std::pmr::get_default_resource();
}

void RendererGL::setViewportSize( uint32_t w, uint32_t h )
{
    glViewport( 0, 0, w, h );
}

void RendererGL::clear()
{
    glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
}

void RendererGL::present()
{
    SDL_GL_SwapWindow( m_window );
}

void RendererGL::deleteTexture( Texture tex )
{
    uint32_t t = static_cast<uint32_t>( tex.m_data );
    glDeleteTextures( 1, &t );
}

void RendererGL::deleteBuffer( const Buffer& b )
{
    m_bufferMap.erase( b );
    m_bufferMap2.erase( b );
    m_bufferMap3.erase( b );
    m_bufferMap4.erase( b );
}

Buffer RendererGL::createBuffer( std::pmr::vector<float>&& vec, Buffer::Lifetime lft )
{
    const Buffer buffer{ reinterpret_cast<uint64_t>( vec.data() ), lft, Buffer::Status::eReady };
    m_bufferMap.emplace( std::make_pair( buffer, std::move( vec ) ) ) ;
    assert( !m_bufferMap[ buffer ].empty() );
    return buffer;
}

Buffer RendererGL::createBuffer( std::pmr::vector<glm::vec2>&& vec, Buffer::Lifetime lft )
{
    const Buffer buffer{ reinterpret_cast<uint64_t>( vec.data() ), lft, Buffer::Status::eReady };
    m_bufferMap2.emplace( std::make_pair( buffer, std::move( vec ) ) ) ;
    assert( !m_bufferMap2[ buffer ].empty() );
    return buffer;
}

Buffer RendererGL::createBuffer( std::pmr::vector<glm::vec3>&& vec, Buffer::Lifetime lft )
{
    const Buffer buffer{ reinterpret_cast<uint64_t>( vec.data() ), lft, Buffer::Status::eReady };
    m_bufferMap3.emplace( std::make_pair( buffer, std::move( vec ) ) );
    assert( !m_bufferMap3[ buffer ].empty() );
    return buffer;
}

Buffer RendererGL::createBuffer( std::pmr::vector<glm::vec4>&& vec, Buffer::Lifetime lft )
{
    const Buffer buffer{ reinterpret_cast<uint64_t>( vec.data() ), lft, Buffer::Status::eReady };
    m_bufferMap4.emplace( std::make_pair( buffer, std::move( vec ) ) );
    assert( !m_bufferMap4[ buffer ].empty() );
    return buffer;
}

void RendererGL::maybeDeleteBuffer( const Buffer& buf )
{
    if ( buf.m_lifetime == Buffer::Lifetime::eOneTimeUse ) {
        deleteBuffer( buf );
    }
}

Texture RendererGL::createTexture( uint32_t w, uint32_t h, Texture::Format fmt, bool genMips, const uint8_t* ptr )
{
    uint32_t textureID = 0;
    glGenTextures( 1, &textureID );
    glBindTexture( GL_TEXTURE_2D, textureID );
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
    if ( genMips ) {
        glHint( GL_GENERATE_MIPMAP_HINT, GL_NICEST );
        glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR );
        glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, 16.0f );
        gluBuild2DMipmaps( GL_TEXTURE_2D
            , typeToInternalFormat( fmt )
            , w
            , h
            , typeToFormat( fmt )
            , GL_UNSIGNED_BYTE
            , ptr
        );
    }
    else {
        glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
        glTexImage2D( GL_TEXTURE_2D
            , 0
            , typeToInternalFormat( fmt )
            , w
            , h
            , 0
            , typeToFormat( fmt )
            , GL_UNSIGNED_BYTE
            , ptr
        );
    }
    return Texture{ textureID };
}

void RendererGL::push( void* buffer, void* constant )
{
    switch ( *reinterpret_cast<Pipeline*>( buffer ) ) {
    case Pipeline::eLine3dStripColor: {
        auto* pushBuffer = reinterpret_cast<PushBuffer<Pipeline::eLine3dStripColor>*>( buffer );
        auto* pushConstant = reinterpret_cast<PushConstant<Pipeline::eLine3dStripColor>*>( constant );

        assert( pushBuffer->m_verticeCount <= pushConstant->m_vertices.size() );
        assert( pushBuffer->m_verticeCount <= pushConstant->m_colors.size() );
        ScopeEnable blend( GL_BLEND );
        ScopeEnable depthTest( GL_DEPTH_TEST );

        glPushMatrix();
        glMatrixMode( GL_PROJECTION );
        glLoadMatrixf( glm::value_ptr( pushConstant->m_projection ) );
        glMatrixMode( GL_MODELVIEW );
        glLoadMatrixf( glm::value_ptr( pushConstant->m_view * pushConstant->m_model ) );

        glLineWidth( pushBuffer->m_lineWidth );
        glBegin( GL_LINE_STRIP );
        for ( size_t i = 0; i < pushBuffer->m_verticeCount; ++i ) {
            glColor4fv( glm::value_ptr( pushConstant->m_colors[ i ] ) );
            glVertex3fv( glm::value_ptr( pushConstant->m_vertices[ i ] ) );
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

        glBindTexture( GL_TEXTURE_2D, static_cast<uint32_t>( pushBuffer->m_texture.m_data ) );
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

        glBindTexture( GL_TEXTURE_2D, static_cast<uint32_t>( pushBuffer->m_texture.m_data ) );
        glBegin( GL_TRIANGLE_FAN );
        glColor4f( 1, 1, 1, 1 );
        for ( size_t i = 0; i < pushConstant->m_vertices.size(); ++i ) {
            glTexCoord2fv( glm::value_ptr( pushConstant->m_uv[ i ] ) );
            glVertex3fv( glm::value_ptr( pushConstant->m_vertices[ i ] ) );
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
        for ( size_t i = 0; i < pushBuffer->m_verticeCount; ++i ) {
            glColor4fv( glm::value_ptr( pushConstant->m_colors[ i ] ) );
            glVertex3fv( glm::value_ptr( pushConstant->m_vertices[ i ] ) );
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
        for ( size_t i = 0; i < pushBuffer->m_verticeCount; ++i ) {
            glVertex3fv( glm::value_ptr( pushConstant->m_vertices[ i ] ) );
        }
        glEnd();
        glPopMatrix();
    } break;

    case Pipeline::eTriangle3dTextureNormal: {
        auto* pushBuffer = reinterpret_cast<PushBuffer<Pipeline::eTriangle3dTextureNormal>*>( buffer );
        auto* pushConstant = reinterpret_cast<PushConstant<Pipeline::eTriangle3dTextureNormal>*>( constant );

        const std::pmr::vector<float>& vertices = m_bufferMap[ pushBuffer->m_vertices ];
        assert( !vertices.empty() );

        ScopeEnable depthTest( GL_DEPTH_TEST );
        ScopeEnable lightning( GL_LIGHTING );
        ScopeEnable texture2d( GL_TEXTURE_2D );

        glPushMatrix();
        glMatrixMode( GL_PROJECTION );
        glLoadMatrixf( glm::value_ptr( pushConstant->m_projection ) );
        glMatrixMode( GL_MODELVIEW );
        glLoadMatrixf( glm::value_ptr( pushConstant->m_view * pushConstant->m_model ) );

        glBindTexture( GL_TEXTURE_2D, static_cast<uint32_t>( pushBuffer->m_texture.m_data ) );
        glBegin( GL_TRIANGLES );
        glColor4f( 1, 1, 1, 1 );
        const float* ptr = vertices.data();
        const size_t count = vertices.size() / 8;
        for ( size_t i = 0; i < count; ++i ) {
            const float* vert = ptr; std::advance( ptr, 3 );
            glTexCoord2fv( ptr ); std::advance( ptr, 2 );
            glNormal3fv( ptr ); std::advance( ptr, 3 );
            glVertex3fv( vert );
        }
        glEnd();
        glPopMatrix();

        maybeDeleteBuffer( pushBuffer->m_vertices );
    } break;

    case Pipeline::count:
        assert( !"not a pipeline" );
        break;
    }
}

void RendererGL::submit() { }
