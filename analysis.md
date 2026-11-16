# تحليل Notepad2 4.2.25 (x64) بأدوات REA

## مصدر الملف والرخصة
- **المصدر:** `https://www.flos-freeware.ch/zip/notepad2_4.2.25_x64.zip`
- **SHA-256 للأرشيف:** `37a62f7e891335a1e671f444cefe761fcd814a2fa1a3141fe2501d8433d8920b`
- **محتوى الأرشيف:** `Notepad2.exe` و `Notepad2.ini` و `Notepad2.txt` و `License.txt`. المحتوى في `original/extracted/`، وهو مستثنى من git عبر `.gitignore`.
- **SHA-256 لـ `Notepad2.exe`:** `c6086336a827a9852ee5cf6f46ffb7b1fccf82f194132a0c8a217d1240654f9f`
- **الرخصة:** BSD ثلاثية البنود، "Copyright © 2004-2011 Florian Balmer". تسمح بالاستعمال والتعديل وإعادة التوزيع بشرط الإبقاء على إشعار حقوق النشر، وتمنع استعمال اسم المؤلف للترويج دون إذن.
- **أثر الرخصة على مشروعنا:** التحليل مسموح. ونلتزم بخطة `plan.md`: نكتب شيفرتنا بأنفسنا، فلا ننسخ شيفرة ولا موارد ولا نصوصًا، ولا نستعمل اسم Notepad2 أو اسم مؤلفه للترويج.

## نوع الملف
| الخاصية | القيمة | الأداة |
|---|---|---|
| الصيغة | PE32+، تطبيق رسومي (GUI) لويندوز | `inspect_pe_imports`، `file` |
| المعمارية | x86-64 (AMD64) | `inspect_pe_imports` |
| الحجم | 919,552 بايت | — |
| قاعدة الصورة | `0x140000000` | `rea inspect` (binary_overview) |
| الأقسام | `.text` (680,960 بايت)، `.data`، `.pdata`، `.rsrc` (107,520 بايت)، `.reloc` | `rea inspect` |
| معلومات الإصدار | FileDescription = `Notepad2 x64`، FileVersion = `4.2.25`، LegalCopyright = `© Florian Balmer 2004-2011` | موارد VERSIONINFO |
| مكوّن التحرير | Scintilla 2.24 مدمج في الملف، وليس عنصر `EDIT` القياسي | `search_strings`: `Scintilla` و `.?AVScintillaWin@@` |
| الرموز | الملف مجرّد من الرموز، فتظهر معظم الدوال بأسماء `FUN_<العنوان>` | `rea search --kind procedures` |

### الموارد
فحصتها بـ `inspect_pe_resources`. التغطية كاملة: 87 موردًا من 185 مدخلًا.

| النوع | العدد |
|---|---|
| حوارات (DIALOG) | 29 |
| جداول نصوص (STRINGTABLE) | 30 |
| أيقونات / مجموعات أيقونات | 11 / 3 |
| صور (BITMAP) | 6 |
| قوائم (MENU) | 2: الرئيسية بـ 164 عنصرًا، وقائمة أيقونة شريط المهام بـ 15 |
| جداول اختصارات (ACCELERATOR) | 2 |
| مؤشر، ومعلومات الإصدار، والبيان (manifest) | 1 لكل منها |

## المكتبات المستوردة من ويندوز
فحصتها بـ `inspect_pe_imports`. التغطية كاملة: 12 مكتبة و506 دوال مستوردة، ولا يوجد استيراد مؤجَّل (delay-load). بصمة الاستيراد: `imphash = 37dbcc3aa03d6ea9633e60bf6bdf58bb`.

