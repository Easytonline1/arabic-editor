# مراجعة استنتاجات `analysis.md` و`menus.md` على شيفرة Notepad2 4.2.25

## الطريقة
- **الشيفرة:** `source/`، من `https://flos-freeware.ch/zip/notepad2_4.2.25_src.zip` (SHA-256 `633b208c…bb019`). وهي الحزمة نفسها التي أُخذت منها `License.txt` سابقًا.
- **ما رُوجع:** كل استنتاج عن **البرنامج الأصلي** في `analysis.md` و`menus.md`. لم أراجع ما يخص نسختنا وتجارب Wine، ولا ملاحظات البيئة.
- **التوزيع:**
  - راجعت أربعة وكلاء أقسامَ الملفين على الشيفرة بالتوازي:
    - A: `analysis.md` 1–123.
    - B: 124–249.
    - C: 250–353.
    - D: 354–518 و`menus.md`.
  - ثم تحققت بنفسي في الشيفرة من كل استنتاج صُنّف خاطئًا، ومن جوابَي البندين غير المحسومين في `comparison.md`.
- **التصنيف:**
  - **مؤكد:** الشيفرة تُظهر السلوك نفسه.
  - **خاطئ:** الشيفرة تُظهر غيره. إن كان الخطأ في جزء من الاستنتاج فقط، يُكتب "خاطئ جزئيًا".
  - **غير محسوم:** الشيفرة لا تحسمه، مثل حقائق الملف الثنائي (الأحجام والعناوين وأعداد Ghidra) أو ما يقع داخل Scintilla الغائبة عن الحزمة.
- **المواضع:** أرقام الأسطر في عمود "الشيفرة" نسبية إلى `source/src/` ما لم يُذكر غير ذلك.
- **ربط عناوين Ghidra بالدوال:** الملف بلا رموز، فهذا الربط ناتج عن مطابقة السلوك بين المفكوك والشيفرة:

| Ghidra | الدالة في الشيفرة | Ghidra | الدالة في الشيفرة |
|---|---|---|---|
| `FUN_140092eb8` | `WinMain` (Notepad2.c:307) | `FUN_14007f724` | `EditLoadFile` (Edit.c:1164) |
| `FUN_140092e3c` | `InitApplication` (Notepad2.c:450) | `FUN_140073ecc` | `IsUnicode` (Edit.c:920) |
| `FUN_140091fe0` | `MainWndProc` (Notepad2.c:766) | `FUN_14007fff8` | `EditSaveFile` (Edit.c:1404) |
| `FUN_14008cf60` | `MsgCommand` (Notepad2.c:1997) | `FUN_14007f174` | `EditFindReplaceDlg` (Edit.c:5208) |
| `FUN_14008bc5c` | `FileLoad` (Notepad2.c:6208) | `FUN_14007e168` | `EditFindReplaceDlgProcW` (Edit.c:4786) |
| `FUN_14008b6cc` | `FileSave` (Notepad2.c:6375) | `FUN_14007ae84` | `EditFindNext` |
| `FUN_140087e38` | `FileIO` (Notepad2.c:6169) | `FUN_14007b2c0` | `EditReplace` |
| `FUN_140087fd4` | `OpenFileDlg` (Notepad2.c:6513) | `FUN_140082c50` | `CreateThemedDialogParam` (Helpers.c:2023) |
| `FUN_140088204` | `SaveFileDlg` (Notepad2.c:6569) | `FUN_140082bdc` | `ThemedDialogBoxParam` |
| `FUN_14009380c` / `FUN_140093a14` | `Style_Import` / `Style_Export` (Styles.c:1159/1210) | `FUN_14006e794` | `AboutDlgProc` (Dialogs.c:205) |
| `FUN_14006e9f0` | `RunDlgProc` (Dialogs.c:292) | `FUN_140097c8c` | `Style_SetDefaultFont` (Styles.c:1914) |
| `FUN_140094544` | `Style_SelectFont` (Styles.c:2211) | `FUN_140087420` | `FindIniFile` (Notepad2.c:5876) |
| `FUN_140087224` | `CheckIniFile` (Notepad2.c:5814) | `FUN_140087354` | `CheckIniFileRedirect` (Notepad2.c:5851) |
| `FUN_1400875a8` | `TestIniFile` (Notepad2.c:5925) | `FUN_140087700` | `CreateIniFileEx` (Notepad2.c:5963) |
| `FUN_140085…` | `LoadSettings` (Notepad2.c:4914) | `FUN_14008a898` | `SaveSettings` (Notepad2.c:5219) |
| `FUN_140083db8` | `CreateBars` (Notepad2.c:1460) | `FUN_1400879f4` | `UpdateStatusbar` (Notepad2.c:6043) |

## الخلاصة
| | مؤكد | خاطئ أو خاطئ جزئيًا | غير محسوم |
|---|---|---|---|
| A: `analysis.md` 1–123 (48) | 33 | 4 | 11 |
| B: `analysis.md` 124–249 (58) | 54 | 3 | 1 |
| C: `analysis.md` 250–353 (24) | 23 | 0 | 1 |
| D: `analysis.md` 354–518 و`menus.md` (47) | 40 | 4 | 3 |
| **المجموع (177)** | **150** | **11** | **16** |

**الاستنتاجات الخاطئة وأثرها:**

