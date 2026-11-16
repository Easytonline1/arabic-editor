/* msgbox.c - نافذة رسالة خاصة بنا تُنشأ من اليمين لليسار من البداية (WS_EX_LAYOUTRTL)،
   بأزرار عربية. لا نستعمل MessageBoxW لأن أزرارها بلغة النظام، ولأن عكس اتجاهها بعد
   إنشائها لا يعكس كل محتواها في بعض البيئات (مثل Wine). */
#ifndef UNICODE
#define UNICODE
#endif
#include "msgbox.h"

#define MARGIN      14
#define ICON_SIZE   32
#define BUTTON_W    86
#define BUTTON_H    26
#define BUTTON_GAP  8
#define TEXT_MAX_W  420
#define ID_TEXT     100
#define ID_ICON     101

typedef struct {
    const wchar_t *text;
    UINT flags;
    int buttons[3];
    int count;
} MsgState;

static const wchar_t *ButtonText(int id)
{
    switch (id) {
    case IDYES:    return L"نعم";
    case IDNO:     return L"لا";
    case IDCANCEL: return L"إلغاء";
    default:       return L"موافق";
    }
}

static LPCWSTR IconFor(UINT flags)
{
    switch (flags & MB_ICONMASK) {
    case MB_ICONERROR:       return IDI_ERROR;
    case MB_ICONWARNING:     return IDI_WARNING;
    case MB_ICONQUESTION:    return IDI_QUESTION;
    case MB_ICONINFORMATION: return IDI_INFORMATION;
    default:                 return NULL;
    }
}

static BOOL HasButton(const MsgState *st, int id)
{
    for (int i = 0; i < st->count; i++)
        if (st->buttons[i] == id)
            return TRUE;
    return FALSE;
}

static void Layout(HWND dlg, MsgState *st)
{
    HFONT font = (HFONT)SendMessageW(dlg, WM_GETFONT, 0, 0);
    HINSTANCE inst = (HINSTANCE)GetWindowLongPtrW(dlg, GWLP_HINSTANCE);

    /* قياس النص بخط النافذة */
    HDC dc = GetDC(dlg);
    HGDIOBJ old = SelectObject(dc, font);
    RECT tr = { 0, 0, TEXT_MAX_W, 0 };
    DrawTextW(dc, st->text, -1, &tr, DT_CALCRECT | DT_WORDBREAK | DT_RTLREADING | DT_NOPREFIX);
    SelectObject(dc, old);
    ReleaseDC(dlg, dc);

    LPCWSTR iconId = IconFor(st->flags);
    int textX = MARGIN + (iconId ? ICON_SIZE + MARGIN : 0);
    int textH = tr.bottom > ICON_SIZE ? tr.bottom : ICON_SIZE;
    int buttonsW = st->count * BUTTON_W + (st->count - 1) * BUTTON_GAP;
    int contentW = textX + tr.right + MARGIN;
    int clientW = contentW > buttonsW + 2 * MARGIN ? contentW : buttonsW + 2 * MARGIN;
    int clientH = MARGIN + textH + MARGIN + BUTTON_H + MARGIN;

    /* الإحداثيات منطقية: في نافذة معكوسة تكون x = 0 عند الحافة اليمنى */
    if (iconId) {
        HWND icon = CreateWindowExW(0, L"STATIC", NULL, WS_CHILD | WS_VISIBLE | SS_ICON,
                                    MARGIN, MARGIN, ICON_SIZE, ICON_SIZE, dlg,
                                    (HMENU)(INT_PTR)ID_ICON, inst, NULL);
        SendMessageW(icon, STM_SETICON, (WPARAM)LoadIconW(NULL, iconId), 0);
    }
    HWND text = CreateWindowExW(0, L"STATIC", st->text, WS_CHILD | WS_VISIBLE | SS_LEFT | SS_NOPREFIX,
                                textX, MARGIN + (textH - tr.bottom) / 2, tr.right, tr.bottom, dlg,
                                (HMENU)(INT_PTR)ID_TEXT, inst, NULL);
    SendMessageW(text, WM_SETFONT, (WPARAM)font, FALSE);

    /* الزر الأول (نعم/موافق) في أقصى اليمين، وهو الافتراضي */
    int x = (clientW - buttonsW) / 2;
    for (int i = 0; i < st->count; i++) {
        DWORD style = WS_CHILD | WS_VISIBLE | WS_TABSTOP | (i == 0 ? BS_DEFPUSHBUTTON : BS_PUSHBUTTON);
        HWND b = CreateWindowExW(0, L"BUTTON", ButtonText(st->buttons[i]), style,
                                 x, clientH - MARGIN - BUTTON_H, BUTTON_W, BUTTON_H, dlg,
                                 (HMENU)(INT_PTR)st->buttons[i], inst, NULL);
        SendMessageW(b, WM_SETFONT, (WPARAM)font, FALSE);
        x += BUTTON_W + BUTTON_GAP;
    }
    SendMessageW(dlg, DM_SETDEFID, (WPARAM)st->buttons[0], 0);
    SetFocus(GetDlgItem(dlg, st->buttons[0]));

    /* حجم النافذة من حجم محتواها، ثم توسيطها فوق مالكها */
    RECT wr = { 0, 0, clientW, clientH };
    AdjustWindowRectEx(&wr, (DWORD)GetWindowLongPtrW(dlg, GWL_STYLE), FALSE,
                       (DWORD)GetWindowLongPtrW(dlg, GWL_EXSTYLE));
    int w = wr.right - wr.left, h = wr.bottom - wr.top;
    RECT owner;
    HWND parent = GetWindow(dlg, GW_OWNER);
    if (!parent || !GetWindowRect(parent, &owner))
        SystemParametersInfoW(SPI_GETWORKAREA, 0, &owner, 0);
    SetWindowPos(dlg, NULL, owner.left + ((owner.right - owner.left) - w) / 2,
                 owner.top + ((owner.bottom - owner.top) - h) / 2, w, h, SWP_NOZORDER);
}

