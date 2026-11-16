/* Arabic Editor - نافذة Win32 فيها مساحة كتابة من Scintilla (مربوطة ربطًا ثابتًا) */
#ifndef UNICODE
#define UNICODE
#endif
#ifndef _UNICODE
#define _UNICODE
#endif
#include <windows.h>
#include <stdint.h>
#include "Scintilla.h"

static const wchar_t CLASS_NAME[] = L"ArabicEditorWindow";

static HWND g_hwndEdit;

static LRESULT SciSend(unsigned int msg, uintptr_t wParam, intptr_t lParam)
{
    return SendMessageW(g_hwndEdit, msg, (WPARAM)wParam, (LPARAM)lParam);
}

/* ينشئ مساحة الكتابة: نافذة ابنة من الصنف "Scintilla" تملأ النافذة الرئيسية */
static BOOL CreateEditor(HWND hwndParent, HINSTANCE hInstance)
{
    g_hwndEdit = CreateWindowExW(0, L"Scintilla", L"",
                                 WS_CHILD | WS_VISIBLE | WS_VSCROLL | WS_HSCROLL | WS_CLIPCHILDREN,
                                 0, 0, 0, 0,
                                 hwndParent, NULL, hInstance, NULL);
    if (!g_hwndEdit)
        return FALSE;

    SciSend(SCI_SETCODEPAGE, SC_CP_UTF8, 0);
    SciSend(SCI_STYLESETFONT, STYLE_DEFAULT, (intptr_t)"Tahoma");
    SciSend(SCI_STYLESETSIZE, STYLE_DEFAULT, 12);
    SciSend(SCI_STYLECLEARALL, 0, 0);
    return TRUE;
}

static LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch (msg) {
    case WM_CREATE:
        if (!CreateEditor(hwnd, ((LPCREATESTRUCTW)lParam)->hInstance))
            return -1;
        return 0;

    case WM_SIZE:
        if (g_hwndEdit)
            MoveWindow(g_hwndEdit, 0, 0, LOWORD(lParam), HIWORD(lParam), TRUE);
        return 0;

    case WM_SETFOCUS:
        if (g_hwndEdit)
            SetFocus(g_hwndEdit);
        return 0;

    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;
    }
    return DefWindowProcW(hwnd, msg, wParam, lParam);
}

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PWSTR pCmdLine, int nCmdShow)
{
    (void)hPrevInstance;
    (void)pCmdLine;

    /* مع الربط الثابت يجب تسجيل أصناف Scintilla بأنفسنا قبل إنشاء نافذتها */
    if (!Scintilla_RegisterClasses(hInstance))
        return 1;

    WNDCLASSEXW wc = {0};
    wc.cbSize        = sizeof(wc);
    wc.lpfnWndProc   = WndProc;
    wc.hInstance     = hInstance;
    wc.lpszClassName = CLASS_NAME;
    wc.hCursor       = LoadCursorW(NULL, IDC_ARROW);
    wc.hIcon         = LoadIconW(NULL, IDI_APPLICATION);
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);

    if (!RegisterClassExW(&wc))
        return 1;

    HWND hwnd = CreateWindowExW(0, CLASS_NAME, L"Arabic Editor",
                                WS_OVERLAPPEDWINDOW,
                                CW_USEDEFAULT, CW_USEDEFAULT, 800, 600,
                                NULL, NULL, hInstance, NULL);
    if (!hwnd)
        return 1;

    ShowWindow(hwnd, nCmdShow);
    UpdateWindow(hwnd);

    MSG msg;
    while (GetMessageW(&msg, NULL, 0, 0) > 0) {
        TranslateMessage(&msg);
        DispatchMessageW(&msg);
    }

    Scintilla_ReleaseResources();
    return (int)msg.wParam;
}
