#pragma once
#include "sa.hpp"

class Quaternion {
private:
    double m_x = 0.0;
    double m_y = 0.0;
    double m_z = 0.0;
    double m_w = 1.0;

public:
    Quaternion() = default;
    Quaternion( const Vertex& v, double w );
    Quaternion operator*( const Quaternion& ) const;

    Vertex toVector() const;
    void conjugate();
    void createFromAngles( double x, double y, double z, double deg );
    void createMatrix( double* mat ) const;
    void inverse();
    void normalize();
    void rotateVector( Vertex& );
};
