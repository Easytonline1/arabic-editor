# خطة: فهم محرر نصوص لويندوز بأدوات REA وبناء محرر خاص بنا

## السياق
- لدينا نافذة Win32 فارغة: `src/main.c`، ويبنيها `build.sh` إلى `build/editor.exe`، وتعمل في Wine على الشاشة `:99`.
- الهدف: فهم سلوك محرر نصوص أصلي لويندوز بأدوات REA، ثم بناء ستة أوامر فقط بشيفرة نكتبها بأنفسنا.
- الأوامر الستة: الفتح، والحفظ، والبحث والاستبدال، وحجم الخط، والتفاف السطور، ونافذة التعريف بالبرنامج.
- لا نسخ لشيفرة الأصلي ولا لموارده ولا لنصوصه: نتعلّم السلوك، ثم نكتب تنفيذًا مستقلًا.
- الملف الثنائي الأصلي يضعه المستخدم في `original/`، ويُضاف إلى `.gitignore` فلا يُلتزم.

**أول خطوة بعد الموافقة:** كتابة هذه الخطة في `plan.md` بجذر المستودع، ثم `git commit` و `git push` إلى `claude/inspiring-carson-3101le`. وبعد كل مرحلة التزام ودفع، كما ينص `CLAUDE.md`.

## أدوات REA المستعملة
| الأداة | ما تقدّمه لمحرر نصوص لويندوز |
|---|---|
| `inspect_artifact` و `inspect_signature` و `inspect_pe_certificates` | هوية الملف ومُصدره وتوقيعه، وهي أساس مراجعة الرخصة |
| `inspect_pe_resources` | معلومات الإصدار (`LegalCopyright` و `CompanyName`)، والقوائم ومفاتيح الاختصار والحوارات وجداول النصوص. منها نحصي الأوامر |
| `inspect_pe_imports` | واجهات API المستعملة: `comdlg32` (`GetOpenFileNameW` و `FindTextW` و `ReplaceTextW` و `ChooseFontW`)، و`user32` و`gdi32` |
| `binary_overview` و `open_binary` (Ghidra) | فتح الملف للتحليل العميق |
| `search_strings` و `list_strings` | نصوص مثل أسماء الترميزات وفلاتر الحفظ ورسائل "غير موجود" |
| `xrefs` و `procedure_callers` و `procedure_callees` و `get_call_graph` | من الاستيراد أو النص إلى الدالة التي تنفّذ الأمر |
| `procedure_pseudo_code` و `analyze_function` | فهم منطق الدالة، كالترميز وBOM وقراءة الملف وكتابته |
| `trace_native_values` و `inspect_native_data_type` | قيم ثابتة مثل أنماط `EDIT` وأعلام `FINDREPLACE`، وبنى مثل `LOGFONTW` |
| `trace_feature` | تتبّع أمر من عنصر القائمة إلى التنفيذ |
| `record_unknown` و `update_unknown` و `verify_unknown_resolution` | تسجيل ما لم يُحسم وتوثيق حسمه |
| `export_evidence_bundle` ثم `close_binary` | حفظ الأدلة في `docs/evidence/` وإغلاق الجلسة |

## المراحل

### المرحلة 1: مراجعة الرخصة (بوابة قرار)
- **الأدوات:** `inspect_artifact` و `inspect_signature` و `inspect_pe_certificates` و `inspect_pe_resources` (VERSIONINFO)، و`search_strings` بالأنماط `license|EULA|copyright`.
- **المخرَج:** `docs/license-review.md`، ويضم:
  - المُصدر والإصدار ونص حقوق النشر.
  - الرخصة المنطبقة، وهل تسمح بالهندسة العكسية لغرض التوافق.
  - قواعد "الغرفة النظيفة".
- **القرار:** إن لم تسمح الرخصة نتوقف، ونكتفي بملاحظة السلوك الظاهر في Wine دون تفكيك الملف. القرار للمستخدم.

### المرحلة 2: جرد الأوامر وتحديد النطاق
- **الأدوات:** `binary_overview`، و`inspect_pe_resources` (القوائم ومفاتيح الاختصار والحوارات)، و`inspect_pe_imports`.
- **المخرَج:** `docs/commands.md`، وفيه جدول بكل أوامر الأصلي ومفاتيح اختصارها، وتصنيف كل أمر "ضمن النطاق" أو "خارج النطاق".
- **خارج النطاق:** كل أوامر الأصلي غير الستة المذكورة. أمثلة: الطباعة وإعداد الصفحة، والتاريخ والوقت، والانتقال إلى سطر، وشريط الحالة، والتكبير، والتراجع المتقدم، والتبويبات، والسمات.

