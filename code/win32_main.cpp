
/* PLATFFORM TODO LIST 

1 - Reimplement Clipboard stuff

2 - Implement Lauch .bat file
*/

#include <windows.h>

#if 1
#include <memory.h>
#include <malloc.h>
#include <stdlib.h>
#endif

#include <GL/gl.h>

#include <stdio.h> // _snprintf_s

// Win32 Specific Assertion Macro
#ifdef EDITOR_DEBUG
#define Assert(expr) do { if (!(expr)) __debugbreak(); } while(0)
#else
#define Assert(expr)
#endif

/* all platform non specific code is in this file */
#include "editor.cpp"

#include <strsafe.h>

global_variable b32 EditorRunning;

struct win32_offscreen_buffer
{
    void *memory;
    BITMAPINFO bmi;
    int width;
    int height;
    int bytes_per_pixel;
};

static void
ProcessKeyboardInput(keyboard_input *keyboard, WPARAM key, bool isDown)
{
    keyboard->keys[key].endedDown = isDown;
}

static void *
win32_memory_alloc(u32 size)
{
    void *memory = NULL;
    DWORD error_id;
    
    // TODO(willian): can't use virtual alloc to do little allocations the way I was doing
    //                gotta see how to make this works or use calloc, or use a arena and
    //                allocate at larges chunks
    
    //memory = VirtualAlloc(0, size, MEM_COMMIT|MEM_RESERVE, PAGE_READWRITE);
    
    memory = malloc(size);
    
    if (memory == NULL)
    {
        error_id = GetLastError();
    }
    
    return memory;
}

static void 
win32_memory_free(void *memory)
{
    if (memory)
    {
        //VirtualFree(memory, 0, MEM_RELEASE);
        free(memory);
    }
}

static bool
win32_copy_from_clipboard(editor_clipboard *paste_clipboard, HWND window)
{
    if(!IsClipboardFormatAvailable(CF_TEXT)) return false;
    
    if(!OpenClipboard(window)) return false;
    
    HANDLE clipboard_handle = GetClipboardData(CF_TEXT);
    if(!clipboard_handle)
    {
        CloseClipboard();
        return false;
    }
    
    LPSTR platform_clipboard = (LPSTR)GlobalLock(clipboard_handle);
    
    if(!platform_clipboard)
    {
        CloseClipboard();
        return false;
    }
    
    // calcucalte size of clipboard content
    u32 size = 0;
    for (char *c = platform_clipboard; *c; c++) size++;
    
    // free last paste clipboard data
    if (paste_clipboard->data)
    {
        free(paste_clipboard->data);
        paste_clipboard->data = 0;
    }
    
    paste_clipboard->data = (char *)calloc(size + 1, sizeof(char));
    Assert(paste_clipboard->data);
    
    memcpy(paste_clipboard->data, platform_clipboard, size);
    
    paste_clipboard->size = size + 1;
    
    CloseClipboard();
    return true;
}

static bool
win32_copy_to_clipboard(editor_clipboard *clipboard, HWND window)
{
    
    LPSTR copy_data_dest = NULL;
    HGLOBAL global_copy = NULL;
    HANDLE clipboard_handle = NULL;
    
    // 
    global_copy = GlobalAlloc(GMEM_MOVEABLE, clipboard->size * sizeof(TCHAR));
    
    if(global_copy == NULL) return 0;
    
    // copy the data to the buffer
    copy_data_dest = (LPSTR)GlobalLock(global_copy);
    CopyMemory(copy_data_dest, clipboard->data, clipboard->size);
    GlobalUnlock(global_copy);
    
    if(!OpenClipboard(window)) return 0;
    
    EmptyClipboard();
    
    clipboard_handle = SetClipboardData(CF_OEMTEXT, global_copy);
    if(!clipboard_handle) return 0;
    CloseClipboard();
    return 1;
}

