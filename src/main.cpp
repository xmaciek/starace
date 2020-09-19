#include "road.hpp"

int main( [[maybe_unused]] int argc, [[maybe_unused]] char** argv )
{
    std::cout << "Launching...\n";
    srand( (unsigned)time( NULL ) );
    Road* road = new Road();
    int ret = road->OnExecute();
    delete road;
    return ret;
}
