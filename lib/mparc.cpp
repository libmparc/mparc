#include "mparc.hpp"

// NAMESPACE ALIASING
namespace MPARC11 = MXPSQL::MPARC11;
namespace Utils = MPARC11::Utils;
using ByteArray = MPARC11::ByteArray;
using Status = MPARC11::Status;
using MPARC = MPARC11::MPARC;
using Entry = MPARC11::Entry;

#ifdef MXPSQL_MPARC_FIX11_CPP17
namespace stdfs = std::filesystem;
#endif


// Utilities
ByteArray Utils::StringToByteArray(std::string content){
    return ByteArray(content.begin(), content.end());
}

std::string Utils::ByteArrayToString(ByteArray bytearr){
    return std::string(bytearr.begin(), bytearr.end());
}

Status::Code Utils::isDirectoryDefaultImplementation(std::string name) {
    // TODO: Add windows and Unix implementation if C++17 is not available
    #ifdef MXPSQL_MPARC_FIX11_CPP17
    return ( stdfs::is_directory(
        stdfs::path(name)
    ) ? Status::Code::OK : (Status::Code)(Status::Code::KEY | Status::Code::KEY_NOEXISTS));
    #else
    return Status::Code::NOT_IMPLEMENTED;
    #endif
}



// STATUS FUNCTIONS
bool Status::isOK(){
    return getCode() == Status::Code::OK;
}

void Status::assert(bool throw_err=true){
    if(!isOK()){
        if(throw_err){
            throw std::runtime_error(str(nullptr));
        }
        else{
            std::abort();
        }
    }
}

std::string Status::str(Status::Code* code = nullptr){
    Status::Code cod = Status::Code::OK;

    if(code){
        cod = *code;
    }
    else{
        cod = getCode();
    }

    switch(cod){
        case OK:
            return "OK";
        
        case GENERIC:
            return "A generic error has been detected.";
        case INTERNAL:
            return "An internal error has been detected.";

        case INVALID_VALUE: {
            if(cod & NULL_VALUE) return "A null value has been provided at an inappropriate time.";
            return "An invalid value has been provided.";
        }

        case KEY: {
            if(cod & KEY_EXISTS) return "The key exists.";
            if(cod & KEY_NOEXISTS) return "The key does not exist.";
            return "A generic/unknown key related error has been detected.";
        }

        case FERROR:
            return "A File I/O related error has been detected.";

        default:
            return "Unknown code";
    }

    // If we get to C++23, I will put std::unreachable here
    return "END";
}

Status::Code Status::getCode(){
    return stat_code;
}

Status::operator bool(){
    return isOK();
}


// MAIN ARCHIVE STRUCTURE

MPARC::MPARC(){};
MPARC::MPARC(std::vector<std::string> entries){}
MPARC::MPARC(MPARC& other){}

void MPARC::init(){
    std::unique_lock<std::recursive_mutex> ulock(sync_mutex);
}


Status MPARC::exists(std::string name){
    std::unique_lock<std::recursive_mutex> ulock(sync_mutex);
    return Status(
        (this->my_code = (
                (Status::Code)
                ( 
                    (entries.count(name) != 0 || entries.find(name) != entries.end()) ?
                    (Status::Code::KEY | Status::Code::KEY_EXISTS) :
                    (Status::Code::OK)
                )
            )
        )
    );
}


Status MPARC::push(std::string name, Entry entry, bool overwrite){
    std::unique_lock<std::recursive_mutex> ulock(sync_mutex);
    if(!overwrite && exists(name)){
        return Status(
            (this->my_code = (Status::Code)(Status::Code::KEY | Status::Code::KEY_EXISTS))
        );
    }

    entries[name] = entry;

    return Status(
        (this->my_code = Status::Code::OK)
    );
}

Status MPARC::push(std::string name, bool directory, ByteArray content, bool overwrite){
    std::unique_lock<std::recursive_mutex> ulock(sync_mutex);
    Entry entreh;
    entreh.directory = directory;
    entreh.content = content;
    return push(name, entreh, overwrite);
}

Status MPARC::push(std::string name, bool directory, std::string content, bool overwrite){
    std::unique_lock<std::recursive_mutex> ulock(sync_mutex);
    return push(name, directory, Utils::StringToByteArray(content), overwrite);
}

Status MPARC::push(std::string name, MPARC::isDirFuncType isDirFunc, bool overwrite){
    std::unique_lock<std::recursive_mutex> ulock(sync_mutex);
    Status::Code isDir = isDirFunc(name);
    if(isDir == Status::Code::NOT_IMPLEMENTED){
        return Status(isDir);
    }
    else if(
        !(
            (
                (isDir & Status::Code::KEY) && (isDir & Status::Code::KEY)
            ) || 
            (isDir == Status::Code::OK)
        )
    ){
        return Status(isDir);
    }

    std::string content;

    {
        std::ifstream ifs(name);
        if(!ifs.is_open() || !ifs.good()){
            return Status(Status::Code::FERROR);
        }

        std::stringstream ssbuf;
        ssbuf << ifs.rdbuf();

        content = ssbuf.str();
    }
    
    return push(name, ((isDir & Status::Code::KEY) && (isDir & Status::Code::KEY)), content, overwrite);
}

Status MPARC::push(std::string name, bool overwrite){
    std::unique_lock<std::recursive_mutex> ulock(sync_mutex);
    return push(name, Utils::isDirectoryDefaultImplementation, overwrite);
}


Status MPARC::pop(std::string name){
    std::unique_lock<std::recursive_mutex> ulock(sync_mutex);
    {
        Status stat = exists(name);
        if(!stat) return stat;
    }

    entries.erase(entries.find(name));

    return Status(
        (this->my_code = Status::Code::OK)  
    );
}


Status MPARC::peek(std::string name){
    std::unique_lock<std::recursive_mutex> ulock(sync_mutex);
    {
        Status stat = exists(name);
        if(!stat) return stat;
    }

    if(entries[name].directory){
        return Status(
            (this->my_code = Status::Code::ISDIR)
        );
    }

    return Status(
        (this->my_code = Status::Code::OK)  
    );    
}


Status MPARC::peek(std::string name, std::string* output_str, ByteArray* output_ba){
    std::unique_lock<std::recursive_mutex> ulock(sync_mutex);
    Status stat = exists(name);
    if(!stat) return stat;

    stat = peek(name);
    if(!stat) return stat;

    ByteArray ba = entries[name].content;
    if(output_ba){
        *output_ba = ba;
    }
    if(output_str){
        *output_str = Utils::ByteArrayToString(ba);
    }

    return Status(
        (this->my_code = Status::Code::OK)  
    );    
}