| المكتبة | عدد الدوال | دورها في المحرر | أمثلة |
|---|---|---|---|
| `USER32.dll` | 173 | النوافذ والرسائل والقوائم والحوارات والحافظة ومفاتيح الاختصار | `CreateWindowExW`، `DialogBoxIndirectParamW`، `TranslateAcceleratorW`، `SetClipboardData` |
| `KERNEL32.dll` | 104 | الملفات والذاكرة والخيوط وتحويل الترميزات وملف الإعدادات INI | `CreateFileW`، `ReadFile`، `WriteFile`، `MultiByteToWideChar`، `GetPrivateProfileStringW` |
| `msvcrt.dll` | 58 | مكتبة C القياسية | `memcpy`، `qsort`، `sprintf` |
| `GDI32.dll` | 50 | الرسم والخطوط والطباعة | `CreateFontIndirectW`، `ExtTextOutW`، `StartDocW` |
| `SHLWAPI.dll` | 50 | معالجة المسارات والنصوص | `PathCanonicalizeW`، `PathFindExtensionW`، `StrStrIW` |
| `msvcp60.dll` | 23 | `std::string` من C++، ويستعمله Scintilla | `basic_string::find` |
| `SHELL32.dll` | 17 | السحب والإفلات وفتح البرامج والمجلدات الخاصة | `DragQueryFileW`، `ShellExecuteExW`، `SHAddToRecentDocs` |
| `ole32.dll` | 10 | OLE للسحب والإفلات وCOM | `OleInitialize`، `RegisterDragDrop`، `DoDragDrop` |
| `COMDLG32.dll` | 6 | الحوارات الجاهزة | `GetOpenFileNameW`، `GetSaveFileNameW`، `ChooseFontW`، `ChooseColorW`، `PrintDlgW`، `PageSetupDlgW` |
| `COMCTL32.dll` | 6 | شريط الأدوات وشريط الحالة | `InitCommonControlsEx`، `CreateStatusWindowW`، `ImageList_Create` |
| `IMM32.dll` | 6 | محرر طرق الإدخال (IME) | `ImmGetContext`، `ImmSetCompositionWindow` |
| `ADVAPI32.dll` | 3 | كشف ترميز Unicode | `IsTextUnicode`، `OpenProcessToken` |

> **ملاحظة مهمة لخطتنا:** الملف لا يستورد `FindTextW` ولا `ReplaceTextW`. أي أن البحث والاستبدال في Notepad2 حوارات خاصة به من موارده، تُعرض بـ `DialogBoxIndirectParamW`/`CreateDialogIndirectParamW`، وليست حوارات COMDLG32 الجاهزة. أما الفتح والحفظ والخط فتستعمل حوارات COMDLG32.

## عدد الدوال
عددتها بـ `rea search --kind procedures` مع Ghidra 12.1.4، وهي قائمة المصدر نفسه الذي يعتمده `list_procedures`.

| الفئة | العدد |
|---|---|
| **المجموع الذي يعرضه Ghidra** | **2,426** |
| دوال خارجية (مستوردة) | 497 |
| دوال داخلية | 1,929 |
| — منها بلا اسم (`FUN_…`) | 1,546 |
| — منها `thunk_FUN_…` | 15 |
| — منها جسور إلى مكتبات (`DLL::…`) | 26 |
| — منها بأسماء تعرّف عليها Ghidra | 342 |
| — — منها كتل فكّ استثناءات (`Unwind@…`) | 314 |

حدود هذا العدد:
- Ghidra يعدّ 497 دالة خارجية، بينما يُظهر جدول الاستيراد 506 مدخلًا، والفرق يحتاج تفسيرًا.
- العدد يمثّل ما اكتشفه التحليل الافتراضي فقط، ولا يثبت أن كل دوال الملف اكتُشفت.
- أحد الأسماء التي أطلقها Ghidra، `DnsFreeInterfaceSettings` عند `0x14005ca6c`، يبدو مطابقة خاطئة لمكتبة لا علاقة لها بالمحرر.

## أهم الدوال ووظائفها
الملف بلا رموز، فحدّدت الدوال بتتبّع المراجع:
1. عنوان الاستيراد في جدول IAT، أو عنوان نص مميز.
2. `rea xrefs` لإيجاد مواقع الاستدعاء.
3. `rea decompile` على موقع الاستدعاء لاستخراج الدالة الحاوية، ثم قراءة ما تستدعيه.

الأسماء في العمود الثاني وصفية من وضعي، وليست أسماء الأصل.

