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
    std::vector<Face> m_faces{};
    std::vector<Vertex> m_thrusters{};
    Vertex m_weapons[ 3 ]{};
    GLuint m_textureID = 0;

public:
    ~Model();
    Model() = default;
    void BindTexture( GLuint TX );
    void CalculateNormal();
    void Draw() const;
    void DrawWireframe();
    void Load_OBJ( const char* filename );
    void NormalizeSize();
    void Scale( GLfloat scale );

    std::vector<Vertex> thrusters() const;
    Vertex weapon( uint32_t ) const;
};

#endif
