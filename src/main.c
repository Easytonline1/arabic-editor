/* Arabic Editor - نافذة Win32 فيها مساحة كتابة Scintilla بترميز UTF-8،
   مع أوامر: جديد، فتح، حفظ، حفظ باسم، والسؤال عن الحفظ عند الإغلاق،
   وقائمة عرض (التفاف السطور، تكبير الخط وتصغيره) تُحفظ في editor.ini،
   وتلوين ملفات الإعدادات بمحلل props من Lexilla. */
#ifndef UNICODE
#define UNICODE
#endif
#ifndef _UNICODE
#define _UNICODE
#endif
#include <windows.h>
#include <commdlg.h>
#include <stdint.h>
#include <stdio.h>
#include "Scintilla.h"
#include "Lexilla.h"
#include "SciLexer.h"
#include "encoding.h"
#include "settings.h"
#include "find.h"
#include "resource.h"

#define APP_NAME         L"Arabic Editor"
#define UNTITLED_NAME    L"بلا عنوان"
#define MAX_FILE_BYTES   (256u * 1024u * 1024u)   /* حد معقول لمحرر نصوص */

static const wchar_t CLASS_NAME[] = L"ArabicEditorWindow";

/* حالة المستند الحالي */
static HWND     g_hwndMain;
static HWND     g_hwndEdit;
static wchar_t  g_path[MAX_PATH];      /* فارغ = مستند جديد بلا اسم */
static Encoding g_encoding;            /* ترميز الملف على القرص */
static Settings g_settings;            /* حجم الخط والالتفاف وحجم النافذة */
static const char *g_fontName = "Tahoma";

/* ---------- أدوات صغيرة ---------- */

static LRESULT Sci(unsigned int msg, uintptr_t wParam, intptr_t lParam)
{
    return SendMessageW(g_hwndEdit, msg, (WPARAM)wParam, (LPARAM)lParam);
}

static int Ask(const wchar_t *text, UINT flags)
{
    /* رسائل عربية: قراءة من اليمين ومحاذاة لليمين */
    return MessageBoxW(g_hwndMain, text, APP_NAME, flags | MB_RTLREADING | MB_RIGHT);
}

static const wchar_t *DisplayName(void)
{
    if (!g_path[0])
        return UNTITLED_NAME;
    const wchar_t *slash = wcsrchr(g_path, L'\\');
    return slash ? slash + 1 : g_path;
}

static BOOL IsModified(void)
{
    return Sci(SCI_GETMODIFY, 0, 0) != 0;
}

static void UpdateTitle(void)
{
    wchar_t enc[32], title[MAX_PATH + 96];
    Enc_Describe(g_encoding, enc, 32);
    _snwprintf(title, MAX_PATH + 96, L"%s%s [%s] %d نقطة - %s",
               IsModified() ? L"*" : L"", DisplayName(), enc, g_settings.fontSize, APP_NAME);
    title[MAX_PATH + 95] = L'\0';
    SetWindowTextW(g_hwndMain, title);
}

/* يضبط نهاية السطر لتطابق أول نهاية سطر في النص (وإلا CRLF) */
static void AdoptLineEnding(const char *text, size_t len)
{
    int mode = SC_EOL_CRLF;
    for (size_t i = 0; i < len; i++) {
        if (text[i] == '\r') { mode = (i + 1 < len && text[i + 1] == '\n') ? SC_EOL_CRLF : SC_EOL_CR; break; }
        if (text[i] == '\n') { mode = SC_EOL_LF; break; }
    }
    Sci(SCI_SETEOLMODE, (uintptr_t)mode, 0);
}

/* يضع نصًا جديدًا في المساحة ويعتبره نقطة "محفوظ" بلا سجل تراجع */
static void ReplaceDocument(const char *utf8, size_t len)
{
    Sci(SCI_CLEARALL, 0, 0);
    if (len)
        Sci(SCI_APPENDTEXT, len, (intptr_t)utf8);
    AdoptLineEnding(utf8, len);
    Sci(SCI_EMPTYUNDOBUFFER, 0, 0);
    Sci(SCI_SETSAVEPOINT, 0, 0);
    Sci(SCI_GOTOPOS, 0, 0);
}