| العنوان | الوظيفة (وصفي) | الدليل |
|---|---|---|
| `FUN_140092eb8` | **WinMain**: نقطة البداية وحلقة الرسائل | تستدعي `OleInitialize` و`InitCommonControlsEx` و`LoadAcceleratorsW`، وتسجّل رسالة `"TaskbarCreated"`، وتحمّل `uxtheme.dll`، وتدير حلقة `TranslateAcceleratorW`/`IsDialogMessageW` |
| `FUN_140092e3c` | تسجيل صنف النافذة الرئيسية | `RegisterClassW` مع `LoadIconW` و`LoadCursorW` |
| `FUN_140087fd4` | **حوار فتح ملف** (أمر File ← Open) | `GetOpenFileNameW` بعد توحيد المسار بـ `ExpandEnvironmentStringsW` و`PathCanonicalizeW` و`PathIsRelativeW` |
| `FUN_140088204` | **حوار حفظ ملف** (Save As) | `GetSaveFileNameW` بنفس معالجة المسار |
| `FUN_14009380c` / `FUN_140093a14` | استيراد إعدادات الألوان والأنماط من ملف `ini` وتصديرها | `GetOpenFileNameW`/`GetSaveFileNameW` مع فلتر `ini`، و`WritePrivateProfileSectionW` |
| `FUN_14006e9f0` | حوار "تشغيل أمر" (Command / Open with) | `GetOpenFileNameW` لزر الاستعراض، و`ShellExecuteExW`، و`SHAutoComplete` |
| `FUN_140094544` | **اختيار الخط** (Default Font) | تحوي `CALL [0x140001068]` = `ChooseFontW` عند `0x1400946d0`. ثبت ذلك بـ `rea instructions`، لأن فك الترجمة فشل بخطأ "Overlapping input varnodes". تُستدعى من `FUN_140097c8c`، انظر `menus.md` |
| موقع الاستدعاء `0x140073f13` | **كشف ترميز الملف** (Unicode أم ANSI) | يستدعي `IsTextUnicode`. فك الترجمة فشل بخطأ `cleanup_incomplete` |
| مواقع الاستدعاء `0x140080205` و`0x14008023b` و`0x140080293` و`0x140080454` و`0x1400877b4` | **كتابة الملف على القرص** | خمس مواقع تستدعي `WriteFile` |
| مواقع الاستدعاء `0x140068dde` و`0x14006d751` و`0x14006d7cc` | تسجيل أصناف نوافذ Scintilla، أي عنصر التحرير ونافذة التلميح | ثلاث مواقع تستدعي `RegisterClassExW` |
| موقع الاستدعاء `0x14006e7d9` | **نافذة التعريف بالبرنامج** (About) | يشير إلى النص `"Notepad2 4.2.25"` |

## ما يعنيه هذا لأوامرنا الستة
| أمرنا | ما يستعمله Notepad2 | ما نستنتجه لتنفيذنا |
|---|---|---|
| الفتح | `GetOpenFileNameW` في `FUN_140087fd4` | نستعمل نفس الحوار الجاهز من COMDLG32 |
| الحفظ | `GetSaveFileNameW` في `FUN_140088204`، ثم `CreateFileW`/`WriteFile` | نفس النهج، والحفظ بترميز UTF-8 |
| كشف الترميز | `IsTextUnicode` وفحص BOM | نفحص BOM أولًا، ثم نجرّب UTF-8 |
| البحث والاستبدال | حوار خاص من الموارد، لا `FindTextW` | نستعمل `FindTextW`/`ReplaceTextW` الجاهزين لأنهما أبسط، أو نصمم حوارًا عربيًا خاصًا بنا |
| حجم الخط | `ChooseFontW` | نفس الحوار، ثم `WM_SETFONT` على عنصر `EDIT` |
| التفاف السطور | خاصية في Scintilla (`Ctrl+W` في القائمة) | مع عنصر `EDIT` نعيد إنشاءه بلا `WS_HSCROLL`/`ES_AUTOHSCROLL` |
| نافذة التعريف | حوار من الموارد | حوار خاص بنا بنص عربي |

بقية أوامر القائمة الرئيسية (164 عنصرًا) خارج النطاق، كما في `plan.md`. منها: الترميزات المتعددة، ونهايات الأسطر، والمفضلة، وأوامر الأسطر والكتل، والتحويلات، والتلوين النحوي، والتكبير، والطباعة، وشريط الأدوات.

