/* msgbox.h - رسائل عربية من اليمين لليسار بأزرار عربية */
#ifndef ARABIC_EDITOR_MSGBOX_H
#define ARABIC_EDITOR_MSGBOX_H

#include <windows.h>

/* مثل MessageBoxW، لكن النافذة معكوسة (من اليمين لليسار) ونصوص الأزرار عربية
   (نعم/لا/إلغاء/موافق) بدل لغة النظام */
int ArabicMessageBox(HWND owner, const wchar_t *text, const wchar_t *caption, UINT flags);

#endif
