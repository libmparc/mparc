#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <inttypes.h>
#include <stdint.h>
#include <stdbool.h>
#if (defined(_WIN32) || defined(_WIN64)) && !(defined(__CYGWIN__))
#include <windows.h>
#include <fileapi.h>
#else
#include <sys/stat.h>
#endif
#include <errno.h>

#define MPARC_WANT_EXTERN_AUX_UTIL_FUNCTIONS
#include <mparc.h>


#ifndef DOXYGEN_SHOULD_SKIP_THIS

#define OPTPARSE_IMPLEMENTATION
#define OPTPARSE_API static

/* Optparse --- portable, reentrant, embeddable, getopt-like option parser
 *
 * This is free and unencumbered software released into the public domain.
 *
 * To get the implementation, define OPTPARSE_IMPLEMENTATION.
 * Optionally define OPTPARSE_API to control the API's visibility
 * and/or linkage (static, __attribute__, __declspec).
 *
 * The POSIX getopt() option parser has three fatal flaws. These flaws
 * are solved by Optparse.
 *
 * 1) Parser state is stored entirely in global variables, some of
 * which are static and inaccessible. This means only one thread can
 * use getopt(). It also means it's not possible to recursively parse
 * nested sub-arguments while in the middle of argument parsing.
 * Optparse fixes this by storing all state on a local struct.
 *
 * 2) The POSIX standard provides no way to properly reset the parser.
 * This means for portable code that getopt() is only good for one
 * run, over one argv with one option string. It also means subcommand
 * options cannot be processed with getopt(). Most implementations
 * provide a method to reset the parser, but it's not portable.
 * Optparse provides an optparse_arg() function for stepping over
 * subcommands and continuing parsing of options with another option
 * string. The Optparse struct itself can be passed around to
 * subcommand handlers for additional subcommand option parsing. A
 * full reset can be achieved by with an additional optparse_init().
 *
 * 3) Error messages are printed to stderr. This can be disabled with
 * opterr, but the messages themselves are still inaccessible.
 * Optparse solves this by writing an error message in its errmsg
 * field. The downside to Optparse is that this error message will
 * always be in English rather than the current locale.
 *
 * Optparse should be familiar with anyone accustomed to getopt(), and
 * it could be a nearly drop-in replacement. The option string is the
 * same and the fields have the same names as the getopt() global
 * variables (optarg, optind, optopt).
 *
 * Optparse also supports GNU-style long options with optparse_long().
 * The interface is slightly different and simpler than getopt_long().
 *
 * By default, argv is permuted as it is parsed, moving non-option
 * arguments to the end. This can be disabled by setting the `permute`
 * field to 0 after initialization.
 */
#ifndef OPTPARSE_H
#define OPTPARSE_H

#ifndef OPTPARSE_API
#  define OPTPARSE_API
#endif

/// @brief Args parser state
struct optparse {
    /**
     * @brief argv from main()
     * 
     */
    char **argv;
    /**
     * shift argv?
    */
    int permute;
    /**
     * @brief current index
     * 
     */
    int optind;
    /**
     * @brief wrong
     * 
     */
    int optopt;
    /**
     * @brief argument value like "-a abc" gives abc
     * 
     */
    char *optarg;
    /**
     * @brief err
     * 
     */
    char errmsg[64];
    /**
     * @brief idk i never used this I did not even write this
     * 
     */
    int subopt;
};

/// @brief Gnu getopt extension magic typing
enum optparse_argtype {
    /// @brief No
    OPTPARSE_NONE,
    /// @brief Required
    OPTPARSE_REQUIRED,
    /// @brief Optional
    OPTPARSE_OPTIONAL
};

/// @brief  Gnu getopt extension magic
struct optparse_long {
    /**
     * @brief long name for cmdline
     * 
     */
    const char *longname;
    /**
     * @brief short name like "-a"
     * 
     */
    int shortname;
    /**
     * arg type
    */
    enum optparse_argtype argtype;
};

/**
 * Initializes the parser state.
 */
OPTPARSE_API
void optparse_init(struct optparse *options, char **argv);

