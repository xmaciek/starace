#include <cooker/common.hpp>

#include <extra/obj.hpp>
#include <extra/args.hpp>
#include <ccmd/ccmd.hpp>

#include <array>
#include <algorithm>
#include <fstream>
#include <iostream>
#include <string>
#include <string_view>
#include <vector>
#include <memory_resource>
#include <cassert>

struct Vec2 { float data[ 2 ]{}; };
struct Vec3 { float data[ 3 ]{}; };

struct Face {
    uint32_t vert[ 3 ]{};
    uint32_t uv[ 3 ]{};
    uint32_t norm[ 3 ]{};
    Face() = default;
};

struct Point {
    uint32_t v{};
    Point() = default;
};

struct Object {
    char name[ 52 ]{};
    Object() = default;
};

struct FullObject : public Object {
    std::pmr::vector<float> data{};
    obj::DataType dataType = obj::DataType::invalid;
    FullObject()
    {
        data.reserve( 0xFFFu );
    }
};

std::ofstream& operator << ( std::ofstream& o, const FullObject& fo )
{
    cooker::write( o, obj::Chunk::MAGIC );
    cooker::write( o, fo.name );
    cooker::write( o, fo.dataType );
    cooker::write( o, (uint32_t)fo.data.size() );
    cooker::write( o, fo.data );
    return o;
}

struct Compiler {

    std::pmr::vector<Vec3> m_vertex{};
    std::pmr::vector<Vec2> m_uv{};
    std::pmr::vector<Vec3> m_normal{};
    std::pmr::vector<FullObject> m_objects{};

    // Blender hack:
    uint32_t m_verticeIndexForChunk = 0;

    Compiler()
    {
        m_vertex.reserve( 0xFFFu );
        m_uv.reserve( 0xFFFu );
        m_normal.reserve( 0xFFFu );
        m_objects.reserve( 0xFu );
    }

    void readObj( std::string_view path )
    {
        const std::array commands = {
            ccmd::Command{ "v", &Compiler::vertex, this },
            ccmd::Command{ "o", &Compiler::object, this },
            ccmd::Command{ "vt", &Compiler::uv, this },
            ccmd::Command{ "vn", &Compiler::normal, this },
            ccmd::Command{ "f", &Compiler::face, this },
            ccmd::Command{ "p", &Compiler::point, this },
            ccmd::Command{ "l", &Compiler::line, this },
            ccmd::Command{ []( ccmd::Vm*, void* ){ return 0u; }, nullptr },
        };

        auto data = cooker::readText( path );
        auto bytecode = ccmd::compile( data );
        if ( bytecode.empty() ) cooker::error( "failed to compile", path );
        auto ec = ccmd::run( commands, bytecode );
        if ( ec != ccmd::eSuccess ) cooker::error( "failed to parse", path );
        blenderHackMissingPoints();
    }

    void blenderHackMissingPoints()
    {
        auto idx = std::exchange( m_verticeIndexForChunk, (uint32_t)m_vertex.size() );
        if ( m_objects.empty() ) return;
        if ( !m_objects.back().data.empty() ) return;
        if ( idx == m_vertex.size() ) return;
        assert( idx < m_vertex.size() );
        for ( auto it = m_vertex.begin() + idx; it != m_vertex.end(); ++it ) {
            std::ranges::copy( it->data, std::back_inserter( m_objects.back().data ) );
        }
    }

    static uint32_t object( ccmd::Vm* vm, void* ctx )
    {
        assert( vm ); assert( ctx );
        auto instance = reinterpret_cast<Compiler*>( ctx );
        instance->blenderHackMissingPoints();
        auto& o = instance->m_objects.emplace_back();
        std::pmr::string name;
        if ( !ccmd::argv( vm, 0, name ) ) cooker::error( "cannot get object name" );
        if ( name.size() >= std::size( o.name ) ) cooker::error( "object name too long", name );
        auto it = std::copy( name.begin(), name.end(), std::begin( o.name ) );
        std::fill( it, std::end( o.name ), '\0' );
        return 0;
    }

    static uint32_t vertex( ccmd::Vm* vm, void* ctx )
    {
        assert( vm ); assert( ctx );
        auto instance = reinterpret_cast<Compiler*>( ctx );
        auto& v = instance->m_vertex.emplace_back();
        ccmd::argv( vm, 0, v.data[ 0 ] );
        ccmd::argv( vm, 1, v.data[ 1 ] );
        ccmd::argv( vm, 2, v.data[ 2 ] );
        v.data[ 1 ] *= -1.0f;
        return 0;
    }

    static uint32_t uv( ccmd::Vm* vm, void* ctx )
    {
        assert( vm ); assert( ctx );
        auto instance = reinterpret_cast<Compiler*>( ctx );
        auto& u = instance->m_uv.emplace_back();
        ccmd::argv( vm, 0, u.data[ 0 ] );
        ccmd::argv( vm, 1, u.data[ 1 ] );
        return 0;
    }

    static uint32_t normal( ccmd::Vm* vm, void* ctx )
    {
        assert( vm ); assert( ctx );
        auto instance = reinterpret_cast<Compiler*>( ctx );
        auto& n = instance->m_normal.emplace_back();
        ccmd::argv( vm, 0, n.data[ 0 ] );
        ccmd::argv( vm, 1, n.data[ 1 ] );
        ccmd::argv( vm, 2, n.data[ 2 ] );
        return 0;
    }

    static uint32_t line( [[maybe_unused]] ccmd::Vm* vm, void* ctx )
    {
        assert( vm ); assert( ctx );
        auto instance = reinterpret_cast<Compiler*>( ctx );
        if ( instance->m_objects.empty() ) cooker::error( "lines not supported", "" );
        cooker::error( "lines not supported in object", instance->m_objects.back().name );
        return 1;
    }