/* ---------- المظهر: الخط والالتفاف والتلوين ---------- */

static BOOL g_propsLexer;   /* TRUE إن كان الملف الحالي ملف إعدادات يُلوَّن بمحلل props */

static int Rgb(int r, int g, int b) { return r | (g << 8) | (b << 16); }   /* ألوان Scintilla بترتيب BGR */

/* يطبّق الخط وحجمه على كل الأنماط ثم ألوان المحلل؛ يُستدعى بعد أي تغيير في الحجم أو المحلل */
static void ApplyStyles(void)
{
    Sci(SCI_STYLESETFONT, STYLE_DEFAULT, (intptr_t)g_fontName);
    Sci(SCI_STYLESETSIZE, STYLE_DEFAULT, (uintptr_t)g_settings.fontSize);
    Sci(SCI_STYLECLEARALL, 0, 0);
    if (g_propsLexer) {
        Sci(SCI_STYLESETFORE, SCE_PROPS_COMMENT, Rgb(0x00, 0x80, 0x00));
        Sci(SCI_STYLESETITALIC, SCE_PROPS_COMMENT, 1);
        Sci(SCI_STYLESETFORE, SCE_PROPS_SECTION, Rgb(0x00, 0x00, 0xA0));
        Sci(SCI_STYLESETBACK, SCE_PROPS_SECTION, Rgb(0xE8, 0xEE, 0xFF));
        Sci(SCI_STYLESETBOLD, SCE_PROPS_SECTION, 1);
        Sci(SCI_STYLESETEOLFILLED, SCE_PROPS_SECTION, 1);
        Sci(SCI_STYLESETFORE, SCE_PROPS_KEY, Rgb(0x80, 0x00, 0x80));
        Sci(SCI_STYLESETFORE, SCE_PROPS_ASSIGNMENT, Rgb(0xC0, 0x00, 0x00));
        Sci(SCI_STYLESETBOLD, SCE_PROPS_ASSIGNMENT, 1);
        Sci(SCI_STYLESETFORE, SCE_PROPS_DEFVAL, Rgb(0x80, 0x80, 0x00));
    }
    Sci(SCI_COLOURISE, 0, -1);
}

/* يختار المحلل حسب امتداد الملف: ملفات الإعدادات تُلوَّن بمحلل props، وغيرها نص عادي */
static void ChooseLexer(void)
{
    static const wchar_t *propsExt[] = { L".ini", L".cfg", L".conf", L".properties", L".inf", L".props" };
    const wchar_t *dot = g_path[0] ? wcsrchr(DisplayName(), L'.') : NULL;
    BOOL props = FALSE;
    for (size_t i = 0; dot && i < sizeof(propsExt) / sizeof(propsExt[0]); i++)
        if (lstrcmpiW(dot, propsExt[i]) == 0)
            props = TRUE;

    if (props != g_propsLexer || !props) {
        /* CreateLexer من Lexilla المربوطة ربطًا ثابتًا؛ Scintilla تتولى تحرير المحلل السابق */
        Sci(SCI_SETILEXER, 0, props ? (intptr_t)CreateLexer("props") : 0);
        g_propsLexer = props;
        ApplyStyles();
    }
}

static void SetFontSize(int size)
{
    if (size < FONT_SIZE_MIN) size = FONT_SIZE_MIN;
    if (size > FONT_SIZE_MAX) size = FONT_SIZE_MAX;
    g_settings.fontSize = size;
    ApplyStyles();
    UpdateTitle();
}

static void SetWordWrap(BOOL on)
{
    g_settings.wordWrap = on;
    Sci(SCI_SETWRAPMODE, on ? SC_WRAP_WORD : SC_WRAP_NONE, 0);
    CheckMenuItem(GetMenu(g_hwndMain), IDM_VIEW_WORDWRAP, on ? MF_CHECKED : MF_UNCHECKED);
}

/* ---------- القراءة والكتابة ---------- */

