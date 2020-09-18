#include "Road.h"

int main( int argc, char** argv )
{
    cout << "Launching...\n";
    srand( (unsigned)time( NULL ) );
    Road* road = new Road();
    int ret = road->OnExecute();
    delete road;
    return ret;
}