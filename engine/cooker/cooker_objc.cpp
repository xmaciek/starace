#include <cooker/common.hpp>

#include <extra/obj.hpp>
#include <extra/args.hpp>
#include <extra/gltf.hpp>
#include <config/config.hpp>
#include <ccmd/ccmd.hpp>
#include <shared/hash.hpp>

#include <algorithm>
#include <array>
#include <cassert>
#include <generator>
#include <memory_resource>
#include <string>
#include <string_view>
#include <vector>

using std::literals::string_view_literals::operator""sv;

static uint32_t format( std::string_view argSrc )
{
    if ( argSrc.ends_with( ".obj" ) ) return 'obj';
    if ( argSrc.ends_with( ".glb" ) ) return 'glb';
    cooker::error( "unknown source format, expected .obj .glb" );
    return 0;
}

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
    obj::VertexAssembly dataType = obj::VertexAssembly::invalid;
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

static void write( std::string_view argDst, std::span<const FullObject> objects )
{
    obj::Header header{};
    header.chunkCount = static_cast<uint32_t>( objects.size() );

    auto ofs = cooker::openWrite( argDst );
    cooker::write( ofs, header );
    std::ranges::for_each( objects, [&ofs]( const auto& f ) { ofs << f; } );
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

        auto data = cooker::readAll<char>( path );
        std::string_view view{ data.data(), data.size() };
        auto bytecode = ccmd::compile( view );
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
        if ( o.dataType == obj::VertexAssembly::invalid ) { o.dataType = obj::VertexAssembly::vnt; };
        if ( o.dataType != obj::VertexAssembly::vnt ) cooker::error( "mixing face with non-face element in object", o.name );

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
        if ( o.dataType == obj::VertexAssembly::invalid ) { o.dataType = obj::VertexAssembly::v; };
        if ( o.dataType != obj::VertexAssembly::v ) cooker::error( "mixing point with non-point element in object", o.name );
        uint32_t v = 0;
        ccmd::argv( vm, 0, v );
        if ( v == 0 ) cooker::error( "point cannot have index 0 in object", o.name );
        if ( v > instance->m_vertex.size() ) cooker::error( "accessing vertex in point out of declared vertexes in object", o.name );
        o.data.emplace_back( instance->m_vertex[ v - 1 ].data[ 0 ] );
        o.data.emplace_back( instance->m_vertex[ v - 1 ].data[ 1 ] );
        o.data.emplace_back( instance->m_vertex[ v - 1 ].data[ 2 ] );
        return 0;
    }

    static int process( std::string_view argSrc, std::string_view argDst )
    {
        Compiler compiler{};
        compiler.readObj( argSrc );
        write( argDst, compiler.m_objects );
        return 0;
    }
};

static auto readBulk( const cfg::Entry& e, auto&& parser )
{
    if ( !e ) cooker::error( "Failed to bulk read" );
    using Ret = decltype(parser(e));
    std::pmr::vector<Ret> ret( e.size() );
    std::ranges::transform( e.toArray(), ret.begin(), parser );
    return ret;
}

static gltf::BufferView parseBuffer( const cfg::Entry& e )
{
    gltf::BufferView b{};
    e[ "buffer"sv ].toValue( b.buffer );
    e[ "byteOffset"sv ].toValue( b.byteOffset );
    e[ "byteLength"sv ].toValue( b.byteLength );
    e[ "target"sv ].toValue( b.target );
    return b;
}

static gltf::Accessor parseAccessor( const cfg::Entry& e )
{
    gltf::Accessor a{};
    e[ "bufferView"sv ].toValue( a.bufferView );
    e[ "count"sv ].toValue( a.count );
    e[ "componentType"sv ].toValue( a.componentType );

    auto&& eType = e[ "type"sv ].toString();
    switch ( Hash{}( eType ) ) {
    case "VEC2"_hash: a.type = gltf::Type::eVec2; break;
    case "VEC3"_hash: a.type = gltf::Type::eVec3; break;
    case "SCALAR"_hash: a.type = gltf::Type::eScalar; break;
    default:
        cooker::error( "unhandled accessor enum type", e );
        break;
    }
    return a;
}

static gltf::Mesh parseMesh( const cfg::Entry& e )
{
    gltf::Mesh m{};
    e[ "name"sv ].toValue( m.name );
    if ( m.name.empty() ) cooker::error( "mesh has no name field" );

    auto&& arr = e[ "primitives"sv ].toArray();
    if ( arr.empty() ) return m;
    arr[ 0 ][ "indices"sv ].toValue( m.indices );

    auto&& attrib = arr[ 0 ][ "attributes"sv ];
    attrib[ "POSITION"sv ].toValue( m.position );
    if ( m.position < 0 ) cooker::error( "mesh has no POSITION field" );
    attrib[ "NORMAL"sv ].toValue( m.normal );
    attrib[ "TEXCOORD_0"sv ].toValue( m.texcoord0 );

    return m;
}

