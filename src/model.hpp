#ifndef MODEL_H
#define MODEL_H
#include "sa.hpp"
#include "texture.hpp"

struct UV {
    UV()
    {
        u = v = 0;
    }
    GLfloat u;
    GLfloat v;
};

struct Face {
    std::vector<Vertex> vertex;
    std::vector<UV> texcoord;
    GLfloat normal[ 3 ];
};

class Model {
private:
    std::vector<Face> faces;
    GLuint i, j;
    GLuint textureID;

public:
    Model();
    ~Model();

    Vertex weapons[ 3 ];
    std::vector<Vertex> thrusters;
    void Draw();
    void DrawWireframe();
    void Load_OBJ( const char* filename );
    void CalculateNormal();
    void NormalizeSize();
    void Scale( GLfloat scale );
    void BindTexture( GLuint TX );
};

#endif
