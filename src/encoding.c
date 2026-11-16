/* encoding.c - كشف الترميز والتحويل؛ مساحة الكتابة تعمل دائمًا بـ UTF-8 (SC_CP_UTF8) */
#include "encoding.h"
#include <stdio.h>

static void *Alloc(size_t n) { return HeapAlloc(GetProcessHeap(), 0, n ? n : 1); }

UINT Enc_LegacyCodePage(void)
{
    /* محرر عربي: الملفات القديمة غالبًا Windows-1256. إن كانت صفحة رموز النظام
       عربية أصلًا (1256) نستعملها، وإلا نفترض 1256 أيضًا بدل صفحة النظام اللاتينية. */
    UINT acp = GetACP();
    return acp == 1256 ? acp : 1256;
}

/* فحص صارم لصحة تسلسلات UTF-8 (يرفض الأشكال الطويلة وأنصاف أزواج UTF-16) */
static BOOL IsValidUtf8(const unsigned char *s, size_t len)
{
    size_t i = 0;
    while (i < len) {
        unsigned char c = s[i];
        size_t need;
        unsigned int cp;
        if (c < 0x80) { i++; continue; }
        else if ((c & 0xE0) == 0xC0) { need = 1; cp = c & 0x1F; }
        else if ((c & 0xF0) == 0xE0) { need = 2; cp = c & 0x0F; }
        else if ((c & 0xF8) == 0xF0) { need = 3; cp = c & 0x07; }
        else return FALSE;
        if (i + need >= len) return FALSE;      /* تسلسل مقطوع في نهاية الملف */
        for (size_t k = 1; k <= need; k++) {
            if ((s[i + k] & 0xC0) != 0x80) return FALSE;
            cp = (cp << 6) | (s[i + k] & 0x3F);
        }
        if ((need == 1 && cp < 0x80) || (need == 2 && cp < 0x800) || (need == 3 && cp < 0x10000))
            return FALSE;                       /* ترميز أطول من اللازم */
        if (cp > 0x10FFFF || (cp >= 0xD800 && cp <= 0xDFFF))
            return FALSE;
        i += need + 1;
    }
    return TRUE;
}

Encoding Enc_Detect(const unsigned char *data, size_t len)
{
    Encoding e = { ENC_UTF8, CP_UTF8 };
    if (len >= 3 && data[0] == 0xEF && data[1] == 0xBB && data[2] == 0xBF) {
        e.kind = ENC_UTF8_BOM;
        return e;
    }
    if (len >= 2 && data[0] == 0xFF && data[1] == 0xFE) { e.kind = ENC_UTF16LE; return e; }
    if (len >= 2 && data[0] == 0xFE && data[1] == 0xFF) { e.kind = ENC_UTF16BE; return e; }

    /* ملف فارغ أو نص صالح بـ UTF-8 (ومنه ASCII الخالص) يُعامَل UTF-8 */
    if (IsValidUtf8(data, len))
        return e;

    /* بلا BOM: نص UTF-16 LE يظهر عادة بصفر في كل بايت ثانٍ للحروف اللاتينية */
    if (len >= 4 && len % 2 == 0) {
        int flags = IS_TEXT_UNICODE_STATISTICS | IS_TEXT_UNICODE_ASCII16;
        if (IsTextUnicode(data, (int)(len > 0x7FFFFFFF ? 0x7FFFFFFF : len), &flags)) {
            e.kind = ENC_UTF16LE;
            return e;
        }
    }

    e.kind = ENC_CODEPAGE;
    e.codePage = Enc_LegacyCodePage();
    return e;
}

void Enc_Describe(Encoding enc, wchar_t *out, size_t outCount)
{
    switch (enc.kind) {
    case ENC_UTF8:     _snwprintf(out, outCount, L"UTF-8"); break;
    case ENC_UTF8_BOM: _snwprintf(out, outCount, L"UTF-8 BOM"); break;
    case ENC_UTF16LE:  _snwprintf(out, outCount, L"UTF-16 LE"); break;
    case ENC_UTF16BE:  _snwprintf(out, outCount, L"UTF-16 BE"); break;
    case ENC_CODEPAGE: _snwprintf(out, outCount, L"Windows-%u", enc.codePage); break;
    }
    if (outCount) out[outCount - 1] = L'\0';
}

/* UTF-16 (بترتيب الآلة) -> UTF-8 */
static char *WideToUtf8(const wchar_t *w, int wlen, size_t *outLen)
{
    int n = wlen ? WideCharToMultiByte(CP_UTF8, 0, w, wlen, NULL, 0, NULL, NULL) : 0;
    if (wlen && n <= 0) return NULL;
    char *u = Alloc((size_t)n + 1);
    if (!u) return NULL;
    if (n) WideCharToMultiByte(CP_UTF8, 0, w, wlen, u, n, NULL, NULL);
    u[n] = '\0';
    *outLen = (size_t)n;
    return u;
}

