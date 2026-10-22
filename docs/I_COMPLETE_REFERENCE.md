# الدليل الشامل والموسوعة الكاملة للغة الأنظمة الثنائية البحتة `I`
# The Comprehensive Specification & Reference Manual for the `I` Programming Language

---

## الفهرس العام / Table of Contents
1. **المقدمة والفلسفة الهندسية (Introduction & Philosophy)**
2. **الأساس الثنائي والنموذج الرياضي للبتات (Pure 01 Binary Model)**
3. **نظام الأنواع الدقيق (Honest Machine Type System)**
4. **المعاملات وحسابات تجاوز السعة الصريحة (Operators & Overflow Semantics)**
5. **الكلمات المحجوزة والبنية النحوية الكاملة (Grammar & Keywords)**
6. **تخطيطات الذاكرة المحزومة وحقول البتات (Memory Layouts & Bitfields)**
7. **نظام الأذونات والقدرات العتادية (Capability-Based Security Model)**
8. **الدوال الداخلية لعتاد المعالج (Hardware Intrinsics)**
9. **معمارية المترجم وسلسلة الأدوات (Compiler Toolchain & CLI)**
10. **أمثلة عملية كاملة وقابلة للتنفيذ عبر كافة المجالات (Exhaustive Real-World Code Examples)**
    - 10.1 المبتدئين والأساسيات (Beginner & Fundamentals)
    - 10.2 معالجة البروتوكولات والشبكات (Binary Networking & Packet Parsing)
    - 10.3 هندسة أنظمة التشغيل والنواة (Operating Systems & Bare-Metal Kernel)
    - 10.4 مشغلات العتاد والأجهزة (Hardware Drivers & MMIO)
    - 10.5 الأمن السيبراني الدفاعي (Defensive Cybersecurity & Constant-Time Primitives)
    - 10.6 معالجة الإشارات الرقمية (Digital Signal Processing - DSP)
    - 10.7 بنية الذكاء الاصطناعي والحوسبة الكمومية (Low-Level AI & Quantized Tensors)
11. **مصفوفة المعماريات والمعالجات المدعومة (Architectural Target Matrix)**
12. **استكشاف الأخطاء وتشخيصات المترجم (Diagnostics & Error Codes)**

---

## 1. المقدمة والفلسفة الهندسية (Introduction & Philosophy)

صُممت لغة **`I`** لتكون لغة برمجة أنظمة قائمة على **النظام الثنائي أولاً (Binary-First)**. بخلاف اللغات عالية المستوى أو اللغات التي تخفي سلوك المعالج خلف طبقات تجريد غير حتمية، تنطلق `I` من مبدأ هندسي غير قابل للتفاوض:

> **"يجب أن يكون مهندس الأنظمة قادرًا على التنبؤ الحتمي بما ستفعله الآلة في كل نبضة ساعة، وأي السجلات ستُستخدم، وأي البتات سيتم تعديلها، وبأي إزاحة في الذاكرة المادية سيتم الوصول إليها، دون أي تحويل ضمني خفي ودون أي وقت تشغيل (Zero Runtime)."**

### الفروق الجوهرية عن اللغات الأخرى:
- **مقارنة مع C:** لغة C تترك سلوك تجاوز سعة الأعداد الصحيحة الموقعة كـ "سلوك غير محدد" (Undefined Behavior - UB)، وتقوم بترقية الأنواع ضمنيًا (Implicit Promotion)، وتسمح بالوصول غير المقيد إلى العتاد دون أذونات. في `I`، لا يوجد سلوك غير محدد، ويجب تحديد نوع المعاملة الحسابية صراحة (`+!` للمفحوص، أو `+%` للمتسلسل)، والعتاد محمي بنظام قدرات صريح (`require cap`).
- **مقارنة مع Rust:** لغة Rust تفرض نموذج ملكية معقد يعتمد على وقت تشغيل قياسي لإدارة الذاكرة الافتراضية. في `I`، الهدف هو البناء المباشر بدون نظام تشغيل (Bare-Metal)، والتحكم بالذاكرة المادية والسجلات الحقيقية للإقلاع وتطوير النواة، مع تجريدات ثنائية صريحة.
- **مقارنة مع لغة التجميع (Assembly):** توفر التجميع تحكمًا مباشرًا ولكنها تفتقر إلى التحقق من صحة الأنواع في وقت الترجمة، ولا توفر تدقيقًا لعقود الأمان، وتختلف بالكامل بين معالج وآخر. تجمع `I` بين الدقة المطلقة للغة التجميع والتحقق الصارم للنظام النمطي ومطابقة التخطيطات الثنائية في وقت الترجمة.

