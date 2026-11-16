/* find.c - البحث والاستبدال بخيار مطابقة حالة الأحرف.
   البحث يتم داخل Scintilla عبر "الهدف": SCI_SETTARGETRANGE ثم SCI_SEARCHINTARGET
   الذي يُرجع موضع أول تطابق أو -1، ثم SCI_REPLACETARGET للاستبدال. */
#ifndef UNICODE
#define UNICODE
#endif
#include "find.h"
#include <stdint.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include "Scintilla.h"
#include "resource.h"

#define MAX_FIND_UTF8 1024

static HINSTANCE g_inst;
static HWND g_main, g_edit, g_dialog;
static BOOL g_replaceMode;

/* آخر ما طلبه المستخدم (يبقى بعد إغلاق النافذة لأجل F3) */
static char    g_findUtf8[MAX_FIND_UTF8];
static char    g_replaceUtf8[MAX_FIND_UTF8];
static wchar_t g_findWide[256];
static BOOL    g_matchCase;

static intptr_t Sci(unsigned int msg, uintptr_t w, intptr_t l)
{
    return (intptr_t)SendMessageW(g_edit, msg, (WPARAM)w, (LPARAM)l);
}

/* سطر تشخيص يظهر في سجل المصحح (في Wine: WINEDEBUG=+debugstr) */
static void Trace(const char *fmt, ...)
{
    char line[512];
    va_list ap;
    va_start(ap, fmt);
    int n = vsnprintf(line, sizeof(line) - 2, fmt, ap);
    va_end(ap);
    if (n < 0) return;
    if (n > (int)sizeof(line) - 3) n = (int)sizeof(line) - 3;
    line[n] = '\n';
    line[n + 1] = '\0';
    OutputDebugStringA(line);
}

static int Ask(const wchar_t *text, UINT flags)
{
    HWND owner = g_dialog && IsWindowVisible(g_dialog) ? g_dialog : g_main;
    return MessageBoxW(owner, text, L"Arabic Editor", flags | MB_RTLREADING | MB_RIGHT);
}

static int SearchFlags(void)
{
    return g_matchCase ? SCFIND_MATCHCASE : 0;
}

/* يبحث في [from, to) ويُرجع ما أرجعته Scintilla؛ عند النجاح يكون الهدف هو التطابق */
static intptr_t SearchRange(intptr_t from, intptr_t to)
{
    size_t len = strlen(g_findUtf8);
    Sci(SCI_SETTARGETRANGE, (uintptr_t)from, to);
    Sci(SCI_SETSEARCHFLAGS, (uintptr_t)SearchFlags(), 0);
    intptr_t pos = Sci(SCI_SEARCHINTARGET, len, (intptr_t)g_findUtf8);
    Trace("ArabicEditor: SCI_SEARCHINTARGET(\"%s\", flags=%d, range=%ld..%ld) -> %ld",
          g_findUtf8, SearchFlags(), (long)from, (long)to, (long)pos);
    return pos;
}

static void NotFound(void)
{
    wchar_t msg[320];
    _snwprintf(msg, 320, L"لم يُعثر على النص \"%s\".", g_findWide);
    msg[319] = L'\0';
    Trace("ArabicEditor: not found; caret=%ld anchor=%ld length=%ld (unchanged)",
          (long)Sci(SCI_GETCURRENTPOS, 0, 0), (long)Sci(SCI_GETANCHOR, 0, 0),
          (long)Sci(SCI_GETLENGTH, 0, 0));
    Ask(msg, MB_OK | MB_ICONINFORMATION);
}

/* يبحث من نهاية التحديد الحالي، ثم من أول النص إن لم يجد (التفاف) */
static BOOL FindFromSelection(void)
{
    intptr_t start = Sci(SCI_GETSELECTIONEND, 0, 0);
    intptr_t length = Sci(SCI_GETLENGTH, 0, 0);
    intptr_t pos = SearchRange(start, length);
    if (pos < 0 && start > 0) {
        Trace("ArabicEditor: reached end, wrapping to start");
        pos = SearchRange(0, start);
    }
    if (pos < 0) {
        NotFound();
        return FALSE;
    }
    Sci(SCI_SETSEL, (uintptr_t)Sci(SCI_GETTARGETSTART, 0, 0), Sci(SCI_GETTARGETEND, 0, 0));
    Sci(SCI_SCROLLCARET, 0, 0);
    return TRUE;
}

/* هل التحديد الحالي هو بالضبط تطابق للنص المطلوب بالخيارات الحالية؟ */
static BOOL SelectionIsMatch(void)
{
    intptr_t s = Sci(SCI_GETSELECTIONSTART, 0, 0), e = Sci(SCI_GETSELECTIONEND, 0, 0);
    if (s == e)
        return FALSE;
    intptr_t pos = SearchRange(s, e);
    return pos == s && Sci(SCI_GETTARGETEND, 0, 0) == e;
}

static void DoReplace(void)
{
    if (!g_findUtf8[0])
        return;
    if (SelectionIsMatch()) {
        /* الهدف الآن هو التحديد المطابق: نستبدله ثم ننتقل إلى التطابق التالي */
        intptr_t s = Sci(SCI_GETTARGETSTART, 0, 0);
        intptr_t newLen = Sci(SCI_REPLACETARGET, strlen(g_replaceUtf8), (intptr_t)g_replaceUtf8);
        Trace("ArabicEditor: SCI_REPLACETARGET(\"%s\") at %ld -> %ld bytes",
              g_replaceUtf8, (long)s, (long)newLen);
        Sci(SCI_SETSEL, (uintptr_t)(s + newLen), s + newLen);
        FindFromSelection();
    } else {
        /* لا تطابق محدد بعد: نبحث أولًا فقط، والضغطة التالية تستبدل (كما في المحررات المألوفة) */
        Trace("ArabicEditor: selection is not a match; finding first");
        FindFromSelection();
    }
}

