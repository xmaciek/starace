#include "cooker_common.hpp"

#include <extra/obj.hpp>

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

struct UV : public Vec2 {
    UV() = default;
    UV( std::string_view sv )
    {
        std::sscanf( sv.data(), "vt %f %f", data, data + 1 );
    }
};

struct Vertex : public Vec3 {
    Vertex() = default;
    Vertex( std::string_view sv )
    {
        std::sscanf( sv.data(), "v %f %f %f", data, data + 1, data + 2 );
    }
};


struct Normal : public Vec3 {
    Normal() = default;
    Normal( std::string_view sv )
    {
        std::sscanf( sv.data(), "vn %f %f %f", data, data + 1, data + 2 );
    }
};

struct Face {
    uint32_t vert[ 3 ]{};
    uint32_t uv[ 3 ]{};
    uint32_t norm[ 3 ]{};
    Face() = default;
    Face( std::string_view sv )
    {
        std::sscanf( sv.data(), "f %u/%u/%u %u/%u/%u %u/%u/%u"
            , vert, uv, norm
            , vert + 1, uv + 1, norm + 1
            , vert + 2, uv + 2, norm + 2
        );
    }
};

struct Point {
    uint32_t v{};
    Point() = default;
    Point( std::string_view sv )
    {
        std::sscanf( sv.data(), "p %u", &v );
    }
};

struct Object {
    char name[ 52 ]{};
    Object() = default;
    Object( std::string_view sv )
    {
        if ( sv.size() >= ( sizeof( name ) + 2 ) ) cooker::error( "object name too long", sv );
        std::copy( sv.begin() + 2, sv.end(), std::begin( name ) );
    }
};

struct Line {};

struct FullObject : public Object {
    std::pmr::vector<float> data{};
    obj::DataType dataType = obj::DataType::invalid;

    FullObject( const Object& o )
    : Object{ o }
    {
        data.reserve( 0xFFFu );
    }
};

std::ofstream& operator << ( std::ofstream& o, const FullObject& fo )
{
    o.write( reinterpret_cast<const char*>( &obj::Chunk::MAGIC ), sizeof( obj::Chunk::MAGIC ) );
    o.write( fo.name, sizeof( fo.name ) );
    o.write( reinterpret_cast<const char*>( &fo.dataType ), sizeof( fo.dataType ) );
    uint32_t s = (uint32_t)fo.data.size();
    o.write( reinterpret_cast<const char*>( &s ), sizeof( s ) );
    std::ranges::for_each( fo.data, [&o]( float f ) { o.write( reinterpret_cast<const char*>( &f ), sizeof( float ) ); } );
    return o;
}

struct Compiler {

    std::pmr::vector<Vec3> m_vertex{};
    std::pmr::vector<Vec2> m_uv{};
    std::pmr::vector<Vec3> m_normal{};
    std::pmr::vector<FullObject> m_objects{};

    // Blender hack:
    uint32_t verticeIndexForChunk = 0;

    Compiler()
    {
        m_vertex.reserve( 0xFFFu );
        m_uv.reserve( 0xFFFu );
        m_normal.reserve( 0xFFFu );
        m_objects.reserve( 0xFu );
    }

    void blenderHackMissingPoints()
    {
        auto idx = std::exchange( verticeIndexForChunk, (uint32_t)m_vertex.size() );
        if ( m_objects.empty() ) return;
        if ( !m_objects.back().data.empty() ) return;
        if ( idx == m_vertex.size() ) return;
        assert( idx < m_vertex.size() );
        for ( auto it = m_vertex.begin() + idx; it != m_vertex.end(); ++it ) {
            std::ranges::copy( it->data, std::back_inserter( m_objects.back().data ) );
        }
    }

