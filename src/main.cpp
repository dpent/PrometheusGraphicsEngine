#include "cpps/engine.cpp"

#include <iostream>
#include <cstdlib>

using namespace Prometheus;

int main(){
    Engine app;

    try{
        app.run();
    }catch( const std::exception& e){
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}