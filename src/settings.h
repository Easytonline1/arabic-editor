/* settings.h - إعدادات البرنامج المحفوظة في %APPDATA%\ArabicEditor\editor.ini */
#ifndef ARABIC_EDITOR_SETTINGS_H
#define ARABIC_EDITOR_SETTINGS_H

#include <windows.h>

#define FONT_SIZE_MIN      6
#define FONT_SIZE_MAX      72
#define FONT_SIZE_DEFAULT  14

typedef struct {
    int  fontSize;      /* بالنقاط */
    BOOL wordWrap;
    int  width;         /* حجم النافذة في وضعها العادي (غير المكبّر) بالبكسل */
    int  height;
    BOOL maximized;
} Settings;

/* يملأ s بالقيم الافتراضية */
void Settings_Defaults(Settings *s);

/* المسار الكامل لملف الإعدادات (لا يُنشئ شيئًا). يُرجع FALSE إن تعذّر معرفة مجلد AppData. */
BOOL Settings_Path(wchar_t *path, size_t count);

/* يقرأ الملف إن وُجد؛ القيم الناقصة أو غير الصالحة تبقى افتراضية */
void Settings_Load(Settings *s);

/* يُنشئ المجلد إن لزم ويكتب الملف كاملًا.
   عند الفشل يُرجع FALSE ويضع رمز الخطأ في *error والمسار الذي فشل في failedPath. */
BOOL Settings_Save(const Settings *s, DWORD *error, wchar_t *failedPath, size_t count);

#endif
