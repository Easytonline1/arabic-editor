# قوائم Notepad2 4.2.25 (x64) وأوامرها

## كيف قرأت القوائم
1. **تحديد الموارد بـ REA:** أداة `inspect_pe_resources` على `original/extracted/Notepad2.exe`، والتغطية كاملة (87 موردًا). أعطت لكل مورد إزاحته في الملف وطوله وبصمة SHA-256:

   | المورد | النوع | الإزاحة | الطول | SHA-256 |
   |---|---|---|---|---|
   | القائمة الرئيسية `MENU 100` | 4 | 865288 | 7578 | `20e9db17…61e61a` |
   | قائمة أيقونة شريط المهام `MENU 101` | 4 | 872872 | 340 | `e9ae5a8e…5c56bf` |
   | جدول الاختصارات `ACCELERATORS 100` | 9 | 873216 | 1272 | `21f8f0ce…fb1f0a` |
   | جدول اختصارات `ACCELERATORS 101` | 9 | 874488 | 80 | `842316d1…59794f` |

2. **فك المحتوى:** REA يحدد مواقع الموارد ولا يفك محتوى القوائم. لذلك قرأت البايتات من هذه المواقع بسكربت Python للقراءة فقط، وتحققت أولًا أن بصمة كل مورد تطابق بصمة REA.
   - **قالب القائمة (MENUTEMPLATE):** لكل عنصر علَم 16 بت:
     - `MF_POPUP = 0x10`: قائمة فرعية بلا رقم.
     - `MF_END = 0x80`: آخر عنصر في مستواه.
     - العنصر العادي يتبعه رقم الأمر (16 بت)، ثم النص بترميز UTF-16 منتهيًا بصفر.
     - النص يحمل الاختصار بعد علامة الجدولة `\t`.
   - **جدول الاختصارات:** مدخلات من 8 بايت: `fVirt` و`key` و`cmd`.
3. **النتيجة:**
   - القائمة الرئيسية: 148 أمرًا، و16 قائمة فرعية، و38 فاصلًا.
   - قائمة أيقونة شريط المهام: 12 أمرًا.
   - جدول الاختصارات الرئيسي: 159 مدخلًا.
   - لا تكرار في أرقام الأوامر.

**أعمدة الجدول:**
- عمود "الاختصار في القائمة" هو النص الظاهر في القائمة.
- عمود "جدول الاختصارات" هو ما يربطه البرنامج فعلًا بالأمر في `ACCELERATORS 100`. قد يختلف العمودان، فبعض الأوامر لها أكثر من اختصار.
- أوامر قائمة شريط المهام لا تُربط بجدول الاختصارات الرئيسي.

## كيف تتبّعت الأوامر المطلوبة بـ REA
اكتفيت بالأوامر التسعة المطلوبة حسب `plan.md`. استعملت أوامر `rea` من الطرفية (`rea decompile` و`rea instructions` مع `--provider ghidra`)، لأن تحليل Ghidra لهذا الملف يتجاوز مهلة MCP البالغة 60 ثانية.

1. **إجراء النافذة الرئيسية:** فك ترجمة `FUN_140092e3c`، دالة تسجيل الصنف، أظهر `lpfnWndProc = FUN_140091fe0`.
2. **معالج الأوامر:** في `FUN_140091fe0`، رسالة `WM_COMMAND` (`0x111`) تذهب إلى `FUN_14008cf60(hwnd, LOWORD(wParam), …)`.
3. **جداول القفز:** `FUN_14008cf60` يوزّع الأرقام بعدة `switch`، وGhidra لم يربط كل الحالات بأرقام الأوامر. لذلك قرأت جداول القفز من الملف للقراءة فقط:
   - **أوامر الملف `0x9c41`–`0x9c54`:** جدول عناوين نسبية (RVA) عند `0x140091c58`. الأمر 40001 يذهب إلى `0x14008de8d`، و40004 إلى `0x14008e0a7`، و40005 إلى `0x14008e0bd`.
   - **البحث والاستبدال:** جدول بايتات عند `0x140091dc8` (الأساس `0x9caa`). الأمر 40364 يقابل الحالة `0x42`، و40366 الحالة `0x44`، و40368 الحالة `0x45`.
   - **العرض:** جدول بايتات عند `0x140091f60` (الأساس `0x9db3`). هنا يرقّم Ghidra الحالات بالفهرس نفسه، فالأمر 40403 هو الحالة `0x20`، و40404 الحالة `0x21`.
   - **التعريف:** الأمر 40500 (`0x9e34`) يُفحص بشرط `if` مباشر.