| # | الاستنتاج الخاطئ | ما في الشيفرة | هل بُني عليه شيء في `editor.exe`؟ |
|---|---|---|---|
| A15 = D27 | `MENU 101` "قائمة أيقونة شريط المهام" | `IDR_POPUPMENU` (Notepad2.rc:331-361) حاوية لثلاث قوائم: سياق الكتابة (7)، وسياق الأشرطة (3)، والأيقونة (2). تُفتح من Notepad2.c:1086 و1095 و1223 | لا |
| D28 | أوامر تلك القائمة لا تُربط بجدول الاختصارات | ذلك صحيح لـ 40600/40601 فقط. Undo وCut وغيرها لها اختصارات (Notepad2.rc:364-564) | لا |
| A31 | دور ADVAPI32 كشف الترميز | `IsTextUnicode` فقط للترميز (Edit.c:933). `OpenProcessToken` و`GetTokenInformation` في `IsElevated()` (Helpers.c:175-182) | لا |
| A40 | `FUN_14006e9f0` حوار "Command / Open with" | هي `RunDlgProc` (Command، Ctrl+R) فقط. Open with حوار آخر هو `OpenWithDlg` (Dialogs.c:618) | لا |
| A43 | مواقع `WriteFile` الخمسة كلها حفظ للمستند | الحفظ في `EditSaveFile` (Edit.c:1509-1579). وفي `CreateIniFileEx` (Notepad2.c:5983) `WriteFile` لترويسة ملف الإعدادات | لا |
| B11 | قائمة "بقية الفئة C" من الترميزات | تُغفل نحو 12 ترميزًا: Croatian Mac، Estonian، French Canadian، Icelandic، Latin 3/9، Nordic، DOS-437، DOS-858، Portuguese، Romanian Mac، Ukrainian Mac (Notepad2.rc:1551-1617) | لا |
| B50 | اسم ملف غير موجود يُكتب في نافذة الفتح يعرض "create this file?" | نافذة الفتح فيها `OFN_FILEMUSTEXIST` (Notepad2.c:6549) فترفضه. السؤال (6280-6285) يظهر فقط لمسار من خارجها: سطر الأوامر، والقائمة الحديثة، والسحب | لا. نافذتنا فيها `OFN_FILEMUSTEXIST` أيضًا، كالأصل الحقيقي |
| B51 | Save يرجع دون كتابة إن لم يتغير شيء | الأمر يستدعي `FileSave(TRUE,…)` (Notepad2.c:2052)، والرجوع المبكر يشترط `!bSaveAlways` (6395). Save يكتب دائمًا | لا. Save عندنا يكتب دائمًا، كالأصل الحقيقي |
| D4 | نافذة البحث تُنشأ بـ `CreateDialogParam`، ويُبرَز المفتوح | تُنشأ بـ `CreateDialogIndirectParam` بقالب مُحمَّل (Helpers.c:2033-2034). وإن كانت مفتوحة بالوضع الآخر تُغلق وتُنشأ بالوضع المطلوب (Notepad2.c:3242-3246) | لا. نافذتنا تبدّل الوضع في مكانها، والنتيجة الظاهرة واحدة |
| D33 | Save يفتح "حفظ باسم" فقط إن كان الملف بلا اسم | يفتحها أيضًا لملف للقراءة فقط إن وافق المستخدم على `IDS_READONLY_SAVE` (Notepad2.c:6416-6433) | لا |

**الحكم على `editor.exe`:** لا ميزة فيه مبنية على استنتاج خاطئ، فلا إصلاح مطلوب بهذا الشرط.
- **الاختلاف العرَضي:** الاختلاف الذي وجدته `comparison.md`، أي سؤال الحفظ لمستند جديد كله مسافات، مبني على استنتاج **مؤكد** (B55). نسختنا خالفته. لم أغيّره لأن الطلب يقتصر على المبني على خطأ.
- **الاختلافات المقصودة:** المكتوبة في `analysis.md` لم تُمَس.

**بندان غير محسومين في `comparison.md` حسمتهما الشيفرة:**
- **Save As:** يسأل قبل الكتابة فوق ملف موجود. الأعلام `OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT | OFN_DONTADDTORECENT | OFN_PATHMUSTEXIST` (Notepad2.c:6605-6607)، فالحكم **متوافق تحليليًا** مع نسختنا.
- **علامة ✓ لـ Word Wrap:** موجودة في الأصل، وتُحسب في `MsgInitMenu` عند `WM_INITMENU`: `CheckCmd(hmenu,IDM_VIEW_WORDWRAP,fWordWrap)` (Notepad2.c:1921، ويُستدعى من 1107-1108). الحكم **متوافق تحليليًا**.

## الجدول الكامل