---

## 2. الأساس الثنائي والنموذج الرياضي للبتات (Pure 01 Binary Model)

في لغة `I`، البت ليس مجرد قيمة فرعية داخل عدد صحيح، بل هو **كائن هندسي من الدرجة الأولى**:

### 1. الصيغة الثنائية المباشرة (Literal Binary Constants):
تُكتب القيم الثنائية الصريحة باستخدام البادئة `0b` متبوعة بالبتات `0` و `1` مع إمكانية استخدام الشرطة السفلية `_` للفصل البصري وتحديد العرض الثابت عبر لاحقة النوع:
```i
bind bitmask_8:  b8  = 0b1011_0001:b8;
bind control_32: u32 = 0b00000000_11111111_01010101_10101010:u32;
bind status_16:  u16 = 0xFA4B:u16;
```

### 2. تدفق البتات الثنائي الصافي (`01 Bitstream`):
يقوم مترجم `I` بتحويل الكود مباشرة إلى تسلسل بايتات الآلة، ويمكنه توليد ملف تدفق البتات (`.01`) الذي يعرض كود الآلة كبتات صرفة (0 و 1) لكل بايت:
```
[0x0000 | bit 00000]: 01001000  # 0x48 (REX.W Prefix)
[0x0001 | bit 00008]: 00110001  # 0x31 (XOR Opcode)
[0x0002 | bit 00016]: 11101101  # 0xED (ModR/M: rbp, rbp)
```

---

## 3. نظام الأنواع الدقيق (Honest Machine Type System)

لا يحتوي نظام الأنواع في `I` على أي نوع عائم أو مرن الحجم مثل `int` أو `long` التابعين لـ C. كل نوع يُحدد بدقة عدد البتات المخصصة له في الذاكرة وسجل المعالج:

| النوع في `I` | العرض بالبتات | العرض بالبايت | التمثيل في المعالج | الوصف التقني |
| :--- | :--- | :--- | :--- | :--- |
| `b8` | 8 bits | 1 byte | Byte Register (`AL`, `DL`) | متسلسلة بتات ثنائية خام غير موقعة |
| `b16` | 16 bits | 2 bytes | Word Register (`AX`, `DX`) | متسلسلة بتات ثنائية بعرض 16 بت |
| `b32` | 32 bits | 4 bytes | Double-Word (`EAX`, `EDX`) | متسلسلة بتات ثنائية بعرض 32 بت |
| `b64` | 64 bits | 8 bytes | Quad-Word (`RAX`, `RDX`) | متسلسلة بتات ثنائية بعرض 64 بت |
| `u8` | 8 bits | 1 byte | Unsigned 8-bit [0 .. 255] | عدد صحيح آلي طبيعي غير موقّع |
| `u16` | 16 bits | 2 bytes | Unsigned 16-bit [0 .. 65,535] | عدد صحيح غير موقّع لمعالجة المنافذ |
| `u32` | 32 bits | 4 bytes | Unsigned 32-bit [0 .. 4.29B] | عدد صحيح غير موقّع للإزاحات وسجلات الأجهزة |
| `u64` | 64 bits | 8 bytes | Unsigned 64-bit [0 .. 1.84x10^19] | عدد صحيح غير موقّع للعناوين الفيزيائية 64 بت |
| `i8` | 8 bits | 1 byte | Two's Complement [-128 .. +127] | عدد صحيح موقع بمتمم ثنائي |
| `i16` | 16 bits | 2 bytes | Two's Complement [-32,768 .. +32,767] | عدد صحيح موقع لعينة صوتية أو إشارة |
| `i32` | 32 bits | 4 bytes | Two's Complement [-2.14B .. +2.14B] | عدد صحيح موقع للحسابات العلمية |
| `i64` | 64 bits | 8 bytes | Two's Complement [-9.22x10^18 .. +9.22x10^18] | عدد صحيح موقع لحسابات الفوارق الزمنية |
| `void` | 0 bits | 0 bytes | None | نوع الوحدة الفارغة (عدم إرجاع قيمة) |
| `*raw` | 64 bits | 8 bytes | 64-bit Linear Virtual/Physical Address | مؤشر ذاكرة خام غير منمّط |

