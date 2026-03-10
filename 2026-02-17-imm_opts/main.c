#define BASE_IMPLEMENTATION
#include "base.h"
#include "stdio.h"
#include "string.h"

typedef struct CmdLineOption CmdLineOption;

struct CmdLineOption
{
    char *beg;
    char *opl;
    bool is_short;
    bool is_long;
};

typedef enum CmdLineError {
    CmdLineErrorUnknown=1,
    CmdLineErrorExcess,
    CmdLineErrorMissing,
} CmdLineError;


static String const CmdLineErrorMsg[] = {
    [CmdLineErrorUnknown] = str_lit("unknown option"),
    [CmdLineErrorExcess] = str_lit("doesn't accept any argument"),
    [CmdLineErrorMissing] = str_lit("is missing an argument")
};


typedef struct CmdLineOptions CmdLineOptions;
struct CmdLineOptions
{
    char **argv;
    CmdLineOption cur;
    CmdLineError err;
};


static bool clopts_next(CmdLineOptions *clopts)
{
    if (clopts->err) return 0;
    if (clopts->cur.is_long && *clopts->cur.opl != '\0') {
        clopts->err = CmdLineErrorExcess;
        return true;
    }
    if (clopts->cur.is_short && *clopts->cur.opl != '\0') {
        clopts->err = CmdLineErrorUnknown;
        clopts->cur.beg++;
        clopts->cur.opl++;
        return true;
    }

    CmdLineOption next = {0};
    next.beg = *clopts->argv++;
    if (next.beg == NULL || next.beg[0] != '-' || next.beg[1] == '\0') {
        clopts->argv--;
        return false;
    }

    next.beg++;
    if (next.beg[0] == '-') {
        if (next.beg[1] == '\0') {
            return false;
        }
        next.is_long = true;
        next.beg++;
        next.opl = cstr_find_char(next.beg, '=');
    } else {
        next.is_short = true;
        next.opl = &next.beg[1];
    }

    clopts->err = CmdLineErrorUnknown;
    clopts->cur = next;
    return true;
}

static bool clopts_match(CmdLineOptions *clopts, char short_opt, String long_opt)
{
    bool match = false;
    if (clopts->cur.is_short) {
        match = (short_opt != 0 && clopts->cur.beg[0] == short_opt);
    } else if (clopts->cur.is_long) {
        String opt = str_from_range((u8*)clopts->cur.beg, (u8*)clopts->cur.opl);
        match = str_match(long_opt, opt);
    }
    if (match && clopts->err == CmdLineErrorUnknown) {
        clopts->err = 0;
    }

    return match;
}

static bool clopts_chomp_arg(CmdLineOptions *clopts, String *result)
{
    if (*clopts->cur.opl != '\0') {
        *result = str_from_cstr(clopts->cur.opl + clopts->cur.is_long);
        clopts->cur.is_long = false;
        clopts->cur.is_short = false;
        return true;
    } else if (*clopts->argv != NULL) {
        *result = str_from_cstr(*clopts->argv);
        clopts->argv++;
        return true;
    }

    clopts->err = CmdLineErrorMissing;
    return false;
}

static CmdLineOptions clopts_init(int argc, char **argv)
{
    CmdLineOptions clopts = {0};
    clopts.argv = argv + (argv[0] != NULL);
    return clopts;
}


int main(int argc, char **argv)
{
    String tmp;
    CmdLineOptions clopts = clopts_init(argc, argv);

    while (clopts_next(&clopts)) {
        if (clopts_match(&clopts, 'h', str_lit(""))) {
            printf("h enabled\n");
        } else if (clopts_match(&clopts, 'y', str_lit(""))) {
            printf("y enabled\n");
        } else if (clopts_match(&clopts, 'n', str_lit("name")) &&
                clopts_chomp_arg(&clopts, &tmp)) {
            printf("name: %.*s\n", str_varg(tmp));
        }
    }

    if (clopts.err) {
        printf("Option <%s> %.*s", clopts.cur.beg, str_varg(CmdLineErrorMsg[clopts.err]));
    }

    return 0;
}

