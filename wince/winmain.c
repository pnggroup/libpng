
/* winmain.c - a simple entry point for WindowsCE
 *
 * Copyright (c) 2000 Tenik Co.,Ltd.
 */

#include <windows.h>
#include <tchar.h>

extern int main(int argc, char *argv[]);

int WINAPI WinMain(HINSTANCE hInst, HINSTANCE hPrevInst, LPWSTR lpszCmdLine, int nCmdShow)
{
#define SIZE_ARGV   (32)

    TCHAR path[MAX_PATH];
    LPTSTR file;
    char prog[MAX_PATH];
    int argc;
    char *argv[SIZE_ARGV];
    int size;
    char *buff;
    char *argp;
    char *args;
    char quote;

    // get program name
    argc = 0;
    GetModuleFileName(GetCurrentProcess(), path, MAX_PATH);
    file = _tcsrchr(path, TEXT('\\'));
    if (NULL == file) {
        file = path;
    }
    else {
        file++;
    }
    WideCharToMultiByte(CP_ACP, 0, file, -1, prog, MAX_PATH, NULL, NULL);
    argv[argc++] = prog;

    // analyze parameters
    size = WideCharToMultiByte(CP_ACP, 0, lpszCmdLine, -1, NULL, 0, NULL, NULL);
    buff = (char *)malloc(size);
    size = WideCharToMultiByte(CP_ACP, 0, lpszCmdLine, -1, buff, size, NULL, NULL);
    quote = 0x00;
    args = argp = buff;
    if (argp && *argp && size) {
        argv[argc++] = args;
        while (*argp) {
            if (quote) {
                if (*argp == quote) {
                    argp++;
                    if (*argp != quote) {
                        quote = 0x00;
                    }
                    else {
                        *args++ = *argp++;
                    }
                }
                else {
                    *args++ = *argp++;
                }
            }
            else {
                if (*argp == ' ') {
                    *args++ = *argp++ = '\0';
                    while (*argp && *argp == ' ') {
                        argp++;
                    }
                    if (*argp && argc < SIZE_ARGV) {
                        argv[argc++] = args;
                    }
                }
                else {
                    if (*argp == '\"') {
                        quote = *argp++;
                    }
                    else {
                        *args++ = *argp++;
                    }
                }
            }
        }
    }
    *args = '\0';
    return main(argc, argv);
}
