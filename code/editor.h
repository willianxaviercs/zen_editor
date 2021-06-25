#ifndef EDITOR_H
#define EDITOR_H

#define STB_TRUETYPE_IMPLEMENTATION
#include "stb_truetype.h"

#define internal static
#define global_variable static

#include "editor_types.h"

#include "editor_math.h"

#define ArrayCount(Arr) (sizeof(Arr) / sizeof(Arr[0]))

#define ZEN_STRING_IMPLEMENTATION
#include "zen_string.h"

#define ZEN_TB_IMPLEMENTATION
#include "zen_tb.h"

/* Microsoft compiler define __inline  specifier */
#if defined(_MSC_VER)
#define inline __inline
#endif

#define OEM_KEYBOARD_KEYS_COUNT 127
#define EDITOR_ROW_POOL_SIZE 10
#define EDITOR_MAX_PATH 250

typedef zen_string editor_string;
typedef zen_tb_text_buffer editor_text_buffer;

#include "editor_parser.h"

// used to classify an operation on the undo stack
typedef enum 
{
    NO_OP,
    INSERT_CHAR,
    BACKSPACE_DELETE_CHAR,
    BACKSPACE_DELETE_WORD,
    DELETE_RANGE,
    COPY_TO_CLIPBOARD,
    PASTE_FROM_CLIPBOARD,
    CUT_TO_CLIPBOARD,
    DELETE_LINE,
    CREATE_LINE
} OP_TYPE;

// this is struct passed zeored means an NO_OP operation
struct editor_operation
{
    OP_TYPE type;
    u32 cursor_x;
    u32 cursor_y;
    u32 mark_x;
    u32 mark_y;
    u8 single_char;
    u32 compound_op_size;
};

/* Undo Ring Buffer */
#define MAX_UNDO_STACK_OPS (1 << 16)
struct editor_operation_stack
{
    editor_operation op[MAX_UNDO_STACK_OPS];
    u32 index;
    u32 pop_count;
};

struct editor_memory_arena
{
    u8 *buffer;
    u32 offset;
    u32 total_size;
};

enum editor_mode
{
    EDIT_MODE,
    SEARCH_MODE,
    OPEN_FILE_MODE,
    SWITCH_FILE_MODE,
    AUTOCOMPLETE_MODE
};

struct editor_theme
{
    // TODO(willian): menus background colors, menus font colors, menu hover color
    //    info bar background color, info bar font color
    v4 default_color;
    v4 cursor_color;
    v4 mark_color;
    v4 identifier_color;
    v4 keyword_color;
    v4 string_constant_color;
    v4 number_constant_color;
    v4 comment_color;
    v4 preprocessor_color;
    v4 font_color;
    v4 line_highlight_color;
    v4 background_color;
};

//////////////// structs
struct key_state
{
    bool wasDown;
    bool endedDown;
};

struct keyboard_input
{
    key_state keys[OEM_KEYBOARD_KEYS_COUNT];
    key_state keyUp;
    key_state keyDown;
    key_state keyLeft;
    key_state keyRight;
    key_state spacebar;
    key_state backspace;
    key_state shift;
    key_state control;
    key_state enter;
    key_state escape;
    key_state alt;
    key_state home;
    key_state end;
    key_state tab;
};

// zero initalized
struct editor_clipboard
{
    u32 size;
    b32 has_changed;
    char *data;
};

struct editor_rectangle
{
    int x, y, dx, dy;
};

struct editor_screenbuffer
{
    int width;
    int height;
    int bytes_per_pixel;
    u32 pitch;
    void *memory;
};

enum
{
    SEARCH_MODE_NORMAL,
    SEARCH_MODE_REVERSE
};

enum
{
    FILE_TYPE_NORMAL,
    FILE_TYPE_DIR,
    FILE_TYPE_NOT_SUPPORTED
};

struct editor_file_info
{
    u32 size;
    u32 type;
    char *filename;
    u32 filename_size;
};

// TODO(willian): check the MAX_FILES
#define EDITOR_MAX_FILES 1024
struct editor_files_listing
{
    editor_file_info files[EDITOR_MAX_FILES];
    u32 max_index;
    u32 current_index;
};

#if 0 // TODO(willian): remove
struct editor_line
{
    char *data;
    u32 buffer_size;
    u32 length;
    bool edited;
};
#endif

struct glyphs
{
    int width;
    int height;
    int x0, y0, x1, y1;
    int yOffset;
    int xOffset;
    int advance;
    unsigned char *bitmap;
};

struct editor_font
{
    char *name;
    int  size;
    u32 width;
    int ascent, descent, lineGap;
    int  x0, x1, y0, y1;
    float scale;
    struct glyphs glyph[255];
};

struct editor_text_buffer_node
{
    editor_text_buffer *text_buffer;
    editor_text_buffer_node *next;
};

struct editor_text_buffer_list
{
    editor_text_buffer_node *opened_file;
    u32 number_of_files;
    u32 current_index;
};

typedef struct token_to_query
{
    char *token;
    u32 length;
    u32 *matches_indexes;
    u32 current_match_index;
    u32 matches_count;
} token_to_query;

struct editor_search_buffer
{
    u32 cursor;
    u8  data[1024];
    u32 total_size;
    u32 current_size;
    b32 found_match;
    b32 skip_match;
    u32 search_mode;
};

#if 0
struct text_panel
{
    u32 caret;
    char *text;
    bool updated;
};

typedef struct editor_lister_panel
{
    char *label;
    text_panel panel;
    
} editor_lister_panel;
#endif

