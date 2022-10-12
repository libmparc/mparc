#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>

#include "mparc.h"

/**
  * @file mparc.h
  * @author MXPSQL
  * @brief MPARC, A Dumb Archiver Format C Rewrite Of MPAR. Test file
  * @version 0.1
  * @date 2022-09-26
  * 
  * @copyright
  * 
  * Licensed To You Under Teh MIT License and the LGPL-2.1-Or-Later License
  * 
  * MIT License
  * 
  * Copyright (c) 2022 MXPSQL
  * 
  * Permission is hereby granted, free of charge, to any person obtaining a copy
  * of this software and associated documentation files (the "Software"), to deal
  * in the Software without restriction, including without limitation the rights
  * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
  * copies of the Software, and to permit persons to whom the Software is
  * furnished to do so, subject to the following conditions:
  * 
  * The above copyright notice and this permission notice shall be included in all
  * copies or substantial portions of the Software.
  * 
  * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
  * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
  * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
  * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
  * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
  * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
  * SOFTWARE.
  * 
  * 
  * MPARC, A rewrite of MPAR IN C, a dumb archiver format
  * Copyright (C) 2022 MXPSQL
  * 
  * This library is free software; you can redistribute it and/or
  * modify it under the terms of the GNU Lesser General Public
  * License as published by the Free Software Foundation; either
  * version 2.1 of the License, or (at your option) any later version.
  * 
  * This library is distributed in the hope that it will be useful,
  * but WITHOUT ANY WARRANTY; without even the implied warranty of
  * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  * Lesser General Public License for more details.
  * 
  * You should have received a copy of the GNU Lesser General Public
  * License along with this library; if not, write to the Free Software
  * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 */

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
	char* filen[] = {/*"mparc.h", */NULL};
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
	{
		struct pod {
			size_t i;
			char* cmd3;
		};
		struct pod* p = malloc(sizeof(struct pod));
		if(p == NULL){
			printf("Struct killed ram\n");
			abort();
		}
		p->i = 9;
		p->cmd3 = "cmd.sexe";
		if((err = MPARC_push_voidfile(archive, "structy-pod.struct", p, sizeof(*p))) != MPARC_OK){
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
		free(p);
	}
	{
		if((err = MPARC_push_voidfile(archive, "mparc.struct", archive, sizeof(archive)))){
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
	/* {
		FILE* fs = fopen("mparc.struct", "wb+");
		if(fs != NULL){
			fwrite(archive, sizeof(archive), 1, fs);
			fclose(fs);
		}
	} */
	if((err =MPARC_extract(archive, ".", NULL)) != MPARC_OK){
		MPARC_sfperror(err, stderr, "DFialed to extract archive");
	}
    MPARC_destroy(&archive);
    return 0;
}
