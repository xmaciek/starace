#pragma once
#include <cstdint>
#include <vector>
#include <utility>

class Circle {
public:
    Circle( uint64_t segs = 64, double rad = 1.0 );
    inline double GetX( uint64_t i ) const { return m_coord[i].first; }
    inline double GetY( uint64_t i ) const { return m_coord[i].second; }
    inline double x( uint64_t i ) const { return m_coord[i].first; }
    inline double y( uint64_t i ) const { return m_coord[i].second; }

    inline uint64_t GetSegments() const { return m_coord.size(); }
    inline double GetRadiust() const { return m_radiust; }
    inline uint64_t segments() const { return m_coord.size(); }
    inline double radiust() const { return m_radiust; }

    void SetSegments( uint64_t segs );
    void SetRadiust( double rads );

private:
   double m_radiust;
   std::vector< std::pair<double, double> > m_coord;

};