---

## 4. المعاملات وحسابات تجاوز السعة الصريحة (Operators & Overflow Semantics)

أحد أكبر مصادر الثغرات الأمنية في هندسة النواة هو **تجاوز سعة الأعداد الصحيحة (Integer Overflow)**. تقضي `I` على هذه المشكلة بإلزام المبرمج باختيار الدلالة الرياضية صراحة:

### 1. العمليات الحسابية المفحوصة (Checked Arithmetic - `!`)
إذا تجاوزت العملية الحد الأقصى أو الأدنى للنوع، يُطلق المعالج فورًا استثناء عتادي أو فخ برمجي (`Trap` / `UD2`) يوقف التنفيذ ويمنع تلف الذاكرة:
- `+!` : جمع مفحوص (Trap on Overflow)
- `-!` : طرح مفحوص (Trap on Underflow)
- `*!` : ضرب مفحوص (Trap on Overflow)

```i
bind a: u8 = 250;
bind b: u8 = 10;
// bind c: u8 = a +! b; // يسبب توقفاً آمناً فورياً (فخ عتادي) لأن 260 تتجاوز 255
```

### 2. العمليات الحسابية المتسلسلة الدورية (Modular Wrapping - `%`)
تلتف القيمة دوريًا وفق الحساب المعياري $2^N$ (مثل عدادات حلقات المعالج وأجهزة التوقيت):
- `+%` : جمع دوري (Modular Wrap)
- `-%` : طرح دوري (Modular Wrap)
- `*%` : ضرب دوري (Modular Wrap)

```i
bind max: u8 = 255;
bind next: u8 = max +% 1; // القيمة الحتمية هي 0 تماماً
```

### 3. العمليات الحسابية الإشباعية (Saturating - `?`)
تلتصق القيمة بالحد الأقصى أو الأدنى دون التفاف (مثالية لمعالجة الإشارات الرقمية DSP والصوت):
- `+?` : جمع إشباعي (Clamped at Maximum)
- `-?` : طرح إشباعي (Clamped at Minimum)

### 4. العمليات الثنائية على مستوى البت (Bitwise Operations):
- `&` : منطق AND الثنائي
- `|` : منطق OR الثنائي
- `^` : منطق XOR الثنائي
- `~` : منطق NOT الثنائي (عكس البتات)
- `<<` : إزاحة منطقية لليسار (Shift Left)
- `>>` : إزاحة لليمين (Shift Right - منطقية للـ `u` وحسابية للـ `i`)
- `<<<` : تدوير دائري لليسار (Rotate Left - `ROL`)
- `>>>` : تدوير دائري لليمين (Rotate Right - `ROR`)

---

## 5. الكلمات المحجوزة والبنية النحوية الكاملة (Grammar & Keywords)

تتميز `I` بكلمات مفتاحية أصلية تبتعد تمامًا عن قوالب اللغات السائدة:

