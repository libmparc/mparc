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
    MPARC11::Status stats;

    for(int i = 1; i < argc; i++){
        if(!(stats = archive.push(
            std::string(argv[i]), true
        ))){
            stats.assertion(false);
        }
    }

    {
        std::string fle;
        if(!(
            stats = archive.construct(fle, 1)
        )){
            stats.assertion(false);
        }

        std::cerr << "archive:" << std::endl;
        std::cout << fle << std::endl;

        archive.clear();

        if(!(
            stats = archive.parse(fle)
        )){
            stats.assertion(false);
        }

        if(!(
            stats = archive.construct(fle)
        )){
            stats.assertion(false);
        }

        std::cerr << "archive1:" << std::endl;
        std::cout << fle << std::endl;

        archive.clear();

        if(!(
            stats = archive.parse(fle)
        )){
            stats.assertion(false);
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
    MPARC11::Status stats;

    CLI::App appParser{"MPAR Archive Editor"};

    appParser.allow_windows_style_options(true);
    appParser.failure_message(CLI::FailureMessage::help);
    appParser.allow_extras(true);

    std::string filename = "";
    bool list = false;
    bool create = false;
    bool verbose = false;
    MPARC11::MPARC::version_type tep = MPARC11::MPARC::mpar_version;
    std::string xor_k;
    std::vector<int> rot_k;
    std::string camellia_k = "";
    appParser.add_option("-f,--file", filename, "Which file?")->required();
    appParser.add_option("-^,--set-version", tep, "Version of the archive to construct");
    appParser.add_flag("-V,--verbose", verbose, "Verbose mode");
    { // Encryption
        auto xor_opt = appParser.add_option("-X,--xor", xor_k, "XOR encryption key go here.");
        auto rot_opt = appParser.add_option("-R,--rot", rot_k, "ROT ecnryption key go here.")->delimiter(',');
        auto camellia_opt = appParser.add_option("-C,--camellia", camellia_k, "Camellia encryption key go here (below 256 bits/32 bytes in size).");

        (static_cast<void>(xor_opt));
        (static_cast<void>(rot_opt));
        (static_cast<void>(camellia_opt));
    }
    { // Operations
        std::vector<CLI::Option*> opts;
        {
            auto list_opt = appParser.add_flag("-t,-l,--list", list, "List the archive?");
            auto create_opt = appParser.add_flag("-c,--create", create, "Create an archive?");

            opts.push_back(list_opt);
            opts.push_back(create_opt);
        }

        for(std::vector<CLI::Option*>::size_type i = 0; i < opts.size(); i++){ // Auto excluder
            for(std::vector<CLI::Option*>::size_type j = 0; j < opts.size(); j++){
                if(j != i){
                    auto iopt = opts[i];
                    auto jopt = opts[j];
                    iopt->excludes(jopt);
                }
            }
        }
    }

    try {
        appParser.parse(CLI::argc(), CLI::argv());
    } catch (const CLI::ParseError &e) {
        return appParser.exit(e);
    }

    {
        archive.set_xor_encryption(xor_k);
        archive.set_rot_encryption(rot_k);

        if(camellia_k != ""){
            std::cout << "Camellia encryption set!" << std::endl;
            if(!(stats = archive.set_camellia_encryption(camellia_k)).isOK()){
                std::cerr << "Camellia encryption misuse detected (key bit length can only be below 256 bits/32 bytes long)." << std::endl;
                return EXIT_FAILURE;
            }
        }
    }

    if(list){
        if(verbose) std::cout << "Parsing archive..." << std::endl;
        stats = archive.read(filename);
        if(!stats.isOK()){
            std::cerr << "Failure to parse archive: " << stats.str() << std::endl;
            return EXIT_FAILURE;
        }

        if(verbose) std::cout << "Listing archive..." << std::endl;
        std::vector<std::string> lists;
        stats = archive.list(lists);
        if(!stats.isOK()){
            std::cerr << "Failure to list archive: " << stats.str() << std::endl;
            return EXIT_FAILURE;
        }

        for(std::string entry : lists){
            std::cout << (verbose ? "V> " : "") 
            << entry << std::endl;
        }
    }
    else if(create){
        std::vector<std::string> files = appParser.remaining(false);
        for(std::string file : files){
            if(verbose){
                std::cout << "C> " << file << std::endl;
            }

            if(verbose) std::cout << "Pushing entries to archive" << std::endl;
            stats = archive.push(file, true);
            if(!stats.isOK()){
                std::cerr << "Failure to push '" << file << "' to archive: " << stats.str() << std::endl;
                return EXIT_FAILURE;
            }
        }

        if(verbose) std::cout << "Constructing archive '" << filename << "'" << std::endl;
        stats = archive.write(filename);
        if(!stats.isOK()){
            MPARC11::Status::Code cod = stats.getCode();
            std::cerr << "Failure to construct archive: " << stats.str(&cod, MPARC11::Status::StrFilter::CONSTRUCT_FAILS) << std::endl;
            return EXIT_FAILURE;
        }
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
