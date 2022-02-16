#include "obj.hpp"

#include <algorithm>
#include <fstream>
#include <iostream>
#include <string>
#include <string_view>
#include <vector>
#include <memory_resource>

struct Vec2{ float data[ 2 ]; };
struct Vec3{ float data[ 3 ]; };
struct Face {
    uint32_t vert;
    uint32_t uv;
    uint32_t norm;
};

int main( int argc, char** argv )
{
    if ( argc != 3 ) {
        std::cout << "[ FAIL ] invalid number of arguments" << std::endl;
        return 1;
    }

    std::filesystem::path src{ argv[ 1 ] };
    std::filesystem::path dst{ argv[ 2 ] };

    if ( std::filesystem::exists( dst ) ) {
        const std::filesystem::file_status dstStat = std::filesystem::status( dst );
        const std::filesystem::perms perm = dstStat.permissions();
        if ( ( perm & std::filesystem::perms::owner_write ) == std::filesystem::perms::none ) {
            std::cout << "[ FAIL ] file not writable " << dst << std::endl;
            return 1;
        }
    }

    std::ifstream ifs( src );
    if ( !ifs.is_open() ) {
        std::cout << "[ FAIL ] failed to open file " << src << std::endl;
        return 1;
    }

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
            if ( line.size() > sizeof( obj::Chunk::name ) + 2 ) {
                std::cout << "[ FAIL ] chunk name too long" << std::endl;
                return 1;
            }
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
            switch ( chunk->first.dataType ) {
            case obj::DataType::invalid:
            case obj::DataType::vtn:
                break;
            default:
                std::cout << "[ FAIL ] data type misamtch" << std::endl;
                return 1;
            }
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
            for ( auto ff : f ) {
                if ( ff.vert >= vertices.size() ) {
                    std::cout << "[ FAIL ] requested index out of vertice range: index " << ff.vert + 1 << std::endl;
                    return 1;
                }
                chunk->second.push_back( vertices[ ff.vert ].data[ 0 ] );
                chunk->second.push_back( vertices[ ff.vert ].data[ 1 ] );
                chunk->second.push_back( vertices[ ff.vert ].data[ 2 ] );
                if ( ff.uv >= uv.size() ) {
                    std::cout << "[ FAIL ] requested index out of uv range: index " << ff.uv + 1 << std::endl;
                    return 1;
                }
                chunk->second.push_back( uv[ ff.uv ].data[ 0 ] );
                chunk->second.push_back( uv[ ff.uv ].data[ 1 ] );
                if ( ff.norm >= normals.size() ) {
                    std::cout << "[ FAIL ] requested index out of normal range: index " << ff.norm + 1 << std::endl;
                    return 1;
                }
                chunk->second.push_back( normals[ ff.norm ].data[ 0 ] );
                chunk->second.push_back( normals[ ff.norm ].data[ 1 ] );
                chunk->second.push_back( normals[ ff.norm ].data[ 2 ] );
            }
        }
        else if ( sv == "p " ) {
            switch ( chunk->first.dataType ) {
            case obj::DataType::invalid:
            case obj::DataType::v:
                break;
            default:
                std::cout << "[ FAIL ] data type misamtch" << std::endl;
                return 1;
            }
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
        if ( it.second.size() > std::numeric_limits<uint32_t>::max() ) {
            std::cout << "[ FAIL ] float count too large, number exceeds max of uint32_t" << std::endl;
            return 1;
        }
        it.first.floatCount = static_cast<uint32_t>( it.second.size() );
    }

    obj::Header header{};
    header.chunkCount = static_cast<uint32_t>( dataOut.size() );

    std::ofstream ofs( dst, std::ios::binary );
    if ( !ofs.is_open() ) {
        std::cout << "[ FAIL ] failed to open " << dst << std::endl;
        return 1;
    }

    ofs.write( reinterpret_cast<const char*>( &header ), sizeof( header ) );
    for ( const auto& it : dataOut ) {
        ofs.write( reinterpret_cast<const char*>( &it.first ), sizeof( obj::Chunk ) );
        ofs.write( reinterpret_cast<const char*>( it.second.data() ), it.second.size() * sizeof( float ) );
    }
    return 0;
}
