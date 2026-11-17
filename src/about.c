/* about.c - نافذة "حول البرنامج": تعريف عربي، ونصّا الرخصتين كاملين من ملف LICENSE
   المضمَّن في البرنامج نفسه (مورد RCDATA) فلا يختلفان عن الملف في المستودع. */
#ifndef UNICODE
#define UNICODE
#endif
#include "about.h"
#include "resource.h"

static const wchar_t ABOUT_TEXT[] =
    L"Arabic Editor\r\n"
    L"محرر نصوص عربي لويندوز: يكتب بترميز UTF-8، ويفتح الملفات العربية القديمة بترميز Windows-1256 ويحفظها به.\r\n"
    L"\r\n"
    L"بُني بعد دراسة سلوك برنامج Notepad2، وصاحبه الأصلي Florian Balmer، دون نسخ شيفرته. "
    L"لا يرتبط هذا البرنامج بصاحب الأصل ولا بالمساهمين فيه، ولا يحظى بتأييدهم.\r\n"
    L"\r\n"
    L"يستعمل مكوّنَي التحرير Scintilla وLexilla. نصّا الرخصتين كاملين أدناه:";

/* يحمّل LICENSE من موارد البرنامج ويحوّله من UTF-8 إلى UTF-16. يحرره المستدعي بـ HeapFree */
static wchar_t *LoadLicenseText(HINSTANCE instance)
{
    HRSRC res = FindResourceW(instance, MAKEINTRESOURCEW(IDR_LICENSE), MAKEINTRESOURCEW(10) /* RT_RCDATA */);
    HGLOBAL mem = res ? LoadResource(instance, res) : NULL;
    const char *data = mem ? (const char *)LockResource(mem) : NULL;
    DWORD size = res ? SizeofResource(instance, res) : 0;
    if (!data || !size)
        return NULL;
    int n = MultiByteToWideChar(CP_UTF8, 0, data, (int)size, NULL, 0);
    wchar_t *w = n > 0 ? HeapAlloc(GetProcessHeap(), 0, ((size_t)n + 1) * sizeof(wchar_t)) : NULL;
    if (!w)
        return NULL;
    MultiByteToWideChar(CP_UTF8, 0, data, (int)size, w, n);
    w[n] = L'\0';
    return w;
}

static INT_PTR CALLBACK AboutProc(HWND dlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch (msg) {
    case WM_INITDIALOG: {
        SetDlgItemTextW(dlg, IDC_ABOUT_TEXT, ABOUT_TEXT);

        /* نصوص الرخص إنجليزية: نجعل حقلها من اليسار لليمين داخل النافذة العربية المعكوسة */
        HWND lic = GetDlgItem(dlg, IDC_ABOUT_LICENSE);
        LONG_PTR ex = GetWindowLongPtrW(lic, GWL_EXSTYLE);
        SetWindowLongPtrW(lic, GWL_EXSTYLE, ex & ~(LONG_PTR)(WS_EX_LAYOUTRTL | WS_EX_RTLREADING | WS_EX_RIGHT));
        wchar_t *text = LoadLicenseText((HINSTANCE)lParam);
        SetWindowTextW(lic, text ? text : L"(تعذّر تحميل نص الرخص من موارد البرنامج)");
        if (text)
            HeapFree(GetProcessHeap(), 0, text);

        SetFocus(GetDlgItem(dlg, IDOK));
        return FALSE;
    }
    case WM_COMMAND:
        if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL) {
            EndDialog(dlg, LOWORD(wParam));
            return TRUE;
        }
        break;
    }
    return FALSE;
}

void About_Show(HINSTANCE instance, HWND owner)
{
    DialogBoxParamW(instance, MAKEINTRESOURCEW(IDD_ABOUT), owner, AboutProc, (LPARAM)instance);
}
