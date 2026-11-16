/* wordcount.h - عدّ الكلمات في نص UTF-8 */
#ifndef ARABIC_EDITOR_WORDCOUNT_H
#define ARABIC_EDITOR_WORDCOUNT_H

#include <stddef.h>

/* الكلمة: أي تتابع من حروف غير المسافات (المسافة وTab ونهاية السطر ومسافات يونيكود).
   "one two three" = 3، و"notes.txt" = 1. */
size_t WordCount_Utf8(const char *text, size_t len);

#endif