/**
 * Read the next option in the argv array.
 * @param optstring a getopt()-formatted option string.
 * @return the next option character, -1 for done, or '?' for error
 *
 * Just like getopt(), a character followed by no colons means no
 * argument. One colon means the option has a required argument. Two
 * colons means the option takes an optional argument.
 */
OPTPARSE_API
int optparse(struct optparse *options, const char *optstring);

/**
 * Handles GNU-style long options in addition to getopt() options.
 * This works a lot like GNU's getopt_long(). The last option in
 * longopts must be all zeros, marking the end of the array. The
 * longindex argument may be NULL.
 */
OPTPARSE_API
int optparse_long(struct optparse *options,
                  const struct optparse_long *longopts,
                  int *longindex);

/**
 * Used for stepping over non-option arguments.
 * @return the next non-option argument, or NULL for no more arguments
 *
 * Argument parsing can continue with optparse() after using this
 * function. That would be used to parse the options for the
 * subcommand returned by optparse_arg(). This function allows you to
 * ignore the value of optind.
 */
OPTPARSE_API
char *optparse_arg(struct optparse *options);

/* Implementation */
#ifdef OPTPARSE_IMPLEMENTATION

#define OPTPARSE_MSG_INVALID "invalid option"
#define OPTPARSE_MSG_MISSING "option requires an argument"
#define OPTPARSE_MSG_TOOMANY "option takes no arguments"

static int
optparse_error(struct optparse *options, const char *msg, const char *data)
{
    unsigned p = 0;
    const char *sep = " -- '";
    while (*msg)
        options->errmsg[p++] = *msg++;
    while (*sep)
        options->errmsg[p++] = *sep++;
    while (p < sizeof(options->errmsg) - 2 && *data)
        options->errmsg[p++] = *data++;
    options->errmsg[p++] = '\'';
    options->errmsg[p++] = '\0';
    return '?';
}

OPTPARSE_API
void
optparse_init(struct optparse *options, char **argv)
{
    options->argv = argv;
    options->permute = 1;
    options->optind = argv[0] != 0;
    options->subopt = 0;
    options->optarg = 0;
    options->errmsg[0] = '\0';
}

static int
optparse_is_dashdash(const char *arg)
{
    return arg != 0 && arg[0] == '-' && arg[1] == '-' && arg[2] == '\0';
}

static int
optparse_is_shortopt(const char *arg)
{
    return arg != 0 && arg[0] == '-' && arg[1] != '-' && arg[1] != '\0';
}

static int
optparse_is_longopt(const char *arg)
{
    return arg != 0 && arg[0] == '-' && arg[1] == '-' && arg[2] != '\0';
}

static void
optparse_permute(struct optparse *options, int index)
{
    char *nonoption = options->argv[index];
    int i;
    for (i = index; i < options->optind - 1; i++)
        options->argv[i] = options->argv[i + 1];
    options->argv[options->optind - 1] = nonoption;
}

static int
optparse_argtype(const char *optstring, char c)
{
    int count = OPTPARSE_NONE;
    if (c == ':')
        return -1;
    for (; *optstring && c != *optstring; optstring++);
    if (!*optstring)
        return -1;
    if (optstring[1] == ':')
        count += optstring[2] == ':' ? 2 : 1;
    return count;
}