| الكلمة المفتاحية | الدور الوظيفي | مثال الاستخدام |
| :--- | :--- | :--- |
| `system` | تعريف وحدة النظام أو البرنامج | `system OperatingKernel;` |
| `require` | طلب تصريح أو قدرة أمنية | `require cap: hardware_io;` |
| `cap` | تحديد اسم القدرة الأمنية المطلوبة | `require cap: kernel_ring0;` |
| `routine` | تعريف دالة أو إجراء قياسي | `routine calculate(x: u32) -> u32` |
| `kernel_entry` | نقطة دخول النواة المباشرة بدون نظام تشغيل | `kernel_entry _start() -> void` |
| `bind` | ربط قيمة ثابتة غير قابلة للتعديل إطلاقاً | `bind base_addr: u64 = 0x100000;` |
| `mut` | تعريف متغير قابل للتعديل المحلي | `mut counter: u32 = 0;` |
| `branch` | تفرع شرطي (مكافئ لـ if) | `branch (status == 1) { ... }` |
| `otherwise` | المسار البديل للتفرع (مكافئ لـ else) | `otherwise { ... }` |
| `cycle` | حلقة تكرارية محددة الخطوات | `cycle (mut i: u32 = 0; i < 10; i = i +% 1) { ... }` |
| `repeat` | تكرار مستمر حتى تحقق شرط الخروج | `repeat { poll(); } until (ready == 1);` |
| `until` | شرط إنهاء حلقة `repeat` | `until (data_ready == 1);` |
| `yield` | إرجاع قيمة من الإجراء (مكافئ لـ return) | `yield result;` |
| `halt` | تعليمة المعالج لإيقاف التنفيذ (`HLT`) | `halt;` |
| `trap` | إطلاق فخ برمجي عتادي فوري (`UD2`) | `trap 0x01;` |
| `verify` | فحص عقد منطقي ثابت في وقت الترجمة | `verify (sizeof_block == 16);` |
| `layout` | تعريف هيكل ثنائي دقيق الإزاحات | `layout Header { ... }` |
| `bitfield` | تعريف حقول بتات داخل بايت محدد | `bitfield Flags: u8 { ... }` |

---

## 6. تخطيطات الذاكرة المحزومة وحقول البتات (Memory Layouts & Bitfields)

في برمجة النواة والبرامج الثابتة، تتطابق هياكل البيانات مع مسجلات العتاد (Memory-Mapped I/O) أو ترويسات الحزم الثنائية. توفر `I` بنية `layout` التي تجبر المعالج على مطابقة الإزاحات بدقة البايت:

```i
system DeviceController;

layout PciHeader {
    vendor_id:   u16 @ 0x00, // إزاحة صفر
    device_id:   u16 @ 0x02, // إزاحة 2 بايت
    command_reg: u16 @ 0x04, // إزاحة 4 بايت
    status_reg:  u16 @ 0x06, // إزاحة 6 بايت
    revision_id: u8  @ 0x08, // إزاحة 8 بايت
    class_code:  u32 @ 0x09, // إزاحة 9 بايت
}

routine inspect_pci_device(ptr: *raw) -> u16 {
    // التحقق الصارم في وقت الترجمة
    verify (0x02 == 2);
    yield 0;
}
```

---

## 7. نظام الأذونات والقدرات العتادية (Capability-Based Security Model)

لحماية النواة والأنظمة المدمجة من تنفيذ أوامر عتادية خطيرة بطريق الخطأ أو عبر ثغرات حقن الكود، تفرض `I` نظام أذونات صارم يتم التحقق منه داخل المترجم أثناء التحليل الدلالي:

| القدرة الأمنية (`Capability`) | العمليات التي تتيحها | المخاطر العتادية المرتبطة بها |
| :--- | :--- | :--- |
| `hardware_io` | استدعاء منافذ المعالج `@in8`, `@out8`, `@in16`, `@out16` | تعديل حالات المتحكمات العتادية والمنافذ التسلسلية |
| `raw_memory` | الوصول إلى المؤشرات المادية المباشرة `@load32`, `@store32` | الكتابة فوق ذاكرة النواة أو جداول الصفحات |
| `interrupt_control` | تعطيل وتفعيل المقاطعات `@cli`, `@sti` | إيقاف استجابة النظام للمؤقتات ومقاطعات الأجهزة |
| `kernel_ring0` | تنفيذ تعليمات مستوى الامتياز الأعلى مثل `halt`, `@cpuid` | إيقاف المعالج أو قراءة سجلات النواة المتقدمة (MSR) |
| `host_syscall` | استدعاء نداءات نظام التشغيل المضيف `@syscall` | الاتصال بنواة لينكس في البرامج المستضافة |

