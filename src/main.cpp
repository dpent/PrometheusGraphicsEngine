#include "engine/headers/engine.h"

#include <iostream>
#include <cstdlib>

using namespace Prometheus;

int main(int argc, char** argv){
    Engine app;
    try{
        app.run(argc, argv);
    }catch( const std::exception& e){
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}