### A — `analysis.md` الأسطر 1–123 (نوع الملف، الموارد، الاستيرادات، أهم الدوال)
| id | الاستنتاج | السطر | الموضع في الشيفرة | التصنيف | ملاحظة |
|---|---|---|---|---|---|
| A1 | الرخصة BSD ثلاثية البنود، © 2004-2011 Florian Balmer | 8 | `source/License.txt:1-17` | مؤكد | |
| A2 | رابط الأرشيف وبصماته ومحتواه | 4-7 | — | غير محسوم | خاص بالملفات الموزعة |
| A3 | PE32+ تطبيق رسومي | 14 | `source/wdkbuild/Notepad2.mak:17` `/SUBSYSTEM:WINDOWS`؛ Notepad2.c:307 `WinMain` | مؤكد | |
| A4 | x86-64 | 15 | version.h:27-28 `_M_AMD64` ← "Notepad2 x64" | مؤكد | |
| A5 | الحجم وقاعدة الصورة والأقسام | 16-18 | — | غير محسوم | خاص بالثنائي |
| A6 | FileDescription "Notepad2 x64"، الإصدار 4.2.25، حقوق النشر | 19 | version.h:22-28؛ Notepad2.ver | مؤكد | |
| A7 | Scintilla 2.24 مدمج، لا عنصر EDIT | 20 | `source/Readme.txt:26`؛ Notepad2.c:402 `Scintilla_RegisterClasses` | مؤكد | |
| A8 | الملف مجرّد من الرموز | 21 | — | غير محسوم | خاص بالثنائي |
| A9 | 87 موردًا | 24 | Notepad2.rc (29+30+11+3+6+2+2+1+1+2) | مؤكد | مجموع الجدول 86، لأن المؤشر موردان |
| A10 | 29 حوارًا | 28 | Notepad2.rc:586-1148 (29 `DIALOGEX`) | مؤكد | |
| A11 | 30 جدول نصوص | 29 | Notepad2.rc:1410-1856 | مؤكد | |
| A12 | 11 أيقونة و3 مجموعات | 30 | Notepad2.rc:37-39 | مؤكد | |
| A13 | 6 صور | 31 | Notepad2.rc:46-51 | مؤكد | |
| A14 | القائمة الرئيسية 164 عنصرًا | 32, 108 | Notepad2.rc:65-329 (148 أمرًا + 16 قائمة) | مؤكد | |
| A15 | القائمة الثانية "قائمة أيقونة شريط المهام" بـ 15 عنصرًا | 32 | Notepad2.rc:331-361؛ Notepad2.c:1086, 1095, 1223 | **خاطئ جزئيًا** | العدد صحيح، والوصف خاطئ: ثلاث قوائم منبثقة |
| A16 | جدولا اختصارات | 33 | Notepad2.rc:364 و566 | مؤكد | |
| A17 | مؤشر ومعلومات إصدار وبيان | 34 | Notepad2.rc:58, 28؛ Notepad2.ver | مؤكد | |
| A18 | 12 مكتبة و506 دوال، بلا delay-load، وimphash | 37 | Notepad2.mak:35-36 | غير محسوم | الأعداد خاصة بالثنائي، والمكتبات المربوطة متسقة |
| A19 | عدد الدوال لكل مكتبة | 41-52 | — | غير محسوم | خاص بالثنائي |
| A20 | أمثلة USER32 | 41 | Notepad2.c:566، Helpers.c:2016، Notepad2.c:419، Edit.c:588 | مؤكد | |
| A21 | أمثلة KERNEL32 | 42 | Edit.c:1192, 1224, 1517, 352؛ Notepad2.c:5854 | مؤكد | |
| A22 | msvcrt مكتبة C | 43 | Edit.c:758 `qsort`؛ Notepad2.mak `/MD` | مؤكد | |
| A23 | GDI32 للرسم والخطوط والطباعة | 44 | Dialogs.c:228، Print.cpp:361, 264 | مؤكد | |
| A24 | أمثلة SHLWAPI | 45 | Helpers.c:1460, 1061؛ Styles.c:1364 | مؤكد | |
| A25 | msvcp60 يستعمله Scintilla | 46 | لا `std::` في `src/` | غير محسوم | مصدر Scintilla غائب |
| A26 | SHELL32: السحب والتشغيل والقائمة الحديثة | 47 | Notepad2.c:1434, 922, 2143, 6331 | مؤكد | |
| A27 | ole32 للسحب وCOM | 48 | Notepad2.c:392؛ Helpers.c:1087 | مؤكد | `RegisterDragDrop`/`DoDragDrop` من Scintilla |
| A28 | COMDLG32 ست دوال | 49 | Notepad2.c:6554, 6610؛ Styles.c:2245, 2335؛ Print.cpp:146, 562 | مؤكد | |
| A29 | COMCTL32 للأشرطة | 50 | Notepad2.c:396, 1600؛ Dialogs.c:1871 | مؤكد | |
| A30 | IMM32 لـ IME | 51 | لا `Imm*` في `src/` | غير محسوم | من Scintilla |
| A31 | ADVAPI32 = كشف ترميز Unicode | 52 | Edit.c:933؛ Helpers.c:175-182 `IsElevated` | **خاطئ جزئيًا** | دالتان منها لفحص صلاحيات المسؤول |
| A32 | لا `FindTextW`/`ReplaceTextW`؛ حوارات بحث خاصة | 54, 103 | Edit.c:5208-5219؛ Helpers.c:2034 | مؤكد | |
| A33 | أعداد Ghidra والفرق 497/506 | 59-71, 115 | — | غير محسوم | خاص بالثنائي |
| A34 | `DnsFreeInterfaceSettings` مطابقة خاطئة | 73 | لا DNS في `src/` | غير محسوم | الشيفرة تؤيد الشك |
| A35 | `FUN_140092eb8` = WinMain | 85 | Notepad2.c:307-443 | مؤكد | |
| A36 | `FUN_140092e3c` = تسجيل الصنف | 86 | Notepad2.c:450-468 `InitApplication` | مؤكد | |
| A37 | `FUN_140087fd4` = حوار الفتح | 87, 100 | Notepad2.c:6513-6560 `OpenFileDlg` | مؤكد | توحيد المسار يخص المجلد الابتدائي |
| A38 | `FUN_140088204` = حوار Save As | 88, 101 | Notepad2.c:6569-6616 `SaveFileDlg` | مؤكد | |
| A39 | استيراد الأنماط وتصديرها | 89 | Styles.c:1159, 1210 | مؤكد | |
| A40 | `FUN_14006e9f0` = Command / Open with | 90 | Dialogs.c:292 `RunDlgProc`؛ Dialogs.c:618 `OpenWithDlg` | **خاطئ جزئيًا** | Command فقط |
| A41 | `FUN_140094544` = اختيار الخط، تستدعيها `FUN_140097c8c` | 91, 104, 111 | Styles.c:2211 `Style_SelectFont` (`ChooseFont` :2245)؛ Styles.c:1914 | مؤكد | الدالة عامة لاختيار الخط |
| A42 | `0x140073f13` (`IsTextUnicode`) = كشف الترميز | 92, 102, 112 | Edit.c:920-950 `IsUnicode` | مؤكد | يحسم السطر 112 |
| A43 | مواقع `WriteFile` الخمسة = حفظ المستند | 93 | Edit.c:1509-1579؛ Notepad2.c:5983 | **خاطئ جزئيًا** | أحدها على الأرجح ترويسة ملف الإعدادات |
| A44 | ثلاثة `RegisterClassExW` من Scintilla | 94, 113 | Notepad2.c:466 `RegisterClass` فقط | غير محسوم | مصدر Scintilla غائب |
| A45 | `0x14006e7d9` = نافذة About | 95, 106, 114 | Dialogs.c:205-217 `AboutDlgProc` | مؤكد | يحسم السطر 114 |
| A46 | الحفظ: `GetSaveFileNameW` ثم `CreateFileW`/`WriteFile` | 101 | Notepad2.c:6444؛ Edit.c:1404-1579 | مؤكد | |
| A47 | التفاف السطور خاصية Scintilla (Ctrl+W) | 105 | Notepad2.rc:257, 547؛ Notepad2.c:3372-3377 | مؤكد | |
| A48 | فئات الأوامر خارج النطاق | 108 | Notepad2.rc:65-329 | مؤكد | |

