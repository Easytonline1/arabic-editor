/* find.h - البحث والبحث عن التالي والاستبدال في مساحة الكتابة (Scintilla) */
#ifndef ARABIC_EDITOR_FIND_H
#define ARABIC_EDITOR_FIND_H

#include <windows.h>

/* يُستدعى مرة بعد إنشاء النافذة الرئيسية ومساحة الكتابة */
void Find_Init(HINSTANCE instance, HWND mainWindow, HWND editor);

/* يعرض نافذة البحث (replace = FALSE) أو الاستبدال (replace = TRUE) غير المشروطة */
void Find_ShowDialog(BOOL replace);

/* البحث عن التالي (F3) بالنص والخيارات الأخيرة؛ يفتح نافذة البحث إن لم يكن هناك نص */
void Find_Next(void);

/* مقبض النافذة المفتوحة (أو NULL) لتمريره إلى IsDialogMessageW في حلقة الرسائل */
HWND Find_Dialog(void);

#endif
