#include "model.hpp"
#include "sa.hpp"

Model::~Model()
{
    glDeleteTextures( 1, &textureID );
}

void Model::Draw() const
{
    glEnable( GL_LIGHTING );
    glDisable( GL_BLEND );
    glEnable( GL_TEXTURE_2D );
    glColor3f( 1, 1, 1 );
    glBindTexture( GL_TEXTURE_2D, textureID );
    for ( const Face& f : faces ) {
        glBegin( GL_POLYGON );
        glNormal3fv( f.normal );
        for ( size_t j = 0; j < f.vertex.size(); j++ ) {
            glTexCoord2f( f.texcoord[ j ].u, f.texcoord[ j ].v );
            glVertex3d( f.vertex[ j ].x, f.vertex[ j ].y, f.vertex[ j ].z );
        }
        glEnd();
    }
    glDisable( GL_LIGHTING );
    glDisable( GL_TEXTURE_2D );
    glEnable( GL_BLEND );
}

void Model::DrawWireframe()
{
    for ( const Face& f : faces ) {
        glBegin( GL_LINES );
        for ( size_t j = 0; j < f.vertex.size() - 1; j++ ) {
            glVertex3d( f.vertex[ j ].x, f.vertex[ j ].y, f.vertex[ j ].z );
            glVertex3d( f.vertex[ j + 1 ].x, f.vertex[ j + 1 ].y, f.vertex[ j + 1 ].z );
        }
        glEnd();
    }
}

void Model::Load_OBJ( const char* filename )
{
    std::cout << "loading model: " << filename << " ... ";

    Vertex tmpv{};
    std::vector<Vertex> vertices{};
    faces.clear();
    UV tmpt{};
    std::vector<UV> tex{};
    bool containsTex = false;
    GLubyte wID = 0;
    std::vector<GLuint> tmpui{};
    Face tmpf{};
    GLubyte dataType = 0;
    GLuint v = 0;
    GLuint t = 0;
    std::ifstream OBJfile( filename );
    std::string line{};
    std::string tmpline{};
    std::stringstream ssline{};
    while ( getline( OBJfile, line ) ) {
        //         cout<<" "<<line.c_str()<<"\n";
        if ( line.substr( 0, 2 ) == "o " ) {
            line.erase( 0, 2 );
            if ( strcmp( line.c_str(), "hull" ) == 0 ) {
                dataType = 1;
            }
            else {
                if ( strcmp( line.c_str(), "shield" ) == 0 ) {
                    dataType = 2;
                }
                else {
                    if ( strcmp( line.c_str(), "weapons" ) == 0 ) {
                        dataType = 3;
                    }
                    else {
                        if ( strcmp( line.c_str(), "thruster" ) == 0 ) {
                            dataType = 4;
                        }
                        else {
                            dataType = 0;
                        }
                    }
                }
            }
        }
        else if ( line.substr( 0, 2 ) == "v " ) {
            std::sscanf( line.c_str(), "v %lf %lf %lf ", &tmpv.x, &tmpv.y, &tmpv.z );
            vertices.push_back( tmpv );
            switch ( dataType ) {
            case 3:
                if ( wID < 3 ) {
                    weapons[ wID ] = tmpv;
                    wID++;
                }
                break;
            case 4:
                thrusters.push_back( tmpv );
                break;
            default:
                break;
            }
        }
        else if ( line.substr( 0, 2 ) == "vt" ) {
            containsTex = true;
            std::sscanf( line.c_str(), "vt %f %f ", &tmpt.u, &tmpt.v );
            tex.push_back( tmpt );
        }
        else if ( line.substr( 0, 2 ) == "f " ) {
            line.erase( 0, 2 );
            ssline << line.c_str();
            while ( getline( ssline, tmpline, ' ' ) ) {
                if ( containsTex ) {
                    sscanf( tmpline.c_str(), "%i/%i", &v, &t );
                    tmpf.texcoord.push_back( tex[ t - 1 ] );
                }
                else {
                    sscanf( tmpline.c_str(), "%i", &v );
                }
                tmpf.vertex.push_back( vertices[ v - 1 ] );
            }
            faces.push_back( tmpf );
            tmpf.vertex.clear();
            tmpf.texcoord.clear();
            ssline.clear();
            tmpui.clear();
        }
    }
    OBJfile.close();

    std::cout << "done.\n";
    NormalizeSize();
}