### B — `analysis.md` الأسطر 124–249 (الفتح والترميز والحفظ والإغلاق)
| id | الاستنتاج | السطر | الموضع في الشيفرة | التصنيف | ملاحظة |
|---|---|---|---|---|---|
| B1 | `FUN_14008bc5c` = New/Open | 129 | Notepad2.c:6208 `FileLoad` | مؤكد | يُستعمل أيضًا لـ Revert |
| B2 | `FUN_14008b6cc` = Save / Save As / سؤال الحفظ | 130 | Notepad2.c:6375 `FileSave` | مؤكد | |
| B3 | `FUN_140087e38` واجهة القراءة والكتابة | 131 | Notepad2.c:6169 `FileIO` | مؤكد | |
| B4 | `FUN_14007f724` قراءة وكشف ترميز | 132 | Edit.c:1164 `EditLoadFile` | مؤكد | |
| B5 | `FUN_140073ecc` كشف UTF-16 | 133 | Edit.c:920 `IsUnicode` | مؤكد | |
| B6 | `FUN_14007fff8` كتابة بالترميز | 134 | Edit.c:1404 `EditSaveFile` | مؤكد | |
| B7 | `FUN_140091fe0` إجراء النافذة | 135 | Notepad2.c:861, 867 | مؤكد | |
| B8 | 30 كتلة جداول نصوص | 137 | — | غير محسوم | تخطيط الموارد المترجمة |
| B9 | 73 ترميزًا بالأرقام 61000–61072 | 140 | Notepad2.rc:1538-1630؛ Edit.c:75-189 | مؤكد | |
| B10 | الفئة A تسعة | 142-148 | Notepad2.rc:1538-1550 | مؤكد | |
| B11 | الفئة C: 56، منها 4 عربية، و"بقيتها" المذكورة | 149-154 | Notepad2.rc:1551-1617, 1630 | **خاطئ جزئيًا** | العدد والعربية صحيحان، والقائمة تُغفل نحو 12 |
| B12 | الفئة B أربعة | 155 | Notepad2.rc:1618-1621 | مؤكد | |
| B13 | الفئة D أربعة EBCDIC | 156 | Notepad2.rc:1622-1625 | مؤكد | |
| B14 | الجدول عند `0x1400a88b0` | 158 | Edit.c:73 `mEncoding[]` | غير محسوم | العنوان خاص بالثنائي |
| B15 | المدخل 0x58 بايت: أعلام ثم صفحة رموز | 158 | Edit.h:155-161 | مؤكد | |
| B16–B22 | الأعلام 0x08 و0x10 و0x20 و0x02 و0x04 و0x40 و0x01 | 162-168 | Edit.h:132-138 (`NCP_UNICODE` … `NCP_DEFAULT`) | مؤكد | الجدول يُغفل 0x80 `NCP_RECODE` |
| B23 | الفتح بمشاركة القراءة والكتابة | 171 | Edit.c:1192-1198 | مؤكد | |
| B24 | سؤال إن تجاوز `FileLoadWarningMB` (1 MB) | 171 | Edit.c:1212-1214 | مؤكد | الوحدة MiB، و0 يعطّله، والنافذة InfoBox خاصة |
| B25 | قراءة الملف كله | 172 | Edit.c:1223-1224 | مؤكد | |
| B26 | الملف الفارغ يأخذ `DefaultEncoding`، أي ANSI | 173 | Edit.c:1244-1252؛ Notepad2.c:5029-5031 | مؤكد | يُغفل `LoadASCIIasUTF8` و`/e` |
| B27 | فحص `FF FE`/`FE FF` | 175 | Edit.c:937-938 | مؤكد | |
| B28 | `IsTextUnicode` إلا مع `SkipUnicodeDetection` | 176 | Edit.c:932-948 | مؤكد | |
| B29 | `_swab` لـ BE | 177 | Edit.c:1274-1275 | مؤكد | |
| B30 | التحويل إلى UTF-8، و`SCI_SETCODEPAGE` 65001 | 178 | Edit.c:1289, 1299 | مؤكد | |
| B31 | رسالة "Error converting this Unicode file" | 179 | Edit.c:1292-1295؛ Notepad2.c:6359-6360 | مؤكد | تعرضها `FileLoad` |
| B32 | وسوم الترميز ما لم يكن `NoEncodingTags` | 180 | Edit.c:6526-6588 | مؤكد | |
| B33 | `EF BB BF` = UTF-8 Signature | 182 | Edit.c:1322-1324 | مؤكد | |
| B34 | UTF-8 صالح فيه حرف غير ASCII = UTF-8 | 183 | Edit.c:1313-1315 | مؤكد | العنوان `FUN_140073fd8` على الأرجح `IsUTF8` |
| B35 | ASCII الخالص = ANSI إلا مع `LoadASCIIasUTF8` | 184 | Edit.c:1316-1317 | مؤكد | يكون UTF-8 أيضًا إن كان الافتراضي UTF-8 |
| B36 | غير ذلك = الترميز الافتراضي، أي ANSI | 185 | Edit.c:1341-1356 | مؤكد | الوسم الصالح أولًا |
| B37 | ANSI: Scintilla على صفحة النظام، بلا تحويل | 186 | Edit.c:1381-1386 | مؤكد | |
| B38 | صفحة رموز أخرى: تحويل إلى UTF-8 | 187 | Edit.c:1359-1375 | مؤكد | |
| B39 | كشف نهاية السطر | 188 | Edit.c:599 `EditDetectEOLMode` | مؤكد | |
| B40 | ملف 1256 بلا وسم على نظام 1252 يظهر مشوهًا، والحل Recode (F8) | 190 | Edit.c:1381؛ Notepad2.rc:96, 520 | مؤكد | أو `DefaultEncoding=1256` |
| B41 | الحفظ بـ `OPEN_ALWAYS` وإعادة محاولة بخاصيتي الإخفاء والنظام | 193 | Edit.c:1421-1446 | مؤكد | |
| B42 | تصحيح اختياري قبل الكتابة | 194 | Edit.c:1452-1459 | مؤكد | |
| B43 | الكتابة في المكان: `SetEndOfFile` ثم `WriteFile` | 195 | Edit.c:1502-1578 | مؤكد | |
| B44 | حفظ UTF-16 مع BOM وقلب BE | 196 | Edit.c:1497-1517 | مؤكد | |
| B45 | حفظ UTF-8 مع `EF BB BF` للـ Signature | 197 | Edit.c:1524-1531 | مؤكد | |
| B46 | صفحة رموز: `WC_NO_BEST_FIT_CHARS` والرسالة 50016 | 198 | Edit.c:1556, 1564 | مؤكد | UTF-7 وGB18030 لا تحذّران |
| B47 | ANSI يُكتب كما هو | 199 | Edit.c:1577-1580 | مؤكد | |
| B48 | `SCI_SETSAVEPOINT` إلا في Save Copy | 200 | Edit.c:1589-1590 | مؤكد | |
| B49 | New: سؤال حفظ، ثم مسح وترميز افتراضي و"Untitled" | 203, 219 | Notepad2.c:2004, 6216-6242 | مؤكد | |
| B50 | Open: سؤال حفظ، ثم `GetOpenFileNameW`، والاسم غير الموجود في النافذة يعرض سؤال الإنشاء | 204, 220 | Notepad2.c:6549 `OFN_FILEMUSTEXIST`؛ 6280-6285 | **خاطئ جزئيًا** | سؤال الإنشاء لا يأتي من النافذة |
| B51 | Save يرجع دون كتابة إن لم يتغير شيء | 205 | Notepad2.c:2052 `FileSave(TRUE,…)`؛ 6395 | **خاطئ** | Save يكتب دائمًا |
| B52 | Save بالترميز الحالي، وسؤال للقراءة فقط، وبلا اسم = Save As | 205, 221 | Notepad2.c:6416-6433, 6470 | مؤكد | |
| B53 | Save As: `GetSaveFileNameW`، والإلغاء لا يغيّر شيئًا | 206, 222 | Notepad2.c:6444-6483 | مؤكد | |
| B54 | سؤال الحفظ نعم/لا/إلغاء ونصه | 207-210 | Notepad2.c:6407-6412؛ Dialogs.c:93-108 | مؤكد | `MB_YESNOCANCEL \| MB_ICONEXCLAMATION` |
| B55 | لا يُسأل عن مستند جديد فارغ أو مسافات فقط | 211 | Notepad2.c:6380-6396 | مؤكد | بلا اسم، وأقل من 1023 بايت، ومسافات/Tab/CR/LF فقط |
| B56 | `WM_CLOSE` و`WM_QUERYENDSESSION` | 212, 223 | Notepad2.c:861-871 | مؤكد | |
| B57 | حالة التغيير في العنوان | 224 | Helpers.c:367 | مؤكد | |
| B58 | نافذة الفتح من COMDLG32 | 225 | Notepad2.c:6554 | مؤكد | |

