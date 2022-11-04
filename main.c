#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <inttypes.h>
#include <stdint.h>
#if (defined(_WIN32) || defined(_WIN64)) && !(defined(__CYGWIN__))
#include <windows.h>
#include <fileapi.h>
#else
#include <sys/stat.h>
#endif
#include <errno.h>

#include "mparc.h"

void xhandler(const char* key){
    printf("x> %s\n", key);
}

int mkdirer(char* dir){
    #if (defined(_WIN32) || defined(_WIN64)) && !(defined(__CYGWIN__))
    return !CreateDirectoryA(dir, NULL);
    #else
    return mkdir(dir, 0777);
    #endif
}

int main(int argc, char** argv){
    MXPSQL_MPARC_t* archive = NULL;
    MXPSQL_MPARC_err err = MPARC_OK;
    int exit_c = EXIT_SUCCESS;
    char* filename = NULL;
    char* opmode = NULL;


    if(argc < 3){
        printf("%s usage: %s [your/archive.mpar] [l, c, a, x, d, e, p, cp] [...]\n", argv[0], argv[0]);
        printf("l to List archive\n");
        printf("c to Create archive\n");
        printf("a to Append archive\n");
        printf("x to eXtract archive\n");
        printf("d to Delete archive entry\n");
        printf("e to check if an archive entry Exists\n");
        printf("p to Print contents of archive (or one entry)\n");
        printf("cp to CoPy archive\n");
        exit_c= EXIT_FAILURE;
        goto exit_handler;
    }

    printf("Initializing archive\n");
    err = MPARC_init(&archive);
    if(err != MPARC_OK){
        MPARC_perror(err);
        exit_c = EXIT_FAILURE;
        goto exit_handler;
    }

    filename = argv[1];

    opmode = argv[2];

    printf("Clearing archive state\n");
    if((err = MPARC_clear_file(archive)) != MPARC_OK){
        MPARC_perror(err);
        exit_c = EXIT_FAILURE;
        goto exit_handler;
    }

    if(strcmp(opmode, "l") == 0){
        err = MPARC_parse_filename(archive, filename);
        if(err != MPARC_OK){
            MPARC_perror(err);
            exit_c = EXIT_FAILURE;
            goto exit_handler;
        }

        char** listy = NULL;
        size_t listys = 0;
        err = MPARC_list_array(archive, &listy, &listys);
        if(err != MPARC_OK){
            MPARC_perror(err);
            exit_c = EXIT_FAILURE;
            goto exit_handler;
        }

        for(size_t i = 0; i < listys; i++){
            printf("l> %s\n", listy[i]);
        }
    }
    else if(strcmp(opmode, "c") == 0){
        int pos = 3;
        if(argc < 4){
            fprintf(stderr, "%s", "You made an empty archive.\n");
            exit_c = EXIT_FAILURE;
            goto exit_handler;
        }

        printf("Queued %d files for archival.\n", argc-pos);
        for(int i = pos; i < argc; i++){
            printf("c> %s\n", argv[i]);
            err = MPARC_push_filename(archive, argv[i]);
            if(err != MPARC_OK){

                MPARC_perror(err);
                exit_c = EXIT_FAILURE;
                goto exit_handler;
            }
        }

        if(strcmp(filename, "-") == 0){
            char* archiveo=NULL;
            err = MPARC_construct_str(archive, &archiveo);
            if(err != MPARC_OK){
                MPARC_perror(err);
                exit_c = EXIT_FAILURE;
                goto exit_handler;
            }  
            fprintf(stderr, "%s\n", archiveo); // stderr due to tainted output if on stdout
        }
        else{
            err = MPARC_construct_filename(archive, filename);
            if(err != MPARC_OK){
                MPARC_perror(err);
                exit_c = EXIT_FAILURE;
                goto exit_handler;
            }  
        }
        if(err != MPARC_OK){
            MPARC_perror(err);
            exit_c = EXIT_FAILURE;
            goto exit_handler;
        }  
    }
    else if(strcmp(opmode, "a") == 0){
        int pos = 3;
        if(argc < 4){
            fprintf(stderr, "%s\n", "You appened nothing to the archive.");
            exit_c = EXIT_FAILURE;
            goto exit_handler;
        }

        err = MPARC_parse_filename(archive, filename);
        if(err != MPARC_OK){
            MPARC_perror(err);
            exit_c = EXIT_FAILURE;
            goto exit_handler;
        }

        
        for(int i = pos; i < argc; i++){
            printf("a> %s\n", argv[i]);
            err = MPARC_push_filename(archive, argv[i]);
            if(err != MPARC_OK){
                MPARC_perror(err);
                exit_c = EXIT_FAILURE;
                goto exit_handler;
            }
        }

        err = MPARC_construct_filename(archive, filename);
        if(err != MPARC_OK){
            MPARC_perror(err);
            exit_c = EXIT_FAILURE;
            goto exit_handler;
        }  
    }
    else if(strcmp(opmode, "x") == 0){
        err = MPARC_parse_filename(archive, filename);
        if(err != MPARC_OK){
            MPARC_perror(err);
            exit_c = EXIT_FAILURE;
            goto exit_handler;
        }

        if(argc < 4){
            fprintf(stderr, "%s\n", "We need the destination directory please");
            exit_c = EXIT_FAILURE;
            goto exit_handler;
        }

        if(1){ // use new extract, it is much user friendlier
            // new extract
            err = MPARC_extract_advance(archive, argv[3], NULL, xhandler, mkdirer);
            if(err != MPARC_OK){
                MPARC_perror(err);
                exit_c = EXIT_FAILURE;
                goto exit_handler;
            }
        }
        else{
            // old extract
            int run = 1;
            MXPSQL_MPARC_err errstat = MPARC_OK;
            char* d2m = NULL;
            while(run == 1){
                errstat = MPARC_extract(archive, argv[3], &d2m);
                if(errstat == MPARC_OPPART){
                    if(mkdirer(d2m) != 0){
                        fprintf(stderr, "Failed to make directory\n");
                        exit_c = EXIT_FAILURE;
                        goto exit_handler;
                    }
                }
                else if(errstat == MPARC_OK){
                    run = 0;
                    break;
                }
                else{
                    MPARC_perror(errstat);
                    exit_c = EXIT_FAILURE;
                    goto exit_handler;
                }
            }
        }
    }
    else if(strcmp(opmode, "d") == 0){
        int pos = 3;
        if(argc < 4){
            fprintf(stderr, "%s", "You deleted nothing from the archive.\n");
            exit_c = EXIT_FAILURE;
            goto exit_handler;
        }

        err = MPARC_parse_filename(archive, filename);
        if(err != MPARC_OK){
            MPARC_perror(err);
            exit_c = EXIT_FAILURE;
            goto exit_handler;
        }

        
        for(int i = pos; i < argc; i++){
            printf("d> %s\n", argv[i]);
            err = MPARC_pop_file(archive, argv[i]);
            if(err == MPARC_KNOEXIST){
                fprintf(stderr, "The file (%s) does not exist and you still try to pop it off the archive???\n", argv[i]);
                exit_c = EXIT_FAILURE;
                goto exit_handler;
            }
            else if(err != MPARC_OK){
                MPARC_perror(err);
                exit_c = EXIT_FAILURE;
                goto exit_handler;
            }
        }

        err = MPARC_construct_filename(archive, filename);
        if(err != MPARC_OK){
            MPARC_perror(err);
            exit_c = EXIT_FAILURE;
            goto exit_handler;
        }  
    }
    else if(strcmp(opmode, "e") == 0){
        int pos = 3;
        if(argc < 4){
            fprintf(stderr, "%s", "You checked nothing from the archive.\n");
            exit_c = EXIT_FAILURE;
            goto exit_handler;
        }

        err = MPARC_parse_filename(archive, filename);
        if(err != MPARC_OK){
            MPARC_perror(err);
            exit_c = EXIT_FAILURE;
            goto exit_handler;
        }

        err = MPARC_exists(archive, argv[pos]);

        if(err == MPARC_OK) {
            fprintf(stderr, "File (%s) exists\n", argv[pos]);
            exit_c = EXIT_FAILURE;
        }
        else if(err == MPARC_KNOEXIST){
            fprintf(stderr, "File (%s) does not exists\n", argv[pos]);
            exit_c = EXIT_SUCCESS;
        }
        else{
            fprintf(stderr, "Internal state error while checking for File (%s)\n", argv[pos]);
            exit_c = EXIT_FAILURE;
        }
        goto exit_handler;
    }
    else if(strcmp(opmode, "p") == 0){
        err = MPARC_parse_filename(archive, filename);
        if(err != MPARC_OK){
            MPARC_perror(err);
            exit_c = EXIT_FAILURE;
            goto exit_handler;
        }
        int pos = 3;
        if(argc < 4){
            char** listy = NULL;
            uint_fast64_t sizy = 0;
            err = MPARC_list_array(archive, &listy, &sizy);
            if(err != MPARC_OK){
                MPARC_perror(err);
                exit_c = EXIT_FAILURE;
                goto exit_handler;
            }
            for(uint_fast64_t i = 0; i < sizy; i++){
                unsigned char* binary_blob;
                uint_fast64_t binary_size;
                err = MPARC_peek_file(archive, listy[i], &binary_blob, &binary_size);
                if(err != MPARC_OK){
                    MPARC_perror(err);
                    exit_c = EXIT_FAILURE;
                    goto exit_handler;
                }
                for(uint_fast64_t i = 0; i < binary_size; i++){
                    printf("%c", (char) binary_blob[i]);
                }
                printf("\n");
            }
            free(listy);
        }
        else{
            for(int i = pos; i < argc; i++){
                char* filename = argv[i];
                if((err = MPARC_exists(archive, filename)) == MPARC_OK){
                    unsigned char* binary_blob;
                    uint_fast64_t binary_size;
                    err = MPARC_peek_file(archive, filename, &binary_blob, &binary_size);
                    if(err != MPARC_OK){
                        MPARC_perror(err);
                        exit_c = EXIT_FAILURE;
                        goto exit_handler;
                    }
                    for(uint_fast64_t i = 0; i < binary_size; i++){
                        printf("%c", (char) binary_blob[i]);
                    }
                    printf("\n");
                }
                else{
                    MPARC_perror(err);
                    exit_c = EXIT_FAILURE;
                    goto exit_handler;
                }
            }
        }
    }
    else if(strcmp(opmode, "cp") == 0){
        int pos = 3;
        if(argc < 4){
            printf("Destination archive filename required\n");
            exit_c = EXIT_FAILURE;
            goto exit_handler;
        }
        MXPSQL_MPARC_t* cp_archive = NULL;
        err = MPARC_parse_filename(archive, filename);
        if(err != MPARC_OK){
            MPARC_perror(err);
            exit_c = EXIT_FAILURE;
            goto pre_exit_handler;
        }
        err = MPARC_copy(&archive, &cp_archive);
        if(err != MPARC_OK){
            MPARC_perror(err);
            exit_c = EXIT_FAILURE;
            goto pre_exit_handler;
        }
        err = MPARC_construct_filename(archive, argv[pos]);
        if(err != MPARC_OK){
            MPARC_perror(err);
            exit_c = EXIT_FAILURE;
            goto pre_exit_handler;
        }
        goto pre_exit_handler;
        pre_exit_handler:
        MPARC_destroy(&cp_archive);
        goto exit_handler;
    }
    else{
        printf("%s\n", "Wrong options [l, c, a, x, d, e]");
        exit_c = EXIT_FAILURE;
        goto exit_handler;
    }


    goto exit_handler; // redundant

    exit_handler:
    printf("Tearing down archive\n");
    if(err != MPARC_OK || exit_c == EXIT_FAILURE) printf("Failure detected\n");
    MPARC_destroy(&archive);
    return exit_c;
}