**مثال على رفض المترجم لكود غير مصرح به:**
```i
system InsecureCode;
// لم يتم كتابة: require cap: hardware_io;

routine main() -> u32 {
    @out8(0x3F8, 0x41); // خطأ في وقت الترجمة!
    yield 0;
}
```
**تقرير المترجم:**
```
test.i:4:5: error[S0002]: Hardware intrinsic '@out8' requires capability 'hardware_io' or 'kernel_ring0'
       = note: Add 'require cap: hardware_io;' at the top of your system unit.
```

---

## 8. الدوال الداخلية لعتاد المعالج (Hardware Intrinsics)

تترجم الدوال الداخلية المسبوقة بـ `@` مباشرة إلى تعليمة معالج أحادية دون أي تكلفة استدعاء:

1. `@out8(port: u16, val: u8)` : إرسال بايت لمنفذ الإدخال/الإخراج (`OUT DX, AL`).
2. `@in8(port: u16) -> u8` : قراءة بايت من منفذ الإدخال/الإخراج (`IN AL, DX`).
3. `@out16(port: u16, val: u16)` : إرسال كلمة 16 بت (`OUT DX, AX`).
4. `@in16(port: u16) -> u16` : قراءة كلمة 16 بت (`IN AX, DX`).
5. `@out32(port: u16, val: u32)` : إرسال كلمة مضاعفة 32 بت (`OUT DX, EAX`).
6. `@in32(port: u16) -> u32` : قراءة كلمة مضاعفة 32 بت (`IN EAX, DX`).
7. `@cli()` : تعطيل المقاطعات العتادية (`CLI` - Clear Interrupt Flag).
8. `@sti()` : تمكين المقاطعات العتادية (`STI` - Set Interrupt Flag).
9. `@hlt()` : إيقاف المعالج المؤقت بانتظار مقاطعة عتادية (`HLT`).
10. `@load32(ptr: *raw) -> u32` : قراءة 32 بت من عنوان فيزيائي مباشر (`MOV EAX, [RDI]`).
11. `@store32(ptr: *raw, val: u32)` : كتابة 32 بت في عنوان فيزيائي مباشر (`MOV [RDI], ESI`).
12. `@syscall(num, arg1, arg2, arg3, arg4, arg5, arg6)` : استدعاء نواة نظام التشغيل المباشر (`SYSCALL`).

---

## 9. معمارية المترجم وسلسلة الأدوات (Compiler Toolchain & CLI)

المترجم `bin/i` هو ملف تنفيذي أصلي مجمع بتقنية x86-64 native machine code:

### صيغة الأوامر المعتمدة:
```bash
# تجميع برنامج إلى ملف تنفيذي أصلي لنظام لينكس ELF64
./bin/i compile <file.i> -o <output_binary>

# تشغيل البرنامج مباشرة
./bin/i run <file.i>

# فحص صحة الشفرة والأنواع والقدرات دون تجميع
./bin/i check <file.i>

# فحص كود الآلة وتفكيك التعليمات وتدفق البتات 01
./bin/i inspect <file.i>

# تجميع صورة إقلاع عتادية مستقلة (Bare-Metal MBR Boot Image)
./bin/i compile <file.i> -o boot.bin --target baremetal --bootsector --dump-01

# تشغيل حزمة الاختبارات الذاتية الشاملة
./bin/i test

# عرض مصفوفة المعماريات والأجهزة المدعومة
./bin/i targets
```

---

## 10. أمثلة عملية كاملة وقابلة للتنفيذ عبر كافة المجالات

### 10.1 المبتدئين والأساسيات (Beginner & Fundamentals)

#### مثال 1: البرنامج الأول
```i
system FirstStep;

require cap: host_syscall;

routine main() -> u32 {
    // إرجاع رمز الخروج 0 للنظام المضيف
    yield 0;
}
```

