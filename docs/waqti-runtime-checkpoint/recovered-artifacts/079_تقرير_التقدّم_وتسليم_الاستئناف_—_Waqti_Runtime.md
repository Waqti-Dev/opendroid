# تقرير التقدّم وتسليم الاستئناف — Waqti Runtime

**تاريخ اللقطة:** 27 سبتمبر 2026، UTC+03:00  
**الغرض:** تمكين مهندس آخر من استئناف العمل مباشرة إذا توقفت الجلسة.  
**حالة الإنجاز:** أساس أصلي صغير لـ GGUF/NDK/JNI بُني واختُبر؛ **لم يُنفّذ بعد استدلال نموذج أو توليد رموز**.

## 1. ملخص سريع

- المستودع: `Waqti-Dev/opendroid`، الدليل `/home/ubuntu/opendroid`.
- الفرع `main`، HEAD الأصلي/الحالي قبل الالتزام `b89a255b37be33096a1ba67a3cff40efcd919b82`، شجرة التعديلات **غير ملتزمة**؛ لا يوجد commit أو push لهذه التغييرات.
- أُنشئ تقرير التدقيق، ووثيقة المعمارية، ومذكرة بحث/ترخيص. كما أُضيفت أول شريحة C++/CMake/JNI: قارئ محدود لبيانات وصفية من GGUF v3 مع اختبارات؛ هذا **ليس** Model Loader للاستدلال ولا Runtime يولّد إجابات.
- بعد تثبيت JDK وAndroid SDK/NDK محليًا: بناء `debug` و540 اختبار JVM نجحت، وبُنيت المكتبة الأصلية لـ `arm64-v8a` و`x86_64`. اختبارات مصدر Android instrumentation تترجم بنجاح؛ تشغيلها فعليًا لم يُتحقق بعد.
- تسع حالات مولّدة، إضافة إلى ملف Qwen حقيقي، نجحت على CMake host وتحت AddressSanitizer وUndefinedBehaviorSanitizer.
- APK الحالي: `/home/ubuntu/opendroid/app/build/outputs/apk/debug/app-debug.apk`، حجمه نحو 74 MB، SHA-256:
  `881d232ab78d13a75ba71b889de182af0d75d85be40c8f28412097610b6d6c8d`.
- **لا يوجد حاليًا مسار Android → JNI → tokenizer → tensor engine → CPU inference → token output، ولا دعم GGUF للمستخدم.** النموذج والاختبار على Redmi Turbo 4 Pro ما زالا مفقودين.

## 2. الملفات المعدّلة أو الجديدة

### وثائق التسليم

- `WAQTI_RUNTIME_AUDIT.md`: حالة المستودع الأصلي، تصنيف REUSE/REFACTOR/REPLACE/MISSING، البناء والاختبارات، المخاطر وتسلسل التنفيذ؛ تم تحديثه ليفصل بين HEAD الأصلي وما أُنجز في هذه الجلسة.
- `WAQTI_RUNTIME_ARCHITECTURE.md`: المعمارية المقترحة وتعايش LiteRT مع Waqti Runtime، حدود JNI، مراحل التنفيذ، والقيود الحالية.
- `docs/RESEARCH.md`: سجل مواصفة GGUF، وموديل Qwen الرسمي، وملف GGUF مرخّص وحالة فحصه؛ لا كود من llama.cpp أو ggml منسوخ.

### البناء الأصلي وواجهة JNI

