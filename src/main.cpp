#include "road.hpp"

int main( [[maybe_unused]] int argc, [[maybe_unused]] char** argv )
{
    Road* road = new Road();
    const int ret = road->run();
    delete road;
    return ret;
}
