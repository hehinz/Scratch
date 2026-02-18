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

typedef struct CmdLine CmdLine;
struct CmdLine
{
    char **argv;
    CmdLineOption cur;
    CmdLineError err;
};

static bool cmd_line_next(CmdLine *cmd_line)
{
    if (cmd_line->err) return 0;
    if (cmd_line->cur.is_long && *cmd_line->cur.opl != '\0') {
        cmd_line->err = CmdLineErrorExcess;
        return true;
    }
    if (cmd_line->cur.is_short && *cmd_line->cur.opl != '\0') {
        cmd_line->err = CmdLineErrorUnknown;
        cmd_line->cur.beg++;
        cmd_line->cur.opl++;
        return true;
    }

    CmdLineOption next = {0};
    next.beg = *cmd_line->argv++;
    if (next.beg == NULL || next.beg[0] != '-' || next.beg[1] == '\0') {
        cmd_line->argv--;
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

    cmd_line->cur = next;
    return true;
}

static bool cmd_line_match(CmdLine *cmd_line, char short_opt, String long_opt)
{
    bool match = false;
    if (cmd_line->cur.is_short) {
        match = (short_opt != 0 && cmd_line->cur.beg[0] == short_opt);
    } else if (cmd_line->cur.is_long) {
        String opt = str_from_range((u8*)cmd_line->cur.beg, (u8*)cmd_line->cur.opl);
        match = str_match(long_opt, opt);
    }
    if (match && cmd_line->err == CmdLineErrorUnknown) {
        cmd_line->err = 0;
    }

    return match;
}

static bool cmd_line_chomp_arg(CmdLine *cmd_line, String *result)
{
    if (*cmd_line->cur.opl != '\0') {
        *result = str_from_cstr(cmd_line->cur.opl + cmd_line->cur.is_long);
        cmd_line->cur.is_long = false;
        cmd_line->cur.is_short = false;
        return true;
    } else if (*cmd_line->argv != NULL) {
        *result = str_from_cstr(*cmd_line->argv);
        cmd_line->argv++;
        return true;
    }

    cmd_line->err = CmdLineErrorMissing;
    return false;
}

int main(int argc, char **argv)
{
    CmdLine cmd_line = {0};
    cmd_line.argv = argv + (argv[0] != NULL);
    String tmp;

    while (cmd_line_next(&cmd_line)) {
        if (cmd_line_match(&cmd_line, 'h', str_lit(""))) {
            printf("h enabled\n");
        } else if (cmd_line_match(&cmd_line, 'y', str_lit(""))) {
            printf("y enabled\n");
        } else if (cmd_line_match(&cmd_line, 'n', str_lit("name")) &&
                cmd_line_chomp_arg(&cmd_line, &tmp)) {
            printf("name: %.*s\n", str_varg(tmp));
        }
    }
    if (cmd_line.err) {
        printf("Option <%s> %.*s", cmd_line.cur.beg, str_varg(CmdLineErrorMsg[cmd_line.err]));
    }

    return 0;
}

