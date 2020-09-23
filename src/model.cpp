#include "model.hpp"

#include "render_pipeline.hpp"

Model::~Model()
{
    destroyTexture( m_textureID );
}

void Model::render( RenderContext rctx ) const
{
    PushConstant<Pipeline::eTriangle3dTextureNormal> pushConstant{};
    pushConstant.m_model = rctx.model;
    pushConstant.m_view = rctx.view;
    pushConstant.m_projection = rctx.projection;

    PushBuffer<Pipeline::eTriangle3dTextureNormal> pushBuffer{ rctx.renderer->allocator() };
    pushBuffer.m_texture = m_textureID;
    pushBuffer.m_vertices.reserve( m_faces.size() * 3 );
    pushBuffer.m_normal.reserve( m_faces.size() * 3 );
    pushBuffer.m_uv.reserve( m_faces.size() * 3 );
    for ( const Face& it : m_faces ) {
        pushBuffer.m_vertices.emplace_back( it.vertex[ 0 ] );
        pushBuffer.m_vertices.emplace_back( it.vertex[ 1 ] );
        pushBuffer.m_vertices.emplace_back( it.vertex[ 2 ] );
        pushBuffer.m_normal.emplace_back( it.normal[ 0 ], it.normal[ 1 ], it.normal[ 2 ] );
        pushBuffer.m_normal.emplace_back( it.normal[ 0 ], it.normal[ 1 ], it.normal[ 2 ] );
        pushBuffer.m_normal.emplace_back( it.normal[ 0 ], it.normal[ 1 ], it.normal[ 2 ] );
        pushBuffer.m_uv.emplace_back( it.texcoord[ 0 ].u, it.texcoord[ 0 ].v );
        pushBuffer.m_uv.emplace_back( it.texcoord[ 1 ].u, it.texcoord[ 1 ].v );
        pushBuffer.m_uv.emplace_back( it.texcoord[ 2 ].u, it.texcoord[ 2 ].v );
    }
    rctx.renderer->push( &pushBuffer, &pushConstant );
}

void Model::draw() const
{
}

void Model::drawWireframe()
{
//     for ( const Face& f : m_faces ) {
//         glBegin( GL_LINES );
//         for ( size_t j = 0; j < f.vertex.size() - 1; j++ ) {
//             glVertex3d( f.vertex[ j ].x, f.vertex[ j ].y, f.vertex[ j ].z );
//             glVertex3d( f.vertex[ j + 1 ].x, f.vertex[ j + 1 ].y, f.vertex[ j + 1 ].z );
//         }
//         glEnd();
//     }
}

void Model::loadOBJ( const char* filename )
{
    m_faces.clear();
    std::vector<glm::vec3> vertices{};
    std::vector<uint32_t> tmpui{};
    std::vector<UV> tex{};
    glm::vec3 tmpv{};
    UV tmpt{};
    bool containsTex = false;
    uint8_t wID = 0;
    Face tmpf{};
    uint8_t dataType = 0;
    uint32_t v = 0;
    uint32_t t = 0;
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
            std::sscanf( line.c_str(), "v %f %f %f ", &tmpv.x, &tmpv.y, &tmpv.z );
            vertices.push_back( tmpv );
            switch ( dataType ) {
            case 3:
                if ( wID < 3 ) {
                    m_weapons[ wID ] = tmpv;
                    wID++;
                }
                break;
            case 4:
                m_thrusters.push_back( tmpv );
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
            m_faces.push_back( tmpf );
            tmpf.vertex.clear();
            tmpf.texcoord.clear();
            ssline.clear();
            tmpui.clear();
        }
    }
    OBJfile.close();

    normalizeSize();
}

void Model::calculateNormal()
{
    double vector1[ 3 ]{};
    double vector2[ 3 ]{};
    double length = 0.0f;
    for ( Face& f : m_faces ) {
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
        if ( length == 0 ) {
            length = 1.0f;
        }
        f.normal[ 0 ] /= length;
        f.normal[ 1 ] /= length;
        f.normal[ 2 ] /= length;
    }
}

void Model::normalizeSize()
{
    float maxZ = m_faces[ 0 ].vertex[ 0 ].z;
    float minZ = m_faces[ 0 ].vertex[ 0 ].z;

    for ( const Face& f : m_faces ) {
        for ( const glm::vec3& v : f.vertex ) {
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
    float total = maxZ + minZ;
    if ( total == 0 ) {
        total = 1.0f;
    }
    float factor = 1.0f / total;
    for ( Face& f : m_faces ) {
        for ( glm::vec3& v : f.vertex ) {
            v *= factor;
        }
    }
    for ( glm::vec3& v : m_thrusters ) {
        v *= factor;
    }

    for ( glm::vec3& v : m_weapons ) {
        v *= factor;
    }
}

void Model::bindTexture( uint32_t tex )
{
    if ( m_textureID != 0 ) {
        destroyTexture( m_textureID );
    }
    m_textureID = tex;
}

void Model::scale( float scale )
{
    for ( Face& f : m_faces ) {
        for ( glm::vec3& v : f.vertex ) {
            v *= scale;
        }
    }
    for ( glm::vec3& v : m_thrusters ) {
        v *= scale;
    }

    for ( glm::vec3& v : m_weapons ) {
        v *= scale;
    }
}

std::vector<glm::vec3> Model::thrusters() const
{
    return m_thrusters;
}

glm::vec3 Model::weapon( uint32_t i ) const
{
    return m_weapons[ i >= 3 ? 0 : i ];
}
