#pragma once

#include "sa.hpp"

#include "render_context.hpp"

#include <glm/vec3.hpp>

struct UV {
    float u = 0;
    float v = 0;
};

struct Face {
    std::vector<glm::vec3> vertex{};
    std::vector<UV> texcoord{};
    double normal[ 3 ]{};
};

class Model {
private:
    std::vector<Face> m_faces{};
    std::vector<glm::vec3> m_thrusters{};
    glm::vec3 m_weapons[ 3 ]{};
    uint32_t m_textureID = 0;

public:
    ~Model();
    Model() = default;

    glm::vec3 weapon( uint32_t ) const;
    std::vector<glm::vec3> thrusters() const;
    void bindTexture( uint32_t );
    void calculateNormal();
    void render( RenderContext ) const;
    void draw() const;
    void drawWireframe();
    void loadOBJ( const char* filename );
    void normalizeSize();
    void scale( float scale );
};
