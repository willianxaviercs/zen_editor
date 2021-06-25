#ifndef PARSER_H
#define PARSER_H

typedef struct parser_token
{
    char *start;
    zen_tb_size length;
} parser_token;

#define AUTOCOMPLETE_BUFFER_SIZE (1024 * 1024)
typedef struct parser_autocomplete_buffer_header
{
    size_t length;
    size_t capacity;
} parser_autocomplete_buffer_header;

typedef char * autocomplete_buffer;

typedef struct token_list
{
    size_t tokens_count;
    size_t capacity;
    char **token_position;
    char *tokens_string;
} token_list;


#define AUTO_BUFFER_HEADER(str) ((parser_autocomplete_buffer_header *)str - 1)

enum token_type
{
    UNKNOWN_TOKEN,
    IDENTIFIER_TOKEN,
    KEYWORD_TOKEN,
    STRING_CONSTANT_TOKEN,
    MULTILINE_STRING_CONSTANT_TOKEN,
    NUMBER_CONSTANT_TOKEN,
    SINGLE_COMMENT_TOKEN,
    MULTI_COMMENT_TOKEN,
    PREPROCESSOR_TOKEN,
    DELIMITER_TOKEN,
    LEFT_CURLY_TOKEN,
    RIGHT_CURLY_TOKEN
};

/* TODO, hardcode by now, but we can define the list of keywords based on file extension name */
#define C_KEYWORDS_COUNT 32
char *editor_c_keywords_list[]= {
    "static", "void",     "char",     "short",
    "long",   "auto",     "break",    "case",
    "const",  "continue", "default",  "do",
    "double", "else",     "enum",     "extern",
    "float",  "for",      "goto",     "if",
    "int",    "register", "return",   "signed",
    "sizeof", "struct",   "switch",   "typedef",
    "union",  "unsigned", "volatile", "while"
};

/* autocomplete */
internal autocomplete_buffer parser_autocomplete_buffer_create(size_t capacity);

internal void parser_autocomplete_buffer_destroy(autocomplete_buffer str);

internal inline size_t parser_autocomplete_buffer_capacity(autocomplete_buffer str);

internal inline size_t parser_autocomplete_buffer_length(autocomplete_buffer str);

internal autocomplete_buffer paser_autocomplete_buffer_append_token(autocomplete_buffer str, parser_token token);

internal inline void parser_autocomplete_buffer_set_length(autocomplete_buffer str, size_t length);

/* parser */
internal parser_token editor_parser_get_token(zen_tb_string line, token_type last_line_token_type);

internal token_type editor_parser_get_token_type(parser_token token, token_type last_line_token);

internal void editor_parse_text_buffer(editor_text_buffer *text_buffer, token_list *list_of_tokens);

internal token_type  editor_parse_line(zen_tb_string line, token_type last_line_token_type, token_list *list_of_tokens);

/* helpers */
internal inline bool editor_parser_isdelimiter(char c);

internal inline bool editor_parser_isalphabetic(char c);

internal inline bool editor_parser_isnumeric(char c);

internal inline bool editor_parser_isalphanumeric(char c);

internal inline bool editor_parser_is_hexadecimal_digit(char c);

internal inline bool editor_parser_is_token_delimiter(char c);

internal bool editor_string_compare(char *str1, size_t str1_size, char *str2, size_t str2_size);

#endif /* PARSER_H */