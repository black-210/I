import * as vscode from 'vscode';

interface IntellisenseDoc {
  label: string;
  kind: vscode.CompletionItemKind;
  detail: string;
  docArabic: string;
  docEnglish: string;
  example: string;
  insertText?: string;
}

const ITEMS: IntellisenseDoc[] = [
  // --- KEYWORDS ---
  {
    label: 'system',
    kind: vscode.CompletionItemKind.Keyword,
    detail: 'system <Identifier>;',
    docArabic: 'تعريف وحدة النظام أو البرنامج الرئيسي في لغة I. كل ملف مصدري يبدأ بتعريف النظام والقدرات العتادية المطلوبة.',
    docEnglish: 'Declares a top-level compilation unit or system in Language I. Defines module scope.',
    example: 'system OperatingKernel;\nrequire cap: hardware_io;\n'
  },
  {
    label: 'require',
    kind: vscode.CompletionItemKind.Keyword,
    detail: 'require cap: <capability>;',
    docArabic: 'طلب تصريح أو قدرة أمنية صريحة في وقت الترجمة. يرفض المترجم أي استدعاء عتادي دون هذا التصريح مسبقاً.',
    docEnglish: 'Requires an explicit compile-time security capability token for hardware and privileged operations.',
    example: 'require cap: hardware_io;\nrequire cap: kernel_ring0;\n'
  },
  {
    label: 'cap',
    kind: vscode.CompletionItemKind.Keyword,
    detail: 'cap: hardware_io | raw_memory | interrupt_control | kernel_ring0 | host_syscall',
    docArabic: 'رمز القدرة الأمنية. يفصل بين كود التطبيق العادي والعمليات ذات الامتيازات العالية في النواة.',
    docEnglish: 'Security capability specifier token used in require statements.',
    example: 'require cap: raw_memory;\n'
  },
  {
    label: 'routine',
    kind: vscode.CompletionItemKind.Function,
    detail: 'routine <name>(<params>) -> <ReturnType> { ... }',
    docArabic: 'تعريف إجراء أو دالة فرعية حتمية. تتبع اصطلاح الاستدعاء القياسي System V AMD64 مع إرجاع القيمة في RAX.',
    docEnglish: 'Defines a deterministic subroutine. Emits standard x86-64 stack frame and registers.',
    example: 'routine calculate(a: u32, b: u32) -> u32 {\n    yield a +% b;\n}'
  },
  {
    label: 'kernel_entry',
    kind: vscode.CompletionItemKind.Function,
    detail: 'kernel_entry <name>() -> void { ... }',
    docArabic: 'نقطة دخول النواة والبرامج المستقلة بدون نظام تشغيل (Bare-Metal). تبدأ التنفيذ المباشر من أول بايت وتنهي بـ halt.',
    docEnglish: 'Declares a freestanding entry point for bare-metal OS kernels, bootloaders, or firmware.',
    example: 'kernel_entry boot_start() -> void {\n    @cli();\n    halt;\n}'
  },
  {
    label: 'bind',
    kind: vscode.CompletionItemKind.Variable,
    detail: 'bind <name>: <type> = <value>;',
    docArabic: 'تعريف قيمة ثابتة رياضياً غير قابلة للتعديل إطلاقاً. يمنع المترجم أي تعديل عليها لمنع الآثار الجانبية غير المتوقعة.',
    docEnglish: 'Declares an immutable constant binding. Strictly enforced at compile-time.',
    example: 'bind pci_base: u64 = 0xE0000000;\nbind divisor: u16 = 3;'
  },
  {
    label: 'mut',
    kind: vscode.CompletionItemKind.Variable,
    detail: 'mut <name>: <type> = <initial_value>;',
    docArabic: 'تعريف متغير محلي قابل للتعديل داخل الإجراء. يتم تخصيص مساحة له في مكدس المعالج بالنسبة لسجل RBP.',
    docEnglish: 'Declares a mutable stack-allocated local variable with explicit bit width.',
    example: 'mut counter: u32 = 0;\ncounter = counter +% 1;'
  },
  {
    label: 'branch',
    kind: vscode.CompletionItemKind.Keyword,
    detail: 'branch (<condition>) { ... } otherwise { ... }',
    docArabic: 'تفرع شرطي حتمي (بديل if/else). يترجم مباشرة لتعليمات المقارنة CMP والقفز المشروط JZ/JNZ.',
    docEnglish: 'Deterministic conditional branch. Compiles to CPU test and jump instructions.',
    example: 'branch (status == 1) {\n    yield 100;\n} otherwise {\n    yield 0;\n}'
  },
  {
    label: 'otherwise',
    kind: vscode.CompletionItemKind.Keyword,
    detail: 'otherwise { ... }',
    docArabic: 'المسار البديل للتفرع في حال كان الشرط خاطئاً.',
    docEnglish: 'Else branch executed when conditional test fails.',
    example: 'branch (ready == 1) { yield 1; } otherwise { yield 0; }'
  },
  {
    label: 'cycle',
    kind: vscode.CompletionItemKind.Keyword,
    detail: 'cycle (mut i: u32 = 0; i < N; i = i +% 1) { ... }',
    docArabic: 'حلقة تكرار منتظمة محددة البداية والنهاية والخطوة. لا تسمح بالحلقات غير المنتهية المخفية.',
    docEnglish: 'Structured cycle loop with explicit counter initialization, condition, and step.',
    example: 'cycle (mut i: u32 = 0; i < 8; i = i +% 1) {\n    @out8(0x3F8, i);\n}'
  },
  {
    label: 'repeat',
    kind: vscode.CompletionItemKind.Keyword,
    detail: 'repeat { ... } until (<condition>);',
    docArabic: 'حلقة تكرار تنفذ كتلة الأوامر أولاً ثم تفحص شرط التوقف في النهاية.',
    docEnglish: 'Repeat-until loop that evaluates termination condition at the end of each iteration.',
    example: 'repeat {\n    poll_hardware();\n} until (is_ready == 1);'
  },
  {
    label: 'until',
    kind: vscode.CompletionItemKind.Keyword,
    detail: 'until (<condition>);',
    docArabic: 'شرط انتهاء حلقة repeat.',
    docEnglish: 'Termination clause for repeat loop.',
    example: 'repeat { step(); } until (done == 1);'
  },
  {
    label: 'yield',
    kind: vscode.CompletionItemKind.Keyword,
    detail: 'yield <expr>;',
    docArabic: 'إرجاع القيمة من الإجراء (مكافئ return). تضع القيمة في سجل RAX وتستعيد المكدس وتنفذ RET.',
    docEnglish: 'Returns from subroutine, loading result into RAX register and executing RET.',
    example: 'yield 42;'
  },
  {
    label: 'halt',
    kind: vscode.CompletionItemKind.Keyword,
    detail: 'halt;',
    docArabic: 'تعليمة المعالج المباشرة HLT (0xF4). توقف استهلاك المعالج وتنتظر ورود مقاطعة عتادية.',
    docEnglish: 'Emits x86 HLT (0xF4) instruction to suspend processor core until next interrupt.',
    example: 'halt;'
  },
  {
    label: 'trap',
    kind: vscode.CompletionItemKind.Keyword,
    detail: 'trap <code_u8>;',
    docArabic: 'إطلاق فخ برمجي عتادي فوري UD2 (0x0F 0x0B). يوقف المعالج عند فشل عقد أمني غير قابل للاستمرار.',
    docEnglish: 'Triggers immediate processor trap via undefined instruction (UD2).',
    example: 'trap 0x01;'
  },
  {
    label: 'verify',
    kind: vscode.CompletionItemKind.Keyword,
    detail: 'verify (<const_expression>);',
    docArabic: 'فحص عقد منطقي ثابت في وقت الترجمة (Static Assertion). إذا كان الناتج 0 يفشل التجميع فوراً مع كود S0008.',
    docEnglish: 'Compile-time static contract verification. Aborts build if expression evaluates to false.',
    example: 'verify (sizeof_pci_hdr == 16);\nverify (0x1000 == 4096);'
  },
  {
    label: 'layout',
    kind: vscode.CompletionItemKind.Class,
    detail: 'layout <Name> { field: type @ 0xOffset, ... }',
    docArabic: 'تعريف هيكل بيانات ثنائي محزوم مع إزاحات صريحة بالبايت والبت. يتطابق حرفياً مع مسجلات العتاد أو ترويسات الشبكة.',
    docEnglish: 'Packed binary structure layout with explicit byte/bit offsets matching hardware registers.',
    example: 'layout UartPort {\n    rx_tx:  u8 @ 0x00,\n    status: u8 @ 0x05,\n}'
  },

  // --- TYPES ---
  {
    label: 'u8',
    kind: vscode.CompletionItemKind.TypeParameter,
    detail: 'u8: 8-bit Unsigned Integer [0 .. 255]',
    docArabic: 'عدد صحيح آلي غير موقّع بعرض 8 بت (بايت واحد). يشغل سجل AL في المعالج.',
    docEnglish: '8-bit unsigned machine integer (1 byte). Maps directly to AL register.',
    example: 'bind port_val: u8 = 0xFF;'
  },
  {
    label: 'u16',
    kind: vscode.CompletionItemKind.TypeParameter,
    detail: 'u16: 16-bit Unsigned Integer [0 .. 65,535]',
    docArabic: 'عدد صحيح غير موقّع بعرض 16 بت (كلمة). مثالي لعناوين منافذ الإدخال والإخراج I/O ports.',
    docEnglish: '16-bit unsigned machine word. Ideal for hardware I/O ports (0x0000-0xFFFF).',
    example: 'bind com1_port: u16 = 0x03F8;'
  },
  {
    label: 'u32',
    kind: vscode.CompletionItemKind.TypeParameter,
    detail: 'u32: 32-bit Unsigned Integer [0 .. 4,294,967,295]',
    docArabic: 'عدد صحيح غير موقّع بعرض 32 بت (كلمة مضاعفة). يشغل سجل EAX.',
    docEnglish: '32-bit unsigned double-word integer. Standard machine word for offsets.',
    example: 'bind max_packets: u32 = 1000000;'
  },
  {
    label: 'u64',
    kind: vscode.CompletionItemKind.TypeParameter,
    detail: 'u64: 64-bit Unsigned Integer [0 .. 18.4 Quintillion]',
    docArabic: 'عدد صحيح غير موقّع بعرض 64 بت (كلمة رباعية). يمثل العناوين الفيزيائية الكاملة في معمارية 64-bit.',
    docEnglish: '64-bit unsigned quad-word integer. Represents 64-bit physical memory addresses.',
    example: 'bind phys_entry: u64 = 0x00100000;'
  },
  {
    label: 'i8',
    kind: vscode.CompletionItemKind.TypeParameter,
    detail: 'i8: 8-bit Signed Two\'s Complement [-128 .. +127]',
    docArabic: 'عدد صحيح موقع بعرض 8 بت بمتمم ثنائي. مستخدم في أوزان شبكات الذكاء الاصطناعي المكممة INT8.',
    docEnglish: '8-bit signed two\'s complement integer. Used for INT8 quantized AI weights.',
    example: 'bind weight: i8 = -12;'
  },
  {
    label: 'i32',
    kind: vscode.CompletionItemKind.TypeParameter,
    detail: 'i32: 32-bit Signed Two\'s Complement',
    docArabic: 'عدد صحيح موقع بعرض 32 بت. مستخدم في حسابات معالجة الإشارات DSP ومجمعات الذكاء الاصطناعي.',
    docEnglish: '32-bit signed integer. Standard for DSP signal processing accumulators.',
    example: 'mut acc: i32 = sample *% coeff;'
  },
  {
    label: 'b8',
    kind: vscode.CompletionItemKind.TypeParameter,
    detail: 'b8: 8-bit Raw Bitvector',
    docArabic: 'متسلسلة ثنائية خام بعرض 8 بت للتعامل مع البتات ككائنات منطقية صريحة.',
    docEnglish: 'Explicit 8-bit raw bit sequence.',
    example: 'bind mask: b8 = 0b1100_0011:b8;'
  },
  {
    label: '*raw',
    kind: vscode.CompletionItemKind.TypeParameter,
    detail: '*raw: Opaque 64-bit Memory Pointer',
    docArabic: 'مؤشر ذاكرة خطي خام بعرض 64 بت. يتطلب تصريح raw_memory للتعامل معه.',
    docEnglish: 'Opaque 64-bit linear memory pointer. Requires raw_memory capability.',
    example: 'routine inspect_buffer(ptr: *raw) -> void'
  },

  // --- OPERATORS ---
  {
    label: '+!',
    kind: vscode.CompletionItemKind.Operator,
    detail: '+! : Checked Addition (Trap on Overflow)',
    docArabic: 'جمع مفحوص: إذا تجاوز الناتج سعة النوع، يُطلق المعالج فوراً فخاً برمجياً يوقف التنفيذ لحماية سلامة الذاكرة.',
    docEnglish: 'Checked addition operator. Traps immediately upon arithmetic overflow.',
    example: 'bind safe_sum: u8 = val1 +! val2;'
  },
  {
    label: '+%',
    kind: vscode.CompletionItemKind.Operator,
    detail: '+% : Modular Wrapping Addition (Modulo 2^N)',
    docArabic: 'جمع متسلسل دوري: عند التجاوز يلتف الناتج دورياً وفق 2 أس N (حسابي حتمي للعدادات والمؤقتات).',
    docEnglish: 'Modular wrapping addition. Guarantees deterministic two\'s complement wrap.',
    example: 'mut next_seq: u32 = seq +% 1;'
  },
  {
    label: '+?',
    kind: vscode.CompletionItemKind.Operator,
    detail: '+? : Saturating Addition (Clamped at Boundary)',
    docArabic: 'جمع إشباعي: يلتصق الناتج بالحد الأقصى أو الأدنى للنوع دون تجاوز أو التفاف (مثالي للصوت ومعالجة الإشارات DSP).',
    docEnglish: 'Saturating addition operator. Clamps value at maximum limit without wrapping.',
    example: 'bind saturated_sample: u8 = audio1 +? audio2;'
  },
  {
    label: '<<<',
    kind: vscode.CompletionItemKind.Operator,
    detail: '<<< : Rotate Bits Left (ROL)',
    docArabic: 'تدوير ثنائي دائري لليسار: البتات الخارجة من الطرف الأيسر تعود للدخول من الطرف الأيمن.',
    docEnglish: 'Circular bitwise rotate left instruction (x86 ROL).',
    example: 'bind rotated: u32 = reg <<< 4;'
  },
  {
    label: '>>>',
    kind: vscode.CompletionItemKind.Operator,
    detail: '>>> : Rotate Bits Right (ROR)',
    docArabic: 'تدوير ثنائي دائري لليمين: البتات الخارجة من الطرف الأيمن تعود للدخول من الطرف الأيسر.',
    docEnglish: 'Circular bitwise rotate right instruction (x86 ROR).',
    example: 'bind rotated: u32 = reg >>> 2;'
  },

  // --- INTRINSICS ---
  {
    label: '@out8',
    kind: vscode.CompletionItemKind.Method,
    detail: '@out8(port: u16, val: u8) -> void',
    docArabic: 'إرسال بايت لمنفذ إدخال/إخراج عتادي مباشر (تعليمة OUT DX, AL). يتطلب تصريح hardware_io.',
    docEnglish: 'Direct x86 OUT DX, AL instruction. Writes 8-bit value to hardware port.',
    example: '@out8(0x3F8, 0x41); // Write \'A\' to COM1'
  },
  {
    label: '@in8',
    kind: vscode.CompletionItemKind.Method,
    detail: '@in8(port: u16) -> u8',
    docArabic: 'قراءة بايت من منفذ إدخال/إخراج عتادي مباشر (تعليمة IN AL, DX). يتطلب تصريح hardware_io.',
    docEnglish: 'Direct x86 IN AL, DX instruction. Reads 8-bit value from hardware port.',
    example: 'bind status: u8 = @in8(0x3FD);'
  },
  {
    label: '@cli',
    kind: vscode.CompletionItemKind.Method,
    detail: '@cli() -> void',
    docArabic: 'تعطيل المقاطعات العتادية بالمعالج (تعليمة CLI). يتطلب تصريح interrupt_control أو kernel_ring0.',
    docEnglish: 'Emits x86 CLI instruction to clear interrupt flag in EFLAGS register.',
    example: '@cli();'
  },
  {
    label: '@sti',
    kind: vscode.CompletionItemKind.Method,
    detail: '@sti() -> void',
    docArabic: 'تفعيل المقاطعات العتادية بالمعالج (تعليمة STI). يتطلب تصريح interrupt_control أو kernel_ring0.',
    docEnglish: 'Emits x86 STI instruction to set interrupt flag in EFLAGS register.',
    example: '@sti();'
  },
  {
    label: '@load32',
    kind: vscode.CompletionItemKind.Method,
    detail: '@load32(ptr_addr: u64) -> u32',
    docArabic: 'قراءة كلمة 32 بت من عنوان ذاكرة مباشر. يتطلب تصريح raw_memory.',
    docEnglish: 'Reads 32-bit integer directly from raw physical/virtual address.',
    example: 'bind val: u32 = @load32(0x000B8000);'
  },
  {
    label: '@store32',
    kind: vscode.CompletionItemKind.Method,
    detail: '@store32(ptr_addr: u64, val: u32) -> void',
    docArabic: 'كتابة كلمة 32 بت في عنوان ذاكرة مباشر. يتطلب تصريح raw_memory.',
    docEnglish: 'Writes 32-bit integer directly to raw physical/virtual address.',
    example: '@store32(0x000B8000, 0x0F480F49);'
  },
  {
    label: '@syscall',
    kind: vscode.CompletionItemKind.Method,
    detail: '@syscall(num, arg1, arg2, arg3, arg4, arg5) -> u64',
    docArabic: 'استدعاء نداء نظام لينكس مباشر عبر تعليمة SYSCALL (0x0F 0x05). يتطلب تصريح host_syscall.',
    docEnglish: 'Invokes Linux x86-64 direct kernel syscall instruction (0x0F 0x05).',
    example: '@syscall(60, 0, 0, 0); // sys_exit(0)'
  }
];