static u8 *
win32_open_ttf_font_buffer(char *filename)
{
    HANDLE file_handle = CreateFile(filename, GENERIC_READ, 0, 0, OPEN_EXISTING, 
                                    FILE_ATTRIBUTE_NORMAL, 0);
    
    if (file_handle == INVALID_HANDLE_VALUE) return 0;
    
    DWORD file_size = GetFileSize(file_handle, 0);
    
    u8 *buffer; 
    buffer = (u8 *)malloc(sizeof(*buffer) * file_size);
    
    DWORD bytes_written;
    ReadFile(file_handle, buffer, file_size, &bytes_written, 0);
    
    CloseHandle(file_handle);
    
    return buffer;
}

static char *
win32_open_file_into_buffer(char *filename)
{
	HANDLE file_handle = CreateFile(filename, GENERIC_READ, 0, 0, OPEN_EXISTING,
                                    FILE_ATTRIBUTE_NORMAL, 0);
    
    if (file_handle == INVALID_HANDLE_VALUE) return 0;
    
    DWORD file_size = GetFileSize(file_handle, 0);
    
    char *buffer; 
    buffer = (char *)malloc((sizeof(*buffer) * file_size) + 1);
    
    DWORD bytes_written;
    ReadFile(file_handle, buffer, file_size, &bytes_written, 0);
    buffer[file_size] = 0;
    
    CloseHandle(file_handle);
    
    return buffer;
}

