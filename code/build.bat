@echo off

rem Disabled Warnings
set WD= -wd4505 -wd4100 -wd4189 -wd4201 -wd4800 -wd4267

rem Preprocessor Definitions
set PD= -D EDITOR_DEBUG -D EDITOR_OPENGL

rem Compiler Flags
set CFLAGS= -nologo -EHa -Od -Zi -FC -W4 -WX %WD% %PD% -F10000000

rem Win32 Import Librarys
set WIN32_LIBS=  user32.lib gdi32.lib opengl32.lib kernel32.lib

rem Linker Flags
set LFLAGS= -incremental:no -nologo -debug

IF NOT EXIST "..\build" (mkdir "..\build\")

pushd ..\build\

cl %CFLAGS% ..\code\win32_main.cpp /link %LFLAGS% %WIN32_LIBS%

popd

REM ===== SOME COMPILER AND LINKER FLAGS COMMONLY USED =====

REM Compiler Options Description:
REM -nologo  -> Suppresses display of sign-on banner. 
REM -Zi      -> Generates complete debugging information.
REM -wd      -> Disable the specified warning.
REM -W4      -> Sets output warning level.
REM -WX      -> Treats all warning as errors.
REM -Oi      -> Genertes instrinsics functions
REM -Od      -> Disables optimization
REM -Gm      -> Enables minimal rebuild
REM -Fm      -> Creates a mapfile
REM -MTd     -> Compiles to create a debug multithreaded executable file, by using LIBCMTD.lib.
REM -EHa     -> Exception Handling option
REM -FC      -> Displays the full path of source code files passed to cl.exe in diagnostic text.
REM -Z7      -> Generates C 7.0-compatible debugging information.

REM Linker Options Description:
REM -incremental:no -> disable incremental building
REM -opt:ref