/* ---------- النافذة ---------- */

static void ReadDialog(void)
{
    wchar_t replaceWide[256];
    GetDlgItemTextW(g_dialog, IDC_FIND_TEXT, g_findWide, 256);
    GetDlgItemTextW(g_dialog, IDC_REPLACE_TEXT, replaceWide, 256);
    g_matchCase = IsDlgButtonChecked(g_dialog, IDC_MATCH_CASE) == BST_CHECKED;
    if (!WideCharToMultiByte(CP_UTF8, 0, g_findWide, -1, g_findUtf8, MAX_FIND_UTF8, NULL, NULL))
        g_findUtf8[0] = '\0';
    if (!WideCharToMultiByte(CP_UTF8, 0, replaceWide, -1, g_replaceUtf8, MAX_FIND_UTF8, NULL, NULL))
        g_replaceUtf8[0] = '\0';
}

static void ApplyMode(void)
{
    int show = g_replaceMode ? SW_SHOW : SW_HIDE;
    ShowWindow(GetDlgItem(g_dialog, IDC_REPLACE_LABEL), show);
    ShowWindow(GetDlgItem(g_dialog, IDC_REPLACE_TEXT), show);
    ShowWindow(GetDlgItem(g_dialog, IDC_REPLACE), show);
    SetWindowTextW(g_dialog, g_replaceMode ? L"استبدال" : L"بحث");
}

static INT_PTR CALLBACK DialogProc(HWND dlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
    (void)lParam;
    switch (msg) {
    case WM_INITDIALOG:
        g_dialog = dlg;
        SetDlgItemTextW(dlg, IDC_FIND_TEXT, g_findWide);
        CheckDlgButton(dlg, IDC_MATCH_CASE, g_matchCase ? BST_CHECKED : BST_UNCHECKED);
        ApplyMode();
        return TRUE;

    case WM_COMMAND:
        switch (LOWORD(wParam)) {
        case IDC_FIND_NEXT:
            ReadDialog();
            if (g_findUtf8[0])
                FindFromSelection();
            return TRUE;
        case IDC_REPLACE:
            ReadDialog();
            DoReplace();
            return TRUE;
        case IDCANCEL:
            DestroyWindow(dlg);
            return TRUE;
        }
        break;

    case WM_DESTROY:
        g_dialog = NULL;
        SetFocus(g_edit);
        return TRUE;
    }
    return FALSE;
}

void Find_Init(HINSTANCE instance, HWND mainWindow, HWND editor)
{
    g_inst = instance;
    g_main = mainWindow;
    g_edit = editor;
}

void Find_ShowDialog(BOOL replace)
{
    g_replaceMode = replace;

    /* إن كان هناك تحديد قصير على سطر واحد نقترحه نصًا للبحث */
    intptr_t s = Sci(SCI_GETSELECTIONSTART, 0, 0), e = Sci(SCI_GETSELECTIONEND, 0, 0);
    if (e > s && e - s < MAX_FIND_UTF8 - 1 &&
        Sci(SCI_LINEFROMPOSITION, (uintptr_t)s, 0) == Sci(SCI_LINEFROMPOSITION, (uintptr_t)e, 0)) {
        char sel[MAX_FIND_UTF8];
        struct Sci_TextRangeFull tr = { { s, e }, sel };
        Sci(SCI_GETTEXTRANGEFULL, 0, (intptr_t)&tr);
        if (!MultiByteToWideChar(CP_UTF8, 0, sel, -1, g_findWide, 256))
            g_findWide[0] = L'\0';
    }

    if (!g_dialog) {
        CreateDialogParamW(g_inst, MAKEINTRESOURCEW(IDD_FINDREPLACE), g_main, DialogProc, 0);
        if (g_dialog) {
            /* نضعها أسفل يمين النافذة الرئيسية حتى لا تغطي أول النص حيث تظهر النتائج */
            RECT m, d;
            GetWindowRect(g_main, &m);
            GetWindowRect(g_dialog, &d);
            SetWindowPos(g_dialog, NULL, m.right - (d.right - d.left) - 24,
                         m.bottom - (d.bottom - d.top) - 24, 0, 0, SWP_NOSIZE | SWP_NOZORDER);
        }
    } else {
        SetDlgItemTextW(g_dialog, IDC_FIND_TEXT, g_findWide);
        ApplyMode();
    }
    if (g_dialog) {
        ShowWindow(g_dialog, SW_SHOW);
        SetFocus(GetDlgItem(g_dialog, IDC_FIND_TEXT));
        SendDlgItemMessageW(g_dialog, IDC_FIND_TEXT, EM_SETSEL, 0, -1);
    }
}

void Find_Next(void)
{
    if (g_dialog)
        ReadDialog();
    if (!g_findUtf8[0]) {
        Find_ShowDialog(FALSE);
        return;
    }
    FindFromSelection();
}

HWND Find_Dialog(void)
{
    return g_dialog;
}
