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

        MPARC11::ParseReturn parret = archive.parse(fle);
        if(!isParseReturnOk(parret)){
            std::cerr << "Parsing failed" << std::endl;
        }

        if(!(
            stats = archive.construct(fle)
        )){
            stats.assertion(false);
        }

        std::cerr << "archive1:" << std::endl;
        std::cout << fle << std::endl;

        archive.clear();

        if(!isParseReturnOk(
            parret = archive.parse(fle)
        )){
            std::cerr << "Parsing 2 failed" << std::endl;
        }
    }

    return EXIT_SUCCESS;
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
    MPARC11::ParseReturn parsestat;

    CLI::App appParser{"MPAR Archive Editor"};

    appParser.allow_windows_style_options(true);
    appParser.failure_message(CLI::FailureMessage::help);
    appParser.allow_extras(true);

    std::string filename = "";
    bool list = false;
    bool create = false;
    int verbosity = 0;
    bool scan = false;
    bool extract = false;
    std::string pull_entry;
    MPARC11::MPARC::version_type tep = MPARC11::MPARC::mpar_version;
    std::string xor_k;
    std::vector<int> rot_k;
    std::string camellia_k = "";
    bool squash_errors = false; // disable errors being printed
    appParser.add_option("-f,--file", filename, "Which file?")->required();
    appParser.add_option("-^,--set-version", tep, "Version of the archive to construct");
    CLI::Option* verbosityFlag = appParser.add_flag("-V,--verbose", "Verbose mode. More of this flag means more verbosity. Only level 1 and 2 are available, but it won't hurt if you go beyond that, for now.");
    appParser.add_flag("-%,--squash-errors", squash_errors, "Squash errors. Prevent error messages from being outputed. You can still use exit codes.");
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
            auto scan_opt = appParser.add_flag("-s,--scan", scan, "Scan the directory to be put into an archive?");
            auto extract_opt = appParser.add_flag("-x,--extract", extract, "Extract the archive into a directory?");
            auto pull_opt = appParser.add_option("-p,--pull", pull_entry, "Pull one entry from an archive?");

            opts.push_back(list_opt);
            opts.push_back(create_opt);
            opts.push_back(scan_opt);
            opts.push_back(extract_opt);
            opts.push_back(pull_opt);
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
        // Do the verbosity check
        verbosity = verbosityFlag->count();
    }

    {
        archive.set_xor_encryption(xor_k);
        archive.set_rot_encryption(rot_k);

        if(camellia_k != ""){
            if(verbosity > 1) std::cout << "Camellia encryption set!" << std::endl;
            if(!(stats = archive.set_camellia_encryption(camellia_k)).isOK()){
                if(!squash_errors) std::cerr << "Camellia encryption misuse detected (key bit length can only be below 256 bits/32 bytes long)." << std::endl;
                return EXIT_FAILURE;
            }
        }
    }

    int exit_code = EXIT_SUCCESS;

    if(list){
        if(verbosity > 0) std::cout << "Parsing archive..." << std::endl;
        parsestat = archive.read(filename);
        if(!isParseReturnOk(parsestat)){
            if(!squash_errors){
                std::cerr << "Some of the archive failed to be parsed." << std::endl;
                std::cerr << "Some entry unrelated errors:" << std::endl;
                std::vector<MPARC11::Status> vstat = parsestat.first;
                if(vstat.size() != 0){
                    for(auto err : vstat){
                        std::cerr << "\t" << err.str() << std::endl;
                    }
                }
                else{
                    std::cerr << "none" << std::endl;
                }
                std::cerr << "And some entry related errors:" << std::endl;
                std::map<std::string, MPARC11::Status> mstat;
                if(!mstat.empty()){
                    for(auto eerr : mstat){
                        MPARC11::Status estat = eerr.second;
                        std::cerr << "\t'" << "" << "'>" << estat.str() << std::endl;
                    }
                }
                else{
                    std::cerr << "none" << std::endl;
                }
            }
            exit_code = EXIT_FAILURE;
        }

        if(verbosity > 0) std::cout << "Listing archive..." << std::endl;
        std::vector<std::string> lists;
        stats = archive.list(lists);
        if(!stats.isOK()){
            if(!squash_errors) std::cerr << "Failure to list archive: " << stats.str() << std::endl;
            return EXIT_FAILURE;
        }

        for(std::string entry : lists){
            std::cout << ((verbosity > 0) ? "V> " : "") 
            << entry << std::endl;
        }
    }
    else if(create){
        std::vector<std::string> files = appParser.remaining(false);
        for(std::string file : files){
            if(verbosity > 0){
                std::cout << "C> " << file << std::endl;
            }

            if(verbosity > 0) std::cout << "Pushing entries to archive" << std::endl;
            stats = archive.push(file, true);
            if(!stats.isOK()){
                if(!squash_errors) std::cerr << "Failure to push '" << file << "' to archive: " << stats.str() << std::endl;
                return EXIT_FAILURE;
            }
        }

        if(verbosity > 0) std::cout << "Constructing archive '" << filename << "'" << std::endl;
        stats = archive.write(filename);
        if(!stats.isOK()){
            MPARC11::Status::Code cod = stats.getCode();
            if(!squash_errors) std::cerr << "Failure to construct archive: " << MPARC11::Status::str(cod, MPARC11::Status::StrFilter::CONSTRUCT_FAILS) << std::endl;
            return EXIT_FAILURE;
        }
    }
    else if(scan){
        std::vector<std::string> dirs = appParser.remaining(false);
        if(dirs.size() < 1) {
            if(!squash_errors) std::cerr << "Directory to be scanned is not specified" << std::endl;
            return EXIT_FAILURE;
        }
        std::string dir = dirs[0];

        if(verbosity > 1) std::cout << "Scanning directory '" << dir << "' into archive" << std::endl;
        stats = archive.scan(false, dir, [&verbosity](std::string paf, bool proc){
            if(verbosity > 0) std::cout << "S" 
            << ((verbosity > 1) ? "(" : "") <<
                ((verbosity > 1) ? (proc ? "post" : "pre") : "") 
            << ((verbosity > 1) ? ")" : "") << "> " 
            << paf << std::endl;
        });
        if(!stats.isOK()){
            std::cerr << "Failure to scan directory: " << stats.str() << std::endl;
            return EXIT_FAILURE;
        }

        if(verbosity) std::cout << "Constructing archive '" << filename << "'" << std::endl;
        stats = archive.write(filename);
        if(!stats.isOK()){
            MPARC11::Status::Code cod = stats.getCode();
            if(!squash_errors) std::cerr << "Failure to construct archive: " << MPARC11::Status::str(cod, MPARC11::Status::StrFilter::CONSTRUCT_FAILS) << std::endl;
            return EXIT_FAILURE;
        }
    }
    else if(extract){
        if(verbosity > 0) std::cout << "Parsing archive..." << std::endl;
        parsestat = archive.read(filename);
        if(!isParseReturnOk(parsestat)){
            if(!squash_errors){
                std::cerr << "Some of the archive failed to be parsed." << std::endl;
                std::cerr << "Some entry unrelated errors:" << std::endl;
                std::vector<MPARC11::Status> vstat = parsestat.first;
                if(vstat.size() != 0){
                    for(auto err : vstat){
                        std::cerr << "\t" << err.str() << std::endl;
                    }
                }
                else{
                    std::cerr << "none" << std::endl;
                }
                std::cerr << "And some entry related errors:" << std::endl;
                std::map<std::string, MPARC11::Status> mstat;
                if(!mstat.empty()){
                    for(auto eerr : mstat){
                        MPARC11::Status estat = eerr.second;
                        std::cerr << "\t'" << "" << "'>" << estat.str() << std::endl;
                    }
                }
                else{
                    std::cerr << "none" << std::endl;
                }
            }
            exit_code = EXIT_FAILURE;
        }


        std::vector<std::string> dirs = appParser.remaining(false);
        if(dirs.size() < 1) {
            if(!squash_errors) std::cerr << "Directory to extract to is not specified" << std::endl;
            return EXIT_FAILURE;
        }
        std::string dir = dirs[0];

        if(verbosity > 0) std::cout << "Extracting to directory '" << dir << "' into archive" << std::endl;
        stats = archive.extract(false, dir, [&verbosity](std::string paf, bool proc){
            if(verbosity > 0) std::cout << "X" 
            << ((verbosity > 1) ? "(" : "") <<
                ((verbosity > 1) ? (proc ? "post" : "pre") : "") 
            << ((verbosity > 1) ? ")" : "") << "> " 
            << paf << std::endl;
        });
        if(!stats.isOK()){
            if(!squash_errors) std::cerr << "Failure to extract to directory: " << stats.str() << std::endl;
            return EXIT_FAILURE;
        }
    }
    else if(!pull_entry.empty()){
        if(verbosity > 0) std::cout << "Parsing archive..." << std::endl;
        parsestat = archive.read(filename);
        if(!isParseReturnOk(parsestat)){
            if(!squash_errors){
                std::cerr << "Some of the archive failed to be parsed." << std::endl;
                std::cerr << "Some entry unrelated errors:" << std::endl;
                std::vector<MPARC11::Status> vstat = parsestat.first;
                if(vstat.size() != 0){
                    for(auto err : vstat){
                        std::cerr << "\t" << err.str() << std::endl;
                    }
                }
                else{
                    std::cerr << "none" << std::endl;
                }
                std::cerr << "And some entry related errors:" << std::endl;
                std::map<std::string, MPARC11::Status> mstat;
                if(!mstat.empty()){
                    for(auto eerr : mstat){
                        MPARC11::Status estat = eerr.second;
                        std::cerr << "\t'" << "" << "'>" << estat.str() << std::endl;
                    }
                }
                else{
                    std::cerr << "none" << std::endl;
                }
            }
            exit_code = EXIT_FAILURE;
        }


        std::vector<std::string> names = appParser.remaining(false);
        if(names.size() < 1) {
            if(!squash_errors) std::cerr << "Target filename to pull entry to is not specified" << std::endl;
            return EXIT_FAILURE;
        }     
        std::string name = names[0];

        stats = archive.pull(pull_entry, name);
        if(!stats.isOK()){
            if(!squash_errors) std::cerr << "Failure to pull entry '" << name << "' from archive: " << stats.str() << std::endl;
            return EXIT_FAILURE;
        }
    }
    else{
        if(!squash_errors) std::cerr << "What are you doing with '" << filename << "'?" << std::endl << "Do you need help? Use the -h flag!" << std::endl;
        return EXIT_FAILURE;
    }

    return exit_code;
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