### المرحلة 3: الهيكل الأساسي لمحررنا
- **الأداة:** `inspect_pe_resources` على الأصلي، لمعرفة نوع عنصر التحرير وبنية القائمة فقط.
- **التنفيذ:**
  - `src/resource.rc` يضم قائمة عربية بأوامرنا الستة ومفاتيح الاختصار.
  - `src/resource.h` يضم المعرّفات.
  - عنصر `EDIT` متعدد الأسطر يملأ النافذة، مع `WM_SIZE` لتغيير حجمه.
  - تحديث `build.sh` ليترجم الموارد بـ `x86_64-w64-mingw32-windres` ويربطها.

### المرحلة 4: الفتح والحفظ
- **الأدوات:**
  - `inspect_pe_imports`، ثم `xrefs` على `GetOpenFileNameW` و`GetSaveFileNameW`.
  - `procedure_pseudo_code` لمعرفة طريقة كشف الترميز (BOM وUTF-8 وUTF-16).
  - `search_strings` لفلاتر الملفات.
- **التنفيذ:** `src/file.c` يفتح الملف ويحفظه بترميز UTF-8، ويقرأ UTF-16 إن وُجد BOM، ويضع اسم الملف في العنوان، ويسأل عند الخروج إن كانت هناك تعديلات غير محفوظة.

### المرحلة 5: البحث والاستبدال
- **الأدوات:**
  - `xrefs` على `FindTextW` و`ReplaceTextW` و`RegisterWindowMessageW` (رسالة `FINDMSGSTRING`).
  - `trace_native_values` لأعلام `FINDREPLACE`.
  - `procedure_pseudo_code` لمنطق المطابقة واتجاه البحث.
- **التنفيذ:** `src/find.c` يضم: بحث، وبحث تالٍ (F3)، واستبدال، واستبدال الكل، مع خيارَي حالة الأحرف والاتجاه.

### المرحلة 6: حجم الخط
- **الأدوات:**
  - `xrefs` على `ChooseFontW` و`CreateFontIndirectW`.
  - `inspect_native_data_type` على `LOGFONTW`.
  - `procedure_callees` لمعرفة طريقة تطبيق الخط وحفظه.
- **التنفيذ:** `src/font.c` يستدعي `ChooseFontW` ثم `WM_SETFONT` على عنصر التحرير. الخط الافتراضي يدعم العربية.

### المرحلة 7: التفاف السطور
- **الأدوات:**
  - `trace_feature` أو `procedure_pseudo_code` على معالج الأمر، لمعرفة هل يعيد الأصلي إنشاء عنصر `EDIT` بلا `ES_AUTOHSCROLL`/`WS_HSCROLL`.
  - `trace_native_values` لقيم الأنماط.
- **التنفيذ:** تبديل الالتفاف بإعادة إنشاء عنصر التحرير مع الحفاظ على النص والتحديد والخط، وعلامة اختيار في القائمة.

### المرحلة 8: نافذة التعريف بالبرنامج
- **الأدوات:** `inspect_pe_resources` لمعرفة نوع الحوار، و`xrefs` على `ShellAboutW` أو `DialogBoxParamW`.
- **التنفيذ:** حوار خاص بنا في `resource.rc`، يضم الاسم Arabic Editor والإصدار ونصًّا عربيًّا من تأليفنا.

### المرحلة 9: التحقق والتوثيق
- إعادة البناء:
  ```bash
  ./build.sh
  ```
- `inspect_pe_imports` على `build/editor.exe`، ومقارنته بواجهات API المتوقعة.
- التشغيل داخل سطح مكتب Wine الافتراضي:
  ```bash
  DISPLAY=:99 wine explorer /desktop=ArabicEditor,1024x700 'Z:\home\user\arabic-editor\build\editor.exe'
  ```
- لقطة شاشة لكل أمر بـ `import -window root` في `docs/screenshots/`.
- فحص يدوي لكل أمر: فتح ملف عربي بترميز UTF-8 وحفظه، وبحث واستبدال نص عربي، وتغيير حجم الخط، وتبديل الالتفاف، وفتح نافذة التعريف.
- تصدير الأدلة بـ `export_evidence_bundle` إلى `docs/evidence/`، ثم `close_binary`.

## الملفات
- **جديدة:**
  - `plan.md`
  - `docs/license-review.md` و `docs/commands.md`
  - `src/resource.rc` و `src/resource.h`
  - `src/file.c` و `src/find.c` و `src/font.c`
- **معدّلة:** `src/main.c` و `build.sh` و `.gitignore` (إضافة `original/`).
