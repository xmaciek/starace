#include "Model.h"
#include "SA.h"

#include <algorithm>
#include <map>

using namespace std;

Model::Model() :
    m_vertices( 0 ),
    m_bufferID( 0 ),
    m_uvID( 0 )
{ }

Model::~Model()
{ }


void Model::Draw() {
    if ( !m_bufferID || !m_bufferID || !m_uvID ) {
        return;
    }
    SHADER::pushMatrix();
    glEnable( GL_DEPTH_TEST );
    SHADER::setTextureCoord( m_uvID );
    m_texture.use();
    SHADER::draw( GL_TRIANGLES, m_bufferID, m_vertices );
    glDisable( GL_DEPTH_TEST );
    SHADER::popMatrix();
}

static void lineToVertices( const std::string& line, std::vector<double>& vertices ) {
    double x, y, z;
    sscanf( line.c_str(), "v %lf %lf %lf ", &x, &y, &z );
    vertices.push_back( x );
    vertices.push_back( y );
    vertices.push_back( z );
}

// no assign operator to glm::vec3 ?
static void lineToVec3( const std::string& line, std::vector<glm::vec3> &vec ) {
    glm::vec3 v;
    sscanf( line.c_str(), "v %f %f %f ", &v[0], &v[1], &v[2] );
    vec.push_back( v );
}


static uint32_t makeMappedBuffer( const std::vector<double>& v, const std::vector<uint32_t>& m ) {
    if ( v.empty() || m.empty() ) {
        return 0;
    }
    std::vector<double> array;
    for ( std::vector<uint32_t>::const_iterator it = m.begin(); it != m.end(); ++it ) {
        array.push_back( v.at( ( *it - 1 ) * 3 + 0 ) );
        array.push_back( v.at( ( *it - 1 ) * 3 + 1 ) );
        array.push_back( v.at( ( *it - 1 ) * 3 + 2 ) );
    }
    return SHADER::makeBuffer( array );
}

static uint32_t makeMappedUVBuffer( const std::vector<double>& uv, const std::vector<uint32_t>& m ) {
    if ( uv.empty() || m.empty() ) {
        return 0;
    }
    std::vector<double> array;
    for ( std::vector<uint32_t>::const_iterator it = m.begin(); it != m.end(); ++it ) {
        array.push_back( uv.at( ( *it - 1 ) * 2 + 0 ) );
        array.push_back( uv.at( ( *it - 1 ) * 2 + 1 ) );
    }
    return SHADER::makeBuffer( array );
}

struct FileSection { enum Enum { Unknown, Hull, Weapons, Thrusters }; };
struct DataSection { enum Enum { Unknown, Ignore, Object, Vertex, UV, Face, FaceTexture, FaceTextureNormal }; };

static FileSection::Enum guessFileSection( const std::string& line )
{
    static std::map<std::string, FileSection::Enum> map;
    if ( map.empty() ) {
        map[ "hull" ] = FileSection::Hull;
        map[ "weapons" ] = FileSection::Weapons;
        map[ "thruster" ] = FileSection::Thrusters;
    }
    std::map<std::string, FileSection::Enum>::const_iterator it = map.find( line.substr( 2 ) );
    return ( it != map.end() ) ? it->second : FileSection::Unknown;
}

static DataSection::Enum guessDataSection( const std::string& line )
{
    assert( !line.empty() );
    switch ( line[0] ) {
        case '#':
        case 's':
        case 'u':
        case 'm': return DataSection::Ignore;
        case 'o': return DataSection::Object;
        case 'v':
            assert( line.size() > 2 );
            switch ( line[1] ) {
                case ' ': return DataSection::Vertex;
                case 't': return DataSection::UV;
                case 'n': return DataSection::Ignore; // ignore normals for now
                default: return DataSection::Unknown;
            }
        case 'f':
            switch ( std::count( line.begin(), line.end(), '/' ) ) {
                case 0: return DataSection::Face;
                case 3: return DataSection::FaceTexture;
                case 6: return DataSection::FaceTextureNormal;
                default: return DataSection::Ignore;
            }
        default: return DataSection::Unknown;
    }
}