4. **التحقق من المحتوى:** فككت ترجمة كل دالة منفذة لأتأكد أنها تؤدي الوظيفة المتوقعة:
   - التحميل يستدعي حوار الفتح، والحفظ يستدعي حوار الحفظ.
   - حوار البحث يُنشأ بالمعامل `0`، وحوار الاستبدال بالمعامل `1`.
   - التفاف السطور يرسل `SCI_SETWRAPMODE`.
   - `FUN_140094544` تحوي استدعاء `ChooseFontW`. هذا حسم المجهول المسجل في `analysis.md`.
   - إجراء حوار التعريف يحوي نص الإصدار.

## الجدول
"مطلوب" تعني أنه من الأوامر الستة في `plan.md`، و"بُني ✅" تعني أنه صار في `editor.exe`. الحفظ يشمل Save وSave As، والبحث يشمل Find وFind Next. وكل ما عداها خارج النطاق.

| # | القائمة | اسم الأمر | الترجمة العربية (للمطلوب) | الاختصار في القائمة | جدول الاختصارات | الرقم | النطاق | الدالة المنفّذة (للمطلوب فقط) |
|---|---|---|---|---|---|---|---|---|
| 1 | File | New | — | Ctrl+N | Ctrl+N / Ctrl+F4 | 40000 (`0x9c40`) | خارج النطاق | — |
| 2 | File | Open... | فتح... | Ctrl+O | Ctrl+O | 40001 (`0x9c41`) | **مطلوب** | `FUN_14008cf60` ← jump table → `FUN_14008bc5c(0,0,0,0,"")` (تحميل ملف)، وعند المسار الفارغ تستدعي `FUN_140087fd4` ← `GetOpenFileNameW` |
| 3 | File | Revert | — | F5 | F5 | 40002 (`0x9c42`) | خارج النطاق | — |
| 4 | File | Save | حفظ | Ctrl+S | Ctrl+S | 40004 (`0x9c44`) | **مطلوب** | `FUN_14008cf60` → `FUN_14008b6cc(1,0,0,0)` (حفظ الملف)؛ تستدعي `FUN_140088204` ← `GetSaveFileNameW` فقط إن لم يكن للملف اسم |
| 5 | File | Save As... | حفظ باسم... | F6 | F6 | 40005 (`0x9c45`) | **مطلوب** | `FUN_14008cf60` → `FUN_14008b6cc(1,0,1,0)` (المعامل الثالث = حفظ باسم) → `FUN_140088204` ← `GetSaveFileNameW` |
| 6 | File | Save Copy... | — | Ctrl+F6 | Ctrl+F6 | 40006 (`0x9c46`) | خارج النطاق | — |
| 7 | File | Read Only | — | — | — | 40007 (`0x9c47`) | خارج النطاق | — |
| 8 | File › Launch | New Window | — | Alt+N | Alt+N | 40011 (`0x9c4b`) | خارج النطاق | — |
| 9 | File › Launch | Empty Window | — | Alt+0 | Alt+0 | 40012 (`0x9c4c`) | خارج النطاق | — |
| 10 | File › Launch | Execute Document | — | Ctrl+L | Ctrl+L | 40008 (`0x9c48`) | خارج النطاق | — |
| 11 | File › Launch | Open with... | — | Alt+L | Alt+L | 40009 (`0x9c49`) | خارج النطاق | — |
| 12 | File › Launch | Command... | — | Ctrl+R | Ctrl+R | 40010 (`0x9c4a`) | خارج النطاق | — |
| 13 | File › Encoding | ANSI | — | — | — | 40100 (`0x9ca4`) | خارج النطاق | — |
| 14 | File › Encoding | Unicode | — | — | — | 40101 (`0x9ca5`) | خارج النطاق | — |
| 15 | File › Encoding | Unicode Big Endian | — | — | — | 40102 (`0x9ca6`) | خارج النطاق | — |
| 16 | File › Encoding | UTF-8 | — | — | — | 40103 (`0x9ca7`) | خارج النطاق | — |
| 17 | File › Encoding | UTF-8 with Signature | — | — | — | 40104 (`0x9ca8`) | خارج النطاق | — |
| 18 | File › Encoding | More... | — | F9 | F9 | 40105 (`0x9ca9`) | خارج النطاق | — |
| 19 | File › Encoding | Recode... | — | F8 | F8 | 40106 (`0x9caa`) | خارج النطاق | — |
| 20 | File › Encoding | Default... | — | — | — | 40107 (`0x9cab`) | خارج النطاق | — |
| 21 | File › Line Endings | Windows (CR+LF) | — | — | — | 40200 (`0x9d08`) | خارج النطاق | — |
| 22 | File › Line Endings | Unix (LF) | — | — | — | 40201 (`0x9d09`) | خارج النطاق | — |
| 23 | File › Line Endings | Mac (CR) | — | — | — | 40202 (`0x9d0a`) | خارج النطاق | — |
| 24 | File › Line Endings | Default... | — | — | — | 40203 (`0x9d0b`) | خارج النطاق | — |
| 25 | File | Page Setup... | — | — | — | 40013 (`0x9c4d`) | خارج النطاق | — |
| 26 | File | Print... | — | Ctrl+P | Ctrl+P | 40014 (`0x9c4e`) | خارج النطاق | — |
| 27 | File | Properties... | — | — | — | 40015 (`0x9c4f`) | خارج النطاق | — |
| 28 | File | Create Desktop Link | — | — | — | 40016 (`0x9c50`) | خارج النطاق | — |
| 29 | File › Favorites | Open Favorites... | — | Alt+I | Alt+I | 40017 (`0x9c51`) | خارج النطاق | — |
| 30 | File › Favorites | Add Current File... | — | Alt+K | Alt+K | 40018 (`0x9c52`) | خارج النطاق | — |
| 31 | File › Favorites | Manage... | — | Alt+F9 | Alt+F9 | 40019 (`0x9c53`) | خارج النطاق | — |
| 32 | File | Recent (History)... | — | Alt+H | Alt+H | 40020 (`0x9c54`) | خارج النطاق | — |
| 33 | File | Exit | — | Alt+F4 | — | 40021 (`0x9c55`) | خارج النطاق | — |
| 34 | Edit | Undo | — | Ctrl+Z | Alt+Backspace / Ctrl+Shift+Y / Ctrl+Z | 40300 (`0x9d6c`) | خارج النطاق | — |
| 35 | Edit | Redo | — | Ctrl+Y | Ctrl+Y / Ctrl+Shift+Z | 40301 (`0x9d6d`) | خارج النطاق | — |
| 36 | Edit | Cut | — | Ctrl+X | Shift+Del / Ctrl+X | 40302 (`0x9d6e`) | خارج النطاق | — |
| 37 | Edit | Copy | — | Ctrl+C | Ctrl+C | 40303 (`0x9d6f`) | خارج النطاق | — |
| 38 | Edit | Copy All | — | Alt+C | Alt+C | 40304 (`0x9d70`) | خارج النطاق | — |
| 39 | Edit | Copy Add | — | Ctrl+E | Ctrl+E | 40305 (`0x9d71`) | خارج النطاق | — |
| 40 | Edit | Paste | — | Ctrl+V | Ctrl+V | 40306 (`0x9d72`) | خارج النطاق | — |
| 41 | Edit | Swap | — | Ctrl+K | Ctrl+K | 40307 (`0x9d73`) | خارج النطاق | — |
| 42 | Edit | Clear | — | Del | — | 40308 (`0x9d74`) | خارج النطاق | — |
| 43 | Edit | Clear Clipboard | — | — | — | 40309 (`0x9d75`) | خارج النطاق | — |
| 44 | Edit | Select All | — | Ctrl+A | Ctrl+A | 40310 (`0x9d76`) | خارج النطاق | — |
| 45 | Edit › Lines | Move Up | — | Ctrl+Shift+Up | Ctrl+Shift+Up | 40313 (`0x9d79`) | خارج النطاق | — |
| 46 | Edit › Lines | Move Down | — | Ctrl+Shift+Down | Ctrl+Shift+Down | 40314 (`0x9d7a`) | خارج النطاق | — |
| 47 | Edit › Lines | Duplicate Line | — | Ctrl+D | Ctrl+D | 40315 (`0x9d7b`) | خارج النطاق | — |
| 48 | Edit › Lines | Cut Line | — | Ctrl+Shift+X | Ctrl+Shift+X | 40316 (`0x9d7c`) | خارج النطاق | — |
| 49 | Edit › Lines | Copy Line | — | Ctrl+Shift+C | Ctrl+Shift+C | 40317 (`0x9d7d`) | خارج النطاق | — |
| 50 | Edit › Lines | Delete Line | — | Ctrl+Shift+D | Ctrl+Shift+D | 40318 (`0x9d7e`) | خارج النطاق | — |
| 51 | Edit › Lines | Column Wrap... | — | Ctrl+Shift+W | Ctrl+Shift+W | 40321 (`0x9d81`) | خارج النطاق | — |
| 52 | Edit › Lines | Split Lines | — | Ctrl+I | Ctrl+I | 40322 (`0x9d82`) | خارج النطاق | — |
| 53 | Edit › Lines | Join Lines | — | Ctrl+J | Ctrl+J | 40323 (`0x9d83`) | خارج النطاق | — |
| 54 | Edit › Lines | Join Paragraphs | — | Ctrl+Shift+J | Ctrl+Shift+J | 40324 (`0x9d84`) | خارج النطاق | — |
| 55 | Edit › Block | Indent | — | Tab | — | 40325 (`0x9d85`) | خارج النطاق | — |
| 56 | Edit › Block | Unindent | — | Shift+Tab | — | 40326 (`0x9d86`) | خارج النطاق | — |
| 57 | Edit › Block | Enclose Selection... | — | Alt+Q | Alt+Q | 40327 (`0x9d87`) | خارج النطاق | — |
| 58 | Edit › Block | Duplicate Selection | — | Alt+D | Alt+D | 40328 (`0x9d88`) | خارج النطاق | — |
| 59 | Edit › Block | Pad With Spaces | — | Alt+B | Alt+B | 40329 (`0x9d89`) | خارج النطاق | — |
| 60 | Edit › Block | Strip First Character | — | Alt+Z | Alt+Z | 40330 (`0x9d8a`) | خارج النطاق | — |
| 61 | Edit › Block | Strip Last Character | — | Alt+U | Alt+U | 40331 (`0x9d8b`) | خارج النطاق | — |
| 62 | Edit › Block | Strip Trailing Blanks | — | Alt+W | Alt+W | 40332 (`0x9d8c`) | خارج النطاق | — |
| 63 | Edit › Block | Compress Whitespace | — | Alt+P | Alt+P | 40333 (`0x9d8d`) | خارج النطاق | — |
| 64 | Edit › Block | Merge Blank Lines | — | Alt+Y | Alt+Y | 40334 (`0x9d8e`) | خارج النطاق | — |
| 65 | Edit › Block | Remove Blank Lines | — | Alt+R | Alt+R | 40335 (`0x9d8f`) | خارج النطاق | — |
| 66 | Edit › Block | Modify Lines... | — | Alt+M | Alt+M | 40336 (`0x9d90`) | خارج النطاق | — |
| 67 | Edit › Block | Align Lines... | — | Alt+J | Alt+J | 40338 (`0x9d92`) | خارج النطاق | — |
| 68 | Edit › Block | Sort Lines... | — | Alt+O | Alt+O | 40337 (`0x9d91`) | خارج النطاق | — |
| 69 | Edit › Convert | Uppercase | — | Ctrl+Shift+U | Ctrl+Shift+U | 40339 (`0x9d93`) | خارج النطاق | — |
| 70 | Edit › Convert | Lowercase | — | Ctrl+U | Ctrl+U | 40340 (`0x9d94`) | خارج النطاق | — |
| 71 | Edit › Convert | Invert Case | — | Ctrl+Alt+U | Ctrl+Alt+U | 40341 (`0x9d95`) | خارج النطاق | — |
| 72 | Edit › Convert | Title Case | — | Ctrl+Alt+I | Ctrl+Alt+I | 40342 (`0x9d96`) | خارج النطاق | — |
| 73 | Edit › Convert | Sentence Case | — | Ctrl+Alt+O | Ctrl+Alt+O | 40343 (`0x9d97`) | خارج النطاق | — |
| 74 | Edit › Convert | Tabify Selection | — | Ctrl+Shift+T | Ctrl+Shift+T | 40345 (`0x9d99`) | خارج النطاق | — |
| 75 | Edit › Convert | Untabify Selection | — | Ctrl+Shift+S | Ctrl+Shift+S | 40344 (`0x9d98`) | خارج النطاق | — |
| 76 | Edit › Convert | Tabify Indent | — | Ctrl+Alt+T | Ctrl+Alt+T | 40347 (`0x9d9b`) | خارج النطاق | — |
| 77 | Edit › Convert | Untabify Indent | — | Ctrl+Alt+S | Ctrl+Alt+S | 40346 (`0x9d9a`) | خارج النطاق | — |
| 78 | Edit › Insert | HTML/XML Tag... | — | Alt+X | Alt+X | 40348 (`0x9d9c`) | خارج النطاق | — |
| 79 | Edit › Insert | Encoding Identifier | — | Ctrl+F8 | Ctrl+F8 | 40349 (`0x9d9d`) | خارج النطاق | — |
| 80 | Edit › Insert | Time/Date (Short Form) | — | Ctrl+F5 | Ctrl+F5 | 40350 (`0x9d9e`) | خارج النطاق | — |
| 81 | Edit › Insert | Time/Date (Long Form) | — | Ctrl+Shift+F5 | Ctrl+Shift+F5 | 40351 (`0x9d9f`) | خارج النطاق | — |
| 82 | Edit › Insert | Filename | — | Ctrl+F9 | Ctrl+F9 | 40352 (`0x9da0`) | خارج النطاق | — |
| 83 | Edit › Insert | Path and Filename | — | Ctrl+Shift+F9 | Ctrl+Shift+F9 | 40353 (`0x9da1`) | خارج النطاق | — |
| 84 | Edit › Special | Line Comment (Toggle) | — | Ctrl+Q | Ctrl+Q | 40354 (`0x9da2`) | خارج النطاق | — |
| 85 | Edit › Special | Stream Comment | — | Ctrl+Shift+Q | Ctrl+Shift+Q | 40355 (`0x9da3`) | خارج النطاق | — |
| 86 | Edit › Special | URL Encode | — | Ctrl+Shift+E | Ctrl+Shift+E | 40356 (`0x9da4`) | خارج النطاق | — |
| 87 | Edit › Special | URL Decode | — | Ctrl+Shift+R | Ctrl+Shift+R | 40357 (`0x9da5`) | خارج النطاق | — |
| 88 | Edit › Special | Escape C Chars | — | Ctrl+Alt+E | Ctrl+Alt+E | 40358 (`0x9da6`) | خارج النطاق | — |
| 89 | Edit › Special | Unescape C Chars | — | Ctrl+Alt+R | Ctrl+Alt+R | 40359 (`0x9da7`) | خارج النطاق | — |
| 90 | Edit › Special | Char To Hex | — | Ctrl+Alt+X | Ctrl+Alt+X | 40360 (`0x9da8`) | خارج النطاق | — |
| 91 | Edit › Special | Hex To Char | — | Ctrl+Alt+C | Ctrl+Alt+C | 40361 (`0x9da9`) | خارج النطاق | — |
| 92 | Edit › Special | Find Matching Brace | — | Ctrl+B | Ctrl+B | 40362 (`0x9daa`) | خارج النطاق | — |
| 93 | Edit › Special | Select To Matching Brace | — | Ctrl+Shift+B | Ctrl+Shift+B | 40363 (`0x9dab`) | خارج النطاق | — |
| 94 | Edit › Special | Select To Next | — | F2 | F2 | 40371 (`0x9db3`) | خارج النطاق | — |
| 95 | Edit › Special | Select To Previous | — | Shift+F2 | Shift+F2 | 40372 (`0x9db4`) | خارج النطاق | — |
| 96 | Edit | Find... | بحث... | Ctrl+F | Ctrl+F | 40364 (`0x9dac`) | **مطلوب — بُني ✅** | `FUN_14008cf60` (case 0x42) → `FUN_14007f174(hwndEdit, …, 0)` ← `FUN_140082c50` يُنشئ حوار البحث غير المشروط (المورد `0x76`) وإجراؤه `FUN_14007e168` |
| 97 | Edit | Save Find Text | — | Alt+F3 | Alt+F3 | 40365 (`0x9dad`) | خارج النطاق | — |
| 98 | Edit | Find Next | البحث عن التالي | F3 | F3 | 40366 (`0x9dae`) | **مطلوب — بُني ✅** | `FUN_14008cf60` (case 0x44) → `FUN_14007ae84` (البحث عن التالي؛ تستعمل النصّين `MsgNotFound` و`MsgFindWrap1`)، وإن لم يكن هناك نص بحث يُرسل الأمر 40364 |
| 99 | Edit | Find Previous | — | Shift+F3 | Shift+F3 | 40367 (`0x9daf`) | خارج النطاق | — |
| 100 | Edit | Replace... | استبدال... | Ctrl+H | Ctrl+H | 40368 (`0x9db0`) | **مطلوب — بُني ✅** | `FUN_14008cf60` (case 0x45) → `FUN_14007f174(hwndEdit, …, 1)` ← `FUN_140082c50` يُنشئ حوار الاستبدال (المورد `0x77`) وإجراؤه `FUN_14007e168`؛ زر Replace (`112`) → `FUN_14007b2c0` |
| 101 | Edit | Replace Next | — | F4 | F4 | 40369 (`0x9db1`) | خارج النطاق | — |
| 102 | Edit | Goto... | — | Ctrl+G | Ctrl+G | 40370 (`0x9db2`) | خارج النطاق | — |
| 103 | View | Syntax Scheme... | — | F12 | F12 | 40400 (`0x9dd0`) | خارج النطاق | — |
| 104 | View | 2nd Default Scheme | — | Shift+F12 | Shift+F12 | 40401 (`0x9dd1`) | خارج النطاق | — |
| 105 | View | Customize Schemes... | — | Ctrl+F12 | Ctrl+F12 | 40402 (`0x9dd2`) | خارج النطاق | — |
| 106 | View | Default Font... | الخط الافتراضي... | Alt+F12 | Alt+F12 | 40403 (`0x9dd3`) | **مطلوب** | `FUN_14008cf60` (case 0x20) → `FUN_140097c8c` → `FUN_140094544`، وفيها `CALL [0x140001068]` = `ChooseFontW` عند `0x1400946d0` |
| 107 | View | Word Wrap | التفاف السطور | Ctrl+W | Ctrl+W | 40404 (`0x9dd4`) | **مطلوب** | `FUN_14008cf60` (case 0x21) ينفّذه مباشرة: يقلب علَم الالتفاف ويرسل `SendMessageW(hwndEdit, 0x8DC = SCI_SETWRAPMODE, …)` |
| 108 | View | Long Line Marker | — | Ctrl+Shift+L | Ctrl+Shift+L | 40405 (`0x9dd5`) | خارج النطاق | — |
| 109 | View | Indentation Guides | — | Ctrl+Shift+G | Ctrl+Shift+G | 40406 (`0x9dd6`) | خارج النطاق | — |
| 110 | View | Show Whitespace | — | Ctrl+Shift+8 | Ctrl+Shift+8 | 40407 (`0x9dd7`) | خارج النطاق | — |
| 111 | View | Show Line Endings | — | Ctrl+Shift+9 | Ctrl+Shift+9 | 40408 (`0x9dd8`) | خارج النطاق | — |
| 112 | View | Show Wrap Symbols | — | Ctrl+Shift+0 | Ctrl+Shift+0 | 40409 (`0x9dd9`) | خارج النطاق | — |
| 113 | View | Visual Brace Matching | — | Ctrl+Shift+V | Ctrl+Shift+V | 40410 (`0x9dda`) | خارج النطاق | — |
| 114 | View | Highlight Current Line | — | Ctrl+Shift+I | Ctrl+Shift+I | 40411 (`0x9ddb`) | خارج النطاق | — |
| 115 | View | Line Numbers | — | Ctrl+Shift+N | Ctrl+Shift+N | 40412 (`0x9ddc`) | خارج النطاق | — |
| 116 | View | Selection Margin | — | Ctrl+Shift+M | Ctrl+Shift+M | 40413 (`0x9ddd`) | خارج النطاق | — |
| 117 | View | Zoom In | — | Ctrl++ | Ctrl+Num+ / Ctrl+= | 40414 (`0x9dde`) | خارج النطاق | — |
| 118 | View | Zoom Out | — | Ctrl+- | Ctrl+- / Ctrl+Num- | 40415 (`0x9ddf`) | خارج النطاق | — |
| 119 | View | Reset Zoom | — | Ctrl+/ | Ctrl+Num/ / Ctrl+/ | 40416 (`0x9de0`) | خارج النطاق | — |
| 120 | Settings | Insert Tabs as Spaces | — | — | — | 40417 (`0x9de1`) | خارج النطاق | — |
| 121 | Settings | Tab Settings... | — | Ctrl+T | Ctrl+T | 40418 (`0x9de2`) | خارج النطاق | — |
| 122 | Settings | Word Wrap Settings... | — | — | — | 40419 (`0x9de3`) | خارج النطاق | — |
| 123 | Settings | Long Line Settings... | — | — | — | 40420 (`0x9de4`) | خارج النطاق | — |
| 124 | Settings | Auto Indent Text | — | — | — | 40421 (`0x9de5`) | خارج النطاق | — |
| 125 | Settings | Auto Close HTML/XML | — | Ctrl+Shift+H | Ctrl+Shift+H | 40422 (`0x9de6`) | خارج النطاق | — |
| 126 | Settings | Reuse Window | — | — | — | 40423 (`0x9de7`) | خارج النطاق | — |
| 127 | Settings | Sticky Window Position | — | — | — | 40424 (`0x9de8`) | خارج النطاق | — |
| 128 | Settings | Always On Top | — | Alt+T | Alt+T | 40425 (`0x9de9`) | خارج النطاق | — |
| 129 | Settings | Minimize To Tray | — | — | — | 40426 (`0x9dea`) | خارج النطاق | — |
| 130 | Settings | Transparent Mode | — | Ctrl+0 | Ctrl+0 | 40427 (`0x9deb`) | خارج النطاق | — |
| 131 | Settings | Single File Instance | — | — | — | 40428 (`0x9dec`) | خارج النطاق | — |
| 132 | Settings | File Change Notification... | — | Alt+F5 | Alt+F5 | 40429 (`0x9ded`) | خارج النطاق | — |
| 133 | Settings › Window Title Display | Filename Only | — | — | — | 40430 (`0x9dee`) | خارج النطاق | — |
| 134 | Settings › Window Title Display | Filename and Directory | — | — | — | 40431 (`0x9def`) | خارج النطاق | — |
| 135 | Settings › Window Title Display | Full Pathname | — | — | — | 40432 (`0x9df0`) | خارج النطاق | — |
| 136 | Settings › Window Title Display | Text Excerpt | — | Ctrl+9 | — | 40433 (`0x9df1`) | خارج النطاق | — |
| 137 | Settings › Esc Key Function | None | — | — | — | 40434 (`0x9df2`) | خارج النطاق | — |
| 138 | Settings › Esc Key Function | Minimize Notepad2 | — | — | — | 40435 (`0x9df3`) | خارج النطاق | — |
| 139 | Settings › Esc Key Function | Exit Notepad2 | — | — | — | 40436 (`0x9df4`) | خارج النطاق | — |
| 140 | Settings | Save Before Running Tools | — | — | — | 40437 (`0x9df5`) | خارج النطاق | — |
| 141 | Settings | Remember Recent Files | — | — | — | 40438 (`0x9df6`) | خارج النطاق | — |
| 142 | Settings | Remember Search Strings | — | — | — | 40439 (`0x9df7`) | خارج النطاق | — |
| 143 | Settings | Show Toolbar | — | — | — | 40440 (`0x9df8`) | خارج النطاق | — |
| 144 | Settings | Customize Toolbar... | — | — | — | 40441 (`0x9df9`) | خارج النطاق | — |
| 145 | Settings | Show Statusbar | — | — | — | 40442 (`0x9dfa`) | خارج النطاق | — |
| 146 | Settings | Save Settings On Exit | — | — | — | 40443 (`0x9dfb`) | خارج النطاق | — |
| 147 | Settings | Save Settings Now | — | F7 | F7 | 40444 (`0x9dfc`) | خارج النطاق | — |
| 148 | ? | About... | حول البرنامج... | F1 | F1 | 40500 (`0x9e34`) | **مطلوب** | `FUN_14008cf60` → `FUN_140082bdc(hInst, 100, hwnd, FUN_14006e794)` ← `DialogBoxIndirectParamW`؛ إجراء الحوار `FUN_14006e794` يضع نصوص الإصدار والروابط |
| 149 | + | Undo | — | — | — | 40300 (`0x9d6c`) | خارج النطاق | — |
| 150 | + | Redo | — | — | — | 40301 (`0x9d6d`) | خارج النطاق | — |
| 151 | + | Cut | — | — | — | 40302 (`0x9d6e`) | خارج النطاق | — |
| 152 | + | Copy | — | — | — | 40303 (`0x9d6f`) | خارج النطاق | — |
| 153 | + | Paste | — | — | — | 40306 (`0x9d72`) | خارج النطاق | — |
| 154 | + | Clear | — | — | — | 40308 (`0x9d74`) | خارج النطاق | — |
| 155 | + | Select All | — | — | — | 40310 (`0x9d76`) | خارج النطاق | — |
| 156 | + | Show Toolbar | — | — | — | 40440 (`0x9df8`) | خارج النطاق | — |
| 157 | + | Customize Toolbar... | — | — | — | 40441 (`0x9df9`) | خارج النطاق | — |
| 158 | + | Show Statusbar | — | — | — | 40442 (`0x9dfa`) | خارج النطاق | — |
| 159 | + | Open Notepad2 | — | — | — | 40600 (`0x9e98`) | خارج النطاق | — |
| 160 | + | Exit Notepad2 | — | — | — | 40601 (`0x9e99`) | خارج النطاق | — |