static INT_PTR CALLBACK MsgProc(HWND dlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
    MsgState *st = (MsgState *)GetWindowLongPtrW(dlg, DWLP_USER);
    switch (msg) {
    case WM_INITDIALOG:
        SetWindowLongPtrW(dlg, DWLP_USER, lParam);
        Layout(dlg, (MsgState *)lParam);
        return FALSE;                       /* التركيز وُضع على الزر الافتراضي */
    case WM_COMMAND: {
        int id = LOWORD(wParam);
        if (id == IDCANCEL && st && !HasButton(st, IDCANCEL))
            id = st->count == 1 ? st->buttons[0] : 0;   /* Esc في رسالة بلا "إلغاء" */
        if (id && st && HasButton(st, id))
            EndDialog(dlg, id);
        return TRUE;
    }
    }
    return FALSE;
}

int ArabicMessageBox(HWND owner, const wchar_t *text, const wchar_t *caption, UINT flags)
{
    MsgState st = { text, flags, { IDOK }, 1 };
    switch (flags & MB_TYPEMASK) {
    case MB_YESNOCANCEL: st.buttons[0] = IDYES; st.buttons[1] = IDNO; st.buttons[2] = IDCANCEL; st.count = 3; break;
    case MB_YESNO:       st.buttons[0] = IDYES; st.buttons[1] = IDNO; st.count = 2; break;
    case MB_OKCANCEL:    st.buttons[0] = IDOK;  st.buttons[1] = IDCANCEL; st.count = 2; break;
    default: break;
    }

    /* قالب نافذة في الذاكرة بلا عناصر؛ العناصر تُنشأ في WM_INITDIALOG بعد قياس النص */
    WORD tmpl[256];
    ZeroMemory(tmpl, sizeof(tmpl));
    DLGTEMPLATE *dt = (DLGTEMPLATE *)tmpl;
    dt->style = DS_MODALFRAME | DS_SETFONT | WS_POPUP | WS_CAPTION | WS_SYSMENU;
    dt->dwExtendedStyle = WS_EX_LAYOUTRTL | WS_EX_DLGMODALFRAME;
    dt->cdit = 0;
    dt->cx = 200;
    dt->cy = 80;
    WORD *p = (WORD *)(dt + 1);
    *p++ = 0;                               /* بلا قائمة */
    *p++ = 0;                               /* صنف الحوار الافتراضي */
    int n = lstrlenW(caption);
    if (n > 100) n = 100;
    CopyMemory(p, caption, (size_t)n * sizeof(WCHAR));
    p += n;
    *p++ = 0;
    *p++ = 9;                               /* حجم الخط */
    static const wchar_t face[] = L"MS Shell Dlg 2";
    CopyMemory(p, face, sizeof(face));

    switch (flags & MB_ICONMASK) {
    case MB_ICONERROR:   MessageBeep(MB_ICONERROR); break;
    case MB_ICONWARNING: MessageBeep(MB_ICONWARNING); break;
    case MB_ICONQUESTION: MessageBeep(MB_ICONQUESTION); break;
    default: break;
    }

    INT_PTR r = DialogBoxIndirectParamW(GetModuleHandleW(NULL), dt, owner, MsgProc, (LPARAM)&st);
    return r > 0 ? (int)r : (st.count == 1 ? IDOK : IDCANCEL);
}