struct editor_state
{
    editor_mode mode;
    
    // an data structure hold all the identifiers tokens parsed
    token_list list_of_tokens;
    token_to_query autocompletion_token;
    
    // pointer to the current focused text buffer
    editor_text_buffer *current_text_buffer;
    
    // linked list of the current opened files in the editor
    editor_text_buffer_list opened_files_list;
    
    // permanent memory arena
    editor_memory_arena permanent_storage;
    
    // transient memory arena
    editor_memory_arena frame_storage;
    
    editor_search_buffer search_buffer;
    editor_search_buffer switch_buffer;
    editor_search_buffer open_file_buffer;
    editor_files_listing files_to_open;
    
    // undo action stack
    editor_operation_stack undo_stack;
    
    // copy clipboard
    editor_clipboard copy_clipboard;
    
    // paste clipboard
    editor_clipboard paste_clipboard;
    
    // color schemes
    editor_theme theme;
    
    // window screen buffer
    editor_screenbuffer screen_buffer;
    
    // platform specific function  pointers
    void * (*platform_memory_alloc)(u32);
    void   (*platform_memory_free)(void *);
};

typedef struct
{
    b32 IsInitialized;
    u64 PermanentSize;
    u64 TemporarySize;
    void *PermanentStorage;
    void *TemporaryStorage;
} editor_memory;


#include "editor_render.h"


// TODO(willian): there is more prototypes to pull from editor.cpp
//                    just for the sake of completion

/* TODO remove platform specif functions from here */
internal char *
win32_open_file_into_buffer(char *filename);
/* TODO  */

// initalize editor state
internal void editor_init(editor_state *ed);

internal void editor_text_buffer_list_add_node(editor_text_buffer *text_buffer, editor_state *ed);

//////////////// prototypes
internal void editor_switch_current_text_buffer(editor_state *ed, editor_text_buffer_list *files_list);

internal void editor_save_file(editor_text_buffer *text_buffer);

// search 
internal void editor_edit_search_prompt(keyboard_input *keyboard, editor_search_buffer *search_buffer, editor_state *ed);

internal void editor_prompt_delete_char(editor_search_buffer *search_buffer);

internal void editor_prompt_insert_char(u8 c, editor_search_buffer *search_buffer);

// undo stack operations 
internal editor_operation editor_pop_op_from_stack(editor_operation_stack *stack);

// initialize font atlas from a ttf file
internal void editor_font_init(editor_font *font, unsigned char *ttf_buffer);

// update cursor state based in input
internal void editor_cursor_update(keyboard_input *keyboard, editor_state *ed, editor_text_buffer *text_buffer);

////// cursor movement
internal void editor_cursor_move_foward_word(editor_text_buffer *text_buffer, editor_state *ed);
internal void editor_cursor_move_backward_word(editor_text_buffer *text_buffer, editor_state *ed);
internal void editor_cursor_move_next_blank_line(editor_text_buffer *text_buffer, editor_state *ed);
internal void editor_cursor_move_previos_blank_line(editor_text_buffer *text_buffer, editor_state *ed);

internal void editor_cursor_move_left(editor_state *ed, editor_text_buffer *text_buffer);
internal void editor_cursor_move_right(editor_state *ed, editor_text_buffer *text_buffer);
internal void editor_cursor_move_up(editor_state *ed, editor_text_buffer *text_buffer);
internal void editor_cursor_move_down(editor_state *ed, editor_text_buffer *text_buffer);

// set the mark position 
internal void editor_set_mark(editor_text_buffer *text_buffer, editor_state *ed);

// swap cursor and mark position
internal void editor_cursor_mark_swap(editor_text_buffer *text_buffer, editor_state *ed);

// insert a char into a line
internal void editor_insert_char(u8 c , editor_text_buffer *text_buffer, editor_state *ed, bool user_op = true);

// insert a new line into the text buffer
internal void editor_insert_newline(editor_text_buffer *text_buffer, editor_state *ed, bool user_op = true);

// create text buffer from a file buffer
internal editor_text_buffer *editor_text_buffer_create(char *file_buffer, editor_state *ed);

// make edits to the text buffer based on input
internal void editor_text_buffer_edit(editor_text_buffer *text_buffer, keyboard_input *keyboard, editor_state *ed);

// delete the range between the mark and the cursor
internal void editor_delete_range(editor_text_buffer *text_buffer, editor_state *ed);

// copy the range  between the mark and the cursor
internal void editor_copy_into_clipboard(editor_text_buffer *text_buffer, editor_state *ed);

// copy and delete the range  between the mark and the cursor
internal void editor_cut_into_clipboard(editor_text_buffer *text_buffer, editor_state *ed);

// swap the current line with the line above
internal void editor_swap_line_up(editor_text_buffer *text_buffer, editor_state *ed);

// swap the current line with the line below
internal void editor_swap_line_down(editor_text_buffer *text_buffer, editor_state *ed);

// delete the current line
internal void editor_delete_line(editor_text_buffer *text_buffer, editor_state *ed, bool user_op = true);

// delete the caracter left to the cursor and moves the cursor
internal void editor_backspace_delete(editor_text_buffer *text_buffer, editor_state *ed, bool user_op = true);

// main functin of the editor
internal void editor_update_and_render(editor_memory *EditorMemory, editor_screenbuffer *screen_buffer, editor_font *font, editor_text_buffer *text_buffer, 
                                       keyboard_input *keyboard, editor_rectangle rect);

#endif /* EDITOR_H */