## خيارات نافذتي البحث والاستبدال في الأصل
قرأتها من الموردين `DIALOG 118` (Find Text) و`DIALOG 119` (Replace Text). حدّد REA مواقعهما بـ `inspect_pe_resources`، وتحققت من بصمة كل منهما.

ربطت كل عنصر بأثره من إجراء الحوار `FUN_14007e168` (دليل REA `ev_229b6a32…`):
- إجراء الحوار يقرأ مربعات الاختيار بـ `IsDlgButtonChecked`.
- يحوّل بعضها إلى أعلام بحث في Scintilla، ويخزّن الباقي أعلامًا داخلية في بنية البحث.
- يوزّع الأزرار على الدوال المنفذة.

| رقم العنصر | العنصر في الأصل | أثره في الأصل | الحالة في Arabic Editor |
|---|---|---|---|
| `100` | Search String (قائمة منسدلة بالسجل) | النص المطلوب | **بُني ✅** حقل نص عادي بلا سجل |
| `101` | Replace with (قائمة منسدلة بالسجل) | نص الاستبدال | **بُني ✅** حقل نص عادي بلا سجل |
| `102` | Match case | `SCFIND_MATCHCASE` (4) | **بُني ✅** |
| `103` | Match whole word only | `SCFIND_WHOLEWORD` (2) | خارج النطاق |
| `104` | Match beginning of word only | `SCFIND_WORDSTART` (0x100000) | خارج النطاق |
| `105` | Regular expression search | `SCFIND_REGEXP \| SCFIND_POSIX` (0x600000)، والاستبدال بـ `SCI_REPLACETARGETRE` | خارج النطاق |
| `106` | Transform backslashes | يحوّل `\n` و`\t` وأمثالها قبل البحث | خارج النطاق |
| `107` | Don't wrap around | يمنع العودة إلى أول النص | خارج النطاق، وعندنا التفاف دائم دون سؤال |
| `108` | Close after find / replace | يغلق الحوار بعد العملية | خارج النطاق |
| `1` | Find Next | `FUN_14007ae84` | **بُني ✅** |
| `109` | Find Previous | `FUN_14007b088` | خارج النطاق |
| `112` | Replace | `FUN_14007b2c0` (دليل `ev_b48d3f66…`) | **بُني ✅** |
| `113` | Replace All | `FUN_14007b6c8` | خارج النطاق |
| `114` | In Selection | `FUN_14007bb0c` | خارج النطاق |
| `2` | Close | يغلق الحوار | **بُني ✅** |
| `115` | رابط Goto Replace / Goto Find | يبدّل بين الحوارين | خارج النطاق، لكن Ctrl+F وCtrl+H يبدّلان الوضع في نافذتنا |