static auto&& fromIndex( auto& container, auto i )
{
    if ( i < 0 ) cooker::error( "index below 0" );
    size_t index = static_cast<size_t>( i );
    if ( index >= container.size() ) cooker::error( "index out of range" );
    return container[ index ];
}

enum Flags : int {
    eVertex = 0,
    fIndexed = 0b1,
    fNormal = 0b10,
    fTexcoord0 = 0b100,
    fTexcoord1 = 0b1000,

    eN0 = fNormal | fTexcoord0,
    eI0= fIndexed | fTexcoord0,
    eIN0 = fIndexed | fNormal | fTexcoord0,
    eIN01 = fIndexed | fNormal | fTexcoord0 | fTexcoord1,
};

constexpr Flags operator | ( Flags a, Flags b )
{
    return (Flags)((int)a | (int)b);
}
constexpr Flags operator |= ( Flags& a, Flags b )
{
    return a = ( a | b );
}

template <typename T> constexpr auto ComponentTypeType = gltf::ComponentType{};
template<> constexpr auto ComponentTypeType<float> = gltf::ComponentType::eFloat;
template<> constexpr auto ComponentTypeType<uint32_t> = gltf::ComponentType::eU32;
template<> constexpr auto ComponentTypeType<uint16_t> = gltf::ComponentType::eU16;


struct Swizzler {
    uint32_t vertexCount = 0;
    std::span<const uint16_t> indexes;
    std::span<const float> position;
    std::span<const float> normal;
    std::span<const float> texcoord0;
    std::span<const float> texcoord1;

    Swizzler( std::span<const uint16_t> i, std::span<const float> p )
    : indexes{ i }
    , position{ p }
    {
        if ( !indexes.empty() ) {
            vertexCount = (uint32_t)indexes.size();
        }
        else {
            vertexCount = (uint32_t)position.size() / 3;
        }
    }

    template <Flags F>
    std::generator<float> coro() const
    {
        for ( uint32_t i = 0; i < vertexCount; ++i ) {
            size_t col = ( F & fIndexed ) ? fromIndex( indexes, i ) : i;
            co_yield fromIndex( position, col * 3 + 0 );
            co_yield fromIndex( position, col * 3 + 1 );
            co_yield fromIndex( position, col * 3 + 2 );
            if constexpr ( F & fNormal ) {
                co_yield fromIndex( normal, col * 3 + 0 );
                co_yield fromIndex( normal, col * 3 + 1 );
                co_yield fromIndex( normal, col * 3 + 2 );
            }
            if constexpr ( F & fTexcoord0 ) {
                co_yield fromIndex( texcoord0, col * 2 + 0 );
                co_yield fromIndex( texcoord0, col * 2 + 1 );
            }
            if constexpr ( F & fTexcoord1 ) {
                co_yield fromIndex( texcoord1, col * 2 + 0 );
                co_yield fromIndex( texcoord1, col * 2 + 1 );
            }
        }
        co_return;
    }

    template <Flags F>
    auto& exec( auto& ret, obj::VertexAssembly va )
    {
        ret.dataType = va;
        size_t count = 3;
        if constexpr ( F & fNormal ) count += 3;
        if constexpr ( F & fTexcoord0 ) count += 2;
        if constexpr ( F & fTexcoord1 ) count += 2;
        count *= vertexCount;
        ret.data.resize( count );
        std::ranges::copy( coro<F>(), ret.data.begin() );
        return ret;
    }
};


struct GltfCompiler {
    std::pmr::vector<std::pmr::vector<uint8_t>> m_bins;
    std::pmr::vector<gltf::BufferView> m_bufferViews;
    std::pmr::vector<gltf::Accessor> m_accessors;
    std::pmr::vector<gltf::Mesh> m_meshes;

    template <typename T>
    std::span<const T> accessorToRawData( int32_t i )
    {
        if ( i < 0 ) return {};
        auto&& accessor = fromIndex( m_accessors, i );
        if ( ComponentTypeType<T> != accessor.componentType ) cooker::error( "unexpected accessor component type", std::to_string( (uint32_t)accessor.componentType ) );
        auto&& bufferView = fromIndex( m_bufferViews, accessor.bufferView );
        auto&& bin = fromIndex( m_bins, bufferView.buffer );
        size_t end = (size_t)bufferView.byteOffset + (size_t)bufferView.byteLength;
        if ( end > bin.size() ) cooker::error( "bufferView endpoint out of range" );
        std::span span = bin;
        span = span.subspan( bufferView.byteOffset, bufferView.byteLength );
        return cooker::cast<T>( span );
    }