## ما لم يُحسم بعد
- ~~الدالة التي تحوي استدعاء `ChooseFontW`~~: حُسمت، وهي `FUN_140094544`، انظر `menus.md`.
- الدالة التي تحوي استدعاء `IsTextUnicode` عند `0x140073f13`: فشل بخطأ `cleanup_incomplete`.
- أي استدعاء من استدعاءات `RegisterClassExW` يخص عنصر التحرير نفسه.
- إجراء حوار التعريف الذي يحوي `0x14006e7d9`: لم يُفكّ بعد.
- الفرق بين 497 دالة خارجية في Ghidra و506 مدخلًا في جدول الاستيراد.

## ملاحظات على البيئة
- تحليل Ghidra الكامل للملف يستغرق نحو 250 ثانية، وهذا يتجاوز مهلة أدوات MCP في هذه الجلسة (60 ثانية). لذلك:
  - أداتا `inspect_pe_imports` و`inspect_pe_resources` عملتا عبر MCP مباشرة، لأنهما لا تحتاجان Ghidra.
  - بقية الاستعلامات نُفّذت بواجهة سطر الأوامر: `rea inspect` و`rea search` و`rea xrefs` و`rea decompile`، كلها مع `--provider ghidra` و`GHIDRA_INSTALL_DIR=/opt/ghidra_12.1.4_PUBLIC`.
- خيار `--snapshot` لم يختصر وقت التحليل في الاستدعاءات اللاحقة.
- استخراج نصوص القائمة ومعلومات الإصدار من الموارد تم بقارئ Python صغير للقراءة فقط، لأن `inspect_pe_resources` يعطي مواقع الموارد وبصماتها دون فك محتواها.

## الفتح وكشف الترميز والحفظ والإغلاق في الأصل
تتبّعتها بأوامر `rea decompile` من الطرفية مع Ghidra. أرقام الأدلة:

| الدالة | الدور (وصفي) | دليل REA |
|---|---|---|
| `FUN_14008bc5c(bDontSave, bNew, bReload, …, path)` | أمرا New وOpen | `rea decompile` (سابقًا) |
| `FUN_14008b6cc(bSaveAlways, bAsk, bSaveAs, bSaveCopy)` | أوامر Save وSave As، وسؤال الحفظ | `rea decompile` (سابقًا) |
| `FUN_140087e38(bLoad, path, …, &encoding, …)` | واجهة القراءة والكتابة | `ev_edeed933…` |
| `FUN_14007f724` | قراءة الملف وكشف ترميزه | `ev_5c42e07d…` |
| `FUN_140073ecc` | كشف UTF-16 | `ev_3c43697b…` |
| `FUN_14007fff8` | كتابة الملف بترميزه | `ev_6abf6027…` |
| `FUN_140091fe0` | إجراء النافذة: `WM_CLOSE` و`WM_QUERYENDSESSION` | `rea decompile` (سابقًا) |

نصوص الرسائل قرأتها من جداول النصوص (`STRINGTABLE`): 30 كتلة حدّد REA مواقعها بـ `inspect_pe_resources`، وتحققت من بصمة كل كتلة.

### الترميزات التي وجدتها
في جدول النصوص 73 ترميزًا، بالأرقام 61000–61072. لكل ترميز نص بالشكل `فئة;الاسم الكامل;الاسم المختصر`:

- **الفئة A — ترميزات أساسية (9):**
  - ANSI (صفحة رموز النظام).
  - OEM.
  - Unicode (UTF-16 LE BOM)، وUnicode (UTF-16 BE BOM).
  - Unicode (UTF-16 LE)، وUnicode (UTF-16 BE).
  - UTF-8، وUTF-8 Signature (مع BOM).
  - UTF-7.
- **الفئة C — صفحات رموز حسب اللغة (56)، منها 4 عربية:**
  - Arabic (DOS-720).
  - Arabic (ISO-8859-6).
  - Arabic (Mac).
  - **Arabic (Windows-1256)**.
- **بقية الفئة C:** البلطيقية، ووسط أوروبا، والصينية (GBK، وBig5، وGB18030)، والسيريلية، واليونانية، والعبرية، واليابانية (Shift-JIS)، والكورية، والتايلندية، والتركية، والفيتنامية.
- **الفئة B — أوروبا الغربية (4):** DOS-850، وISO-8859-1، وMac، وWindows-1252.
- **الفئة D — EBCDIC من IBM (4).**

