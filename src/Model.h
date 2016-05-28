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

public:
    Model();
    ~Model();

    void BindTexture( uint32_t );
    void Draw();
    void Load_OBJ( const std::string& fileName );
    void CalculateNormal();
    void Scale(GLfloat scale);
};

#endif