    void operator () ( const Object& o )
    {
        blenderHackMissingPoints();
        m_objects.emplace_back( o );
    }
    void operator () ( const Vertex& v ) { m_vertex.emplace_back( v ); }
    void operator () ( const UV& uv ) { m_uv.emplace_back( uv ); }
    void operator () ( const Normal& n ) { m_normal.emplace_back( n ); }
    void operator () ( Line )
    {
        if ( m_objects.empty() ) cooker::error( "lines not supported", "" );
        cooker::error( "lines not supported in object", m_objects.back().name );
    }
    void operator () ( Face f )
    {
        if ( m_objects.empty() ) cooker::error( "requesting to add face to null object" );
        auto& o = m_objects.back();
        if ( o.dataType == obj::DataType::invalid ) { o.dataType = obj::DataType::vtn; };
        if ( o.dataType != obj::DataType::vtn ) cooker::error( "mixing face with non-face element in object", o.name );

        auto doVert = [size=m_vertex.size(), &o]( auto& u )
        {
            if ( u == 0 ) cooker::error( "vertex cannot have index 0 in object", o.name );
            if ( u > size ) cooker::error( "accessing vertex in face out of declared vertices in object", o.name );
            u--;
        };
        auto doUV = [size=m_uv.size(), &o]( auto& u )
        {
            if ( u == 0 ) cooker::error( "uv cannot have index 0 in object", o.name );
            if ( u > size ) cooker::error( "accessing uv in face out of declared ivs in object", o.name );
            u--;
        };
        auto doNormal = [size=m_normal.size(), &o]( auto& u )
        {
            if ( u == 0 ) cooker::error( "normal cannot have index 0 in object", o.name );
            if ( u > size ) cooker::error( "accessing normal in face out of declared normals in object", o.name );
            u--;
        };
        std::ranges::for_each( f.vert, doVert );
        std::ranges::for_each( f.uv, doUV );
        std::ranges::for_each( f.norm, doNormal );

        for ( uint32_t i = 0; i < 3; ++i ) {
            o.data.emplace_back( m_vertex[ f.vert[ i ] ].data[ 0 ] );
            o.data.emplace_back( -m_vertex[ f.vert[ i ] ].data[ 1 ] ); // Blender hack, negative value on y axis
            o.data.emplace_back( m_vertex[ f.vert[ i ] ].data[ 2 ] );
            o.data.emplace_back( m_uv[ f.uv[ i ] ].data[ 0 ] );
            o.data.emplace_back( m_uv[ f.uv[ i ] ].data[ 1 ] );
            o.data.emplace_back( m_normal[ f.norm[ i ] ].data[ 0 ] );
            o.data.emplace_back( m_normal[ f.norm[ i ] ].data[ 1 ] );
            o.data.emplace_back( m_normal[ f.norm[ i ] ].data[ 2 ] );
        }
    }
    void operator () ( const Point& p )
    {
        if ( m_objects.empty() ) cooker::error( "requesting to add point to null object" );
        auto& o = m_objects.back();
        if ( o.dataType == obj::DataType::invalid ) { o.dataType = obj::DataType::v; };
        if ( o.dataType != obj::DataType::v ) cooker::error( "mixing point with non-point element in object", o.name );

        if ( p.v == 0 ) cooker::error( "point cannot have index 0 in object", o.name );
        if ( p.v > m_vertex.size() ) cooker::error( "accessing vertex in point out of declared vertexes in object", o.name );
        o.data.emplace_back( m_vertex[ p.v - 1 ].data[ 0 ] );
        o.data.emplace_back( -m_vertex[ p.v - 1 ].data[ 1 ] );
        o.data.emplace_back( m_vertex[ p.v - 1 ].data[ 2 ] );
    }

};

int main( int argc, char** argv )
{
    ( argc == 3 ) || cooker::error( "expected 2 arguments: <src.obj> <dst.objc>", "" );

    std::filesystem::path src{ argv[ 1 ] };
    std::filesystem::path dst{ argv[ 2 ] };

    Compiler compiler{};
    std::string line{};
    line.reserve( 64 );

    std::ifstream ifs( src );
    ifs.is_open() || cooker::error( "failed to open file:", src );

    while ( std::getline( ifs, line ) ) {
        if ( line.size() < 3 ) { continue; }
        const std::string_view sv{ line.c_str(), 2 };
        if ( sv == "v " ) compiler( Vertex( line ) );
        else if ( sv == "o " ) compiler( Object( line ) );
        else if ( sv == "vt" ) compiler( UV( line ) );
        else if ( sv == "vn" ) compiler( Normal( line ) );
        else if ( sv == "f " ) compiler( Face( line ) );
        else if ( sv == "p " ) compiler( Point( line ) );
        else if ( sv == "l " ) compiler( Line{} );
    }
    ifs.close();
    compiler.blenderHackMissingPoints();

    obj::Header header{};
    header.chunkCount = static_cast<uint32_t>( compiler.m_objects.size() );

    std::ofstream ofs( dst, std::ios::binary );
    ofs.is_open() || cooker::error( "failed to open:", dst );

    ofs.write( reinterpret_cast<const char*>( &header ), sizeof( header ) );
    std::ranges::for_each( compiler.m_objects, [&ofs]( const auto& f ) { ofs << f; } );
    return 0;
}