    FullObject convertMesh( const gltf::Mesh& mesh )
    {
        if ( mesh.position < 0 ) cooker::error( "mesh accessor for position below 0" );
        if ( mesh.name.empty() ) cooker::error( "mesh does not have name" );
        FullObject o;
        if ( mesh.name.size() >= std::size( o.name ) ) cooker::error( "mesh name cannot fit into obj.chunk.name field", mesh.name );
        std::ranges::copy( mesh.name, std::begin( o.name ) );

        Flags flags{};
        if ( mesh.indices >= 0 ) flags |= fIndexed;
        if ( mesh.normal >= 0 ) flags |= fNormal;
        if ( mesh.texcoord0 >= 0 ) flags |= fTexcoord0;
        if ( mesh.texcoord1 >= 0 ) flags |= fTexcoord1;
        Swizzler swizzler{
            accessorToRawData<uint16_t>( mesh.indices ),
            accessorToRawData<float>( mesh.position )
        };
        swizzler.normal = accessorToRawData<float>( mesh.normal );
        swizzler.texcoord0 = accessorToRawData<float>( mesh.texcoord0 );
        swizzler.texcoord1 = accessorToRawData<float>( mesh.texcoord1 );

        switch ( flags ) {
        case eVertex: return swizzler.exec<eVertex>( o, obj::VertexAssembly::v );
        case fIndexed: return swizzler.exec<fIndexed>( o, obj::VertexAssembly::v );
        case eI0: return swizzler.exec<eI0>( o, obj::VertexAssembly::vt );
        case eIN0: return swizzler.exec<eIN0>( o, obj::VertexAssembly::vnt );
        case eIN01: return swizzler.exec<eIN01>( o, obj::VertexAssembly::vntt );
        case fTexcoord0: return swizzler.exec<fTexcoord0>( o, obj::VertexAssembly::vt );
        default: cooker::error( "unhandled mesh vertex assembly combination", std::to_string( flags ) );
        }
        return o;
    }

    int process( std::string_view argSrc, std::string_view argDst )
    {
        auto&& [ ifs, sizeLeft ] = cooker::openRead( argSrc );
        gltf::glb::Header header{};
        cooker::read( ifs, header );
        if ( header.magic != gltf::glb::Magic::eGltf ) cooker::error( "glb wrong header magic" );
        if ( header.version != 2 ) cooker::error( "glb wrong header version" );
        if ( header.size != sizeLeft ) cooker::error( "glb wrong header size" );
        sizeLeft -= sizeof( header );

        while ( sizeLeft ) {
            gltf::glb::Chunk chunk{};
            cooker::read( ifs, chunk );
            sizeLeft -= sizeof( chunk );
            if ( sizeLeft < chunk.size ) cooker::error( "not enoguh file data to process further glb chunks, .glb file possibly corrupted" );
            sizeLeft -= chunk.size;
            switch ( chunk.magic ) {
            default:
                cooker::error( "unknown glb chunk magic:", std::to_string( chunk.magic ) );

            case gltf::glb::Magic::eBin:
                m_bins.emplace_back( cooker::read( ifs, (size_t)chunk.size ) );
                continue;

            case gltf::glb::Magic::eJson: {
                auto blob = cooker::read( ifs, (size_t)chunk.size );
                auto root = cfg::Entry::fromJSON( blob );
                if ( !root ) cooker::error( "failed to parse JSON chunk" );
                m_bufferViews = readBulk( root[ "bufferViews"sv ], &parseBuffer );
                m_accessors = readBulk( root[ "accessors"sv ], &parseAccessor );
                m_meshes = readBulk( root[ "meshes"sv ], &parseMesh );
                continue;
            }
            }
        };

        std::pmr::vector<FullObject> objects;
        objects.resize( m_meshes.size() );
        std::ranges::transform( m_meshes, objects.begin(), [this]( auto& m ) { return convertMesh( m ); } );
        write( argDst, objects );
        return 0;
    }

};

int main( int argc, const char** argv )
{
    Args args{ argc, argv };
    if ( !args || args.read( "-h" ) || args.read( "--help" ) ) {
        std::cout <<
            "Required arguments:\n"
            "\t--src \"<file/path.obj>\" \u2012 specifies source object\n"
            "\t--dst \"<file/path.objc>\" \u2012 specifies output object\n"
            "\nOptional arguments:\n"
            "\t-h --help \u2012 prints this message\n"
            ;
    }

    std::string_view argSrc{};
    std::string_view argDst{};
    args.read( "--src", argSrc ) || cooker::error( "--src <file/path.obj> \u2012 argument not specified" );
    args.read( "--dst", argDst ) || cooker::error( "--dst <file/path.objc> \u2012 argument not specified" );
    switch ( format( argSrc ) ) {
    case 'obj': return Compiler::process( argSrc, argDst );
    case 'glb': return GltfCompiler{}.process( argSrc, argDst );
    }

    cooker::error( "unknown source format, expected .obj .glb" );
    return 1;
}