export function activate(context: vscode.ExtensionContext) {
  // 1. CompletionItemProvider (IntelliSense Auto-Complete)
  const completionProvider = vscode.languages.registerCompletionItemProvider(
    'i',
    {
      provideCompletionItems(document: vscode.TextDocument, position: vscode.Position) {
        const linePrefix = document.lineAt(position).text.substr(0, position.character);
        const completionItems: vscode.CompletionItem[] = [];

        for (const item of ITEMS) {
          const ci = new vscode.CompletionItem(item.label, item.kind);
          ci.detail = item.detail;
          
          const md = new vscode.MarkdownString();
          md.appendMarkdown(`### \`${item.label}\`\n\n`);
          md.appendMarkdown(`**الوصف بالعربية:** ${item.docArabic}\n\n`);
          md.appendMarkdown(`**Description:** ${item.docEnglish}\n\n`);
          md.appendCodeblock(item.example, 'i');
          ci.documentation = md;

          if (item.insertText) {
            ci.insertText = item.insertText;
          }
          completionItems.push(ci);
        }

        return completionItems;
      }
    },
    '@', ':', '+', '-', '*', '/', '%', '<', '>', '&', '|', '^', '~'
  );

  // 2. HoverProvider (Rich Tooltip Explanation on Hover)
  const hoverProvider = vscode.languages.registerHoverProvider('i', {
    provideHover(document: vscode.TextDocument, position: vscode.Position) {
      const range = document.getWordRangeAtPosition(position, /[@#a-zA-Z0-9_+%!?-]+/);
      if (!range) return null;
      const word = document.getText(range);

      const found = ITEMS.find(item => item.label === word);
      if (found) {
        const md = new vscode.MarkdownString();
        md.appendMarkdown(`## Language I: \`${found.label}\`\n`);
        md.appendMarkdown(`*${found.detail}*\n\n---\n\n`);
        md.appendMarkdown(`**شرح المفهوم:** ${found.docArabic}\n\n`);
        md.appendMarkdown(`**Specification:** ${found.docEnglish}\n\n`);
        md.appendMarkdown(`**مثال برمجي (Code Example):**\n`);
        md.appendCodeblock(found.example, 'i');
        return new vscode.Hover(md);
      }
      return null;
    }
  });

  context.subscriptions.push(completionProvider, hoverProvider);
}

export function deactivate() {}
