/* settings.c - قراءة editor.ini وكتابته */
#include "settings.h"
#include <shlobj.h>
#include <stdio.h>

#define APP_DIR_NAME  L"ArabicEditor"
#define INI_NAME      L"editor.ini"

void Settings_Defaults(Settings *s)
{
    s->fontSize  = FONT_SIZE_DEFAULT;
    s->wordWrap  = FALSE;
    s->width     = 800;
    s->height    = 600;
    s->maximized = FALSE;
}

static BOOL AppDir(wchar_t *dir, size_t count)
{
    wchar_t appData[MAX_PATH];
    if (FAILED(SHGetFolderPathW(NULL, CSIDL_APPDATA, NULL, SHGFP_TYPE_CURRENT, appData)))
        return FALSE;
    int n = _snwprintf(dir, count, L"%s\\%s", appData, APP_DIR_NAME);
    return n > 0 && (size_t)n < count;
}

BOOL Settings_Path(wchar_t *path, size_t count)
{
    wchar_t dir[MAX_PATH];
    if (!AppDir(dir, MAX_PATH))
        return FALSE;
    int n = _snwprintf(path, count, L"%s\\%s", dir, INI_NAME);
    return n > 0 && (size_t)n < count;
}

static int Clamp(int v, int lo, int hi) { return v < lo ? lo : (v > hi ? hi : v); }

void Settings_Load(Settings *s)
{
    wchar_t path[MAX_PATH];
    Settings_Defaults(s);
    if (!Settings_Path(path, MAX_PATH) || GetFileAttributesW(path) == INVALID_FILE_ATTRIBUTES)
        return;

    s->fontSize  = Clamp((int)GetPrivateProfileIntW(L"View", L"FontSize", s->fontSize, path),
                         FONT_SIZE_MIN, FONT_SIZE_MAX);
    s->wordWrap  = GetPrivateProfileIntW(L"View", L"WordWrap", s->wordWrap, path) != 0;

    /* لا نسمح بنافذة أصغر من حد معقول ولا أكبر من الشاشات المتصلة */
    int maxW = GetSystemMetrics(SM_CXVIRTUALSCREEN), maxH = GetSystemMetrics(SM_CYVIRTUALSCREEN);
    s->width     = Clamp((int)GetPrivateProfileIntW(L"Window", L"Width", s->width, path), 300, maxW > 300 ? maxW : 300);
    s->height    = Clamp((int)GetPrivateProfileIntW(L"Window", L"Height", s->height, path), 200, maxH > 200 ? maxH : 200);
    s->maximized = GetPrivateProfileIntW(L"Window", L"Maximized", s->maximized, path) != 0;
}

BOOL Settings_Save(const Settings *s, DWORD *error, wchar_t *failedPath, size_t count)
{
    wchar_t dir[MAX_PATH], path[MAX_PATH];
    *error = ERROR_SUCCESS;
    failedPath[0] = L'\0';

    if (!AppDir(dir, MAX_PATH) || !Settings_Path(path, MAX_PATH)) {
        *error = ERROR_PATH_NOT_FOUND;
        lstrcpynW(failedPath, L"%APPDATA%", (int)count);
        return FALSE;
    }
    if (!CreateDirectoryW(dir, NULL) && GetLastError() != ERROR_ALREADY_EXISTS) {
        *error = GetLastError();
        lstrcpynW(failedPath, dir, (int)count);
        return FALSE;
    }

    char text[512];
    int len = snprintf(text, sizeof(text),
        "; Arabic Editor settings - written on exit\r\n"
        "[View]\r\n"
        "FontSize=%d\r\n"
        "WordWrap=%d\r\n"
        "\r\n"
        "[Window]\r\n"
        "Width=%d\r\n"
        "Height=%d\r\n"
        "Maximized=%d\r\n",
        s->fontSize, s->wordWrap ? 1 : 0, s->width, s->height, s->maximized ? 1 : 0);

    /* ملف للقراءة فقط لا يُكتب فوقه. ويندوز يرفض CREATE_ALWAYS عليه، لكننا نفحص الخاصية بأنفسنا
       أيضًا لأن بعض البيئات (مثل Wine تحت المستخدم root) تتجاهلها فيُكتب الملف رغمها. */
    DWORD attrs = GetFileAttributesW(path);
    if (attrs != INVALID_FILE_ATTRIBUTES && (attrs & FILE_ATTRIBUTE_READONLY)) {
        *error = ERROR_ACCESS_DENIED;
        lstrcpynW(failedPath, path, (int)count);
        return FALSE;
    }

    HANDLE h = CreateFileW(path, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    if (h == INVALID_HANDLE_VALUE) {
        *error = GetLastError();
        lstrcpynW(failedPath, path, (int)count);
        return FALSE;
    }
    DWORD written = 0;
    BOOL ok = WriteFile(h, text, (DWORD)len, &written, NULL) && written == (DWORD)len;
    if (!ok)
        *error = GetLastError() ? GetLastError() : ERROR_WRITE_FAULT;
    CloseHandle(h);
    if (!ok)
        lstrcpynW(failedPath, path, (int)count);
    return ok;
}