static BOOL ReadWholeFile(const wchar_t *path, unsigned char **data, size_t *len)
{
    *data = NULL;
    *len = 0;
    HANDLE h = CreateFileW(path, GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL,
                           OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (h == INVALID_HANDLE_VALUE)
        return FALSE;

    LARGE_INTEGER size;
    BOOL ok = GetFileSizeEx(h, &size) && size.QuadPart <= MAX_FILE_BYTES;
    unsigned char *buf = ok ? HeapAlloc(GetProcessHeap(), 0, (size_t)size.QuadPart + 1) : NULL;
    size_t got = 0;
    while (buf && got < (size_t)size.QuadPart) {
        DWORD n = 0;
        if (!ReadFile(h, buf + got, (DWORD)((size_t)size.QuadPart - got), &n, NULL) || n == 0)
            break;
        got += n;
    }
    CloseHandle(h);

    if (!buf || got != (size_t)size.QuadPart) {
        if (buf) HeapFree(GetProcessHeap(), 0, buf);
        return FALSE;
    }
    *data = buf;
    *len = got;
    return TRUE;
}

/* يكتب إلى ملف مؤقت بجانب الهدف ثم يستبدل الهدف به، فلا يتلف الملف إن فشلت الكتابة */
static BOOL WriteWholeFile(const wchar_t *path, const unsigned char *data, size_t len)
{
    wchar_t tmp[MAX_PATH + 8];
    if (_snwprintf(tmp, MAX_PATH + 8, L"%s.~tmp", path) < 0)
        return FALSE;
    tmp[MAX_PATH + 7] = L'\0';

    HANDLE h = CreateFileW(tmp, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    if (h == INVALID_HANDLE_VALUE)
        return FALSE;
    size_t done = 0;
    BOOL ok = TRUE;
    while (ok && done < len) {
        DWORD chunk = (DWORD)(len - done > 0x40000000 ? 0x40000000 : len - done), n = 0;
        ok = WriteFile(h, data + done, chunk, &n, NULL) && n == chunk;
        done += n;
    }
    ok = ok && FlushFileBuffers(h);
    CloseHandle(h);
    if (ok)
        ok = MoveFileExW(tmp, path, MOVEFILE_REPLACE_EXISTING | MOVEFILE_WRITE_THROUGH);
    if (!ok)
        DeleteFileW(tmp);
    return ok;
}

static BOOL LoadFile(const wchar_t *path)
{
    unsigned char *data;
    size_t len, utf8Len = 0;
    if (!ReadWholeFile(path, &data, &len)) {
        wchar_t msg[MAX_PATH + 64];
        _snwprintf(msg, MAX_PATH + 64, L"تعذّر فتح الملف \"%s\".", path);
        msg[MAX_PATH + 63] = L'\0';
        Ask(msg, MB_OK | MB_ICONERROR);
        return FALSE;
    }

    Encoding enc = Enc_Detect(data, len);
    char *utf8 = Enc_FileBytesToUtf8(data, len, enc, &utf8Len);
    HeapFree(GetProcessHeap(), 0, data);
    if (!utf8) {
        Ask(L"تعذّر تحويل ترميز الملف.", MB_OK | MB_ICONERROR);
        return FALSE;
    }

    ReplaceDocument(utf8, utf8Len);
    HeapFree(GetProcessHeap(), 0, utf8);
    lstrcpynW(g_path, path, MAX_PATH);
    g_encoding = enc;
    ChooseLexer();
    UpdateTitle();
    return TRUE;
}

static BOOL SaveToPath(const wchar_t *path)
{
    size_t len = (size_t)Sci(SCI_GETLENGTH, 0, 0);
    char *text = HeapAlloc(GetProcessHeap(), 0, len + 1);
    if (!text)
        return FALSE;
    Sci(SCI_GETTEXT, len + 1, (intptr_t)text);

    Encoding enc = g_encoding;
    size_t outLen = 0;
    BOOL lossy = FALSE;
    unsigned char *bytes = Enc_Utf8ToFileBytes(text, len, enc, &outLen, &lossy);

    if (bytes && lossy) {
        wchar_t name[32], msg[256];
        Enc_Describe(enc, name, 32);
        _snwprintf(msg, 256, L"بعض الحروف لا يمثّلها ترميز %s وستُستبدل.\n"
                             L"هل تحفظ الملف بترميز UTF-8 بدلًا من ذلك؟", name);
        msg[255] = L'\0';
        int r = Ask(msg, MB_YESNOCANCEL | MB_ICONWARNING);
        if (r == IDCANCEL) {
            HeapFree(GetProcessHeap(), 0, bytes);
            HeapFree(GetProcessHeap(), 0, text);
            return FALSE;
        }
        if (r == IDYES) {
            HeapFree(GetProcessHeap(), 0, bytes);
            enc.kind = ENC_UTF8;
            enc.codePage = CP_UTF8;
            bytes = Enc_Utf8ToFileBytes(text, len, enc, &outLen, &lossy);
        }
    }
    HeapFree(GetProcessHeap(), 0, text);

    BOOL ok = bytes && WriteWholeFile(path, bytes, outLen);
    if (bytes)
        HeapFree(GetProcessHeap(), 0, bytes);
    if (!ok) {
        wchar_t msg[MAX_PATH + 64];
        _snwprintf(msg, MAX_PATH + 64, L"تعذّر حفظ الملف \"%s\".", path);
        msg[MAX_PATH + 63] = L'\0';
        Ask(msg, MB_OK | MB_ICONERROR);
        return FALSE;
    }

    lstrcpynW(g_path, path, MAX_PATH);
    g_encoding = enc;
    Sci(SCI_SETSAVEPOINT, 0, 0);
    ChooseLexer();
    UpdateTitle();
    return TRUE;
}

/* ---------- نوافذ الفتح والحفظ (GetOpenFileNameW / GetSaveFileNameW) ---------- */

static const wchar_t FILE_FILTER[] =
    L"ملفات نصية (*.txt)\0*.txt\0كل الملفات (*.*)\0*.*\0";

static BOOL AskOpenPath(wchar_t *path)
{
    OPENFILENAMEW ofn = {0};
    path[0] = L'\0';
    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner   = g_hwndMain;
    ofn.lpstrFilter = FILE_FILTER;
    ofn.lpstrFile   = path;
    ofn.nMaxFile    = MAX_PATH;
    ofn.Flags       = OFN_EXPLORER | OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST | OFN_HIDEREADONLY;
    return GetOpenFileNameW(&ofn);
}

static BOOL AskSavePath(wchar_t *path)
{
    OPENFILENAMEW ofn = {0};
    lstrcpynW(path, g_path, MAX_PATH);
    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner   = g_hwndMain;
    ofn.lpstrFilter = FILE_FILTER;
    ofn.lpstrFile   = path;
    ofn.nMaxFile    = MAX_PATH;
    ofn.lpstrDefExt = L"txt";
    ofn.Flags       = OFN_EXPLORER | OFN_OVERWRITEPROMPT | OFN_PATHMUSTEXIST | OFN_HIDEREADONLY;
    return GetSaveFileNameW(&ofn);
}

/* ---------- الأوامر ---------- */

static BOOL CmdSaveAs(void)
{
    wchar_t path[MAX_PATH];
    if (!AskSavePath(path))
        return FALSE;               /* أُلغيت نافذة الحفظ */
    return SaveToPath(path);
}

static BOOL CmdSave(void)
{
    if (!g_path[0])
        return CmdSaveAs();         /* مستند بلا اسم: نطلب اسمًا */
    return SaveToPath(g_path);
}

/* يُستدعى قبل استبدال المستند أو الإغلاق. يُرجع FALSE إن اختار المستخدم الإلغاء. */
static BOOL ConfirmDiscardOrSave(void)
{
    if (!IsModified())
        return TRUE;
    wchar_t msg[MAX_PATH + 64];
    _snwprintf(msg, MAX_PATH + 64, L"هل تريد حفظ التغييرات في \"%s\"؟", DisplayName());
    msg[MAX_PATH + 63] = L'\0';
    switch (Ask(msg, MB_YESNOCANCEL | MB_ICONQUESTION)) {
    case IDYES: return CmdSave();   /* إن أُلغي الحفظ نفسه نبقى في المستند */
    case IDNO:  return TRUE;
    default:    return FALSE;
    }
}

static void CmdNew(void)
{
    if (!ConfirmDiscardOrSave())
        return;
    ReplaceDocument("", 0);
    Sci(SCI_SETEOLMODE, SC_EOL_CRLF, 0);
    g_path[0] = L'\0';
    g_encoding.kind = ENC_UTF8;     /* الملف الجديد يُحفظ بـ UTF-8 */
    g_encoding.codePage = CP_UTF8;
    ChooseLexer();
    UpdateTitle();
}

static void CmdOpen(void)
{
    if (!ConfirmDiscardOrSave())
        return;
    wchar_t path[MAX_PATH];
    if (AskOpenPath(path))
        LoadFile(path);
}

/* ---------- الخط ---------- */

static int CALLBACK FontFound(const LOGFONTW *lf, const TEXTMETRICW *tm, DWORD type, LPARAM found)
{
    (void)lf; (void)tm; (void)type;
    *(BOOL *)found = TRUE;
    return 0;
}

/* يختار أول خط مثبت يدعم الحروف العربية */
static const char *PickArabicFont(void)
{
    static const struct { const wchar_t *w; const char *a; } candidates[] = {
        { L"Segoe UI", "Segoe UI" }, { L"Tahoma", "Tahoma" }, { L"Arial", "Arial" },
        { L"DejaVu Sans", "DejaVu Sans" }, { L"FreeSerif", "FreeSerif" },
    };
    HDC dc = GetDC(NULL);
    const char *chosen = "Tahoma";
    for (size_t i = 0; i < sizeof(candidates) / sizeof(candidates[0]); i++) {
        LOGFONTW lf = {0};
        BOOL found = FALSE;
        lf.lfCharSet = ARABIC_CHARSET;
        lstrcpynW(lf.lfFaceName, candidates[i].w, LF_FACESIZE);
        EnumFontFamiliesExW(dc, &lf, FontFound, (LPARAM)&found, 0);
        if (found) { chosen = candidates[i].a; break; }
    }
    ReleaseDC(NULL, dc);
    return chosen;
}

/* ---------- حفظ الإعدادات عند الإغلاق ---------- */

static void SaveSettingsOnExit(void)
{
    WINDOWPLACEMENT wp;
    ZeroMemory(&wp, sizeof(wp));
    wp.length = sizeof(wp);
    if (GetWindowPlacement(g_hwndMain, &wp)) {
        g_settings.width     = wp.rcNormalPosition.right - wp.rcNormalPosition.left;
        g_settings.height    = wp.rcNormalPosition.bottom - wp.rcNormalPosition.top;
        g_settings.maximized = wp.showCmd == SW_SHOWMAXIMIZED;
    }

    DWORD err;
    wchar_t failed[MAX_PATH];
    if (Settings_Save(&g_settings, &err, failed, MAX_PATH))
        return;

    wchar_t reason[256] = L"";
    FormatMessageW(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, NULL, err, 0,
                   reason, 256, NULL);
    for (wchar_t *p = reason; *p; p++)            /* إزالة سطر جديد في نهاية نص النظام */
        if (*p == L'\r' || *p == L'\n') *p = L' ';

    wchar_t msg[MAX_PATH + 512];
    _snwprintf(msg, MAX_PATH + 512,
               L"تعذّر حفظ الإعدادات (حجم الخط، والتفاف السطور، وحجم النافذة) في الملف:\n"
               L"%s\n\n"
               L"السبب: %s(رمز الخطأ %lu)\n\n"
               L"لن تُحفظ التغييرات الأخيرة، وستُقرأ الإعدادات السابقة عند التشغيل القادم.",
               failed, reason[0] ? reason : L"غير معروف ", (unsigned long)err);
    msg[MAX_PATH + 511] = L'\0';
    Ask(msg, MB_OK | MB_ICONWARNING);
}

/* ---------- النافذة ---------- */

static BOOL CreateEditor(HWND hwndParent, HINSTANCE hInstance)
{
    /* الواجهة كلها معكوسة (من اليمين لليسار)، لكن مساحة الكتابة لا تُعكس:
       Scintilla ترسم النص بنفسها، واتجاهه تتولاه خاصية النص ثنائي الاتجاه أدناه */
    g_hwndEdit = CreateWindowExW(WS_EX_NOINHERITLAYOUT, L"Scintilla", L"",
                                 WS_CHILD | WS_VISIBLE | WS_VSCROLL | WS_HSCROLL | WS_CLIPCHILDREN,
                                 0, 0, 0, 0, hwndParent, NULL, hInstance, NULL);
    if (!g_hwndEdit)
        return FALSE;

    Sci(SCI_SETCODEPAGE, SC_CP_UTF8, 0);       /* كل النص داخل المساحة UTF-8 (شرط للنص ثنائي الاتجاه) */

    /* DirectWrite شرط للنص ثنائي الاتجاه على ويندوز. إن لم يتوفر Direct2D تبقى Scintilla على GDI */
    Sci(SCI_SETTECHNOLOGY, SC_TECHNOLOGY_DIRECTWRITE, 0);
    int technology = (int)Sci(SCI_GETTECHNOLOGY, 0, 0);
    /* وثائق Scintilla: الدعم تجريبي، ولا يعمل إلا بـ UTF-8 وفي وضع L2R؛ أما R2L فـ"قد يُنفَّذ مستقبلًا" */
    Sci(SCI_SETBIDIRECTIONAL, SC_BIDIRECTIONAL_L2R, 0);
    int bidi = (int)Sci(SCI_GETBIDIRECTIONAL, 0, 0);
    /* التحديد المعتم (SC_LAYER_BASE) لا يعمل مع النص ثنائي الاتجاه */
    Sci(SCI_SETSELECTIONLAYER, SC_LAYER_UNDER_TEXT, 0);
    char line[160];
    snprintf(line, sizeof(line),
             "ArabicEditor: SCI_GETTECHNOLOGY -> %d (1 = DirectWrite), SCI_GETBIDIRECTIONAL -> %d (1 = L2R, 0 = disabled)\n",
             technology, bidi);
    OutputDebugStringA(line);

    g_fontName = PickArabicFont();
    Sci(SCI_SETMARGINWIDTHN, 1, 0);
    /* التكبير عندنا يغيّر حجم الخط نفسه، فنلغي اختصارات التكبير الداخلية في Scintilla */
    Sci(SCI_CLEARCMDKEY, SCK_ADD + (SCMOD_CTRL << 16), 0);
    Sci(SCI_CLEARCMDKEY, SCK_SUBTRACT + (SCMOD_CTRL << 16), 0);
    Sci(SCI_CLEARCMDKEY, SCK_DIVIDE + (SCMOD_CTRL << 16), 0);
    ApplyStyles();
    return TRUE;
}

static LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch (msg) {
    case WM_CREATE:
        g_hwndMain = hwnd;
        if (!CreateEditor(hwnd, ((LPCREATESTRUCTW)lParam)->hInstance))
            return -1;
        g_encoding.kind = ENC_UTF8;
        g_encoding.codePage = CP_UTF8;
        ApplyStyles();
        SetWordWrap(g_settings.wordWrap);
        Find_Init(((LPCREATESTRUCTW)lParam)->hInstance, hwnd, g_hwndEdit);
        UpdateTitle();
        return 0;

    case WM_SIZE:
        if (g_hwndEdit)
            MoveWindow(g_hwndEdit, 0, 0, LOWORD(lParam), HIWORD(lParam), TRUE);
        return 0;

    case WM_SETFOCUS:
        if (g_hwndEdit)
            SetFocus(g_hwndEdit);
        return 0;

    case WM_NOTIFY: {
        const SCNotification *n = (const SCNotification *)lParam;
        if (n->nmhdr.hwndFrom == g_hwndEdit &&
            (n->nmhdr.code == SCN_SAVEPOINTREACHED || n->nmhdr.code == SCN_SAVEPOINTLEFT))
            UpdateTitle();              /* إظهار/إخفاء علامة * في العنوان */
        if (n->nmhdr.hwndFrom == g_hwndEdit && n->nmhdr.code == SCN_ZOOM) {
            /* Ctrl+عجلة الفأرة: نحوّل تكبير Scintilla إلى تغيير في حجم الخط المحفوظ */
            int z = (int)Sci(SCI_GETZOOM, 0, 0);
            if (z != 0) {
                Sci(SCI_SETZOOM, 0, 0);
                SetFontSize(g_settings.fontSize + z);
            }
        }
        return 0;
    }

    case WM_COMMAND:
        switch (LOWORD(wParam)) {
        case IDM_FILE_NEW:    CmdNew();    return 0;
        case IDM_FILE_OPEN:   CmdOpen();   return 0;
        case IDM_FILE_SAVE:   CmdSave();   return 0;
        case IDM_FILE_SAVEAS: CmdSaveAs(); return 0;
        case IDM_FILE_EXIT:   SendMessageW(hwnd, WM_CLOSE, 0, 0); return 0;
        case IDM_VIEW_WORDWRAP:  SetWordWrap(!g_settings.wordWrap); return 0;
        case IDM_VIEW_ZOOMIN:    SetFontSize(g_settings.fontSize + 1); return 0;
        case IDM_VIEW_ZOOMOUT:   SetFontSize(g_settings.fontSize - 1); return 0;
        case IDM_VIEW_ZOOMRESET: SetFontSize(FONT_SIZE_DEFAULT); return 0;
        case IDM_SEARCH_FIND:    Find_ShowDialog(FALSE); return 0;
        case IDM_SEARCH_NEXT:    Find_Next(); return 0;
        case IDM_SEARCH_REPLACE: Find_ShowDialog(TRUE); return 0;
        }
        break;

    case WM_CLOSE:
        if (ConfirmDiscardOrSave()) {
            SaveSettingsOnExit();       /* عند الفشل تظهر رسالة ثم يُغلق البرنامج */
            DestroyWindow(hwnd);
        }
        return 0;                       /* الإلغاء يُبقي النافذة مفتوحة */

    case WM_QUERYENDSESSION:
        return ConfirmDiscardOrSave();

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

    /* كل النوافذ التي ينشئها البرنامج (الرئيسية وقوائمها ونافذة البحث والرسائل) من اليمين لليسار */
    SetProcessDefaultLayout(LAYOUT_RTL);

    /* مع الربط الثابت يجب تسجيل أصناف Scintilla بأنفسنا قبل إنشاء نافذتها */
    if (!Scintilla_RegisterClasses(hInstance))
        return 1;

    WNDCLASSEXW wc = {0};
    wc.cbSize        = sizeof(wc);
    wc.lpfnWndProc   = WndProc;
    wc.hInstance     = hInstance;
    wc.lpszClassName = CLASS_NAME;
    wc.lpszMenuName  = MAKEINTRESOURCEW(IDR_MAINMENU);
    wc.hCursor       = LoadCursorW(NULL, IDC_ARROW);
    wc.hIcon         = LoadIconW(NULL, IDI_APPLICATION);
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);

    if (!RegisterClassExW(&wc))
        return 1;

    Settings_Load(&g_settings);        /* قبل إنشاء النافذة لنستعمل حجمها المحفوظ */

    HWND hwnd = CreateWindowExW(0, CLASS_NAME, APP_NAME, WS_OVERLAPPEDWINDOW,
                                CW_USEDEFAULT, CW_USEDEFAULT,
                                g_settings.width, g_settings.height,
                                NULL, NULL, hInstance, NULL);
    if (!hwnd)
        return 1;

    if (g_settings.maximized && (nCmdShow == SW_SHOWNORMAL || nCmdShow == SW_SHOWDEFAULT))
        nCmdShow = SW_SHOWMAXIMIZED;
    ShowWindow(hwnd, nCmdShow);
    UpdateWindow(hwnd);

    HACCEL accel = LoadAcceleratorsW(hInstance, MAKEINTRESOURCEW(IDR_ACCEL));
    MSG msg;
    while (GetMessageW(&msg, NULL, 0, 0) > 0) {
        /* الاختصارات (Ctrl+F وF3 وCtrl+H...) تعمل أولًا حتى والتركيز في نافذة البحث،
           ثم تأخذ نافذة البحث غير المشروطة مفاتيح التنقل (Tab وEnter وEsc) */
        if (TranslateAcceleratorW(hwnd, accel, &msg))
            continue;
        HWND findDlg = Find_Dialog();
        if (findDlg && IsDialogMessageW(findDlg, &msg))
            continue;
        {
            TranslateMessage(&msg);
            DispatchMessageW(&msg);
        }
    }

    Scintilla_ReleaseResources();
    return (int)msg.wParam;
}
