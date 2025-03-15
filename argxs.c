/*
 *                 __
 *                / _)
 *       _.----._/ /
 *      /         /     argxs
 *   __/ (  | (  |
 *  /__.-'|_|--|_|
 */
#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>

#include "argxs.h"

#define IS_PATH(a)    ((*a == '/') || (*a == '~') || (*a == '.'))
#define MAX_No_FLAGS  26 + 26 + 10

/* Points to the current flag in case there is an error
 * any information can be obtained from here.
 * Defined as extern.
 */
struct argxs_flag *argxs_current_flag = NULL;

/* Points to the last element read within argv, useful
 * for error handling.
 */
char *argxs_current_element_in_argv = NULL;

struct flagsummary
{
    struct argxs_flag *flag;
    size_t namelen;
};

static struct flagsummary Summary[MAX_No_FLAGS];

enum error_type
{
    err_no_error          = 0,
    err_bad_id_definition = 1,
    err_flag_isnt_defined = 2,
};

static enum error_type Fatal = err_no_error;

enum argvs_kind
{
    argvs_unknown          = 0,
    argvs_double_dash_flag = 1,
    argvs_single_dash_flag = 2,
    argvs_argument         = 3,
};

static uint32_t is_flaglist_ok (struct argxs_flag *const);
static uint16_t get_store_position (const char);

static enum argvs_kind get_kind_of (const char *const);

static enum error_type handle_single_dash (const char);
static enum error_type handle_double_dash (const char*, struct argxs_flag *const flags, const uint32_t);

struct argxs_seen* argxs_get (const int32_t argc, char **argv, struct argxs_flag *const flags)
{
    memset(&Summary, 0, sizeof(Summary));
    const uint32_t nflags = is_flaglist_ok(flags);

    if (Fatal == err_bad_id_definition)
    {
        fprintf(stderr, "[argxs]: programmer's error, IDs should only be [a-zA-Z0-9].\n");
        exit(EXIT_FAILURE);
    }

    struct argxs_seen *seen = (struct argxs_seen*) calloc(nflags, sizeof(struct argxs_seen));

    for (int32_t a = 1; a < argc; a++)
    {
        const enum argvs_kind kind = get_kind_of(argv[a]);
        argxs_current_element_in_argv = argv[a];

        switch (kind)
        {
            case argvs_double_dash_flag:
            case argvs_single_dash_flag:
            {
                const enum error_type e = (kind == argvs_single_dash_flag) ?
                                          handle_single_dash(argv[a][1])   :
                                          handle_double_dash(argv[a] + 2, flags, nflags);

                if (e == err_flag_isnt_defined)
                {
                    exit(69);
                }
                break;
            }
            case argvs_argument:
            {
                printf("argument\n");
                break;
            }
            case argvs_unknown:
            {
                printf("unknown\n");
                break;
            }
        }
    }
    return seen;
}

static uint32_t is_flaglist_ok (struct argxs_flag *const flags)
{
    uint32_t f = 0;

    for (; flags[f].name != NULL; f++)
    {
        char id = flags[f].id;
        if (isalnum(id) == false) { Fatal = err_bad_id_definition; return 0; }

        uint16_t at         = get_store_position(id);
        Summary[at].flag    = &flags[f];
        Summary[at].namelen = strlen(flags[f].name);
    }
    return f;
}

static uint16_t get_store_position (const char id)
{
    if (islower(id)) { return id - 'a'; }
    if (isupper(id)) { return (id - 'A') + 26; }
    return (id - '0') + 26 * 2;
}

static enum argvs_kind get_kind_of (const char *const ele)
{
    if ((*ele == '-') && (ele[1] == '-') && isalnum(ele[2])) { return argvs_double_dash_flag; }
    if ((*ele == '-') && isalnum(ele[1]))                    { return argvs_single_dash_flag; }
    if (isalnum(*ele) || IS_PATH(ele) || *ele == '"')        { return argvs_argument; }
    return argvs_unknown;
}

static enum error_type handle_single_dash (const char id)
{
    const uint16_t at = get_store_position(id);
    if (Summary[at].flag == NULL) { return err_flag_isnt_defined; }

    argxs_current_flag = Summary[at].flag;
    return err_no_error;
}

static enum error_type handle_double_dash (const char *name, struct argxs_flag *const flags, const uint32_t nflags)
{
    for (uint16_t i = 0; i < nflags; i++)
    {
        const uint16_t found_at = get_store_position(flags[i].id);
        const size_t ncmp = Summary[found_at].namelen;

        if (!strncmp(name, flags[i].name, ncmp))
        {
            argxs_current_flag = &flags[i];
            return err_no_error;
        }
    }

    return err_flag_isnt_defined;
}