- `app/build.gradle`: تفعيل CMake/NDK وتحديد NDK `28.2.13676358`، CMake `3.22.1`، وABI `arm64-v8a`, `x86_64`.
- `app/src/main/cpp/CMakeLists.txt`: مكتبة `waqti_runtime` على Android واختبار host لقارئ metadata.
- `app/src/main/cpp/gguf_reader.h`, `gguf_reader.cpp`: قارئ little-endian GGUF v3 محدود وآمن الحدود؛ يقرأ architecture/alignment/context وبعض موصوفات الـ tensors. لا يقرأ/ينفذ الأوزان ولا يتحقق من كل ترميز tensor.
- `app/src/main/cpp/native_bridge.cpp`: غلاف JNI وإرجاع JSON؛ لا ينبغي عبور استثناء C++ إلى JVM.
- `app/src/main/java/com/opendroid/ai/core/runtime/jni/NativeGgufInspector.kt`: واجهة Kotlin داخلية؛ نتيجة الفحص تحتوي صراحة `inferenceSupported = false`.
- `app/src/main/cpp/tests/gguf_reader_tests.cpp`: تسع حالات fixture مولّدة للمدخل الصحيح/غير الصحيح والحدود.
- `app/src/androidTest/java/com/opendroid/ai/core/runtime/jni/NativeGgufInspectorTest.kt`: اختبار Android/JNI لحالة metadata سليمة ورفض magic غير صحيح؛ مصدر الاختبار تُرجم، ولم يُثبت أنه يعمل على جهاز بعد.
- `app/proguard-rules.pro`: قاعدة keep لاسم JNI عند R8 في release.

## 3. أدلة الاختبار والبناء

آخر تشغيل ناجح في هذه الجلسة:

```bash
cd /home/ubuntu/opendroid
export ANDROID_HOME="$HOME/Android/Sdk"
export ANDROID_SDK_ROOT="$HOME/Android/Sdk"
export JAVA_HOME=/usr/lib/jvm/java-21-openjdk-amd64
export PATH="$JAVA_HOME/bin:$PATH"
./gradlew testDebugUnitTest assembleDebug compileDebugAndroidTestKotlin --no-daemon --console=plain
```

النتيجة `BUILD SUCCESSFUL`، وشملت بناء CMake للـ ABI الاثنين. تقارير JUnit: `540 tests, 0 failures, 0 errors, 0 skipped` في 64 ملف XML. هدف `compileDebugAndroidTestKotlin` ناجح؛ **لم يُشغّل** `connectedDebugAndroidTest` بنجاح حتى الآن.

اختبارات C++ العادية:

```bash
cd /home/ubuntu/opendroid
export PATH="$HOME/Android/Sdk/cmake/3.22.1/bin:$PATH"
cmake --build /tmp/waqti-runtime-host-build --parallel 2
export WAQTI_TEST_GGUF_PATH=/tmp/qwen2.5-0.5b-instruct-q4_k_m.gguf
sha256sum "$WAQTI_TEST_GGUF_PATH"
ctest --test-dir /tmp/waqti-runtime-host-build --output-on-failure
```

اختبارات sanitizers:

```bash
export WAQTI_TEST_GGUF_PATH=/tmp/qwen2.5-0.5b-instruct-q4_k_m.gguf
ASAN_OPTIONS=detect_leaks=1 ctest --test-dir /tmp/waqti-runtime-asan --output-on-failure
```

**ملاحظة:** قاعدة `app/proguard-rules.pro` أُضيفت بعد آخر بناء Debug؛ لا تؤثر في `debug` لكن يجب اختبارها بتجميع Release غير موقع:

```bash
./gradlew assembleRelease -PallowUnsignedRelease=true --no-daemon --console=plain
```

هذا ينتج Release غير موقع للاختبار فقط؛ لا توزّعه. لا تشغّل `assembleRelease` دون الخيار إلا مع إعداد توقيع مصرح به.

## 4. البيئة المتاحة

- JDK: OpenJDK `21.0.12.1`، مثبت خلال الجلسة.
- Android SDK: `$HOME/Android/Sdk`; platform `android-36` revision 2، build-tools `36.0.0`، platform-tools `37.0.1`.
- Android NDK `28.2.13676358`، CMake `3.22.1`، Android command-line tools/emulator، وصورة `system-images;android-36;google_apis;x86_64`.
- لا تضف `local.properties` إلى git؛ مرّر متغيرات SDK كما في الأوامر أعلاه.
- لم يُعثر على `/dev/kvm`. أُنشئ AVD باسم `waqti_api36_x86_64` وشُغّل بالـ software acceleration؛ فحص boot استمر حتى مهلة 240 ثانية، ثم ظهر المحاكي لفترة قصيرة `device`، وبعد إيقاف الخدمة أصبح آخر فحص `adb` يقول `no emulators found`. لم يُشغّل instrumentation. أعد إنشاء/تشغيل AVD أو استخدم KVM/هاتفًا، وتأكد من `sys.boot_completed=1` قبل متابعة الاختبار.
- جهاز Redmi Turbo 4 Pro الفعلي غير متصل. المحاكي لا يختبر Snapdragon أو NEON الفعلي أو UFS/thermal أو QNN/Hexagon.