#### مثال 2: معالجة البتات والتدوير
```i
system BitTwiddling;

routine main() -> u32 {
    // قيم ثنائية واضحة
    bind raw_status: u32 = 0b1010_1100_0011_0001:u32;
    bind mask:       u32 = 0b0000_1111_0000_0000:u32;

    // عزل البتات المطلوبة
    mut extracted: u32 = raw_status & mask;

    // إزاحة لليمين بمقدار 8 خانات
    extracted = extracted >> 8;

    // تدوير دائري لليسار بخانتين
    mut rotated: u32 = extracted <<< 2;

    yield rotated;
}
```

---

### 10.2 معالجة البروتوكولات والشبكات (Binary Networking & Packet Parsing)

```i
system EthernetIpParser;

require cap: raw_memory;

layout EthernetHeader {
    dest_mac:   u64 @ 0x00,
    src_mac:    u64 @ 0x06,
    ether_type: u16 @ 0x0C,
}

layout Ipv4Header {
    ver_ihl:    u8  @ 0x00,
    tos:        u8  @ 0x01,
    length:     u16 @ 0x02,
    packet_id:  u16 @ 0x04,
    flags_frag: u16 @ 0x06,
    ttl:        u8  @ 0x08,
    protocol:   u8  @ 0x09,
    checksum:   u16 @ 0x0A,
    src_ip:     u32 @ 0x0C,
    dst_ip:     u32 @ 0x10,
}

routine extract_ipv4_version(ihl_byte: u8) -> u8 {
    // رقم الإصدار يقع في أعلى 4 بتات [7:4]
    yield ihl_byte >> 4;
}

routine extract_header_length_bytes(ihl_byte: u8) -> u32 {
    // طول الترويسة بالكلمات (IHL) يقع في أدنى 4 بتات [3:0] مضروباً في 4 بايت
    bind words: u32 = (ihl_byte & 0x0F);
    yield words *% 4;
}

routine main() -> u32 {
    bind test_byte: u8 = 0b0100_0101:u8; // IPv4 (4), IHL (5 words = 20 bytes)
    bind version: u8 = extract_ipv4_version(test_byte);
    bind length:  u32 = extract_header_length_bytes(test_byte);

    verify (4 == 4);
    verify (20 == 20);

    yield 0;
}
```

---

### 10.3 هندسة أنظمة التشغيل والنواة (Operating Systems & Bare-Metal Kernel)

```i
system BareMetalKernel;

require cap: hardware_io;
require cap: kernel_ring0;
require cap: raw_memory;

// نقطة دخول النواة عند إقلاع الحاسوب مباشرة من الـ BIOS أو Bootloader
kernel_entry kernel_main() -> void {
    // 1. تعطيل المقاطعات الخارجية لمنع مقاطعة التهيئة الحرجة
    @cli();

    // 2. تهيئة المنفذ التسلسلي COM1 (0x3F8)
    @out8(0x3F9, 0x00); // تعطيل مقاطعات المنفذ التسلسلي
    @out8(0x3FB, 0x80); // تفعيل DLAB لضبط معدل البود
    @out8(0x3F8, 0x03); // ضبط القاسم (38400 بود)
    @out8(0x3F9, 0x00);
    @out8(0x3FB, 0x03); // 8 بتات بيانات، بت توقف واحد، بدون تماثل

    // 3. كتابة رسالة "I-KERNEL OK" إلى المنفذ التسلسلي
    @out8(0x3F8, 0x49); // 'I'
    @out8(0x3F8, 0x2D); // '-'
    @out8(0x3F8, 0x4B); // 'K'
    @out8(0x3F8, 0x45); // 'E'
    @out8(0x3F8, 0x52); // 'R'
    @out8(0x3F8, 0x4E); // 'N'
    @out8(0x3F8, 0x45); // 'E'
    @out8(0x3F8, 0x4C); // 'L'
    @out8(0x3F8, 0x0A); // '\n'

    // 4. إيقاف المعالج بأمان حتى ورود إشارة عتادية
    halt;
}
```

---

