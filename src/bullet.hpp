#ifndef SA_BULLET_H
#define SA_BULLET_H

#include "model.hpp"
#include "sa.hpp"
#include "saobject.hpp"
#include "texture.hpp"
#include "tail.hpp"

struct BulletProto {
    GLdouble x = 0.0f;
    GLdouble y = 0.0f;
    GLdouble z = 0.0f;
    GLdouble speed = 0.0f;
    GLdouble damage = 0.0f;
    GLdouble delay = 0.0f;
    GLuint type = 0;
    GLuint texture1 = 0;
    GLuint texture2 = 0;
    GLuint energy = 0;
    GLuint score_per_hit = 0;
    GLfloat color1[ 4 ] {};
    GLfloat color2[ 4 ] {};
};

class Bullet : public SAObject {
private:
    GLdouble max_range = 0.0f;
    GLdouble range = 0.0f;

    Tail m_tail{};
    GLdouble rotX = 0.0f;
    GLdouble rotY = 0.0f;
    GLdouble rotZ = 0.0f;
    GLuint type = 0;
    GLdouble damage = 0.0f;
    GLuint rotation = 0;
    GLfloat color1[ 4 ]{};
    GLfloat color2[ 4 ]{};

    virtual Vertex collisionRay() const;
    virtual bool collisionTest( const SAObject* object ) const;
    inline void Draw1();
    inline void Draw2();
    inline void DrawLaser();

public:
    Bullet( BulletProto bp );
    ~Bullet();
    void SetDirection( Vertex v );
    void Draw();
    void Update();
    GLuint getDamage();
    GLuint GetType();

    virtual void ProcessCollision( SAObject* object );

    static const GLuint SLUG = 0;
    static const GLuint BLASTER = 1;
    static const GLuint TORPEDO = 2;
    static const GLuint WAVE = 3;
    static const GLuint MINE = 4;
    //   GLFloat[] getCoords() { return {x,y,z}; }
};

// inline bool Collision(Vertex A, Vertex B, GLfloat Range);
#endif