static void
DecodeKeyboardInput(keyboard_input *keyboard, WPARAM key, bool isDown, bool wasDown)
{
    // TODO(willian): implement keyboard layout
    // NOTE(willian): need and heavy rework in the way we
    // process keyboard input and layout
    // this is only a prototype for testing
    
    // OEM ASCII KEYS
    WPARAM semicolon = VkKeyScanA(';') & 0x00FF;
    WPARAM comma = VkKeyScanA(',') & 0x00FF;
    WPARAM dot = VkKeyScanA('.') & 0x00FF;
    WPARAM backslash = VkKeyScanA('\\') & 0x00FF;
    WPARAM tilde = VkKeyScanA('~') & 0x00FF;
    WPARAM leftBracket = VkKeyScanA('[') & 0x00FF;
    WPARAM rightBracket = VkKeyScanA(']') & 0x00FF;
    WPARAM singleQuote = VkKeyScanA('\'') & 0x00FF;
    WPARAM minusSignal = VkKeyScanA('-') & 0x00FF;
    WPARAM equalSignal = VkKeyScanA('=') & 0x00FF;
    WPARAM slash = VkKeyScanA('/') & 0x00FF;
    WPARAM capslock = GetKeyState(VK_CAPITAL);
    WPARAM capsToggled = capslock & 1;
    
    switch(key)
    {
        case VK_TAB:
        {
            keyboard->tab.endedDown = isDown;
            break;
        }
        // SYSTEM KEYS
        case VK_SPACE:
        {
            keyboard->spacebar.endedDown = isDown;
            break;
        }
        case VK_BACK:
        {
            keyboard->backspace.endedDown = isDown;
            break;
        }
        case VK_SHIFT:
        {
            keyboard->shift.endedDown = isDown;
            break;
        }
        case VK_CONTROL:
        {
            keyboard->control.endedDown = isDown;
            break;
        }
        case VK_MENU:
        {
            keyboard->alt.endedDown = isDown;
            break;
        }
        case VK_RETURN:
        {
            keyboard->enter.endedDown = isDown;
            break;
        }
        case VK_LEFT:
        {
            keyboard->keyLeft.endedDown = isDown;
            break;
        }
        case VK_RIGHT:
        {
            keyboard->keyRight.endedDown = isDown;
            break;
        }
        case VK_UP:
        {
            keyboard->keyUp.endedDown = isDown;
            break;
        }
        case VK_DOWN:
        {
            keyboard->keyDown.endedDown = isDown;
            break;
        }
        case VK_ESCAPE:
        {
            keyboard->escape.endedDown = isDown;
            break;
        }
        case VK_HOME:
        {
            keyboard->home.endedDown = isDown;
            break;
        }
        case VK_END:
        {
            keyboard->end.endedDown = isDown;
            break;
        }
        default:
        {
            // ALPHABETIC KEYS
            if((key >= 'A') && (key <= 'Z'))
            {
                if(keyboard->shift.endedDown ||
                   capsToggled)
                {
                    ProcessKeyboardInput(keyboard, key, isDown);
                }
                else
                {
                    ProcessKeyboardInput(keyboard, key + 32, isDown);
                }
            }
            // NUMERIC KEYS
            else if(key == '0')
            {
                if(keyboard->shift.endedDown)
                {
                    ProcessKeyboardInput(keyboard, ')', isDown);
                }
                else
                {
                    ProcessKeyboardInput(keyboard, '0', isDown);
                }
            }
            else if(key == '1')
            {
                if(keyboard->shift.endedDown)
                {
                    ProcessKeyboardInput(keyboard, '!', isDown);
                }
                else
                {
                    ProcessKeyboardInput(keyboard, '1', isDown);
                }
            }
            
            else if(key == '2')
            {
                if(keyboard->shift.endedDown)
                {
                    ProcessKeyboardInput(keyboard, '@', isDown);
                }
                else
                {
                    ProcessKeyboardInput(keyboard, '2', isDown);
                }
            }
            else if(key == '3')
            {
                if(keyboard->shift.endedDown)
                {
                    ProcessKeyboardInput(keyboard, '#', isDown);
                }
                else
                {
                    ProcessKeyboardInput(keyboard, '3', isDown);
                }
            }
            else if(key == '4')
            {
                if(keyboard->shift.endedDown)
                {
                    ProcessKeyboardInput(keyboard, '$', isDown);
                }
                else
                {
                    ProcessKeyboardInput(keyboard, '4', isDown);
                }
            }
            
            else if(key == '5')
            {
                if(keyboard->shift.endedDown)
                {
                    ProcessKeyboardInput(keyboard, '%', isDown);
                }
                else
                {
                    ProcessKeyboardInput(keyboard, '5', isDown);
                }
            }
            else if(key == '6')
            {
                ProcessKeyboardInput(keyboard, '6', isDown);
            }
            else if(key == '7')
            {
                if(keyboard->shift.endedDown)
                {
                    ProcessKeyboardInput(keyboard, '&', isDown);
                }
                else
                {
                    ProcessKeyboardInput(keyboard, '7', isDown);
                }
            }
            else if(key == '8')
            {
                if(keyboard->shift.endedDown)
                {
                    ProcessKeyboardInput(keyboard, '*', isDown);
                }
                else
                {
                    ProcessKeyboardInput(keyboard, '8', isDown);
                }
            }
            else if(key == '9')
            {
                if(keyboard->shift.endedDown)
                {
                    ProcessKeyboardInput(keyboard, '(', isDown);
                }
                else
                {
                    ProcessKeyboardInput(keyboard, '9', isDown);
                }
            }
            
            // OEM ASCII KEYS
            else if(key == semicolon)
            {
                if(keyboard->shift.endedDown)
                {
                    ProcessKeyboardInput(keyboard, ':', isDown);
                }
                else
                {
                    ProcessKeyboardInput(keyboard, ';', isDown);
                }
            }
            else if(key == comma)
            {
                if(keyboard->shift.endedDown)
                {
                    ProcessKeyboardInput(keyboard, '<', isDown);
                }
                else
                {
                    ProcessKeyboardInput(keyboard, ',', isDown);
                }
            }
            else if(key == dot)
            {
                if(keyboard->shift.endedDown)
                {
                    ProcessKeyboardInput(keyboard, '>', isDown);
                }
                else
                {
                    ProcessKeyboardInput(keyboard, '.', isDown);
                }
            }
            else if(key == backslash)
            {
                if(keyboard->shift.endedDown)
                {
                    ProcessKeyboardInput(keyboard, '|', isDown);
                }
                else
                {
                    ProcessKeyboardInput(keyboard, '\\', isDown);
                }
            }
            else if(key == tilde)
            {
                if(keyboard->shift.endedDown)
                {
                    ProcessKeyboardInput(keyboard, '^', isDown);
                }
                else
                {
                    ProcessKeyboardInput(keyboard, '~', isDown);
                }
            }
            else if(key == leftBracket)
            {
                if(keyboard->shift.endedDown)
                {
                    ProcessKeyboardInput(keyboard, '{', isDown);
                }
                else
                {
                    ProcessKeyboardInput(keyboard, '[', isDown);
                }
            }
            else if(key == rightBracket)
            {
                if(keyboard->shift.endedDown)
                {
                    ProcessKeyboardInput(keyboard, '}', isDown);
                }
                else
                {
                    ProcessKeyboardInput(keyboard, ']', isDown);
                }
            }
            else if(key == singleQuote)
            {
                if(keyboard->shift.endedDown)
                {
                    ProcessKeyboardInput(keyboard, '"', isDown);
                }
                else
                {
                    ProcessKeyboardInput(keyboard, '\'', isDown);
                }
            }
            else if(key == minusSignal)
            {
                if(keyboard->shift.endedDown)
                {
                    ProcessKeyboardInput(keyboard, '_', isDown);
                }
                else
                {
                    ProcessKeyboardInput(keyboard, '-', isDown);
                }
            }
            else if(key == equalSignal)
            {
                if(keyboard->shift.endedDown)
                {
                    ProcessKeyboardInput(keyboard, '+', isDown);
                }
                else
                {
                    ProcessKeyboardInput(keyboard, '=', isDown);
                }
            }
            else if(key == slash)
            {
                if(keyboard->shift.endedDown)
                {
                    ProcessKeyboardInput(keyboard, '?', isDown);
                }
                else
                {
                    ProcessKeyboardInput(keyboard, '/', isDown);
                }
            }
            else if(key == 0xC1)
            {
                if(keyboard->shift.endedDown)
                {
                    ProcessKeyboardInput(keyboard, '?', isDown);
                }
                else
                {
                    ProcessKeyboardInput(keyboard, '/', isDown);
                }
            }
            break;
        }
    }
}