في الشيفرة، جدول الترميزات عند `0x1400a88b0`، وكل مدخل فيه 0x58 بايت: أعلام ثم صفحة رموز. الأعلام كما تستعملها دالتا القراءة والكتابة:

| العلَم | المعنى |
|---|---|
| `0x08` | UTF-16 |
| `0x10` | Big Endian |
| `0x20` | يُكتب BOM لـ UTF-16 |
| `0x02` | UTF-8 |
| `0x04` | يُكتب BOM لـ UTF-8 |
| `0x40` | تحويل عبر صفحة رموز |
| `0x01` | ANSI |

### كيف يفتح الأصل ملفًا (`FUN_14007f724`)
1. يفتح الملف للقراءة مع السماح للآخرين بالقراءة والكتابة. إن تجاوز حجمه الإعداد `FileLoadWarningMB` (افتراضيًا 1 MB) يسأل المستخدم قبل المتابعة.
2. يقرأ الملف كله إلى الذاكرة دفعة واحدة.
3. **الملف الفارغ:** يأخذ الترميز الافتراضي من الإعداد `DefaultEncoding`، وهو فارغ في `Notepad2.ini` المرفق، أي ANSI.
4. **كشف UTF-16** بالدالة `FUN_140073ecc`:
   - تفحص علامة `FF FE` (LE) أو `FE FF` (BE).
   - وبدون علامة تستعمل `IsTextUnicode` الإحصائية، إلا إذا كان الإعداد `SkipUnicodeDetection` مفعّلًا.
   - إن كان BE يقلب ترتيب البايتات بـ `_swab`.
   - ثم يحوّل النص بـ `WideCharToMultiByte` إلى UTF-8، ويضبط Scintilla على UTF-8 (`SCI_SETCODEPAGE 65001`).
   - إن فشل التحويل يعرض: "Error converting this Unicode file. Data will be lost if the file is saved!".
5. **وسوم الترميز داخل النص:** يبحث عنها مثل `coding:` و`encoding=`، ما لم يكن `NoEncodingTags` مفعّلًا.
6. **كشف UTF-8:**
   - علامة `EF BB BF` تعني "UTF-8 Signature".
   - إن كانت البايتات UTF-8 صالحة وفيها حرف غير ASCII (تفحصها `FUN_140073fd8`) فهي UTF-8.
   - النص ASCII الخالص يُعامَل ANSI، إلا إذا كان `LoadASCIIasUTF8` مفعّلًا.
7. **غير ذلك:** يأخذ الترميز الافتراضي، وهو ANSI.
   - **ANSI:** يضع Scintilla على صفحة رموز النظام (لا UTF-8) ويعرض البايتات كما هي دون تحويل.
   - **صفحة رموز أخرى** (مثل 1256 إذا اختارها المستخدم): يحوّل بـ `MultiByteToWideChar(cp)` ثم `WideCharToMultiByte(UTF-8)`.
8. أخيرًا يكتشف نوع نهاية السطر من النص (`FUN_1400735d8`).

**نتيجة مهمة:** ملف Windows-1256 بلا وسم، على نظام صفحة رموزه 1252، يعامله الأصل ANSI بصفحة 1252، فيظهر رموزًا غير مفهومة. يجب أن يختار المستخدم الترميز يدويًا بأمر Recode (F8).