## 5. حدود التنفيذ المعروفة — لا تبالغ في وصفها

1. ملفات GGUF تُرفض حاليًا في `ModelRepository`; لم يتغير ذلك. لا تضفها إلى سجل النماذج باعتبارها قابلة للتشغيل.
2. قارئ metadata لا يفك tokenizer metadata إلى tokenizer؛ لا توجد BPE/merges، tensor byte-size validation كاملة، dequantization، matmul/attention، transformer، KV cache، sampler، streaming، cancellation، أو توليد رموز.
3. `NativeGgufInspector` غير مربوط بـ `LLMProvider` أو `LLMProviderFactory` ولا يستخدم في واجهة استيراد المستخدم؛ هو أساس native صغير/اختبار JNI فقط.
4. لا توجد نتيجة على هاتف حقيقي أو قياسات TTFT/tokens-sec/RAM/UFS/thermal. لا تدّعِ دعم Qwen/أي GGUF أو سرعة/ميزة أداء.
5. LiteRT-LM مسار مستقل مبني على SDK طرف ثالث. لا تخلطه مع Waqti-owned inference.
6. لا تغيّر حدود `DefaultToolExecutor` أو provider fallback في إطار هذه الخطوة.

## 6. الأبحاث والترخيص

- المرجع الأولي: [مواصفة GGUF الرسمية](https://github.com/ggml-org/ggml/blob/master/docs/gguf.md). موديل المرشح: [Qwen2.5-0.5B-Instruct](https://huggingface.co/Qwen/Qwen2.5-0.5B-Instruct) مع [GGUF رسمي](https://huggingface.co/Qwen/Qwen2.5-0.5B-Instruct-GGUF)؛ سجل الحقول والروابط موجود في `docs/RESEARCH.md`.
- بحث المقارنة المنسّق حول مراجع runtime/backends/MoE فشل مبكرًا قبل نتائج موثوقة؛ تفاصيله في job `68dbbc352414`، ولا تُعدّ نتائجه مصدرًا. أُجري بدلًا منه بحث رسمي مباشر في Qwen وllama.cpp؛ تبقى أبحاث BigMoeOnEdge وMLC/TVM وExecuTorch وQNN/Hexagon وواجهات Android بحاجة إلى مصادر أصلية.
- بطاقة النموذج وصفحة مستودع GGUF الرسميتين تعرضان Apache-2.0، وقد تمت مراجعة [ملف LICENSE الفعلي](https://huggingface.co/Qwen/Qwen2.5-0.5B-Instruct-GGUF/raw/main/LICENSE). لم تُراجع بعد تفاصيل NOTICE/التوزيع داخل التطبيق ولا أداء النموذج. تم تنزيل عيّنة Q4_K_M للاختبار إلى `/tmp` فقط؛ لا تحزمها أو تلتزم بها.
- التطبيق الأصلي Apache-2.0. الكود الجديد موسوم SPDX Apache-2.0، وهو كتابة مستقلة وفق سجل البحث، وليس كودًا منسوخًا.

## 7. سجل قرارات الجلسة

- حافظنا على التطبيق ومزودي LiteRT/cloud الحاليين؛ لا مبرر لإعادة كتابة المشروع.
- عيّنا Qwen2.5-0.5B-Instruct Q4_K_M كمرشح أولي: بطاقة المصدر Apache-2.0؛ ملف GGUF الرسمي من commit `6dd44a1fb35d11b5d1b28902876ce3cc9e882d0e`، حجمه 491400032 بايت وSHA-256 `74a4da8c9fdbcd15bd1f6d01d621410d31c6fc00986f5eb687824e7b93d7a9db`. قارئ Waqti قبله كـ GGUF v3 `qwen2` metadata فقط، 291 tensor و26 metadata وcontext 32768/alignment 32. هذا **تحقق parsing على host فقط**؛ لا inference ولا نجاح JNI/device.
- بدأنا بـ parser metadata محدود لأنه يمكن بناؤه واختباره كقطعة مستقلة بلا ادعاء inference. الخطوة التالية ليست MoE ولا تحسين throughput، بل إكمال التحقق على JNI، ثم قراءة metadata اللازمة لتطبيق tokenizer/نموذج dense صغير مع مقارنة مرجعية.
- لا تغييرات في إعدادات الحساب/الخدمات، ولا رسائل أو PR أو push. المستودع غير ملتزم بالتغييرات حتى الآن.

## 8. خطوات الاستئناف بالترتيب

1. افحص حالة الأدوات والمستودع:

   ```bash
   cd /home/ubuntu/opendroid
   git status --short --branch
   "$HOME/Android/Sdk/platform-tools/adb" devices -l
   ```

2. إذا ظهر `emulator-5554 device`، تحقق من boot وشغّل اختبار JNI:

   ```bash
   "$HOME/Android/Sdk/platform-tools/adb" -e shell getprop sys.boot_completed
   ANDROID_HOME="$HOME/Android/Sdk" ANDROID_SDK_ROOT="$HOME/Android/Sdk" \
     JAVA_HOME=/usr/lib/jvm/java-21-openjdk-amd64 \
     ./gradlew connectedDebugAndroidTest --no-daemon --console=plain
   ```

   إذا كان offline أو لم يقل `1`، لا تنسب النجاح إلى instrumentation. أعد تشغيل AVD بمهلة كافية/سرّع بمضيف KVM، أو اطلب جهاز فعلي عند الحاجة.

3. أعد تشغيل host CTest بعد كل تغيير C++؛ أضف fixtures للـ arrays/length overflow/invalid boolean/offsets، ثم fuzz parser محدودًا قبل توسيع الصيغ.
4. شغّل `assembleRelease -PallowUnsignedRelease=true` للتحقق من R8/ProGuard بعد قاعدة JNI الجديدة.
5. ثبّت config/tokenizer/chat-template revision/hash؛ أنشئ differential tests against tokenizer/reference pinned to a specific llama.cpp MIT commit (no code import). سجّل بحث BigMoeOnEdge وMLC/TVM وExecuTorch وQNN/Hexagon وAndroid OS docs قبل الوصول لهذه المراحل. الملف 491 MB موجود مؤقتًا فقط في `/tmp`؛ لا تضعه في Git أو APK.
6. قرر عقد model manifest/metadata، ثم نفّذ tokenizer كقطعة مستقلة واختبر token IDs ضد مرجع موثوق. بعد ذلك tensor primitives واختبارات دقيقة، ثم نموذج dense صغير حقيقي وCPU generation. لا تضف provider adapter قبل أن يوجد ناتج token فعلي قابل للإلغاء والاختبار.
7. بعد تحقق inference محليًا، اربطه بسياسة `LLMProvider`/fallback الحالية، ثم اختبر على Redmi Turbo 4 Pro، ثم اجمع القياسات وأعد ترتيب أولويات MoE/IO/backend من القياسات لا الافتراضات.
8. أنشئ commit منطقيًا بعد كل شريحة ناجحة. لا تعمل push/PR إلا بعد توجيه مناسب من المستخدم.

## 9. الموارد القابلة للتسليم

- التدقيق: [`WAQTI_RUNTIME_AUDIT.md`](./WAQTI_RUNTIME_AUDIT.md)
- المعمارية: [`WAQTI_RUNTIME_ARCHITECTURE.md`](./WAQTI_RUNTIME_ARCHITECTURE.md)
- بحث المراجع والترخيص: [`docs/RESEARCH.md`](./docs/RESEARCH.md)
- تقرير التسليم هذا: [`WAQTI_RUNTIME_PROGRESS_HANDOFF.md`](./WAQTI_RUNTIME_PROGRESS_HANDOFF.md)
- APK Debug الناتج (يتضمن المكتبة الأصلية، لكنه لا يشغّل GGUF inference): [`app-debug.apk`](./app/build/outputs/apk/debug/app-debug.apk)