    static uint32_t face( ccmd::Vm* vm, void* ctx )
    {
        assert( vm ); assert( ctx );
        auto instance = reinterpret_cast<Compiler*>( ctx );

        if ( instance->m_objects.empty() ) cooker::error( "requesting to add face to null object" );
        auto& o = instance->m_objects.back();
        if ( o.dataType == obj::DataType::invalid ) { o.dataType = obj::DataType::vtn; };
        if ( o.dataType != obj::DataType::vtn ) cooker::error( "mixing face with non-face element in object", o.name );

        auto doVert = [size=instance->m_vertex.size(), &o]( auto& u )
        {
            if ( u == 0 ) cooker::error( "vertex cannot have index 0 in object", o.name );
            if ( u > size ) cooker::error( "accessing vertex in face out of declared vertices in object", o.name );
            u--;
        };
        auto doUV = [size=instance->m_uv.size(), &o]( auto& u )
        {
            if ( u == 0 ) cooker::error( "uv cannot have index 0 in object", o.name );
            if ( u > size ) cooker::error( "accessing uv in face out of declared ivs in object", o.name );
            u--;
        };
        auto doNormal = [size=instance->m_normal.size(), &o]( auto& u )
        {
            if ( u == 0 ) cooker::error( "normal cannot have index 0 in object", o.name );
            if ( u > size ) cooker::error( "accessing normal in face out of declared normals in object", o.name );
            u--;
        };
        auto parse = [&o]( auto& s, uint32_t& a, uint32_t& b, uint32_t& c )
        {
            if ( std::sscanf( s.c_str(), "%u/%u/%u", &a, &b, &c ) != 3 )
                cooker::error( "failed to parse face in object", o.name );
        };
        Face f{};
        std::pmr::string arg;
        ccmd::argv( vm, 0, arg ); parse( arg, f.vert[ 0 ], f.uv[ 0 ], f.norm[ 0 ] );
        ccmd::argv( vm, 1, arg ); parse( arg, f.vert[ 1 ], f.uv[ 1 ], f.norm[ 1 ] );
        ccmd::argv( vm, 2, arg ); parse( arg, f.vert[ 2 ], f.uv[ 2 ], f.norm[ 2 ] );

        std::ranges::for_each( f.vert, doVert );
        std::ranges::for_each( f.uv, doUV );
        std::ranges::for_each( f.norm, doNormal );

        for ( uint32_t i = 0; i < 3; ++i ) {
            o.data.emplace_back( instance->m_vertex[ f.vert[ i ] ].data[ 0 ] );
            o.data.emplace_back( instance->m_vertex[ f.vert[ i ] ].data[ 1 ] );
            o.data.emplace_back( instance->m_vertex[ f.vert[ i ] ].data[ 2 ] );
            o.data.emplace_back( instance->m_uv[ f.uv[ i ] ].data[ 0 ] );
            o.data.emplace_back( instance->m_uv[ f.uv[ i ] ].data[ 1 ] );
            o.data.emplace_back( instance->m_normal[ f.norm[ i ] ].data[ 0 ] );
            o.data.emplace_back( instance->m_normal[ f.norm[ i ] ].data[ 1 ] );
            o.data.emplace_back( instance->m_normal[ f.norm[ i ] ].data[ 2 ] );
        }
        return 0;
    }

    static uint32_t point( ccmd::Vm* vm, void* ctx )
    {
        assert( vm ); assert( ctx );
        auto instance = reinterpret_cast<Compiler*>( ctx );

        if ( instance->m_objects.empty() ) cooker::error( "requesting to add point to null object" );
        auto& o = instance->m_objects.back();
        if ( o.dataType == obj::DataType::invalid ) { o.dataType = obj::DataType::v; };
        if ( o.dataType != obj::DataType::v ) cooker::error( "mixing point with non-point element in object", o.name );
        uint32_t v = 0;
        ccmd::argv( vm, 0, v );
        if ( v == 0 ) cooker::error( "point cannot have index 0 in object", o.name );
        if ( v > instance->m_vertex.size() ) cooker::error( "accessing vertex in point out of declared vertexes in object", o.name );
        o.data.emplace_back( instance->m_vertex[ v - 1 ].data[ 0 ] );
        o.data.emplace_back( instance->m_vertex[ v - 1 ].data[ 1 ] );
        o.data.emplace_back( instance->m_vertex[ v - 1 ].data[ 2 ] );
        return 0;
    }

};

int main( int argc, const char** argv )
{
    Args args{ argc, argv };
    if ( !args || args.read( "-h" ) || args.read( "--help" ) ) {
        std::cout <<
            "Required arguments:\n"
            "\t--obj \"<file/path.obj>\" \u2012 specifies source object\n"
            "\t--dst \"<file/path.objc>\" \u2012 specifies output object\n"
            "\nOptional arguments:\n"
            "\t-h --help \u2012 prints this message\n"
            ;
    }

    std::string_view argObj{};
    std::string_view argDst{};
    args.read( "--obj", argObj ) || cooker::error( "--obj <file/path.obj> \u2012 argument not specified" );
    args.read( "--dst", argDst ) || cooker::error( "--dst <file/path.objc> \u2012 argument not specified" );

    Compiler compiler{};
    compiler.readObj( argObj );

    obj::Header header{};
    header.chunkCount = static_cast<uint32_t>( compiler.m_objects.size() );

    auto ofs = cooker::openWrite( argDst );
    cooker::write( ofs, header );
    std::ranges::for_each( compiler.m_objects, [&ofs]( const auto& f ) { ofs << f; } );
    return 0;
}
