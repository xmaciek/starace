#include "Model.h"
#include "SA.h"

using namespace std;

Model::Model() :
    m_vertices( 0 ),
    m_bufferID( 0 ),
    m_uvID( 0 )
{ }

Model::~Model()
{ }


void Model::Draw() {
    SHADER::pushMatrix();
    SHADER::setTextureCoord( m_uvID );
    SHADER::draw( GL_TRIANGLES, m_bufferID, m_vertices );
    SHADER::popMatrix();
}

static void lineToVertices( const std::string& line, std::vector<double>& vertices ) {
    double x, y, z;
    sscanf( line.c_str(), "v %lf %lf %lf ", &x, &y, &z );
    vertices.push_back( x );
    vertices.push_back( y );
    vertices.push_back( z );
}

static void lineToVec3( const std::string& line, std::vector<glm::vec3>& vector ) {
    glm::vec3 v;
    sscanf( line.c_str(), "v %f %f %f ", &v[0], &v[1], &v[2] );
    vector.push_back( v );
}


static uint32_t makeMappedBuffer( const std::vector<double>& v, const std::vector<uint32_t>& m ) {
    if ( v.empty() || m.empty() ) {
        return 0;
    }
    std::vector<double> array( m.size() * 3 );
    uint32_t j = 0;
    for ( uint32_t i = 0; i < m.size(); i++ ) {
        array[j++] = v[ m[i] * 3 + 0 ];
        array[j++] = v[ m[i] * 3 + 1 ];
        array[j++] = v[ m[i] * 3 + 2 ];
    }
    return SHADER::makeBuffer( &array[0], array.size() );
}

static uint32_t makeMappedUVBuffer( const std::vector<double>& uv, const std::vector<uint32_t>& m ) {
    if ( uv.empty() || m.empty() ) {
        return 0;
    }
    std::vector<double> array( m.size() * 2 );
    uint32_t j = 0;
    for ( uint32_t i = 0; i < m.size(); i++ ) {
        array[j++] = uv[ m[i] * 2 + 0 ];
        array[j++] = uv[ m[i] * 2 + 1 ];
    }
    return SHADER::makeBuffer( &array[0], array.size() );
}

void Model::Load_OBJ( const std::string& fileName ) {
    fprintf( stdout, "Loading model: %s", fileName.c_str() );

    std::fstream modelFile( fileName, std::fstream::in );
    std::string line;
    std::vector<double> vertices;
    std::vector<uint32_t> verticesMap;
    std::vector<double> uv;
    std::vector<uint32_t> uvMap;
    uint8_t verticesDataMode = 0;
    bool modelContainsTexture = false;

    while ( getline( modelFile, line ) ) {
        const std::string subStr( line.substr( 0, 2 ) );
        if ( subStr == "v " ) {
            switch ( verticesDataMode ) {
                case 1: lineToVertices( line, vertices ); break;
                case 2: lineToVec3( line, m_weaponBanks ); break;
                case 3: lineToVec3( line, m_thrusterBanks ); break;
                default: break;
            }
        } else if ( subStr == "o " ) {
            line.erase( 0, 2 );
            if ( line == "hull" ) { verticesDataMode = 1; }
            else if ( line == "weapons" ) { verticesDataMode = 2; }
            else if ( line == "thruster") { verticesDataMode = 3; }
            else { verticesDataMode = 0; }
        } else if ( subStr == "vt" ) {
            modelContainsTexture = true;
            double u, v;
            sscanf( line.c_str(), "vt %lf %lf ", &u, &v );
            uv.push_back( u );
            uv.push_back( v );
        } else if ( subStr == "f " ) {
            uint32_t v1, v2, v3;
            if ( modelContainsTexture ) {
                uint32_t t1, t2, t3;
                sscanf( line.c_str(), "f %i/%i %i/%i %i/%i", &v1, &t1, &v2, &t2, &v3, &t3 );
                uvMap.push_back( t1 );
                uvMap.push_back( t2 );
                uvMap.push_back( t3 );
            } else {
                sscanf( line.c_str(), "f %i %i %i", &v1, &v2, &v3 );
            }
            verticesMap.push_back( v1 );
            verticesMap.push_back( v2 );
            verticesMap.push_back( v3 );
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

void Model::BindTexture( uint32_t ) {}

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
  