نص الاستبدال الخاص `^c` يعني "محتوى الحافظة" (`FUN_140073110`)، وهو خارج النطاق أيضًا.

## ملاحظات
- **أوامر بلا مدخل في جدول الاختصارات:** Exit (Alt+F4)، وClear (Del)، وIndent (Tab)، وUnindent (Shift+Tab)، وText Excerpt (Ctrl+9). هذه يعالجها النظام أو عنصر Scintilla، أو تُلتقط في مكان آخر.
- **Find Previous (40367) وReplace Next (40369):** خارج النطاق. لكن معالجهما يشارك Find Next الحالة `0x44` نفسها.
- **الدوال التي سيعتمد عليها تصميمنا**، بنسختنا المكتوبة بأنفسنا:
  - فتح الملف: `FUN_14008bc5c`.
  - الحفظ: `FUN_14008b6cc`.
  - حوارا البحث والاستبدال: `FUN_14007f174` و`FUN_14007e168`.
  - البحث عن التالي: `FUN_14007ae84`.
  - الخط: `FUN_140094544`.
  - التعريف: `FUN_14006e794`.
- **الأسماء:** أسماء الدوال كما يعرضها Ghidra (`FUN_<العنوان>`)، لأن الملف بلا رموز. الوصف بين القوسين من استنتاجي بناءً على ما تستدعيه كل دالة.
