#include "mparc.hpp"
#include <cstdlib>
#include <iostream>

namespace MPARC11 = MXPSQL::MPARC11;

int main(int argc, char* argv[]){
    MPARC11::MPARC archive;
    MPARC11::Status stat;

    for(int i = 1; i < argc; i++){
        if(!(stat = archive.push(
            std::string(argv[i]), true
        ))){
            stat.assertion(false);
        }
    }

    {
        std::string fle;
        if(!(
            stat = archive.construct(fle)
        )){
            stat.assertion(false);
        }

        archive.clear();

        if(!(
            stat = archive.parse(fle)
        )){
            stat.assertion(false);
        }

        if(!(
            stat = archive.construct(fle)
        )){
            stat.assertion(false);
        }

        std::cout << fle << std::endl;
    }

    return EXIT_SUCCESS;
}
