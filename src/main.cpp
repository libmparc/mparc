#include "mparc.hpp"
#include <cstdlib>
#include <iostream>
#include <fstream>
#include <sstream>
#include "CLI/CLI.hpp"

namespace MPARC11 = MXPSQL::MPARC11;

/// @brief Test function
/// @param argc argc
/// @param argv argv
/// @return 1/0/?
int test_main(int argc, char* argv[]){
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
            stat = archive.construct(fle, 1)
        )){
            stat.assertion(false);
        }

        std::cerr << "archive:" << std::endl;
        std::cout << fle << std::endl;

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

        std::cerr << "archive1:" << std::endl;
        std::cout << fle << std::endl;

        archive.clear();

        if(!(
            stat = archive.parse(fle)
        )){
            stat.assertion(false);
        }
    }

    return EXIT_SUCCESS;
}


/// @brief A function to read a string. Falsely named though.
/// @param filename The file to read
/// @param file Output
/// @return true = success. false = loser.
bool read_archive(std::string filename, std::string& file){
    if(filename == "-"){
        char c;
        std::stringstream ss;
        while(std::cin.get(c)){
            ss << c;
        }
        file = ss.str();
    }
    else{
        {
            std::string msg = "";
            if((msg = CLI::ExistingFile(filename)) != ""){
                std::cerr << msg << std::endl;
                return false;
            }
        }

        std::ifstream ifs(filename, std::ios::binary);
        if(!ifs.is_open() || !ifs.good()){
            std::cerr << "Failed to open '" << filename << "'" << std::endl;
            if(!ifs.is_open()){
                std::cerr << "File reading did not actually open.";
            }
            else if(!ifs.good()){
                std::cerr << "File reading not in a great state.";
            }
            else{
                std::cerr << "Unknown reasons.";
            }
            std::cerr << std::endl;
            return false;
        }
        std::stringstream ss;
        ss << ifs.rdbuf();
        file = ss.str();
    }
    return true;
}

/// @brief A function to write a string. Falsely named though.
/// @param filename The file to write
/// @param file Input
/// @return true = success. false = loser.
bool write_archive(std::string filename, std::string file){
    if(filename == "-"){
        std::cout << file;
    }
    else{
        std::ofstream ofs(filename, std::ios::binary);
        if(!ofs.is_open() || !ofs.good()){
            std::cerr << "Failed to open '" << filename << "'" << std::endl;
            if(!ofs.is_open()){
                std::cerr << "File writing did not actually open.";
            }
            else if(!ofs.good()){
                std::cerr << "File writing not in a great state.";
            }
            else{
                std::cerr << "Unknown reasons.";
            }
            std::cerr << std::endl;
            return false;
        }

        ofs << file;
    }
    return true;
}

/// @brief Useful main function/Useful function
/// @param argc argc
/// @param argv argv
/// @return 1/0/?
int exec_main(int argc, char* argv[]){
    (static_cast<void>(argc));
    (static_cast<void>(argv));

    MPARC11::MPARC archive;
    MPARC11::Status stat;

    CLI::App appParser{"MPAR Archive Editor"};

    appParser.allow_windows_style_options(true);
    appParser.failure_message(CLI::FailureMessage::help);
    appParser.allow_extras(true);

    std::string filename = "";
    bool list = false;
    bool create = false;
    bool verbose = false;
    appParser.add_option("-f,--file", filename, "Which file?")->required();
    appParser.add_flag("-v,--verbose", verbose, "Verbose mode");
    {
        auto list_opt = appParser.add_flag("-t,-l,--list", list, "List the archive?");
        auto create_opt = appParser.add_flag("-c,--create", create, "Create an archive?");

        list_opt->excludes(create_opt);
        
        create_opt->excludes(list_opt);
    }

    try {
        appParser.parse(CLI::argc(), CLI::argv());
    } catch (const CLI::ParseError &e) {
        return appParser.exit(e);
    }

    if(list){
        std::string file = "";
        if(!read_archive(filename, file)){
            return EXIT_FAILURE;
        }

        stat = archive.parse(file);
        if(!stat.isOK()){
            std::cerr << "Failure to parse archive: " << stat.str() << std::endl;
            return EXIT_FAILURE;
        }

        std::vector<std::string> lists;
        stat = archive.list(lists);
        if(!stat.isOK()){
            std::cerr << "Failure to list archive: " << stat.str() << std::endl;
            return EXIT_FAILURE;
        }

        for(std::string entry : lists){
            std::cout << (verbose ? "V> " : "") 
            << entry << std::endl;
        }
    }
    else if(create){
        std::string file = "";

        std::vector<std::string> files = appParser.remaining(false);
        for(std::string file : files){
            if(verbose){
                std::cout << "C> " << file << std::endl;
            }

            stat = archive.push(file, true);
            if(!stat.isOK()){
                std::cerr << "Failure to push '" << file << "' to archive: " << stat.str() << std::endl;
                return EXIT_FAILURE;
            }
        }

        stat = archive.construct(file);
        if(!stat.isOK()){
            std::cerr << "Failure to construct archive: " << stat.str() << std::endl;
            return EXIT_FAILURE;
        }

        write_archive(filename, file);
    }
    else{
        std::cerr << "What are you doing with '" << filename << "'?";
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

/// @brief Real deal main function
/// @param argc argc
/// @param argv argv
/// @return 1/0/?
int main(int argc, char* argv[]){
    #ifdef MPARC_X_DOTEST
    (static_cast<void>(exec_main));
    return test_main(argc, argv);
    #else
    (static_cast<void>(test_main));
    return exec_main(argc, argv);
    #endif
}