/* intializes opengl on windows, any failure on the initialization
 *  we can log and switch  to the software renderer
*/
static void
win32_init_opengl(HDC hdc)
{
    PIXELFORMATDESCRIPTOR desired_pixel_format = {};
    desired_pixel_format.nSize = sizeof(PIXELFORMATDESCRIPTOR);
    desired_pixel_format.nVersion = 1;
    desired_pixel_format.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL| PFD_DOUBLEBUFFER;
    desired_pixel_format.iPixelType = PFD_TYPE_RGBA;
    desired_pixel_format.cColorBits = 32;
    desired_pixel_format.cAlphaBits = 8;
    desired_pixel_format.iLayerType = PFD_MAIN_PLANE;
    
    int sugested_pixel_format_index = ChoosePixelFormat(hdc, &desired_pixel_format);
    
    if (sugested_pixel_format_index == 0)
    {
        // log: could not find a desired pixel format
    }
    
    PIXELFORMATDESCRIPTOR sugested_pixel_format = {};
    
    DescribePixelFormat(hdc, sugested_pixel_format_index, sizeof(PIXELFORMATDESCRIPTOR), &sugested_pixel_format);
    
    SetPixelFormat(hdc, sugested_pixel_format_index, &sugested_pixel_format);
    
    HGLRC opengl_rc = wglCreateContext(hdc);
    
    if (wglMakeCurrent(hdc, opengl_rc))
    {
        // success log 
    }
    else
    {
        // TODO(willian): we can log the error, than switch to the software renderer
    }
}