OPTPARSE_API
int
optparse(struct optparse *options, const char *optstring)
{
    int type;
    char *next;
    char *option = options->argv[options->optind];
    options->errmsg[0] = '\0';
    options->optopt = 0;
    options->optarg = 0;
    if (option == 0) {
        return -1;
    } else if (optparse_is_dashdash(option)) {
        options->optind++; /* consume "--" */
        return -1;
    } else if (!optparse_is_shortopt(option)) {
        if (options->permute) {
            int index = options->optind++;
            int r = optparse(options, optstring);
            optparse_permute(options, index);
            options->optind--;
            return r;
        } else {
            return -1;
        }
    }
    option += options->subopt + 1;
    options->optopt = option[0];
    type = optparse_argtype(optstring, option[0]);
    next = options->argv[options->optind + 1];
    switch (type) {
    case -1: {
        char str[2] = {0, 0};
        str[0] = option[0];
        options->optind++;
        return optparse_error(options, OPTPARSE_MSG_INVALID, str);
    }
    case OPTPARSE_NONE:
        if (option[1]) {
            options->subopt++;
        } else {
            options->subopt = 0;
            options->optind++;
        }
        return option[0];
    case OPTPARSE_REQUIRED:
        options->subopt = 0;
        options->optind++;
        if (option[1]) {
            options->optarg = option + 1;
        } else if (next != 0) {
            options->optarg = next;
            options->optind++;
        } else {
            char str[2] = {0, 0};
            str[0] = option[0];
            options->optarg = 0;
            return optparse_error(options, OPTPARSE_MSG_MISSING, str);
        }
        return option[0];
    case OPTPARSE_OPTIONAL:
        options->subopt = 0;
        options->optind++;
        if (option[1])
            options->optarg = option + 1;
        else
            options->optarg = 0;
        return option[0];
    }
    return 0;
}

OPTPARSE_API
char *
optparse_arg(struct optparse *options)
{
    char *option = options->argv[options->optind];
    options->subopt = 0;
    if (option != 0)
        options->optind++;
    return option;
}

static int
optparse_longopts_end(const struct optparse_long *longopts, int i)
{
    return !longopts[i].longname && !longopts[i].shortname;
}

static void
optparse_from_long(const struct optparse_long *longopts, char *optstring)
{
    char *p = optstring;
    int i;
    for (i = 0; !optparse_longopts_end(longopts, i); i++) {
        if (longopts[i].shortname && longopts[i].shortname < 127) {
            int a;
            *p++ = longopts[i].shortname;
            for (a = 0; a < (int)longopts[i].argtype; a++)
                *p++ = ':';
        }
    }
    *p = '\0';
}

/* Unlike strcmp(), handles options containing "=". */
static int
optparse_longopts_match(const char *longname, const char *option)
{
    const char *a = option, *n = longname;
    if (longname == 0)
        return 0;
    for (; *a && *n && *a != '='; a++, n++)
        if (*a != *n)
            return 0;
    return *n == '\0' && (*a == '\0' || *a == '=');
}

/* Return the part after "=", or NULL. */
static char *
optparse_longopts_arg(char *option)
{
    for (; *option && *option != '='; option++);
    if (*option == '=')
        return option + 1;
    else
        return 0;
}

static int
optparse_long_fallback(struct optparse *options,
                       const struct optparse_long *longopts,
                       int *longindex)
{
    int result;
    char optstring[96 * 3 + 1]; /* 96 ASCII printable characters */
    optparse_from_long(longopts, optstring);
    result = optparse(options, optstring);
    if (longindex != 0) {
        *longindex = -1;
        if (result != -1) {
            int i;
            for (i = 0; !optparse_longopts_end(longopts, i); i++)
                if (longopts[i].shortname == options->optopt)
                    *longindex = i;
        }
    }
    return result;
}

OPTPARSE_API
int
optparse_long(struct optparse *options,
              const struct optparse_long *longopts,
              int *longindex)
{
    int i;
    char *option = options->argv[options->optind];
    if (option == 0) {
        return -1;
    } else if (optparse_is_dashdash(option)) {
        options->optind++; /* consume "--" */
        return -1;
    } else if (optparse_is_shortopt(option)) {
        return optparse_long_fallback(options, longopts, longindex);
    } else if (!optparse_is_longopt(option)) {
        if (options->permute) {
            int index = options->optind++;
            int r = optparse_long(options, longopts, longindex);
            optparse_permute(options, index);
            options->optind--;
            return r;
        } else {
            return -1;
        }
    }

    /* Parse as long option. */
    options->errmsg[0] = '\0';
    options->optopt = 0;
    options->optarg = 0;
    option += 2; /* skip "--" */
    options->optind++;
    for (i = 0; !optparse_longopts_end(longopts, i); i++) {
        const char *name = longopts[i].longname;
        if (optparse_longopts_match(name, option)) {
            char *arg;
            if (longindex)
                *longindex = i;
            options->optopt = longopts[i].shortname;
            arg = optparse_longopts_arg(option);
            if (longopts[i].argtype == OPTPARSE_NONE && arg != 0) {
                return optparse_error(options, OPTPARSE_MSG_TOOMANY, name);
            } if (arg != 0) {
                options->optarg = arg;
            } else if (longopts[i].argtype == OPTPARSE_REQUIRED) {
                options->optarg = options->argv[options->optind];
                if (options->optarg == 0)
                    return optparse_error(options, OPTPARSE_MSG_MISSING, name);
                else
                    options->optind++;
            }
            return options->optopt;
        }
    }
    return optparse_error(options, OPTPARSE_MSG_INVALID, option);
}