### كيف يحفظ الأصل (`FUN_14007fff8`)
- **فتح الملف:** يفتح الملف الهدف نفسه بـ `OPEN_ALWAYS`. إن فشل يعيد المحاولة مع خاصيتي الإخفاء والنظام.
- **التصحيح قبل الكتابة:** اختياري حسب الإعدادات، مثل إزالة المسافات في نهايات الأسطر.
- **الكتابة في المكان نفسه:** ينفّذ `SetEndOfFile` عند الموضع 0 ثم `WriteFile`، دون ملف مؤقت. إن فشلت الكتابة في منتصفها قد يضيع المحتوى القديم.
- **UTF-16:** يحوّل UTF-8 إلى UTF-16، ويكتب BOM إن كان الترميز يتطلبه، ويقلب البايتات لـ BE.
- **UTF-8:** يكتب البايتات كما هي، ويسبقها `EF BB BF` إن كان الترميز "Signature".
- **صفحة رموز:** يحوّل بـ `WC_NO_BEST_FIT_CHARS`. إن وُجدت حروف لا يمثلها الترميز يعرض الرسالة 50016 ("Certain characters … may be replaced …")، وإلغاؤها يلغي الحفظ.
- **ANSI:** يكتب البايتات كما هي، لأن المحرر كان على صفحة رموز النظام.
- **بعد النجاح:** يرسل `SCI_SETSAVEPOINT` إلا في Save Copy.

### الأوامر والإغلاق في الأصل
- **New:** يمر أولًا بـ `FileSave(0,1,0,0)` الذي يسأل عن الحفظ. ثم يمسح النص ويضع الترميز الافتراضي (ANSI ما لم يُغيَّر الإعداد)، والعنوان "Untitled".
- **Open:** يسأل عن الحفظ بالطريقة نفسها، ثم يعرض `GetOpenFileNameW`. إن كُتب اسم ملف غير موجود يسأل: "\"%s\" not found. Would you like to create this file?".
- **Save:** إن لم يكن تغيير ولم يكن الأمر Save As، يرجع دون كتابة. وإن كان للملف اسم يكتبه بترميزه الحالي. وإن كان الملف للقراءة فقط يسأل: "\"%s\" is read only. Save to a different file?". وإن لم يكن له اسم يتصرف كأمر Save As.
- **Save As:** `GetSaveFileNameW`، والإلغاء لا يغيّر شيئًا. بعد النجاح يحدّث الاسم والعنوان وقائمة الملفات الحديثة.
- **سؤال الحفظ:** `MessageBox` بأزرار Yes/No/Cancel والنص "Save changes to \"%s\"?":
  - **Cancel:** يلغي العملية.
  - **No:** يتابع دون حفظ.
  - **Yes:** يحفظ، وإن أُلغي الحفظ تُلغى العملية.
  - المستند الجديد الفارغ، أو الذي فيه مسافات فقط، لا يُسأل عنه.
- **الإغلاق:** في `WM_CLOSE` يستدعي `FileSave(0,1,0,0)`، ولا يستدعي `DestroyWindow` إلا إن رجع بنجاح. ويفعل الشيء نفسه في `WM_QUERYENDSESSION` عند إيقاف ويندوز.

### الأصل بجانب ما بنيناه
كتبتُ تنفيذنا بنفسي، في `src/main.c` و`src/encoding.c`، بناءً على السلوك الموصوف أعلاه، لا بنسخ ما فُكّ.

