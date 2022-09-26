#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>

#include "mparc.h"

int main(int argc, char* argv[]){
	((void)argc);
	((void)argv);

    MXPSQL_MPARC_t* archive = NULL;
    MXPSQL_MPARC_err err = MPARC_OK;
    if((err = MPARC_init(&archive)) != MPARC_OK) {
        printf("A big no happened when trying to initialize structure\n");
                	    switch(err){
    	        case MPARC_OK:
    	        printf( "it fine\n");
    	        break;

    	        case MPARC_IDK:
    	        printf( "it fine cause idk\n");
				return 1;
    	        case MPARC_INTERNAL:
    	        printf( "Internal error detected\n");
				return 1;


    	        case MPARC_IVAL:
    	        printf( "Bad vals\n");
    	        break;
    	        case MPARC_NOEXIST:
    	        printf( "It does not exist you dumb dumb\n");
    	        break;
    	        case MPARC_OOM:
    	        printf( "Oh noes I run out of memory\n");
    	        break;

    	        case MPARC_NOTARCHIVE:
    	        printf( "You dumb person what you put in is not an archive by the 25 character long magic number it has\n");
    	        break;
    	        case MPARC_ARCHIVETOOSHINY:
    	        printf( "You dumb person the valid archive you put in me is too new for me to process\n");
    	        break;
    	        case MPARC_CHKSUM:
    	        printf( "My content is gone :P\n");
    	        break;

    	        case MPARC_FERROR:
    	        printf( "FILE.exe has stopped responding\n");
    	        break;

    	        default:
    	        printf( "Man what happened here\n");
    	        return 1;
    	    };
        printf("Init failed ok\n");
        abort();
    }
    if(archive == NULL){
                	    switch(err){
    	        case MPARC_OK:
    	        printf( "it fine\n");
    	        break;

    	        case MPARC_IDK:
    	        printf( "it fine cause idk\n");
				return 1;
    	        case MPARC_INTERNAL:
    	        printf( "Internal error detected\n");
				return 1;


    	        case MPARC_IVAL:
    	        printf( "Bad vals\n");
    	        break;
    	        case MPARC_NOEXIST:
    	        printf( "It does not exist you dumb dumb\n");
    	        break;
    	        case MPARC_OOM:
    	        printf( "Oh noes I run out of memory\n");
    	        break;

    	        case MPARC_NOTARCHIVE:
    	        printf( "You dumb person what you put in is not an archive by the 25 character long magic number it has\n");
    	        break;
    	        case MPARC_ARCHIVETOOSHINY:
    	        printf( "You dumb person the valid archive you put in me is too new for me to process\n");
    	        break;
    	        case MPARC_CHKSUM:
    	        printf( "My content is gone :P\n");
    	        break;

    	        case MPARC_FERROR:
    	        printf( "FILE.exe has stopped responding\n");
    	        break;

    	        default:
    	        printf( "Man what happened here\n");
    	        return 1;
    	    };
        printf("Init failed ok\n");
        abort();
    }
	char* filen[] = {"mparc.h", NULL};
    for(size_t i = 0; filen[i] != NULL; i++){
    	if((err = MPARC_push_filename(archive, "mparc.h")) != MPARC_OK){
    	    printf("A big no happened when trying to push files\n");
    	            	    switch(err){
    		        case MPARC_OK:
    		        printf( "it fine\n");
    		        break;

    		        case MPARC_IDK:
    		        printf( "it fine cause idk\n");
					return 1;
    		        case MPARC_INTERNAL:
    		        printf( "Internal error detected\n");
					return 1;


    		        case MPARC_IVAL:
    		        printf( "Bad vals\n");
    		        break;
    		        case MPARC_NOEXIST:
    		        printf( "It does not exist you dumb dumb\n");
    		        break;
    		        case MPARC_OOM:
    		        printf( "Oh noes I run out of memory\n");
    		        break;

    		        case MPARC_NOTARCHIVE:
    		        printf( "You dumb person what you put in is not an archive by the 25 character long magic number it has\n");
    		        break;
    		        case MPARC_ARCHIVETOOSHINY:
    		        printf( "You dumb person the valid archive you put in me is too new for me to process\n");
    		        break;
    		        case MPARC_CHKSUM:
    		        printf( "My content is gone :P\n");
    		        break;

    		        case MPARC_FERROR:
    		        printf( "FILE.exe has stopped responding\n");
    		        break;

    		        default:
    		        printf( "Man what happened here\n");
    		        return 1;
    		    };
    	    printf("File push failed\n");
    	    abort();
    	}
	}
    /* {
        char** listprintf( NULL;
        size_t length = 0;
        MPARC_list(archive, &listout, &length);
        if(listout != NULL)
            for(size_t i = 0; i < length; i++){
                printf("%s\n", listout[i]);
            }
    } */
    /* {
        if(MPARC_exists(archive, "mparc.c") == MPARC_OK) {
            printf("File actually is in the entry.\n");
            unsigned char* bprintf( NULL;
            MPARC_peek_file(archive, "mparc.c", &bout, NULL);
            printf("%s\n", bout);
        }
        else{
            printf("File not in entry :P\n");
        }
    } */

    MPARC_construct_filename(archive, "ck_chorder.mpar");
    // printf("%s", PRIuFAST32);
	MPARC_clear_file(archive);
    {
        MXPSQL_MPARC_err err = MPARC_parse_filename(archive, "ck_chorder.mpar");
        if(err != MPARC_OK){
            printf("A big no happened when trying to parse files\n");
                	    switch(err){
    	        case MPARC_OK:
    	        printf( "it fine\n");
    	        break;

    	        case MPARC_IDK:
    	        printf( "it fine cause idk\n");
				return 1;
    	        case MPARC_INTERNAL:
    	        printf( "Internal error detected\n");
				return 1;


    	        case MPARC_IVAL:
    	        printf( "Bad vals\n");
    	        break;
    	        case MPARC_NOEXIST:
    	        printf( "It does not exist you dumb dumb\n");
    	        break;
    	        case MPARC_OOM:
    	        printf( "Oh noes I run out of memory\n");
    	        break;

    	        case MPARC_NOTARCHIVE:
    	        printf( "You dumb person what you put in is not an archive by the 25 character long magic number it has\n");
    	        break;
    	        case MPARC_ARCHIVETOOSHINY:
    	        printf( "You dumb person the valid archive you put in me is too new for me to process\n");
    	        break;
    	        case MPARC_CHKSUM:
    	        printf( "My content is gone :P\n");
    	        break;

    	        case MPARC_FERROR:
    	        printf( "FILE.exe has stopped responding\n");
    	        break;

    	        default:
    	        printf( "Man what happened here\n");
    	        return 1;
    	    };
            printf("Archive parsing failed\n");
            abort();
        }
    }
	{
		char** e = NULL;
		err = MPARC_list(archive, &e, NULL);
        if(err != MPARC_OK){
            printf("A big no happened when trying to list files\n");
                	    switch(err){
    	        case MPARC_OK:
    	        printf( "it fine\n");
    	        break;

    	        case MPARC_IDK:
    	        printf( "it fine cause idk\n");
				return 1;
    	        case MPARC_INTERNAL:
    	        printf( "Internal error detected\n");
				return 1;


    	        case MPARC_IVAL:
    	        printf( "Bad vals\n");
    	        break;
    	        case MPARC_NOEXIST:
    	        printf( "It does not exist you dumb dumb\n");
    	        break;
    	        case MPARC_OOM:
    	        printf( "Oh noes I run out of memory\n");
    	        break;

    	        case MPARC_NOTARCHIVE:
    	        printf( "You dumb person what you put in is not an archive by the 25 character long magic number it has\n");
    	        break;
    	        case MPARC_ARCHIVETOOSHINY:
    	        printf( "You dumb person the valid archive you put in me is too new for me to process\n");
    	        break;
    	        case MPARC_CHKSUM:
    	        printf( "My content is gone :P\n");
    	        break;

    	        case MPARC_FERROR:
    	        printf( "FILE.exe has stopped responding\n");
    	        break;

    	        default:
    	        printf( "Man what happened here\n");
    	        return 1;
    	    };
            printf("Archive listing failed\n");
            abort();
        }
		for(size_t i = 0; e[i] != NULL; i++){
			printf(">%s\n", e[i]);
		}
	}
    MPARC_destroy(archive);
    return 0;
}