static void
Win32WindowUpdate(HDC hdc, RECT rect, win32_offscreen_buffer *buffer)
{
#if EDITOR_OPENGL
    // window coords
    glViewport(0, 0, buffer->width, buffer->height);
    
    static bool init = false;
    
    GLuint texture_name = 1;
    
    glBindTexture(GL_TEXTURE_2D, texture_name);
    
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, buffer->width, buffer->height, 0,
                 GL_BGRA_EXT, GL_UNSIGNED_BYTE, buffer->memory);
    
    // GL_LINEAR GL_NEAREST
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
    
    // GL_MODULATE GL_DECAL, GL_BLEND, or GL_REPLACE.
    glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
    
    glEnable(GL_TEXTURE_2D);
    
    // clear screen
    //glClearColor(1.0f, 0, 1.0f, 0);
    //glClear(GL_COLOR_BUFFER_BIT);
    
    glMatrixMode(GL_TEXTURE);
    glLoadIdentity();
    
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    
    glBegin(GL_TRIANGLES);
    
    float p = 1.0f;
    
    // UPPER
    glTexCoord2f(0, 1.0f);
    glVertex2f(-p, -p);
    
    glTexCoord2f(1.0f, 0);
    glVertex2f(p, p);
    
    glTexCoord2f(0, 0);
    glVertex2f(-p, p);
    
    // lower triangle
    glTexCoord2f(0 , 1.0f);
    glVertex2f(-p, -p);
    
    glTexCoord2f(1.0f, 1.0f);
    glVertex2f(p, -p);
    
    glTexCoord2f(1.0f, 0);
    glVertex2f(p, p);
    
    glEnd();
    
    SwapBuffers(hdc);
    
#else
    
    StretchDIBits(hdc,
                  // dest
                  rect.left,
                  rect.top,
                  rect.right - rect.left,
                  rect.bottom - rect.top,
                  // src
                  rect.left,
                  rect.top,
                  rect.right - rect.left,
                  rect.bottom - rect.top,
                  buffer->memory,
                  &buffer->bmi,
                  DIB_RGB_COLORS,
                  SRCCOPY);
#endif
}

LRESULT CALLBACK 
window_proc(HWND window_handle, UINT message, WPARAM w_param, LPARAM l_param)
{
    LRESULT result = 0;
    switch (message)
    {
        case WM_CLOSE:
        {
            PostQuitMessage(0);
            break;
        }
        case WM_SIZE:
        {
            PostMessage(window_handle, message, w_param, l_param);
            break;
        }
        default:
        {
            result = DefWindowProc(window_handle, message, w_param, l_param);
        }
    }
    return result;
}

static void 
Win32ResizeWindow(HWND window_handle, win32_offscreen_buffer *screen_buffer)
{
    RECT rect; 
    GetClientRect(window_handle, &rect);;
    screen_buffer->width = rect.right - rect.left;
    screen_buffer->height = rect.bottom - rect.top;
    screen_buffer->bmi.bmiHeader.biWidth = screen_buffer->width;
    screen_buffer->bmi.bmiHeader.biHeight = -screen_buffer->height;
    
    int buffer_size = screen_buffer->width * screen_buffer->height * screen_buffer->bytes_per_pixel;
    
    if (screen_buffer->memory)
    {
        VirtualFree(screen_buffer->memory, 0 , MEM_RELEASE);
    }
    
    screen_buffer->memory = VirtualAlloc(0, buffer_size, MEM_COMMIT, PAGE_READWRITE);
}

