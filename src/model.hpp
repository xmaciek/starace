#ifndef MODEL_H
#define MODEL_H
#include "sa.hpp"
#include "texture.hpp"

struct UV {
    GLfloat u = 0;
    GLfloat v = 0;
};

struct Face {
    std::vector<Vertex> vertex{};
    std::vector<UV> texcoord{};
    GLfloat normal[ 3 ]{};
};

class Model {
private:
    std::vector<Face> faces{};
    GLuint textureID = 0;

public:
    Model() = default;
    ~Model();

    Vertex weapons[ 3 ]{};
    std::vector<Vertex> thrusters{};
    void Draw();
    void DrawWireframe();
    void Load_OBJ( const char* filename );
    void CalculateNormal();
    void NormalizeSize();
    void Scale( GLfloat scale );
    void BindTexture( GLuint TX );
};

#endif