### C — `analysis.md` الأسطر 250–353 (ملف الإعدادات)
| id | الاستنتاج | السطر | الموضع في الشيفرة | التصنيف | ملاحظة |
|---|---|---|---|---|---|
| C1 | `/f` و`/f0` (يُخزَّن `*?`) | 270, 300 | Notepad2.c:5467-5474, 5884, 5927-5930 | مؤكد | |
| C2 | الاسمان: اسم البرنامج.ini ثم `Notepad2.ini` | 271, 298 | Notepad2.c:5900-5906 | مؤكد | |
| C3 | الترتيب: مجلد البرنامج ثم `%APPDATA%` ثم `SearchPathW` | 272-275, 298 | Notepad2.c:5821-5840 | مؤكد | للأسماء النسبية |
| C4 | تحويل `Notepad2.ini=` مع توسيع المتغيرات | 276, 299 | Notepad2.c:5854, 5818, 5861 | مؤكد | حتى تحويلين |
| C5 | مسار افتراضي جانبي، والعمل بلا ملف | 277 | Notepad2.c:5917-5918, 5947-5950 | مؤكد | |
| C6 | لا إنشاء عند التشغيل | 280, 297 | Notepad2.c:359-361, 5965 | مؤكد | `CreateIniFile` يُستدعى، لكن على مسار فارغ |
| C7 | الإنشاء الفعلي فقط بـ F7 | 281-282, 297 | Notepad2.c:3788-3796 | مؤكد | الدالة تُستدعى من أماكن أخرى، لكن على ملف موجود |
| C8 | `SHCreateDirectoryExW` ثم `OPEN_ALWAYS` | 282 | Notepad2.c:5970-5978 | مؤكد | |
| C9 | ترويسة 26 بايت `FF FE` + "[Notepad2]\r\n" | 283, 301 | Notepad2.c:5981-5983 | مؤكد | |
| C10 | اختبار `WriteTest`، والنصوص 50036–50038 | 284-285, 302 | Notepad2.c:3805-3823؛ Notepad2.rc:1523-1525 | مؤكد | |
| C11 | F7 = `IDM_VIEW_SAVESETTINGSNOW` | 266, 281-285 | resource.h:324؛ Notepad2.c:3784-3825 | مؤكد | |
| C12 | `WM_DESTROY`: `GetWindowPlacement` ثم `SaveSettings(FALSE)` | 267, 286 | Notepad2.c:792-824 | مؤكد | |
| C13 | لا ملف = لا شيء، و`SaveSettings=0` = سطر واحد | 287-288 | Notepad2.c:5226-5234 | مؤكد | |
| C14 | `[Settings]` كاملًا، ثم `[Window]` والأنماط | 289 | Notepad2.c:5239-5350؛ Styles.c:1102-1152 | مؤكد | الأنماط فقط إن عُدّلت (`fStylesModified`) |
| C15 | الفشل عند الإغلاق صامت | 290, 302 | Notepad2.c:824, 5219 (`void`) | مؤكد | |
| C16 | "No ini-file is created by default" | 297 | `source/Notepad2.txt:163-165` | مؤكد | |
| C17 | "/f … /f0" | 300 | `source/Notepad2.txt:833` | مؤكد | |
| C18 | اقتباسات صفحة FAQ | 298-302 | — | غير محسوم | الصفحة ليست في الحزمة، والسلوك المنسوب إليها مؤكد |
| C19 | "multiple window position settings" | 303 | `source/Notepad2.txt:174` | مؤكد | |
| C20 | مفاتيح `[Window] "%ix%i PosX/…"`، إلا مع `StickyWindowPosition` | 303, 313-314 | Notepad2.c:5337-5350, 5171-5188 | مؤكد | |
| C21 | التكبير لا يُحفظ، و`PrintZoom` للطباعة | 304, 311 | Notepad2.c:3573-3588, 5060, 5282 | مؤكد | |
| C22 | `Default Style=font:Lucida Console; size:10` | 304, 311 | Styles.c:44-45 | مؤكد | |
| C23 | `[Settings] WordWrap` ومعه `WordWrapMode` و`WordWrapIndent` و`WordWrapSymbols` | 312 | Notepad2.c:5250-5253, 4919, 4957 | مؤكد | |
| C24 | عمود Notepad2 في جدول المقابلة | 320-324 | Notepad2.c:5917, 3791, 5983, 824 | مؤكد | |