void message_loop(HWND window_handle,
                  win32_offscreen_buffer *screen_buffer, keyboard_input *keyboard)
{
    MSG msg;
    while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
    {
        switch (msg.message)
        {
            case WM_KEYDOWN:
            case WM_KEYUP:
            case WM_SYSKEYDOWN:
            case WM_SYSKEYUP:
            {
                WPARAM key = msg.wParam;
                bool isDown = ((msg.lParam & (1 << 31)) == 0);
                bool wasDown = ((msg.lParam & (1 << 30)) != 0);
                DecodeKeyboardInput(keyboard, key, isDown, wasDown);
                break;
            }
            case WM_SIZE:
            {
                OutputDebugStringA("resize\n");
                Win32ResizeWindow(window_handle, screen_buffer);
                break;
            }
            case WM_QUIT:
            {
                EditorRunning = 0;
                break;
            }
            case WM_PAINT:
            {
                OutputDebugStringA("paint\n");
                RECT rect = {};
                GetClientRect(window_handle, &rect);
                
                PAINTSTRUCT ps;
                HDC hdc = BeginPaint(window_handle, &ps);
                
                //Win32WindowUpdate(hdc, rect, screen_buffer);
                
                EndPaint(window_handle, &ps);
                break;
            }
            case WM_CLIPBOARDUPDATE:
            {
                // TODO(willian): REIMPLEMENT
                //ed->paste_clipboard.has_changed = true;
            }
            default:
            {
                TranslateMessage(&msg);
                DispatchMessage(&msg);
            }
        }
    }
}

static void
win32_execute_bat_file(editor_state *ed)
{
    // TODO(willian): we are running the process from the editor executable folder,
    ///   what we need to do is run the cmd.exe from the file we are building from
    if (!ed->current_text_buffer->fullpath) return;
    
    zen_tb_string dir_path = zen_tb_line_create(ed->current_text_buffer->fullpath, 0);
    
    // delete filename from the fullpath
    while (zen_tb_line_length(dir_path) > 0)
    {
        zen_tb_size length = zen_tb_line_length(dir_path);
        
        if (dir_path[length - 1] == '/') break;
        
        zen_tb_line_delete_char(dir_path, zen_tb_line_length(dir_path));
    }
    zen_tb_string command_line = zen_tb_line_create("cmd.exe /C ", 100);
    
    // append directory
    command_line = zen_tb_line_append(command_line, dir_path);
    
    char current_dir[260];
	GetCurrentDirectory(260, current_dir);
    
    // append build.bat and pipe file
    command_line = zen_tb_line_append(command_line, "build.bat > \"");
    //
    command_line = zen_tb_line_append(command_line, current_dir);
    
    command_line = zen_tb_line_append(command_line, "/compilation\"");
    
    
    STARTUPINFOA startup_info = {};
    PROCESS_INFORMATION process_info = {};
    
    startup_info.cb = sizeof(startup_info);
    
    bool cp_result = CreateProcessA(0, command_line, 0, 0, FALSE, CREATE_NEW_CONSOLE, 
                                    0, dir_path, &startup_info, &process_info);
    
    WaitForSingleObject(process_info.hProcess, INFINITE);
    
    // Close process and thread handles. 
    CloseHandle(process_info.hProcess);
    CloseHandle(process_info.hThread);
    
    zen_tb_line_destroy(command_line);
    zen_tb_line_destroy(dir_path);
    
    if(!cp_result)
    {
        // log error
    }
    
    if(cp_result)
    {
        char *file_buffer = win32_open_file_into_buffer("compilation");
        
        if (file_buffer)
        {
            ed->current_text_buffer = editor_text_buffer_create(file_buffer, ed);
            
            ed->current_text_buffer->filename = zen_string_make("COMPILATION");
            
            editor_text_buffer_list_add_node(ed->current_text_buffer, ed);
            
            free(file_buffer);
        }
    }
}

////// time measurement helpers
inline float
win32_get_time_elapsed(s64 end, s64 start, s64 ticks_per_sec)
{
    float result = ((float)(end - start) / (float)ticks_per_sec);
    return result;
}

inline s64 
win32_get_ticks_elapsed()
{
    LARGE_INTEGER result;
    QueryPerformanceCounter(&result);
    return result.QuadPart;
}

inline s64
win32_get_ticks_per_second()
{
    LARGE_INTEGER result;
    QueryPerformanceFrequency(&result);
    return result.QuadPart;
}
///////