| الأمر | Notepad2 4.2.25 | Arabic Editor |
|---|---|---|
| **New** (Ctrl+N) | يسأل عن الحفظ إن وُجد تغيير. الترميز الافتراضي ANSI، والعنوان "Untitled". | يسأل عن الحفظ إن وُجد تغيير (نعم/لا/إلغاء). **الترميز UTF-8 بلا BOM**، والعنوان "بلا عنوان [UTF-8]". |
| **Open** (Ctrl+O) | يسأل عن الحفظ ثم يعرض `GetOpenFileNameW`. يكشف UTF-16 وUTF-8 وBOM ووسوم الترميز. غير ذلك ANSI بصفحة النظام، وScintilla على صفحة النظام. يعرض الملف غير الموجود للإنشاء. | يسأل عن الحفظ ثم يعرض `GetOpenFileNameW` مع `OFN_FILEMUSTEXIST`. يكشف BOM لـ UTF-8 وUTF-16 LE/BE، ثم UTF-8 صالح (ومنه ASCII)، ثم UTF-16 بلا BOM بـ `IsTextUnicode`. **غير ذلك Windows-1256.** **Scintilla دائمًا على `SC_CP_UTF8`**، والتحويل عند الفتح. الترميز يظهر في العنوان. |
| **Save** (Ctrl+S) | يكتب بالترميز الحالي في الملف نفسه (`SetEndOfFile` ثم `WriteFile`). يحذّر عند ضياع حروف في صفحة رموز. يسأل عن الملف المقروء فقط. | يحوّل من UTF-8 إلى ترميز الملف، ويكتب إلى ملف مؤقت `.~tmp` ثم يستبدل به الأصل بـ `MoveFileExW`، فلا يتلف الملف إن فشلت الكتابة. إن وُجدت حروف لا يمثلها الترميز يسأل: نعم = يحفظ UTF-8، لا = يحفظ مع الاستبدال، إلغاء = لا يحفظ. بلا اسم = Save As. |
| **Save As** (الأصل F6، ونحن Ctrl+Shift+S) | `GetSaveFileNameW`، والإلغاء لا يغيّر شيئًا. | `GetSaveFileNameW` مع `OFN_OVERWRITEPROMPT` والامتداد الافتراضي `txt`، والاسم الحالي معبّأ مسبقًا. الإلغاء لا يغيّر شيئًا. |
| **الإغلاق** | `WM_CLOSE` يسأل "Save changes to …?" بنعم/لا/إلغاء، والإلغاء يُبقي النافذة. | السلوك نفسه بنص عربي: "هل تريد حفظ التغييرات في "…"؟"، مع `MB_RTLREADING`. ويطبَّق أيضًا على `WM_QUERYENDSESSION`. |
| **مؤشر التغيير** | يُظهر حالة التغيير في العنوان. | `*` في أول العنوان، يتحدّث من إشعارَي Scintilla `SCN_SAVEPOINTLEFT/REACHED`. |
| **نافذة الفتح** | `GetOpenFileNameW` من COMDLG32. | اخترتُ `GetOpenFileNameW` و`GetSaveFileNameW` من COMDLG32 أيضًا: بسيطة، لا تحتاج COM، وتعمل في Wine. |

**الاختلافات المقصودة:**
- **UTF-8 افتراضي:** الملف الجديد UTF-8 لا ANSI، ومساحة الكتابة دائمًا UTF-8.
- **Windows-1256 للملفات القديمة:** الملف الذي ليس Unicode يُقرأ 1256، لا صفحة رموز النظام، لأن المحرر عربي.
- **حفظ آمن:** الحفظ عبر ملف مؤقت.
- **لم ننفذ:** أمر Recode لاختيار الترميز يدويًا، ووسوم الترميز، و`.LOG`.

### نتيجة التجربة في Wine
اللقطات في `docs/screenshots/file-commands/`. ملفات الاختبار في `build/testfiles/`، وهي غير ملتزمة.

| الخطوة | ما ظهر | الملف على القرص |
|---|---|---|
| New من القائمة | "بلا عنوان [UTF-8]" | — |
| كتابة "مرحبا" | النص بحروف متصلة، والعنوان صار "*بلا عنوان [UTF-8]" | — |
| Save As ← `notes.txt` | نافذة "Save As" بفلتر "ملفات نصية (*.txt)"، ثم العنوان "notes.txt [UTF-8]" | `d9 85 d8 b1 d8 ad d8 a8 d8 a7` (UTF-8 بلا BOM، 10 بايت) |
| Open ← `notes.txt` | "مرحبا"، والعنوان "notes.txt [UTF-8]" | — |
| فتح Open ثم Cancel | ظهرت النافذة وأُغلقت، ولم يتغير شيء | — |
| فتح Save As ثم Cancel | ظهرت النافذة والاسم `notes.txt` معبّأ، وأُغلقت دون حفظ | لم يُنشأ ملف |
| Open ← `old.txt` (1256: `d3 e1 c7 e3`) | "سلام"، والعنوان "old.txt [Windows-1256]" | — |
| تعديل ← "سلام عليكم" ثم Alt+F4 | سؤال "هل تريد حفظ التغييرات في "old.txt"؟" بأزرار Yes/No/Cancel | — |
| اختيار Cancel | بقيت النافذة، والعنوان "*old.txt [Windows-1256]" | لم يتغير |
| Save (Ctrl+S) | العنوان "old.txt [Windows-1256]" دون `*` | `d3 e1 c7 e3 20 da e1 ed df e3` = "سلام عليكم" بترميز 1256 |
| Alt+F4 بعد الحفظ | أُغلق البرنامج دون سؤال | — |
