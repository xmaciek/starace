#include "model.hpp"
#include "sa.hpp"

using namespace std;

Model::Model()
{
    cout << "Creating default model.\n";
    textureID = 0;
}

Model::~Model()
{
    glDeleteTextures( 1, &textureID );
}

void Model::Draw()
{
    glEnable( GL_LIGHTING );
    glDisable( GL_BLEND );
    glEnable( GL_TEXTURE_2D );
    glColor3f( 1, 1, 1 );
    glBindTexture( GL_TEXTURE_2D, textureID );
    for ( i = 0; i < faces.size(); i++ ) {
        glBegin( GL_POLYGON );
        glNormal3f( faces[ i ].normal[ 0 ], faces[ i ].normal[ 1 ], faces[ i ].normal[ 2 ] );
        for ( j = 0; j < faces[ i ].vertex.size(); j++ ) {
            glTexCoord2f( faces[ i ].texcoord[ j ].u, faces[ i ].texcoord[ j ].v );
            //           cout<<faces[i].texcoord[j].v<<"\n";
            glVertex3d( faces[ i ].vertex[ j ].x, faces[ i ].vertex[ j ].y, faces[ i ].vertex[ j ].z );
        }
        glEnd();
    }
    glDisable( GL_LIGHTING );
    glDisable( GL_TEXTURE_2D );
    glEnable( GL_BLEND );
}

void Model::DrawWireframe()
{
    for ( i = 0; i < faces.size(); i++ ) {
        glBegin( GL_LINES );
        for ( j = 0; j < faces[ i ].vertex.size() - 1; j++ ) {
            glVertex3d( faces[ i ].vertex[ j ].x, faces[ i ].vertex[ j ].y, faces[ i ].vertex[ j ].z );
            glVertex3d( faces[ i ].vertex[ j + 1 ].x, faces[ i ].vertex[ j + 1 ].y, faces[ i ].vertex[ j + 1 ].z );
        }
        glEnd();
    }
}

void Model::Load_OBJ( const char* filename )
{
    cout << "loading model: " << filename << " ... ";

    Vertex tmpv;
    vector<Vertex> vertices;
    faces.clear();
    UV tmpt;
    vector<UV> tex;
    bool containsTex = false;
    GLubyte wID = 0;
    vector<GLuint> tmpui;
    Face tmpf;
    GLubyte dataType;
    GLuint v, t;
    fstream OBJfile( filename, ios::in );
    string line, tmpline;
    stringstream ssline;
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
            sscanf( line.c_str(), "v %lf %lf %lf ", &tmpv.x, &tmpv.y, &tmpv.z );
            vertices.push_back( tmpv );
            switch ( dataType ) {
            case 1:
                break;
            case 2:
                break;
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
            sscanf( line.c_str(), "vt %f %f ", &tmpt.u, &tmpt.v );
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

    cout << "done.\n";
    NormalizeSize();
}

void Model::CalculateNormal()
{
    cout << "| +-- calclulating normals... ";
    GLfloat wektor1[ 3 ], wektor2[ 3 ], length;
    for ( GLuint k = 0; k < faces.size(); k++ ) {
        wektor1[ 0 ] = faces[ k ].vertex[ 0 ].x - faces[ k ].vertex[ 1 ].x;
        wektor1[ 1 ] = faces[ k ].vertex[ 0 ].y - faces[ k ].vertex[ 1 ].y;
        wektor1[ 2 ] = faces[ k ].vertex[ 0 ].z - faces[ k ].vertex[ 1 ].z;

        //       wektor1[0] = (faces[k].vertex[0] - faces[k].vertex[1]).x;

        wektor2[ 0 ] = faces[ k ].vertex[ 1 ].x - faces[ k ].vertex[ 2 ].x;
        wektor2[ 1 ] = faces[ k ].vertex[ 1 ].y - faces[ k ].vertex[ 2 ].y;
        wektor2[ 2 ] = faces[ k ].vertex[ 1 ].z - faces[ k ].vertex[ 2 ].z;

        faces[ k ].normal[ 0 ] = ( wektor1[ 1 ] * wektor2[ 2 ] ) - ( wektor1[ 2 ] * wektor2[ 1 ] );
        faces[ k ].normal[ 1 ] = ( wektor1[ 2 ] * wektor2[ 0 ] ) - ( wektor1[ 0 ] * wektor2[ 2 ] );
        faces[ k ].normal[ 2 ] = ( wektor1[ 0 ] * wektor2[ 1 ] ) - ( wektor1[ 1 ] * wektor2[ 0 ] );
        length = (GLfloat)sqrt( ( faces[ k ].normal[ 0 ] * faces[ k ].normal[ 0 ] ) + ( faces[ k ].normal[ 1 ] * faces[ k ].normal[ 1 ] ) + ( faces[ k ].normal[ 2 ] * faces[ k ].normal[ 2 ] ) );
        if ( length == 0 ) {
            length = 1.0f;
        }
        faces[ k ].normal[ 0 ] /= length;
        faces[ k ].normal[ 1 ] /= length;
        faces[ k ].normal[ 2 ] /= length;
    }
    cout << "done.\n";
}

void Model::NormalizeSize()
{
    cout << "| +-- normalizing model... ";
    GLuint k, l;
    GLdouble maxZ = faces[ 0 ].vertex[ 0 ].z, minZ = faces[ 0 ].vertex[ 0 ].z;

    for ( k = 0; k < faces.size(); k++ ) {
        for ( l = 0; l < faces[ k ].vertex.size(); l++ ) {
            if ( faces[ k ].vertex[ l ].z > maxZ ) {
                maxZ = faces[ k ].vertex[ l ].z;
            }
            if ( faces[ k ].vertex[ l ].z < minZ ) {
                minZ = faces[ k ].vertex[ l ].z;
            }
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
    cout << "max: " << maxZ << " min: " << minZ << " ";
    cout << "factor: " << factor << " ... ";
    for ( k = 0; k < faces.size(); k++ ) {
        for ( l = 0; l < faces[ k ].vertex.size(); l++ ) {
            faces[ k ].vertex[ l ].x *= factor;
            faces[ k ].vertex[ l ].y *= factor;
            faces[ k ].vertex[ l ].z *= factor;
        }
    }
    for ( GLuint k = 0; k < thrusters.size(); k++ ) {
        thrusters[ k ].x *= factor;
        thrusters[ k ].y *= factor;
        thrusters[ k ].z *= factor;
    }

    for ( GLuint k = 0; k < 3; k++ ) {
        weapons[ k ].x *= factor;
        weapons[ k ].y *= factor;
        weapons[ k ].z *= factor;
    }

    cout << "done.\n";
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
    cout << "| +-- scaling model to " << scale << " ... ";
    for ( GLuint k = 0; k < faces.size(); k++ ) {
        for ( GLuint l = 0; l < faces[ k ].vertex.size(); l++ ) {
            faces[ k ].vertex[ l ].x *= scale;
            faces[ k ].vertex[ l ].y *= scale;
            faces[ k ].vertex[ l ].z *= scale;
        }
    }
    for ( GLuint k = 0; k < thrusters.size(); k++ ) {
        thrusters[ k ].x *= scale;
        thrusters[ k ].y *= scale;
        thrusters[ k ].z *= scale;
    }
    for ( GLuint k = 0; k < 3; k++ ) {
        weapons[ k ].x *= scale;
        weapons[ k ].y *= scale;
        weapons[ k ].z *= scale;
    }

    cout << "done.\n";
}
