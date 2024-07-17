#include <extra/obj.hpp>

#include <array>
#include <algorithm>
#include <fstream>
#include <iostream>
#include <string>
#include <string_view>
#include <vector>
#include <memory_resource>

#define RED "\x1b[31m"
#define DEFAULT "\x1b[0m"

static constexpr char FAIL[] = "[ " RED "FAIL" DEFAULT " ] ";

[[noreturn]]
static bool exitOnFailed( std::string_view msg, auto arg )
{
    std::cout << FAIL << msg << " " << arg << std::endl;
    std::exit( 1 );
}

struct Vec2{ float data[ 2 ]; };
struct Vec3{ float data[ 3 ]; };
struct Face {
    uint32_t vert;
    uint32_t uv;
    uint32_t norm;
};

static bool testIntegrity( obj::DataType a, obj::DataType expected )
{
    if ( a == obj::DataType::invalid ) return true;
    return a == expected;
}

int main( int argc, char** argv )
{
    ( argc == 3 ) || exitOnFailed( "expected 2 arguments: <src.obj> <dst.objc>", "" );

    std::filesystem::path src{ argv[ 1 ] };
    std::filesystem::path dst{ argv[ 2 ] };

    std::ifstream ifs( src );
    ifs.is_open() || exitOnFailed( "failed to open file:", src );

    decltype( obj::load( {} ) ) dataOut{};
    decltype( dataOut )::value_type* chunk = nullptr;

    std::pmr::vector<Vec3> vertices{};
    std::pmr::vector<Vec2> uv{};
    std::pmr::vector<Vec3> normals{};

    vertices.reserve( 2000 );
    uv.reserve( 2000 );
    normals.reserve( 2000 );

    std::string line{};
    line.reserve( 64 );
    while ( std::getline( ifs, line ) ) {
        if ( line.size() < 3 ) { continue; }
        const std::string_view sv{ line.c_str(), 2 };
        if ( sv == "o " ) {
            ( line.size() <= sizeof( obj::Chunk::name ) + 2 ) || exitOnFailed( "object name too long to fit into Chunk.name field", line );
            dataOut.emplace_back();
            chunk = &dataOut.back();
            std::copy_n( line.c_str() + 2, line.size() - 2, chunk->first.name );
        }
        else if ( sv == "v " ) {
            Vec3& v = vertices.emplace_back();
            std::sscanf( line.c_str() + 2, "%f %f %f", v.data, v.data + 1, v.data + 2 );
            v.data[ 1 ] *= -1.0f; // blender fix
        }
        else if ( sv == "vt" ) {
            Vec2& v = uv.emplace_back();
            std::sscanf( line.c_str() + 3, "%f %f", v.data, v.data + 1 );
        }
        else if ( sv == "vn" ) {
            Vec3& v = normals.emplace_back();
            std::sscanf( line.c_str() + 3, "%f %f %f", v.data, v.data + 1, v.data + 2 );
        }
        else if ( sv == "f " ) {
            testIntegrity( chunk->first.dataType, obj::DataType::vtn ) || exitOnFailed( "data type mismatch, possibly interleaving f with l", chunk->first.name );
            chunk->first.dataType = obj::DataType::vtn;

            std::array<Face, 3> f{};
            std::sscanf( line.c_str() + 2, "%u/%u/%u %u/%u/%u %u/%u/%u"
                , &f[ 0 ].vert, &f[ 0 ].uv, &f[ 0 ].norm
                , &f[ 1 ].vert, &f[ 1 ].uv, &f[ 1 ].norm
                , &f[ 2 ].vert, &f[ 2 ].uv, &f[ 2 ].norm
            );
            f[ 0 ].vert--; f[ 0 ].uv--; f[ 0 ].norm--;
            f[ 1 ].vert--; f[ 1 ].uv--; f[ 1 ].norm--;
            f[ 2 ].vert--; f[ 2 ].uv--; f[ 2 ].norm--;
            for ( auto&& ff : f ) {
                ( ff.vert < vertices.size() ) || exitOnFailed( "requested index out of vertice range: index", ff.vert + 1 );
                chunk->second.push_back( vertices[ ff.vert ].data[ 0 ] );
                chunk->second.push_back( vertices[ ff.vert ].data[ 1 ] );
                chunk->second.push_back( vertices[ ff.vert ].data[ 2 ] );
                ( ff.uv < uv.size() ) || exitOnFailed( "requested index out of uv range: index", ff.uv + 1 );
                chunk->second.push_back( uv[ ff.uv ].data[ 0 ] );
                chunk->second.push_back( uv[ ff.uv ].data[ 1 ] );
                ( ff.norm < normals.size() ) || exitOnFailed( "requested index out of normal range: index", ff.norm + 1 );
                chunk->second.push_back( normals[ ff.norm ].data[ 0 ] );
                chunk->second.push_back( normals[ ff.norm ].data[ 1 ] );
                chunk->second.push_back( normals[ ff.norm ].data[ 2 ] );
            }
        }
        else if ( sv == "l " ) {
            testIntegrity( chunk->first.dataType, obj::DataType::v ) || exitOnFailed( "data type mismatch, possibly interleaving l with f", chunk->first.name );
            chunk->first.dataType = obj::DataType::v;
            uint32_t i = 0;
            uint32_t j = 0;
            std::sscanf( line.c_str() + 2, "%u %u", &i, &j );
            i--;
            j--;
            chunk->second.push_back( vertices[ i ].data[ 0 ] );
            chunk->second.push_back( vertices[ i ].data[ 1 ] );
            chunk->second.push_back( vertices[ i ].data[ 2 ] );
            chunk->second.push_back( vertices[ j ].data[ 0 ] );
            chunk->second.push_back( vertices[ j ].data[ 1 ] );
            chunk->second.push_back( vertices[ j ].data[ 2 ] );
        }
        else if ( sv == "p " ) {
            testIntegrity( chunk->first.dataType, obj::DataType::v ) || exitOnFailed( "data type mismatch, possibly interleaving p with f", chunk->first.name );
            chunk->first.dataType = obj::DataType::v;

            uint32_t idx = 0;
            std::sscanf( line.c_str() + 2, "%u", &idx );
            idx--;
            chunk->second.push_back( vertices[ idx ].data[ 0 ] );
            chunk->second.push_back( vertices[ idx ].data[ 1 ] );
            chunk->second.push_back( vertices[ idx ].data[ 2 ] );
        }
    }
    ifs.close();

    for ( auto& it : dataOut ) {
        ( it.second.size() <= std::numeric_limits<uint32_t>::max() )
            || exitOnFailed( "float count too large, number exceeds max of uint32_t:", it.second.size() );
        it.first.floatCount = static_cast<uint32_t>( it.second.size() );
    }

    obj::Header header{};
    header.chunkCount = static_cast<uint32_t>( dataOut.size() );

    std::ofstream ofs( dst, std::ios::binary );
    ofs.is_open() || exitOnFailed( "failed to open:", dst );

    ofs.write( reinterpret_cast<const char*>( &header ), sizeof( header ) );
    for ( const auto& it : dataOut ) {
        ofs.write( reinterpret_cast<const char*>( &it.first ), sizeof( obj::Chunk ) );
        ofs.write( reinterpret_cast<const char*>( it.second.data() ), static_cast<std::streamsize>( it.second.size() * sizeof( float ) ) );
    }
    return 0;
}
