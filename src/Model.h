#ifndef MODEL_H
#define MODEL_H
#include "SA.h"
#include "Texture.h"


class Model {
private:
    uint32_t m_vertices;
    uint32_t m_bufferID;
    uint32_t m_uvID;
    std::vector<glm::vec3> m_weaponBanks, m_thrusterBanks;
    Texture m_texture;

public:
    Model();
    ~Model();

    inline void setTexture( const std::string& fileName ) { m_texture.load( fileName ); }
    void Draw();
    void Load_OBJ( const std::string& fileName );
    void CalculateNormal();
    void Scale(GLfloat scale);
};

#endif
