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
| موقع الاستدعاء `0x1400946d0` | **اختيار الخط** (Default Font) | يستدعي `ChooseFontW`. فك الترجمة فشل بخطأ Ghidra "Overlapping input varnodes"، فالدالة الحاوية غير محسومة |
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
- الدالة التي تحوي استدعاء `ChooseFontW` عند `0x1400946d0`: فشل فك الترجمة. يمكن المحاولة بـ `procedure_assembly` أو `read_function_instructions` بدل فك الترجمة.
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
