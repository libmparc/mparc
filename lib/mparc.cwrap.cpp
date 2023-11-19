#include "mparc.h"
#include "mparc.hpp"

#include <new>

#include <stdlib.h>

namespace MPARC11 = MXPSQL::MPARC11;
using MPARC = MPARC11::MPARC;

struct mparc{
    void* instance_handle;
};

bool mparc_new(mparc** obj){
    if(obj){
        mparc* objec = static_cast<mparc*>(calloc(1, sizeof(*obj)));
        if(!objec){
            return false;
        }

        try{
            objec->instance_handle = static_cast<void*>(new MPARC());
        }
        catch(std::bad_alloc& ex){
            free(objec);
            return false;
        }
    }

    return true;
}

void mparc_destroy(mparc* obj){
    if(obj){
        delete static_cast<MPARC*>(obj->instance_handle);
        free(obj);
    }
}