### D — `analysis.md` الأسطر 354–518 و`menus.md`
| id | الاستنتاج | الموضع في المستند | الموضع في الشيفرة | التصنيف | ملاحظة |
|---|---|---|---|---|---|
| D1 | الأوامر 40364/40366/40368 في معالج `WM_COMMAND` | analysis 359؛ menus 35 | Notepad2.c:1116-1117, 3239-3322 | مؤكد | |
| D2 | حوار غير مشروط `0x76`/`0x77` | analysis 360؛ menus 43, 148, 152 | Edit.c:5208-5221؛ resource.h:97-98 | مؤكد | |
| D3 | الإجراء يقرأ الخيارات ويوزّع الأزرار | analysis 361؛ menus 218-221 | Edit.c:4786, 5084-5113 | مؤكد | |
| D4 | إنشاء الحوار بـ `CreateDialogParam` وإبراز المفتوح | analysis 367 | Helpers.c:2033-2034؛ Notepad2.c:3239-3247 | **خاطئ جزئيًا** | `CreateDialogIndirectParam`، ويُعاد إنشاؤه عند تبديل الوضع |
| D5 | النص إلى UTF-8 وحفظه في السجل | analysis 369-370 | Edit.c:4985-5030 | مؤكد | |
| D6 | الأعلام 4 و2 و0x100000 و0x600000 | analysis 371؛ menus 227-230 | Edit.c:5002-5011 | مؤكد | |
| D7 | تحويل الشرطات أولًا | analysis 373 | Edit.c:5244-5247 | مؤكد | |
| D8 | `SCI_FINDTEXT` من نهاية التحديد إلى الطول | analysis 374 | Edit.c:5260-5264 | مؤكد | |
| D9 | التحديد عند التطابق | analysis 375 | Edit.c:5281-5284 | مؤكد | |
| D10 | سؤال الالتفاف OK/Cancel | analysis 376 | Edit.c:5266-5272؛ Notepad2.rc:1482 | مؤكد | InfoBox يمكن كتمه |
| D11 | "The specified text was not found." | analysis 377 | Edit.c:5274-5279؛ Notepad2.rc:1484 | مؤكد | |
| D12 | Replace يبحث من بداية التحديد | analysis 379 | Edit.c:5409-5413 | مؤكد | |
| D13 | تطابق غير التحديد = تحديد فقط | analysis 380 | Edit.c:5434-5438 | مؤكد | |
| D14 | الاستبدال بالهدف، و`RE` مع Regex، ثم التالي | analysis 381 | Edit.c:5371, 5440-5459 | مؤكد | |
| D15 | لا تطابق بعدها = المؤشر بعد الهدف ورسالة | analysis 382 | Edit.c:5461-5466 | مؤكد | |
| D16 | `^c` = الحافظة | analysis 383؛ menus 242 | Edit.c:5390-5392, 434 | مؤكد | |
| D17 | اقتباس FAQ عن النص ثنائي الاتجاه | analysis 415-420 | — | غير محسوم | الصفحة ليست في الحزمة |
| D18 | `Notepad2.txt` لا يذكر RTL | analysis 422 | `source/Notepad2.txt` (لا نتائج) | مؤكد | |
| D19 | Scintilla 2.24 | analysis 424 | `source/Readme.txt:26`؛ `Notepad2.txt:87` | مؤكد | |
| D20 | لا استيراد لـ DirectWrite | analysis 424 | لا `dwrite`/`d2d1`/`SC_TECHNOLOGY` في `src/` | غير محسوم | جدول الاستيراد خاص بالثنائي |
| D21 | `CreateBars` و`CreateStatusWindowW(…,0xfb00)` | analysis 474؛ menus 213 | Notepad2.c:1460, 1597-1600؛ Notepad2.h:61 | مؤكد | |
| D22 | تحديث الشريط بست خانات | analysis 475-488 | Notepad2.c:6043, 6123-6128 | مؤكد | الخانة 1 طول المستند، والخانة 5 اسم المحلل |
| D23 | الخانة 0 بالنصين 10006/10007 | analysis 483؛ menus 213 | resource.h:124-125؛ Notepad2.rc:1418-1419 | مؤكد | |
| D24 | لا عدّاد كلمات في الأصل | analysis 490-493؛ menus 213 | لا نص في `.rc`/`.c`/`.txt` | مؤكد | |
| D25 | Show Statusbar يُظهر ويُخفي فقط | menus 213 | Notepad2.c:3610-3620 | مؤكد | |
| D26 | 148 أمرًا و16 قائمة و38 فاصلًا، بلا تكرار | menus 21, 24 | Notepad2.rc:65-329 | مؤكد | |
| D27 | `MENU 101` = قائمة أيقونة الشريط | menus 9, 22 | Notepad2.rc:331-356؛ Notepad2.c:1086, 1095, 1223 | **خاطئ جزئيًا** | ثلاث قوائم منبثقة، والعدد 12 صحيح |
| D28 | أوامر `MENU 101` بلا ربط بجدول الاختصارات | menus 29 | Notepad2.rc:364-564 | **خاطئ جزئيًا** | صحيح لـ 40600/40601 فقط |
| D29 | 159 مدخلًا في جدول الاختصارات | menus 23 | Notepad2.rc:364-564 | مؤكد | |
| D30 | إجراء النافذة و`WM_COMMAND` | menus 34-35 | Notepad2.c:456, 1116-1117 | مؤكد | |
| D31 | عناوين جداول القفز وأرقام الحالات | menus 37-40 | — | غير محسوم | خاص بالثنائي |
| D32 | Open: 40001 Ctrl+O ← `FileLoad` ← `OpenFileDlg` | menus 54 | resource.h:174؛ Notepad2.rc:70؛ Notepad2.c:2008-2009, 6252-6254 | مؤكد | |
| D33 | Save: 40004 ← `FileSave(1,0,0,0)`، و`SaveFileDlg` فقط بلا اسم | menus 56 | Notepad2.c:2051-2052, 6416-6444 | **خاطئ جزئيًا** | أيضًا لملف للقراءة فقط بموافقة المستخدم |
| D34 | Save As: 40005 F6 ← `FileSave(1,0,1,0)` | menus 57 | Notepad2.c:2056-2057, 6433-6444, 6610 | مؤكد | |
| D35 | Find: 40364 Ctrl+F، الحوار `0x76` | menus 148 | resource.h:271؛ Notepad2.rc:252؛ Notepad2.c:3239-3241 | مؤكد | |
| D36 | Find Next: 40366 F3 ← `EditFindNext`، وبلا نص يرسل 40364 | menus 150 | Notepad2.c:3263-3267, 3295-3296 | مؤكد | يتوقف أيضًا إن كان المستند فارغًا |
| D37 | Replace: 40368 Ctrl+H، والزر 112 ← `EditReplace` | menus 152 | resource.h:275؛ Notepad2.c:3322-3324؛ Edit.c:5099-5101 | مؤكد | |
| D38 | Find Previous وReplace Next يشاركان حالة Find Next | menus 246 | Notepad2.c:3255-3259 | مؤكد | |
| D39 | Default Font: 40403 Alt+F12 ← `ChooseFontW` | menus 45, 158 | resource.h:283؛ Notepad2.c:3365-3366؛ Styles.c:2245 | مؤكد | |
| D40 | Word Wrap: 40404 Ctrl+W ← `SCI_SETWRAPMODE` | menus 44, 159 | resource.h:284؛ Notepad2.c:3372-3380 | مؤكد | `SC_WRAP_WORD` أو `SC_WRAP_CHAR` حسب `iWordWrapMode` |
| D41 | About: 40500 F1 ← `AboutDlgProc` | menus 46, 200 | Notepad2.c:3828-3830؛ Dialogs.c:205-237 | مؤكد | |
| D42 | عناصر حوار البحث 100–108 وأثرها | menus 225-233 | resource.h:7-77؛ Edit.c:5002-5063 | مؤكد | |
| D43 | الأزرار 1 و109 و112 و113 و114 و2 | menus 234-239 | Edit.c:5084-5121 | مؤكد | |
| D44 | الرابط 115 يبدّل بين الحوارين | menus 240 | Edit.c:5174-5181 | مؤكد | |
| D45 | `SCI_REPLACETARGETRE` مع Regex | menus 230 | Edit.c:5371 | مؤكد | |
| D46 | أوامر بلا مدخل اختصار: Exit وClear وIndent وUnindent وText Excerpt | menus 245 | Notepad2.rc:364-564 | مؤكد | Ctrl+9 مربوط بـ `CMD_TOGGLETITLE` |
| D47 | نصوص القوائم واختصارات الأوامر التسعة | menus 54-200 | Notepad2.rc:70, 72, 73, 252, 254, 256, 262, 264, 317 | مؤكد | |

