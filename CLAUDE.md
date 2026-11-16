# CLAUDE.md

## اللغة
- رُدّ على المستخدم دائمًا باللغة العربية، مع إبقاء الأوامر والمسارات وأسماء الملفات والشيفرة كما هي.

## حفظ العمل
- احفظ عملك بعد كل خطوة: بعد كل تعديل مكتمل نفّذ `git add` ثم `git commit` برسالة واضحة.
- ادفع الالتزامات إلى فرع العمل بـ `git push -u origin <الفرع>` ولا تدفع إلى `main` دون إذن صريح.

## بيئة الهندسة العكسية
- `setup.sh` يثبت: Ghidra 12.1.4 في `/opt/ghidra_12.1.4_PUBLIC`، و `rea-agents@6.9.0` (أحدث إصدار في npm عند الكتابة)، و mingw-w64، و Wine على شاشة Xvfb الافتراضية `:99`.
- شغّله بـ `sudo bash setup.sh` (أو كـ root)؛ يجب أن ينتهي في أقل من 5 دقائق.
- `.mcp.json` يسجل خادم REA (`rea mcp`) ويمرر له `GHIDRA_INSTALL_DIR=/opt/ghidra_12.1.4_PUBLIC`.
- للتحقق: `rea doctor --json`.
- لتشغيل برامج Windows: `DISPLAY=:99 wine program.exe`، وللترجمة: `x86_64-w64-mingw32-gcc`.
