
#include "stdio.h"
#include "stdlib.h"

#define BASE_IMPLEMENTATION
#include "base.h"

typedef struct ParseAttribResult ParseAttribResult;
struct ParseAttribResult {
    String rest;
    String attrib;
    bool ok;
};

static ParseAttribResult parse_attrib(String source, String name)
{
    ParseAttribResult result = {};
    bool errored = !str_starts_with(source, name);
    if (!errored) {
        String rest = str_skip(source, name.len);
        uz newline = str_find_newline(rest, 0);
        String attrib = str_from_range(&rest.str[0], &rest.str[newline]);
        attrib = str_trim(attrib);

        rest = str_skip(rest, newline);
        rest = str_trim_left(rest);

        result.rest = rest;
        result.attrib = attrib;
        result.ok = true;
    }

    return result;
}

typedef struct TextBlock TextBlock;
struct TextBlock {
    String text;
    bool is_code;
    bool is_quote;
    bool is_ref;
};

typedef struct TextBlockArray TextBlockArray;
struct TextBlockArray {
    uz count;
    uz cap;
    TextBlock *first[10];
};


typedef struct LbEntry LbEntry;
struct LbEntry {
    TextBlockArray text_blocks;
    LbEntry *next;
    String title;
    String link;
    String via;
    String tags;
};

static LbEntry lb_entry_nil = {
    .text_blocks={},
    .next=&lb_entry_nil,
    .title={0},
    .link={0},
    .via={0},
    .tags={0},
};

typedef struct LbEntryList LbEntryList;
struct LbEntryList {
    LbEntry *first;
    uz count;
};

static void text_block_push(TextBlockArray *tba, TextBlock *text_block)
{
    ASSERT(tba->count < tba->cap);
    tba->first[tba->count] = text_block;
    tba->count++;
}

static String parse_link_blog_file(Arena *arena, String source)
{
    String result = {};
    String rest = source;

    TextBlockArray text_blocks = {0};

    bool ok = true;

    rest = str_trim_left(rest);
    while (rest.len && rest.str[0] == '=' && ok) {
        rest = str_trim_left(rest);
        uz newline = str_find_newline(rest, 0);

        String title = str_trim(str_from_range(&rest.str[1], &rest.str[newline]));
        printf("title: %.*s\n", (int)title.len, title.str);
        rest = str_skip(rest, newline);
        rest = str_trim_left(rest);

        ParseAttribResult parsed_link = parse_attrib(rest, str_lit("link:"));
        ok &= parsed_link.ok;
        rest = parsed_link.rest;
        printf("link: %.*s\n", (int)parsed_link.attrib.len, parsed_link.attrib.str);

        ParseAttribResult parsed_via = parse_attrib(rest, str_lit("via:"));
        ok &= parsed_via.ok;
        rest = parsed_via.rest;
        printf("via: %.*s\n", (int)parsed_via.attrib.len, parsed_via.attrib.str);

        ParseAttribResult parsed_tags = parse_attrib(rest, str_lit("tags:"));
        ok &= parsed_tags.ok;
        rest = parsed_tags.rest;
        printf("tags: %.*s\n", (int)parsed_tags.attrib.len, parsed_tags.attrib.str);

        rest = str_trim_left(rest);
        while (rest.len && rest.str[0] != '=') {
            if (char_is_alpha(rest.str[0])) {
                uz at = str_find_newline(rest, 0);
                TextBlock *paragraph = arena_push(arena, TextBlock, 1);
                text_block_push(&text_blocks, paragraph);
                paragraph->text = str_from_range(&rest.str[0], &rest.str[at]);

                rest = str_skip(rest, at);
            } else if (rest.str[0] == '-') {
                if (str_starts_with(rest, str_lit("--code--"))) {
                    rest = str_skip_line(rest);
                    String search_closing = rest;

                    while (search_closing.len && !str_starts_with(search_closing, str_lit("----"))) {
                        search_closing = str_skip_line(search_closing);
                    }

                    TextBlock *code_block = arena_push(arena, TextBlock, 1);
                    uz len_code_block = rest.len - search_closing.len;
                    code_block->text = str_prefix(rest, len_code_block);
                    code_block->is_code = true;
                    text_block_push(&text_blocks, code_block);

                    rest = str_skip_line(search_closing);

                } else if (str_starts_with(rest, str_lit("--quote--"))) {

                }
            } else if (char_is_digit(rest.str[0], 10)) {
                while (char_is_digit(rest.str[0], 10)) {
                    newline = str_find_newline(rest, 0);
                    TextBlock *ref = arena_push(arena, TextBlock, 1);
                    ref->text = str_trim(str_from_range(&rest.str[0], &rest.str[newline]));
                    ref->is_ref = true;
                    text_block_push(&text_blocks, ref);

                    rest = str_skip(rest, newline);
                }
            }
            rest = str_trim_left(rest);
        }
    }

    printf("block count %zu\n", text_blocks.count);

    for (uz i = 0; i < text_blocks.count; i++) {
        TextBlock *tb = text_blocks.first[i];
        printf("%.*s\n", (int)tb->text.len, tb->text.str);
    }

    return result;
}


int main(void)
{
    Arena arena = arena_init(malloc(MiB(4)), MiB(4));
    String link_file_data = str_from_file(&arena, str_lit("links.hhl"));

    parse_link_blog_file(&arena, link_file_data);

    return 0;
}
