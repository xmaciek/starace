#pragma once

#include "sa.hpp"

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

    Vertex weapon( uint32_t ) const;
    std::vector<Vertex> thrusters() const;
    void bindTexture( GLuint );
    void calculateNormal();
    void draw() const;
    void drawWireframe();
    void loadOBJ( const char* filename );
    void normalizeSize();
    void scale( GLfloat scale );
};