## تجربة `editor.exe` في Wine بعد المراجعة
لم يتغير شيء في `src/` لأنه لا ميزة مبنية على استنتاج خاطئ. أُعيد البناء بـ `./build.sh` (SHA-256 `57c56f78…919ba8`)، وجُرّبت السلوكيات المرتبطة بالاستنتاجات الخاطئة. اللقطات في `docs/screenshots/verdict/`.

| السلوك (الاستنتاج) | ما ظهر | اللقطة |
|---|---|---|
| اسم غير موجود في نافذة الفتح (B50) | رفضته النافذة برسالة النظام "File does not exist"، فلم يُفتح شيء ولم يُنشأ ملف. وهذا ما يفعله الأصل الحقيقي بـ `OFN_FILEMUSTEXIST` | `02-open-nonexistent-rejected.png`، `05-open-missing-in-folder.png` |
| Save لملف مفتوح لم يتغير (B51) | فُتح `notes.txt` ("مرحبا") ثم Ctrl+S دون تعديل. تغيّر وقت تعديل الملف من `2026-11-16 20:47:04` إلى `2026-11-17 01:05:59`، وبقي المحتوى نفسه (md5 `9530db02…1861`). أي أن الحفظ يكتب دائمًا كالأصل | `03-notes-opened.png`، `04-save-unmodified.png` |
| التبديل بين البحث والاستبدال (D4) | Ctrl+F فتح "بحث"، وCtrl+H حوّلها إلى "استبدال" بحقل "استبدل بـ" وزرّيه، وCtrl+F أرجعها إلى "بحث". النتيجة الظاهرة كالأصل، والآلية مختلفة: الأصل يغلق النافذة وينشئها من جديد | `06-find.png`، `07-replace-switch.png`، `08-find-again.png` |
