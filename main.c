#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <inttypes.h>
#include <unistd.h>
#include <errno.h>

#include "mparc.h"

int main(int argc, char** argv){
    MXPSQL_MPARC_t* archive = NULL;
    MXPSQL_MPARC_err err = MPARC_OK;
    int exit_c = EXIT_SUCCESS;
    char* filename = NULL;
    char* opmode = NULL;


    if(argc < 3){
        printf("%s usage\n", argv[0]);
        exit_c= EXIT_FAILURE;
        goto exit_handler;
    }


    MPARC_init(&archive);

    filename = argv[1];

    opmode = argv[2];

    if(MPARC_clear_file(archive) != MPARC_OK){
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
        err = MPARC_list(archive, &listy, &listys);
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
            fprintf(stderr, "%s", "You made an empty archive.");
            exit_c = EXIT_FAILURE;
            goto exit_handler;
        }

        
        for(int i = pos; i < argc; i++){
            printf("c> %s\n", argv[i]);
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
    else if(strcmp(opmode, "a") == 0){
        int pos = 3;
        if(argc < 4){
            fprintf(stderr, "%s", "You appened nothing to the archive.");
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

        if(argc < 3){
            fprintf(stderr, "%s\n", "We need the destination directory please");
            exit_c = EXIT_FAILURE;
            goto exit_handler;
        }

        int run = 1;
        while(run == 1){
            char* basedir2make = NULL;
            MXPSQL_MPARC_err status = MPARC_OK;

            status = MPARC_extract(archive, "", &basedir2make);

            if(status == MPARC_OK){
                run = 0;
            }
            else if(status == MPARC_OPPART){
                // make directory somehow
                #if defined(_WIN32) || defined(_WIN64)
                #else
                #endif
                continue;
            }
            else if(status != MPARC_OK || status != MPARC_OPPART){
                MPARC_perror(err);
                exit_c = EXIT_FAILURE;
                goto exit_handler;
            }
        }
    }
    else{
        printf("%s", "Wrong options [l, c, a, x]");
        exit_c = EXIT_FAILURE;
        goto exit_handler;
    }


    goto exit_handler; // redundant

    exit_handler:
    MPARC_destroy(archive);
    return exit_c;
}