#endif /* OPTPARSE_IMPLEMENTATION */
#endif /* OPTPARSE_H */

#endif

/// Print out magic stuff
/// @param key print this key with magic print
/// @param ctx would be context, but unused
void xhandler(const char* key, void* ctx){
    ((void)ctx);
    printf("x> %s\n", key);
}


/// REPL mode
/// @param argc argc
/// @param exe executable path
/// @param argv argv
int repl_main(int argc, char* exe, char** argv){
    printf("Read-Eval-Print-Loop interface mode.\n");
    ((void)argc);
    ((void)exe);

    char* arg = NULL;

    struct optparse parser;
    optparse_init(&parser, argv);

    ((void)arg);

    return 0;
}

/// Command Line Interface mode
/// @param argc argc
/// @param exe executable path
/// @param argv argv
int cmdline_main(int argc, char* exe, char** argv){
    printf("Command line interface mode.\n");
    ((void)argc);

    char* arg = NULL;

    MXPSQL_MPARC_t* archive = NULL;


    char* filename = NULL;
    char mode = '\0';
    int verbose = 0;
    char* XORcKey = NULL;
    int* ROTcKey = NULL;

    ((void)mode);

    struct optparse parser;
    optparse_init(&parser, argv);

    int opt = 0;

    while((opt = optparse(&parser, "f:q:z:rksuypedxaltchv")) != -1){
        switch(opt){
            case 'h': {
                printf("Help shall be here\n");
                return EXIT_SUCCESS;
            }
            
            case 'f': {
                filename = parser.optarg;
                break;
            }

            case 'q': {
                // ROT Cipher TODO
                break;
            }

            case 'z': {
                XORcKey = parser.optarg;
                break;
            }

            case 'v': {
                verbose = 1;
                break;
            }
            
            case 'c':
            case 'l':
            case 'a':
            case 'x':
            case 'd':
            case 'e':
            case 'p':
            case 'y':
            case 'u':
            case 's':
            case 'k':
            case 'r':
            case 't': 
            {
                mode = opt;
                break;
            }

            case '?': {
                fprintf(stderr, "%s: %s\n", exe, parser.errmsg);
                return EXIT_FAILURE;
            }
        }
    }

    if(!filename){
        fprintf(stderr, "%s: -f required.\n", exe);
        return EXIT_FAILURE;
    }

    #define MPARC_CHECKXIT(err) \
    do{ \
        if(err != MPARC_OK){ \
            ; /* printf("An error occured ok: %d\n", err); */ \
            MPARC_perror(err); \
            ex = EXIT_FAILURE; \
            goto exit_handler; \
        } \
    } while(0)

    MXPSQL_MPARC_err err = MPARC_init(&archive);
    int ex = EXIT_SUCCESS;
    MPARC_CHECKXIT(err);

    err = MPARC_cipher(archive, 
    true, (unsigned char*) XORcKey, (XORcKey ? strlen(XORcKey) : 0), NULL, NULL,
    false, ROTcKey, 0, NULL, NULL);

    if(mode == 'c'){
        int flag = 0;
        while((arg = optparse_arg(&parser))){
            if(verbose) printf("CMD (c)> %s\n", arg);
            err = MPARC_push_filename(archive, arg);
            MPARC_CHECKXIT(err);
            flag = 1;
        }

        if(strcmp(filename, "-") == 0){
            if(!flag && verbose){
                fprintf(stdout, "I would refuse as a coward cus u try to make an empty archive, \nbut a gun ('-') is pointed at my head and so I have no choice but to do it against my will you prick coleslaw nerb!\n");
            }
            err = MPARC_construct_filestream(archive, stderr);
            MPARC_CHECKXIT(err);
        }
        else{
            if(!flag){
                if(verbose)fprintf(stderr, "Refusing to make an empty archive like a coward, what are you trying to do man?\nYou should prolly use a text editor or \"pass '-' to '-f'\" to make an empty one, you coleslaw nerb!\n");
                ex = EXIT_FAILURE;
                goto exit_handler;
            }
            err = MPARC_construct_filename(archive, filename);
            MPARC_CHECKXIT(err);
        }
    }
    else if(mode == 'l' || mode == 't'){
        err = MPARC_parse_filename(archive, filename);
        MPARC_CHECKXIT(err);

        {
            char** listy_out = NULL;
            MXPSQL_MPARC_uint_repr_t listy_len = 0;
            err = MPARC_list_array(archive, &listy_out, &listy_len);
            if(err != MPARC_OK) goto myexit;
            for(MXPSQL_MPARC_uint_repr_t i = 0; i < listy_len; i++){
                printf("CMD (l)> %s\n", listy_out[i]);
            }
            myexit:
            MPARC_list_array_free(&listy_out);
            MPARC_CHECKXIT(err);
        }
    }
    else if(mode == 'a'){
        err = MPARC_parse_filename(archive, filename);
        MPARC_CHECKXIT(err);

        while((arg = optparse_arg(&parser))){
            err = MPARC_push_filename(archive, arg);
            MPARC_CHECKXIT(err);
            if(verbose) printf("CMD (a)> %s\n", arg);
        }

        err = MPARC_construct_filename(archive, filename);
        MPARC_CHECKXIT(err);
    }
    else if(mode == 'x'){
        if(MPARC_mkdirer(NULL, NULL) == 10){
            fprintf(stderr, "%s\n", "I cannot make the directory for extraction. The function to do so does not exists.");
            ex = EXIT_FAILURE;
            goto exit_handler;
        }

        err = MPARC_parse_filename(archive, filename);
        MPARC_CHECKXIT(err);
        arg = optparse_arg(&parser);
        if(!arg){
            fprintf(stderr, "No destination directory specified.\n");
            ex = EXIT_FAILURE;
            goto exit_handler;
        }

        err = MPARC_extract_advance(archive, arg, NULL, xhandler, MPARC_mkdirer, NULL, NULL);
        MPARC_CHECKXIT(err);
    }
    else if(mode == 'd'){
        err = MPARC_parse_filename(archive, filename);
        MPARC_CHECKXIT(err);
        while((arg = optparse_arg(&parser))){
            err = MPARC_pop_file(archive, arg);
            MPARC_CHECKXIT(err);
            if(verbose) printf("CMD (d)> %s\n", arg);
        }
        err = MPARC_construct_filename(archive, filename);
        MPARC_CHECKXIT(err);
    }
    else if(mode == 'e'){
        err = MPARC_parse_filename(archive, filename);
        MPARC_CHECKXIT(err);
        while((arg = optparse_arg(&parser))){
            err = MPARC_exists(archive, arg);
            printf("CMD (e)> %s ", arg);
            if(err == MPARC_OK){
                printf("(exists)");
            }
            else{
                printf("(not)");
            }
            printf("\n");
        }
    }
    else if(mode == 'p'){
        err = MPARC_parse_filename(archive, filename);
        MPARC_CHECKXIT(err);
        while((arg = optparse_arg(&parser))){
            err = MPARC_exists(archive, arg);
            MPARC_CHECKXIT(err);
            {
                unsigned char* b = 0;
                MXPSQL_MPARC_uint_repr_t bs = 0;
                err = MPARC_peek_file(archive, arg, &b, &bs);
                MPARC_CHECKXIT(err);
                for(MXPSQL_MPARC_uint_repr_t i = 0; i < bs; i++){
                    printf("%c", (char) b[i]);
                }
                printf("\n");
            }
        }
    }
    else if(mode == 'y' || mode == 'u'){
        err = MPARC_parse_filename(archive, filename);
        MPARC_CHECKXIT(err);
        while((arg = optparse_arg(&parser))){
            MXPSQL_MPARC_t* cp_archive = NULL;
            err = MPARC_copy(&archive, &cp_archive);
            if(err != MPARC_OK) goto lh;

            err = MPARC_construct_filename(cp_archive, arg);
            if(err != MPARC_OK) goto lh;
            if(verbose) printf("CMD (%c)> %s\n", mode, arg);

            lh:
            MPARC_destroy(&cp_archive);
            MPARC_CHECKXIT(err);
        }
    }
    else if(mode == 's'){
        err = MPARC_parse_filename(archive, filename);
        MPARC_CHECKXIT(err);
        char* f1 = optparse_arg(&parser);
        if(!f1){
            printf("%s: Missing argument for swapping entry\n", exe);
            ex = EXIT_FAILURE;
            goto exit_handler;
        }
        char* f2 = optparse_arg(&parser);
        if(!f2){
            printf("%s: Missing argument for swapping entry\n", exe);
            ex = EXIT_FAILURE;
            goto exit_handler;
        }
        err = MPARC_swap_file(archive, f1, f2);
        MPARC_CHECKXIT(err);
        err = MPARC_construct_filename(archive, filename);
        MPARC_CHECKXIT(err);
    }
    else if(mode == 'k'){
        err = MPARC_parse_filename(archive, filename);
        MPARC_CHECKXIT(err);
        char* f1 = optparse_arg(&parser);
        if(!f1){
            printf("%s: Missing argument for duplicating entry (source)\n", exe);
            ex = EXIT_FAILURE;
            goto exit_handler;
        }
        char* f2 = optparse_arg(&parser);
        if(!f2){
            printf("%s: Missing argument for duplicating entry (destination)\n", exe);
            ex = EXIT_FAILURE;
            goto exit_handler;
        }
        err = MPARC_duplicate_file(archive, 0, f1, f2);
        MPARC_CHECKXIT(err);
        err = MPARC_construct_filename(archive, filename);
        MPARC_CHECKXIT(err);
    }
    else if(mode == 'r'){
        err = MPARC_parse_filename(archive, filename);
        MPARC_CHECKXIT(err);
        char* f1 = optparse_arg(&parser);
        if(!f1){
            printf("%s: Missing argument for renaming entry (old name)\n", exe);
            ex = EXIT_FAILURE;
            goto exit_handler;
        }
        char* f2 = optparse_arg(&parser);
        if(!f2){
            printf("%s: Missing argument for renaming entry (new name)\n", exe);
            ex = EXIT_FAILURE;
            goto exit_handler;
        }
        err = MPARC_rename_file(archive, 0, f1, f2);
        MPARC_CHECKXIT(err);
        err = MPARC_construct_filename(archive, filename);
        MPARC_CHECKXIT(err);
    }
    else{
        printf("What are you trying to do? (%s)\n", filename);
    }

    exit_handler:
    if(archive){
        MPARC_destroy(&archive);
    }
    return ex;

    #undef MPARC_CHECKXIT
}

int main(int argc, char* argv[]){
    printf("MPARC Archive Editor v2\n");
    ((void)optparse_long);
    ((void)optparse_arg);

    char* exe = argv[0];
    int exitC = EXIT_SUCCESS;

    if(argc < 2){
        printf("Usage: \n");
        printf("%s [REPL|CMD] ...\n", exe);
        printf("REPL: access the repl to manipulate the archive.\n");
        printf("CMD: use the command line api to manipulate the archive.\n");
        return EXIT_FAILURE;
    }

    char* mode = argv[1];

    if(strcmp(mode, "REPL") == 0){
        exitC = repl_main(argc-3, exe, &argv[1]);
    }
    else if(strcmp(mode, "CMD") == 0){
        exitC = cmdline_main(argc-3, exe, &argv[1]);
    }
    else{
        printf("Invalid operation (%s)!\n", mode);
        return EXIT_FAILURE;
    }

    return exitC;
}