INT WINAPI
WinMain(HINSTANCE instance, HINSTANCE prev_instance,
        PSTR cmd_line, INT cmd_show)
{
    // Register the window class
    WNDCLASSEX wc = {};
    wc.cbSize = sizeof(WNDCLASSEX);
    wc.style = CS_VREDRAW | CS_HREDRAW | CS_OWNDC;
    wc.lpfnWndProc   = window_proc;
    wc.hInstance     = instance;
	wc.lpszClassName = "ZenEditorClass";
    
    // log error code later
    if (!RegisterClassEx(&wc)) return 1;
    
    // Create the window
    HWND window_handle = CreateWindowEx(
        0,
        wc.lpszClassName,
        "Zen Editor",
        WS_OVERLAPPEDWINDOW|WS_VISIBLE,
        CW_USEDEFAULT,
        CW_USEDEFAULT,
        CW_USEDEFAULT,
        CW_USEDEFAULT,
        0,
        0,
        instance,
        0);
    
    if (!window_handle) return 1;
    
    // device context
    HDC hdc = GetDC(window_handle);
    RECT client_rect = {};
    GetClientRect(window_handle, &client_rect);
    
    win32_init_opengl(hdc);
    
    // Register an listerner for when the app receive an clipboard
    // message update
    Assert(AddClipboardFormatListener(window_handle));
    
    // time init
    s64 ticks_per_second = win32_get_ticks_per_second();
    
    // window offscreen buffer
    win32_offscreen_buffer global_backbuffer = {};
    global_backbuffer.width = client_rect.right - client_rect.left;
    global_backbuffer.height = client_rect.bottom - client_rect.top;
    global_backbuffer.bytes_per_pixel = 4;
    global_backbuffer.bmi.bmiHeader.biSize = sizeof(BITMAPINFO);
    global_backbuffer.bmi.bmiHeader.biWidth = global_backbuffer.width;
    global_backbuffer.bmi.bmiHeader.biHeight = -global_backbuffer.height;
    global_backbuffer.bmi.bmiHeader.biPlanes = 1;
    global_backbuffer.bmi.bmiHeader.biBitCount = 32;
    global_backbuffer.bmi.bmiHeader.biCompression = BI_RGB;
    
    HBITMAP bitmap_handle = CreateDIBSection(
        hdc, &global_backbuffer.bmi, DIB_RGB_COLORS, &global_backbuffer.memory, 0, 0);
    
    // Editor Memory
    editor_memory EditorMemory = {};
    EditorMemory.PermanentSize = MegaBytes(32);
    EditorMemory.TemporarySize = MegaBytes(64);
    
    u64 MemSize = EditorMemory.PermanentSize + EditorMemory.TemporarySize;
    
    void *EditorMem = VirtualAlloc(0, (size_t)MemSize, MEM_COMMIT|MEM_RESERVE, PAGE_READWRITE);
    
    Assert(EditorMem);
    
    EditorMemory.PermanentStorage = EditorMem;
    EditorMemory.TemporaryStorage = (u8 *)EditorMem + EditorMemory.PermanentSize;
    
    // editor initialization code goes here
    //editor_state ed = {};// TODO(willian): remove it
#if 0
    editor_init(&ed);
    ed.platform_memory_alloc = &win32_memory_alloc;
    ed.platform_memory_free = &win32_memory_free;
    ed.paste_clipboard.has_changed = true;
#endif
    
    editor_screenbuffer ScreenBuffer = {};
    
    ScreenBuffer.width = global_backbuffer.width;
    ScreenBuffer.height = global_backbuffer.height;
    ScreenBuffer.memory = global_backbuffer.memory;
    ScreenBuffer.bytes_per_pixel = global_backbuffer.bytes_per_pixel;
    ScreenBuffer.pitch = ScreenBuffer.bytes_per_pixel * ScreenBuffer.width;
    
    // font initialization
    editor_font font = {};
    font.size = 14;
    font.width = 7;
    font.name = "../fonts/LiberationMono-Regular.ttf";
    u8 *ttf_buffer = win32_open_ttf_font_buffer(font.name);
    editor_font_init(&font, ttf_buffer);
    free(ttf_buffer);
    
    editor_rectangle window_rect;
    
    RECT rect = {};
    GetClientRect(window_handle, &rect);
    
    // input
    keyboard_input old_keyboard = {};
    
    // start frame timing
    s32 monitor_refresh_rate = 60; /// we gonna query this
    
    float target_seconds_per_frame = 1.0f / (float)monitor_refresh_rate;
    
    u64 old_frame_time = win32_get_ticks_elapsed();
    u64 UpdateTitle = ticks_per_second + old_frame_time;
    
    EditorRunning= 1;
    while (EditorRunning)
    {
        keyboard_input keyboard = {};
        keyboard.shift.endedDown = old_keyboard.shift.wasDown;
        keyboard.control.endedDown = old_keyboard.control.wasDown;
        keyboard.alt.endedDown = old_keyboard.alt.wasDown;
        
        // we going to get user input and populate the input buffer
        message_loop(window_handle, &global_backbuffer, &keyboard);
        
        GetClientRect(window_handle, &rect);
        
        // we alias the win32 buffer with the editor buffer so its platform independent
        window_rect.x = rect.left;
        window_rect.y = rect.top;
        window_rect.dx = rect.right;
        window_rect.dy = rect.bottom;
        ScreenBuffer.width = global_backbuffer.width;
        ScreenBuffer.height = global_backbuffer.height;
        ScreenBuffer.memory = global_backbuffer.memory;
        ScreenBuffer.bytes_per_pixel = global_backbuffer.bytes_per_pixel;
        ScreenBuffer.pitch = ScreenBuffer.bytes_per_pixel * ScreenBuffer.width;
#if 0
        if (keyboard.alt.endedDown && keyboard.keys['m'].endedDown)
        {
            win32_execute_bat_file(&ed);
        }
        
        // copy editor clipboard to platform clipboard
        if (ed.copy_clipboard.has_changed)
        {
            win32_copy_to_clipboard(&ed.copy_clipboard, window_handle);
            ed.copy_clipboard.has_changed = false;
        }
        
        // platform clipboard got updated
        if (ed.paste_clipboard.has_changed)
        {
            win32_copy_from_clipboard(&ed.paste_clipboard, window_handle);
            ed.paste_clipboard.has_changed = false;
        }
#endif
#if 1
        // update and render
        editor_update_and_render(&EditorMemory, &ScreenBuffer, &font, &keyboard, window_rect);
#endif
        // calc this frame time
        u64 new_frame_time = win32_get_ticks_elapsed();
        float seconds_elapsed =
            win32_get_time_elapsed(new_frame_time, old_frame_time, ticks_per_second);
        
        float MsPerFrame =  seconds_elapsed * 1000.0f;
        
        float FramesPerSecond = (float)ticks_per_second / (new_frame_time - old_frame_time);
        
#if EDITOR_DEBUG
        if (new_frame_time >= UpdateTitle)
        {
            UpdateTitle = ticks_per_second + new_frame_time;
            char window_title[1024];
            
            StringCbPrintfA(window_title, 1024, "Zen Editor FPS: %.2f MsPF: %.2f", FramesPerSecond, MsPerFrame);
            
            SetWindowText(window_handle, window_title);
        }
#endif
        
        // sleep
        if (seconds_elapsed < target_seconds_per_frame)
        {
            // truncation
            DWORD sleep_ms = (DWORD)(1000.0f * (target_seconds_per_frame - seconds_elapsed));
            Sleep(sleep_ms);
        }
        
        // remember frame time
        old_frame_time = new_frame_time;
        
        // render our back buffer to window
        Win32WindowUpdate(hdc, rect, &global_backbuffer);
        
        // remember keyboard state for the next frame
        old_keyboard.shift.wasDown = keyboard.shift.endedDown;
        old_keyboard.control.wasDown = keyboard.control.endedDown;
        old_keyboard.alt.wasDown = keyboard.alt.endedDown;
        
    }
    return 0;
}