char *Enc_FileBytesToUtf8(const unsigned char *data, size_t len, Encoding enc, size_t *outLen)
{
    if (len > 0x7FFFFFFE) return NULL;   /* حدود واجهات التحويل في ويندوز */

    if (enc.kind == ENC_UTF8 || enc.kind == ENC_UTF8_BOM) {
        size_t skip = enc.kind == ENC_UTF8_BOM ? 3 : 0;
        char *u = Alloc(len - skip + 1);
        if (!u) return NULL;
        memcpy(u, data + skip, len - skip);
        u[len - skip] = '\0';
        *outLen = len - skip;
        return u;
    }

    if (enc.kind == ENC_UTF16LE || enc.kind == ENC_UTF16BE) {
        size_t skip = (len >= 2 && ((data[0] == 0xFF && data[1] == 0xFE) ||
                                    (data[0] == 0xFE && data[1] == 0xFF))) ? 2 : 0;
        size_t units = (len - skip) / 2;
        wchar_t *w = Alloc(units * sizeof(wchar_t) + sizeof(wchar_t));
        if (!w) return NULL;
        for (size_t i = 0; i < units; i++) {
            unsigned char a = data[skip + 2 * i], b = data[skip + 2 * i + 1];
            w[i] = enc.kind == ENC_UTF16LE ? (wchar_t)(a | (b << 8)) : (wchar_t)((a << 8) | b);
        }
        char *u = WideToUtf8(w, (int)units, outLen);
        HeapFree(GetProcessHeap(), 0, w);
        return u;
    }

    /* صفحة رموز: بايتات -> UTF-16 -> UTF-8 */
    int wlen = len ? MultiByteToWideChar(enc.codePage, 0, (const char *)data, (int)len, NULL, 0) : 0;
    if (len && wlen <= 0) return NULL;
    wchar_t *w = Alloc((size_t)wlen * sizeof(wchar_t) + sizeof(wchar_t));
    if (!w) return NULL;
    if (wlen) MultiByteToWideChar(enc.codePage, 0, (const char *)data, (int)len, w, wlen);
    char *u = WideToUtf8(w, wlen, outLen);
    HeapFree(GetProcessHeap(), 0, w);
    return u;
}

unsigned char *Enc_Utf8ToFileBytes(const char *text, size_t len, Encoding enc,
                                   size_t *outLen, BOOL *lossy)
{
    *lossy = FALSE;
    if (len > 0x7FFFFFFE) return NULL;

    if (enc.kind == ENC_UTF8 || enc.kind == ENC_UTF8_BOM) {
        size_t bom = enc.kind == ENC_UTF8_BOM ? 3 : 0;
        unsigned char *b = Alloc(len + bom);
        if (!b) return NULL;
        if (bom) { b[0] = 0xEF; b[1] = 0xBB; b[2] = 0xBF; }
        memcpy(b + bom, text, len);
        *outLen = len + bom;
        return b;
    }

    int wlen = len ? MultiByteToWideChar(CP_UTF8, 0, text, (int)len, NULL, 0) : 0;
    if (len && wlen <= 0) return NULL;
    wchar_t *w = Alloc((size_t)wlen * sizeof(wchar_t) + sizeof(wchar_t));
    if (!w) return NULL;
    if (wlen) MultiByteToWideChar(CP_UTF8, 0, text, (int)len, w, wlen);

    unsigned char *b = NULL;
    if (enc.kind == ENC_UTF16LE || enc.kind == ENC_UTF16BE) {
        b = Alloc((size_t)wlen * 2 + 2);
        if (b) {
            BOOL le = enc.kind == ENC_UTF16LE;
            b[0] = le ? 0xFF : 0xFE;
            b[1] = le ? 0xFE : 0xFF;
            for (int i = 0; i < wlen; i++) {
                unsigned short u = (unsigned short)w[i];
                b[2 + 2 * i]     = (unsigned char)(le ? (u & 0xFF) : (u >> 8));
                b[2 + 2 * i + 1] = (unsigned char)(le ? (u >> 8) : (u & 0xFF));
            }
            *outLen = (size_t)wlen * 2 + 2;
        }
    } else {
        BOOL usedDefault = FALSE;
        int n = wlen ? WideCharToMultiByte(enc.codePage, WC_NO_BEST_FIT_CHARS, w, wlen,
                                           NULL, 0, NULL, &usedDefault) : 0;
        if (!wlen || n > 0) {
            b = Alloc((size_t)n);
            if (b) {
                if (n) WideCharToMultiByte(enc.codePage, WC_NO_BEST_FIT_CHARS, w, wlen,
                                           (char *)b, n, NULL, &usedDefault);
                *outLen = (size_t)n;
                *lossy = usedDefault;
            }
        }
    }
    HeapFree(GetProcessHeap(), 0, w);
    return b;
}
