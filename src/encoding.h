/* encoding.h - كشف ترميز الملفات وتحويلها من/إلى UTF-8 (ترميز مساحة الكتابة) */
#ifndef ARABIC_EDITOR_ENCODING_H
#define ARABIC_EDITOR_ENCODING_H

#include <windows.h>
#include <stddef.h>

typedef enum {
    ENC_UTF8,        /* UTF-8 بلا علامة BOM (ترميز الملف الجديد) */
    ENC_UTF8_BOM,    /* UTF-8 يبدأ بـ EF BB BF */
    ENC_UTF16LE,     /* UTF-16 LE يبدأ بـ FF FE */
    ENC_UTF16BE,     /* UTF-16 BE يبدأ بـ FE FF */
    ENC_CODEPAGE     /* ترميز بايت واحد/متعدد عبر صفحة رموز ويندوز (مثل 1256) */
} EncodingKind;

typedef struct {
    EncodingKind kind;
    UINT codePage;   /* يُستعمل فقط مع ENC_CODEPAGE */
} Encoding;

/* صفحة الرموز التي نفترضها للملفات القديمة التي ليست UTF-8 ولا UTF-16 */
UINT Enc_LegacyCodePage(void);

/* يحدد ترميز محتوى ملف مقروء كما هو من القرص */
Encoding Enc_Detect(const unsigned char *data, size_t len);

/* اسم الترميز للعرض في شريط العنوان */
void Enc_Describe(Encoding enc, wchar_t *out, size_t outCount);

/* يحوّل بايتات الملف إلى UTF-8 للعرض في Scintilla.
   يُرجع ذاكرة من HeapAlloc (يحررها المستدعي بـ HeapFree) وطولها في *outLen، أو NULL عند الفشل. */
char *Enc_FileBytesToUtf8(const unsigned char *data, size_t len, Encoding enc, size_t *outLen);

/* يحوّل نص UTF-8 من Scintilla إلى بايتات الملف بالترميز المطلوب (مع BOM إن لزم).
   *lossy يصبح TRUE إن وُجدت حروف لا يمثلها الترميز المختار. */
unsigned char *Enc_Utf8ToFileBytes(const char *text, size_t len, Encoding enc,
                                   size_t *outLen, BOOL *lossy);

#endif