void Model::Load_OBJ( const std::string& fileName ) {
    std::string line;
    std::vector<double> vertices;
    std::vector<uint32_t> verticesMap;
    std::vector<double> uv;
    std::vector<uint32_t> uvMap;
    m_weaponBanks.clear();
    m_thrusterBanks.clear();

    FileSection::Enum fileSection = FileSection::Unknown;

    std::fstream modelFile( fileName, std::fstream::in );
    while ( getline( modelFile, line ) ) {
        switch ( guessDataSection( line ) ) {
            case DataSection::Object:
                fileSection = guessFileSection( line );
                break;
            case DataSection::Vertex:
                switch ( fileSection ) {
                    case FileSection::Hull:
                        lineToVertices( line, vertices );
                        break;
                    case FileSection::Weapons:
                        lineToVec3( line, m_weaponBanks );
                        lineToVertices( line, vertices );
                        break;
                    case FileSection::Thrusters:
                        lineToVec3( line, m_thrusterBanks );
                        lineToVertices( line, vertices );
                        break;
                    default:
                        assert( !"Wrong file section in vertex parse" );
                        break;
                } break;
                case DataSection::UV: {
                    double u, v;
                    sscanf( line.c_str(), "vt %lf %lf", &u, &v );
                    uv.push_back( u );
                    uv.push_back( v );
                } break;
            case DataSection::Face: {
                uint32_t v1, v2, v3;
                sscanf( line.c_str(), "f %i %i %i", &v1, &v2, &v3 );
                verticesMap.push_back( v1 );
                verticesMap.push_back( v2 );
                verticesMap.push_back( v3 );
            } break;
            case DataSection::FaceTexture: {
                uint32_t v1, v2, v3;
                uint32_t t1, t2, t3;
                sscanf( line.c_str(), "f %i/%i %i/%i %i/%i", &v1, &t1, &v2, &t2, &v3, &t3 );
                uvMap.push_back( t1 );
                uvMap.push_back( t2 );
                uvMap.push_back( t3 );
                verticesMap.push_back( v1 );
                verticesMap.push_back( v2 );
                verticesMap.push_back( v3 );
            } break;
            case DataSection::FaceTextureNormal: {
                uint32_t v1, v2, v3;
                uint32_t t1, t2, t3;
                uint32_t n1, n2, n3;
                sscanf( line.c_str(), "f %i/%i/%i %i/%i/%i %i/%i/%i", &v1, &t1, &n1, &v2, &t2, &n2, &v3, &t3, &n3 );
                uvMap.push_back( t1 );
                uvMap.push_back( t2 );
                uvMap.push_back( t3 );
                verticesMap.push_back( v1 );
                verticesMap.push_back( v2 );
                verticesMap.push_back( v3 );
            } break;
            case DataSection::Ignore: break;
            default: assert( !"Unknown data section" ); break;
        }
    }
    modelFile.close();

    m_vertices = verticesMap.size();
    m_bufferID = makeMappedBuffer( vertices, verticesMap );
    m_uvID = makeMappedUVBuffer( uv, uvMap );
  }

  void Model::CalculateNormal() {
#if 0
    cout<<"| +-- calclulating normals... ";
    GLfloat wektor1[3], wektor2[3], length;
    for (GLuint k=0; k<faces.size(); k++) {
      wektor1[0] = faces[k].vertex[0].x - faces[k].vertex[1].x;
      wektor1[1] = faces[k].vertex[0].y - faces[k].vertex[1].y;
      wektor1[2] = faces[k].vertex[0].z - faces[k].vertex[1].z;
   
//       wektor1[0] = (faces[k].vertex[0] - faces[k].vertex[1]).x;
   
      wektor2[0] = faces[k].vertex[1].x - faces[k].vertex[2].x;
      wektor2[1] = faces[k].vertex[1].y - faces[k].vertex[2].y;
      wektor2[2] = faces[k].vertex[1].z - faces[k].vertex[2].z;
      
      faces[k].normal[0] = (wektor1[1]*wektor2[2]) - (wektor1[2]*wektor2[1]);
      faces[k].normal[1] = (wektor1[2]*wektor2[0]) - (wektor1[0]*wektor2[2]);
      faces[k].normal[2] = (wektor1[0]*wektor2[1]) - (wektor1[1]*wektor2[0]);
      length = (GLfloat)sqrt((faces[k].normal[0]*faces[k].normal[0]) + (faces[k].normal[1]*faces[k].normal[1]) + (faces[k].normal[2]*faces[k].normal[2]));
      if (length==0) { length = 1.0f; }
      faces[k].normal[0]/=length;
      faces[k].normal[1]/=length;
      faces[k].normal[2]/=length;
        
        
      
    }
    cout <<"done.\n";
#endif
  }

  void Model::Scale(GLfloat scale) {
#if 0
    cout << "| +-- scaling model to " << scale << " ... ";
    for (GLuint k=0; k<faces.size(); k++) {
      for (GLuint l=0; l<faces[k].vertex.size(); l++) { 
        faces[k].vertex[l].x *= scale; 
        faces[k].vertex[l].y *= scale; 
        faces[k].vertex[l].z *= scale;
      }
    }
    for (GLuint k=0; k<thrusters.size(); k++) {
      thrusters[k].x *= scale;
      thrusters[k].y *= scale;
      thrusters[k].z *= scale;
    }
    for (GLuint k=0; k<3; k++) {
      weapons[k].x *= scale;
      weapons[k].y *= scale;
      weapons[k].z *= scale;
    }
    
    cout << "done.\n";
#endif
  }
  