void Model::CalculateNormal()
{
    std::cout << "| +-- calclulating normals... ";
    GLfloat vector1[ 3 ]{};
    GLfloat vector2[ 3 ]{};
    GLfloat length = 0.0f;
    for ( Face& f : faces ) {
        vector1[ 0 ] = f.vertex[ 0 ].x - f.vertex[ 1 ].x;
        vector1[ 1 ] = f.vertex[ 0 ].y - f.vertex[ 1 ].y;
        vector1[ 2 ] = f.vertex[ 0 ].z - f.vertex[ 1 ].z;

        //       vector1[0] = (faces[k].vertex[0] - faces[k].vertex[1]).x;

        vector2[ 0 ] = f.vertex[ 1 ].x - f.vertex[ 2 ].x;
        vector2[ 1 ] = f.vertex[ 1 ].y - f.vertex[ 2 ].y;
        vector2[ 2 ] = f.vertex[ 1 ].z - f.vertex[ 2 ].z;

        f.normal[ 0 ] = ( vector1[ 1 ] * vector2[ 2 ] ) - ( vector1[ 2 ] * vector2[ 1 ] );
        f.normal[ 1 ] = ( vector1[ 2 ] * vector2[ 0 ] ) - ( vector1[ 0 ] * vector2[ 2 ] );
        f.normal[ 2 ] = ( vector1[ 0 ] * vector2[ 1 ] ) - ( vector1[ 1 ] * vector2[ 0 ] );
        length = std::sqrt( ( f.normal[ 0 ] * f.normal[ 0 ] ) + ( f.normal[ 1 ] * f.normal[ 1 ] ) + ( f.normal[ 2 ] * f.normal[ 2 ] ) );
        if ( length < 0.0001f ) {
            length = 1.0f;
        }
        f.normal[ 0 ] /= length;
        f.normal[ 1 ] /= length;
        f.normal[ 2 ] /= length;
    }
    std::cout << "done.\n";
}

void Model::NormalizeSize()
{
    std::cout << "| +-- normalizing model... ";
    GLdouble maxZ = faces[ 0 ].vertex[ 0 ].z;
    GLdouble minZ = faces[ 0 ].vertex[ 0 ].z;

    for ( const Face& f : faces ) {
        for ( const Vertex& v : f.vertex ) {
            maxZ = std::max( maxZ, v.z );
            minZ = std::min( minZ, v.z );
        }
    }

    if ( maxZ < 0 ) {
        maxZ *= -1;
    }
    if ( minZ < 0 ) {
        minZ *= -1;
    }
    GLdouble total = maxZ + minZ;
    if ( total == 0 ) {
        total = 1.0f;
    }
    GLdouble factor = 1.0f / total;
    std::cout << "max: " << maxZ << " min: " << minZ << " ";
    std::cout << "factor: " << factor << " ... ";
    for ( Face& f : faces ) {
        for ( Vertex& v : f.vertex ) {
            v.x *= factor;
            v.y *= factor;
            v.z *= factor;
        }
    }
    for ( Vertex& v : thrusters ) {
        v.x *= factor;
        v.y *= factor;
        v.z *= factor;
    }

    for ( Vertex& v : weapons ) {
        v.x *= factor;
        v.y *= factor;
        v.z *= factor;
    }

    std::cout << "done.\n";
}

void Model::BindTexture( GLuint TX )
{
    if ( textureID != 0 ) {
        glDeleteTextures( 1, &textureID );
    }
    textureID = TX;
}

void Model::Scale( GLfloat scale )
{
    std::cout << "| +-- scaling model to " << scale << " ... ";
    for ( Face& f : faces ) {
        for ( Vertex& v : f.vertex ) {
            v.x *= scale;
            v.y *= scale;
            v.z *= scale;
        }
    }
    for ( Vertex& v : thrusters ) {
        v.x *= scale;
        v.y *= scale;
        v.z *= scale;
    }

    for ( Vertex& v : weapons ) {
        v.x *= scale;
        v.y *= scale;
        v.z *= scale;
    }

    std::cout << "done.\n";
}
