/* wordcount.c - عدّ الكلمات: نحوّل إلى UTF-16 ونعدّ كل بداية تتابع ليس مسافة.
   تصنيف المسافة من ويندوز (GetStringTypeW + C1_SPACE) فيشمل مسافات يونيكود مثل U+00A0. */
#include "wordcount.h"
#include <windows.h>

#define CHUNK_UNITS 4096

size_t WordCount_Utf8(const char *text, size_t len)
{
    if (!text || !len)
        return 0;

    int wlen = len > 0x7FFFFFFE ? 0 : MultiByteToWideChar(CP_UTF8, 0, text, (int)len, NULL, 0);
    if (wlen <= 0)
        return 0;
    wchar_t *w = HeapAlloc(GetProcessHeap(), 0, (size_t)wlen * sizeof(wchar_t));
    if (!w)
        return 0;
    MultiByteToWideChar(CP_UTF8, 0, text, (int)len, w, wlen);

    size_t words = 0;
    BOOL inWord = FALSE;
    WORD types[CHUNK_UNITS];
    for (int start = 0; start < wlen; start += CHUNK_UNITS) {
        int n = wlen - start < CHUNK_UNITS ? wlen - start : CHUNK_UNITS;
        if (!GetStringTypeW(CT_CTYPE1, w + start, n, types))
            ZeroMemory(types, sizeof(types));
        for (int i = 0; i < n; i++) {
            /* نصف زوج UTF-16 الثاني ليس بداية حرف، فيتبع حالة ما قبله */
            wchar_t c = w[start + i];
            if (c >= 0xDC00 && c <= 0xDFFF)
                continue;
            BOOL space = (types[i] & C1_SPACE) != 0;
            if (!space && !inWord)
                words++;
            inWord = !space;
        }
    }
    HeapFree(GetProcessHeap(), 0, w);
    return words;
}