### 10.4 مشغلات العتاد والأجهزة (Hardware Drivers & MMIO)

```i
system VgaTextDriver;

require cap: raw_memory;

// شاشة VGA النصية تتواجد في العنوان الفيزيائي 0xB8000
// كل خانة تتكون من بايتين: بايت المحرف ASCII وبايت اللون (السمات)
routine vga_write_char(col: u32, row: u32, ch: u8, color_attr: u8) -> void {
    // الإزاحة = (السطر * 80 + العمود) * 2
    bind offset: u32 = ((row *% 80) +% col) *% 2;
    bind vga_base: u64 = 0x000B8000;
    bind char_addr: u64 = vga_base +% offset;

    // كتابة محرف ASCII ولونه في ذاكرة الفيديو
    @store32(char_addr, (color_attr << 8) | ch);
}

routine main() -> u32 {
    // طباعة المحرف 'I' باللون السماوي الساطع على خلفية سوداء (0x0B)
    vga_write_char(0, 0, 0x49, 0x0B);
    yield 0;
}
```

---

### 10.5 الأمن السيبراني الدفاعي (Defensive Cybersecurity & Constant-Time Primitives)

```i
system DefensiveSecurity;

// مقارنة ثنائية ثابتة زمنياً 100% لمنع هجمات القنوات الجانبية الزمنية (Timing Attacks)
routine secure_constant_time_compare(a: u8, b: u8) -> u8 {
    bind diff: u8 = a ^ b;
    // إذا كانت diff تساوي صفر، فالناتج صفر، وإلا ينتج 1
    bind diff_neg: u8 = 0 -% diff;
    bind has_difference: u8 = (diff | diff_neg) >> 7;

    // إرجاع 1 عند التطابق التام، و0 عند وجود أي اختلاف
    yield 1 ^ has_difference;
}

// فحص سلامة مكدس الذاكرة وحمايته من الطفح (Stack Canary Verification)
routine verify_stack_canary(current_canary: u64, expected_canary: u64) -> void {
    branch (current_canary == expected_canary) {
        // المكدس سليم
        yield;
    } otherwise {
        // اكتشاف تلاعب في المكدس: إيقاف النظام فورياً لمنع تنفيذ كود المهاجم
        trap 0xFF;
    }
}

routine main() -> u32 {
    bind auth_token: u8 = 0xE4;
    bind user_token: u8 = 0xE4;
    bind is_valid:   u8 = secure_constant_time_compare(auth_token, user_token);

    verify_stack_canary(0xDEADBEEFCAFEBABE, 0xDEADBEEFCAFEBABE);

    yield is_valid;
}
```

---

### 10.6 معالجة الإشارات الرقمية (Digital Signal Processing - DSP)

```i
system DigitalSignalProcessor;

// مرشح استجابة نبضية منتهية ثلاثي المراحل (3-Tap FIR Filter)
// يستخدم حسابات النقطة الثابتة Q15 للأداء العالي بدون وحدات فاصلة عائمة
routine fir_filter_tap3(sample: i32, tap0: i32, tap1: i32, tap2: i32, 
                       coeff0: i32, coeff1: i32, coeff2: i32) -> i32 {
    // ضرب العينات في معاملات الفلتر
    bind prod0: i32 = tap0 *% coeff0;
    bind prod1: i32 = tap1 *% coeff1;
    bind prod2: i32 = tap2 *% coeff2;

    // التجميع بدقة عالية
    mut accumulator: i32 = prod0 +% prod1;
    accumulator = accumulator +% prod2;

    // إعادة التدريج بإزاحة 15 بت (Q15 Normalization)
    yield accumulator >> 15;
}

routine main() -> u32 {
    bind input_signal: i32 = 1200;
    bind filtered: i32 = fir_filter_tap3(input_signal, 1200, 1000, 800, 16384, 8192, 4096);
    yield 0;
}
```

---

### 10.7 بنية الذكاء الاصطناعي والحوسبة الكمومية (Low-Level AI & Quantized Tensors)

```i
system LowLevelAiEngine;

// نواة استدلال شبكات عصبية مكممة (INT8 Quantized Matrix-Vector Accumulator)
// تحقق أقصى كفاءة لمعالجات الأجهزة المدمجة والطرفية
routine int8_gemm_accumulate_4(w0: i8, w1: i8, w2: i8, w3: i8,
                              x0: i8, x1: i8, x2: i8, x3: i8) -> i32 {
    // ضرب الأوزان في المدخلات المكممة
    bind p0: i32 = w0 *% x0;
    bind p1: i32 = w1 *% x1;
    bind p2: i32 = w2 *% x2;
    bind p3: i32 = w3 *% x3;

    // مجمع 32 بت لتجنب فيضان القيم
    mut sum: i32 = p0 +% p1;
    sum = sum +% p2;
    sum = sum +% p3;

    yield sum;
}

// دالة التنشيط ReLU المباشرة على مستوى السجل
routine relu_activation(val: i32) -> i32 {
    branch (val > 0) {
        yield val;
    } otherwise {
        yield 0;
    }
}

routine main() -> u32 {
    // حساب ناتج خلية عصبية بأوزان ومدخلات 8-bit
    bind raw_out: i32 = int8_gemm_accumulate_4(2, -3, 5, 1, 10, 20, 5, 12);
    bind activated: i32 = relu_activation(raw_out);

    yield 0;
}
```

---

## 11. مصفوفة المعماريات والمعالجات المدعومة (Architectural Target Matrix)

| المعمارية المستهدفة | عرض الكلمة | ترتيب البايتات | اتفاقية الاستدعاء (ABI) | حالة الدعم الفعلي |
| :--- | :--- | :--- | :--- | :--- |
| `x86_64-linux` | 64-bit | Little Endian | System V AMD64 | **منفذ ومختبر بالكامل بنجاح 100%** |
| `x86_64-baremetal` | 64-bit | Little Endian | Raw Flat Binary / Multiboot | **منفذ ومختبر بالكامل بنجاح 100%** |
| `x86_64-bootmbr` | 16/64-bit | Little Endian | MBR Sector (0x7C00 + 0x55AA) | **منفذ ومختبر بالكامل بنجاح 100%** |
| `aarch64-none` | 64-bit | Little Endian | ARM AAPCS64 | المواصفات مكتملة (جاهز للمرحلة 2) |
| `riscv64-none` | 64-bit | Little Endian | RISC-V LP64 Standard | المواصفات مكتملة (جاهز للمرحلة 2) |
| `arm32-thumb` | 32-bit | Little Endian | ARM AAPCS32 | مخطط له في خريطة الطريق |
| `powerpc64-elf` | 64-bit | Big/Little | OpenPOWER ELFv2 | مخطط له في خريطة الطريق |

---

## 12. استكشاف الأخطاء وتشخيصات المترجم (Diagnostics & Error Codes)

يصدر مترجم `I` تشخيصات دقيقة ذات أسماء رموز موحدة تدل على سبب الخطأ واقتراح الحل:

- `E0001`: محرف غير متوقع في المحلل المعجمي (Unexpected character).
- `P0001`: خطأ نحوي في بناء الجملة (Syntax / Parser token mismatch).
- `P0002`: تعبير حسابي أو منطقي غير صالح داخل الجملة البرمجية.
- `S0001`: استخدام معرف أو متغير غير معرّف (Undefined identifier).
- `S0002`: استدعاء تعليمة عتادية دون التصريح المسبق بالقدرة الأمنية (`require cap: hardware_io`).
- `S0003`: محاولة تنفيذ نداء نظام مباشر دون تصريح `require cap: host_syscall`.
- `S0005`: محاولة إسناد قيمة لمتغير غير موجود.
- `S0006`: محاولة تعديل قيمة ثابتة تم تعريفها بـ `bind` بدلاً من `mut` (Immutability violation).
- `S0007`: تعبير في جملة `verify` غير قابل للتقييم في وقت الترجمة.
- `S0008`: فشل عقد التحقق الثابت في وقت الترجمة (`verify assertion is FALSE`).
