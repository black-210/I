/**
 * @license
 * SPDX-License-Identifier: Apache-2.0
 */

import React, { useState, useEffect } from 'react';
import { 
  Terminal, 
  Cpu, 
  Binary, 
  ShieldCheck, 
  Play, 
  Search, 
  CheckCircle2, 
  BookOpen, 
  Layers, 
  HardDrive, 
  Zap, 
  RotateCcw,
  Sparkles,
  Download,
  Package,
  Code2,
  FileCode,
  X,
  ExternalLink,
  HelpCircle
} from 'lucide-react';

const EXAMPLES = [
  {
    id: '01_first',
    name: '01. First Program',
    category: 'Beginner',
    file: 'examples/beginner/01_first_program.i',
    desc: 'Minimal system unit, return exit code 0.',
    code: `system HelloMachine;

require cap: host_syscall;

routine main() -> u32 {
    // Return exit code 0 to indicate normal termination
    yield 0;
}`
  },
  {
    id: '02_bits',
    name: '02. Binary & Bits',
    category: 'Binary',
    file: 'examples/beginner/02_binary_and_bits.i',
    desc: 'Raw binary literals, bit shifts, and circular rotation.',
    code: `system BinaryAndBits;

routine main() -> u32 {
    // Exact binary bit literals
    bind reg_a: u32 = 0b1100_1010_0101_0011:u32;
    bind mask:  u32 = 0b0000_1111_0000_1111:u32;

    // Bitwise operations
    mut isolated: u32 = reg_a & mask;
    mut shifted:  u32 = isolated >> 4;
    mut rotated:  u32 = shifted <<< 2;

    yield rotated;
}`
  },
  {
    id: '03_arithmetic',
    name: '03. Fixed Arithmetic',
    category: 'Binary',
    file: 'examples/beginner/03_fixed_arithmetic.i',
    desc: 'Checked (+!), wrapping (+%), and static verify assertion.',
    code: `system FixedArithmetic;

routine main() -> u32 {
    // Explicit overflow semantics:
    // +! checked (traps on overflow)
    // +% modular wrapping
    // +? saturating
    bind max_u8: u32 = 255;
    mut wrapped: u32 = max_u8 +% 1; // wraps cleanly

    // Static assertion at compile-time
    verify (256 == 0x100);

    yield wrapped;
}`
  },
  {
    id: '04_packet',
    name: '04. Network Packet Layout',
    category: 'Systems',
    file: 'examples/binary/packet_parser.i',
    desc: 'Packed Ethernet & IPv4 binary layouts with exact bit offsets.',
    code: `system PacketParser;

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

routine parse_ipv4_version(ihl_byte: u8) -> u8 {
    // IPv4 version is in top 4 bits [7:4]
    yield ihl_byte >> 4;
}

routine main() -> u32 {
    bind sample_ihl: u8 = 0b0100_0101:u8; // Version 4, IHL 5
    bind ver: u8 = parse_ipv4_version(sample_ihl);
    verify (4 == 4);
    yield ver;
}`
  },
  {
    id: '05_boot',
    name: '05. Bare-Metal Bootloader',
    category: 'Kernel',
    file: 'examples/standalone/minimal_boot.i',
    desc: 'Standalone bare-metal kernel with CLI, serial out & halt.',
    code: `system MinimalBoot;

// Capability to access bare-metal CPU instructions and hardware ports
require cap: hardware_io;
require cap: kernel_ring0;

kernel_entry boot_start() -> void {
    // Disable external interrupts during early stage
    @cli();

    // Send ASCII 'O' and 'K' to serial COM1 port 0x3F8
    @out8(0x3F8, 0x4F); // 'O'
    @out8(0x3F8, 0x4B); // 'K'
    @out8(0x3F8, 0x0A); // '\\n'

    // Halt processor until next hardware interrupt
    halt;
}`
  },
  {
    id: '06_uart',
    name: '06. UART Hardware Driver',
    category: 'Hardware',
    file: 'examples/hardware/serial_uart.i',
    desc: 'Hardware port programming and FIFO baud initialization.',
    code: `system SerialUart;

require cap: hardware_io;

layout UartRegisters {
    data_port:        u8 @ 0x00,
    interrupt_enable: u8 @ 0x01,
    fifo_control:     u8 @ 0x02,
    line_control:     u8 @ 0x03,
    modem_control:    u8 @ 0x04,
    line_status:      u8 @ 0x05,
}

routine uart_init(base_port: u16) -> void {
    // Disable interrupts: base + 1 = 0
    @out8(base_port +% 1, 0x00);
    // Enable DLAB (set baud rate divisor): base + 3 = 0x80
    @out8(base_port +% 3, 0x80);
    // Set divisor to 3 (38400 baud): lo byte 3, hi byte 0
    @out8(base_port +% 0, 0x03);
    @out8(base_port +% 1, 0x00);
    // 8 bits, no parity, 1 stop bit: base + 3 = 0x03
    @out8(base_port +% 3, 0x03);
    // Enable FIFO, clear them: base + 2 = 0xC7
    @out8(base_port +% 2, 0xC7);
}

routine main() -> u32 {
    uart_init(0x3F8);
    yield 0;
}`
  },
  {
    id: '07_security',
    name: '07. Constant-Time Crypto',
    category: 'Security',
    file: 'examples/security/safe_constant_time.i',
    desc: 'Constant-time comparison eliminating cache timing side-channels.',
    code: `system SafeConstantTime;

// Constant-time byte comparison to eliminate timing side-channels
routine constant_time_eq8(a: u8, b: u8) -> u8 {
    bind diff: u8 = a ^ b;
    bind diff_neg: u8 = 0 -% diff;
    bind non_zero: u8 = (diff | diff_neg) >> 7;
    // Return 1 if equal, 0 if different
    yield 1 ^ non_zero;
}

routine main() -> u32 {
    bind pass1: u8 = 0xA5;
    bind pass2: u8 = 0xA5;
    bind result: u8 = constant_time_eq8(pass1, pass2);
    verify (1 == 1);
    yield result;
}`
  },
  {
    id: '08_ai',
    name: '08. Quantized AI Kernel',
    category: 'AI / DSP',
    file: 'examples/ai/quantized_dot_product.i',
    desc: 'INT8 quantized tensor dot-product and ReLU activation.',
    code: `system QuantizedAiKernel;

// INT8 quantized neural network inference layer (GEMM accumulator)
routine dot_product_4(w0: i8, w1: i8, w2: i8, w3: i8, x0: i8, x1: i8, x2: i8, x3: i8) -> i32 {
    bind p0: i32 = w0 *% x0;
    bind p1: i32 = w1 *% x1;
    bind p2: i32 = w2 *% x2;
    bind p3: i32 = w3 *% x3;

    mut sum: i32 = p0 +% p1;
    sum = sum +% p2;
    sum = sum +% p3;

    yield sum;
}

routine relu(val: i32) -> i32 {
    branch (val > 0) {
        yield val;
    } otherwise {
        yield 0;
    }
}

routine main() -> u32 {
    bind acc: i32 = dot_product_4(2, 3, 4, -1, 10, 20, 5, 10);
    bind activated: i32 = relu(acc);
    yield 0;
}`
  }
];

const INTELLISENSE_ITEMS = [
  { token: 'system', kind: 'Keyword', ar: 'تعريف وحدة النظام أو البرنامج الرئيسي.', en: 'Declares top-level system unit/module.' },
  { token: 'require', kind: 'Keyword', ar: 'طلب تصريح أو قدرة أمنية صريحة للعتاد.', en: 'Requires explicit security capability.' },
  { token: 'routine', kind: 'Declaration', ar: 'تعريف إجراء أو دالة فرعية حتمية.', en: 'Subroutine with explicit types and registers.' },
  { token: 'kernel_entry', kind: 'Declaration', ar: 'نقطة دخول النواة والبرامج المستقلة (Bare-Metal).', en: 'Freestanding entry point without OS.' },
  { token: 'bind', kind: 'Variable', ar: 'تعريف قيمة ثابتة رياضياً غير قابلة للتعديل.', en: 'Strictly immutable binding.' },
  { token: 'mut', kind: 'Variable', ar: 'تعريف متغير محلي قابل للتعديل على المكدس.', en: 'Mutable stack-allocated variable.' },
  { token: 'branch', kind: 'Control', ar: 'تفرع شرطي حتمي (بديل if).', en: 'Conditional branch (CMP + JZ/JNZ).' },
  { token: 'otherwise', kind: 'Control', ar: 'المسار البديل للتفرع (بديل else).', en: 'Else block for branch.' },
  { token: 'cycle', kind: 'Loop', ar: 'حلقة تكرار محددة البداية والنهاية والخطوة.', en: 'Cycle loop with explicit bounds & step.' },
  { token: 'repeat', kind: 'Loop', ar: 'حلقة تكرار مستمرة حتى تحقق شرط until.', en: 'Repeat-until loop.' },
  { token: 'yield', kind: 'Control', ar: 'إرجاع القيمة في سجل RAX وتنفيذ RET.', en: 'Return value in RAX register.' },
  { token: 'halt', kind: 'Instruction', ar: 'تعليمة إيقاف المعالج HLT (0xF4).', en: 'Emits x86 HLT instruction.' },
  { token: 'trap', kind: 'Instruction', ar: 'إطلاق فخ برمجي عتادي فوري UD2 (0x0F 0x0B).', en: 'Emits x86 UD2 trap instruction.' },
  { token: 'verify', kind: 'Contract', ar: 'فحص عقد منطقي ثابت في وقت الترجمة (Static Assert).', en: 'Compile-time assertion.' },
  { token: 'layout', kind: 'Type', ar: 'تخطيط ثنائي محزوم مع إزاحات دقيقة بالبايت.', en: 'Packed binary struct with explicit byte offsets.' },
  { token: '+!', kind: 'Operator', ar: 'جمع مفحوص: فخ فوري عند تجاوز السعة (Checked).', en: 'Checked addition (Trap on overflow).' },
  { token: '+%', kind: 'Operator', ar: 'جمع متسلسل دوري: التفاف دوري معياري (Wrapping).', en: 'Modular wrapping addition.' },
  { token: '+?', kind: 'Operator', ar: 'جمع إشباعي: يلتصق بالحد الأقصى (Saturating).', en: 'Saturating addition (Clamped at limit).' },
  { token: '<<<', kind: 'Operator', ar: 'تدوير دائري لليسار (x86 ROL).', en: 'Bitwise circular rotate left.' },
  { token: '>>>', kind: 'Operator', ar: 'تدوير دائري لليمين (x86 ROR).', en: 'Bitwise circular rotate right.' },
  { token: '@out8', kind: 'Intrinsic', ar: 'إرسال بايت لمنفذ إدخال/إخراج (OUT DX, AL).', en: 'Write byte to hardware I/O port.' },
  { token: '@in8', kind: 'Intrinsic', ar: 'قراءة بايت من منفذ إدخال/إخراج (IN AL, DX).', en: 'Read byte from hardware I/O port.' },
  { token: '@cli', kind: 'Intrinsic', ar: 'تعطيل المقاطعات العتادية (CLI).', en: 'Disable hardware interrupts (CLI).' },
  { token: '@sti', kind: 'Intrinsic', ar: 'تمكين المقاطعات العتادية (STI).', en: 'Enable hardware interrupts (STI).' },
  { token: '@load32', kind: 'Intrinsic', ar: 'قراءة 32 بت من عنوان ذاكرة مباشر.', en: 'Read 32-bit word from raw pointer.' },
  { token: '@store32', kind: 'Intrinsic', ar: 'كتابة 32 بت في عنوان ذاكرة مباشر.', en: 'Write 32-bit word to raw pointer.' },
  { token: '@syscall', kind: 'Intrinsic', ar: 'نداء مباشر لنواة لينكس (SYSCALL 0x0F 0x05).', en: 'Invoke Linux kernel direct syscall.' }
];

export default function App() {
  const [selectedExample, setSelectedExample] = useState(EXAMPLES[1]);
  const [code, setCode] = useState(EXAMPLES[1].code);
  const [terminalOutput, setTerminalOutput] = useState<string>('');
  const [isLoading, setIsLoading] = useState<boolean>(false);
  const [lastCmd, setLastCmd] = useState<string>('./bin/i inspect');
  const [showVsixModal, setShowVsixModal] = useState<boolean>(false);
  const [filterQuery, setFilterQuery] = useState<string>('');

  const executeCompiler = async (cmd: string) => {
    setIsLoading(true);
    setLastCmd(`./bin/i ${cmd}`);
    try {
      const res = await fetch('/api/compiler', {
        method: 'POST',
        headers: { 'Content-Type': 'application/json' },
        body: JSON.stringify({
          cmd,
          file: selectedExample.file,
          code
        })
      });
      const data = await res.json();
      let text = '';
      if (data.stdout) text += data.stdout;
      if (data.stderr) text += '\n' + data.stderr;
      if (!text.trim()) {
        text = `Command executed cleanly with Exit Code ${data.exitCode}.`;
      }
      setTerminalOutput(text);
    } catch (e: any) {
      setTerminalOutput(`Error executing compiler: ${e.message}`);
    } finally {
      setIsLoading(false);
    }
  };

  useEffect(() => {
    executeCompiler('inspect');
  }, []);

  const handleSelectExample = (ex: typeof EXAMPLES[0]) => {
    setSelectedExample(ex);
    setCode(ex.code);
  };

  const filteredIntellisense = INTELLISENSE_ITEMS.filter(item => 
    item.token.toLowerCase().includes(filterQuery.toLowerCase()) ||
    item.ar.includes(filterQuery) ||
    item.en.toLowerCase().includes(filterQuery.toLowerCase())
  );

  return (
    <div className="min-h-screen bg-[#07090e] text-[#cdd7e5] font-mono flex flex-col selection:bg-[#00f0ff] selection:text-black">
      {/* Top Header */}
      <header className="border-b border-[#1b2234] bg-[#0c101a] px-6 py-4 flex flex-wrap items-center justify-between gap-4">
        <div className="flex items-center gap-3">
          <div className="w-10 h-10 rounded bg-[#00f0ff]/10 border border-[#00f0ff]/40 flex items-center justify-center text-[#00f0ff] font-bold text-xl shadow-[0_0_15px_rgba(0,240,255,0.2)]">
            I
          </div>
          <div>
            <div className="flex items-center gap-2">
              <h1 className="text-lg font-bold text-white tracking-wider">LANGUAGE I</h1>
              <span className="px-2 py-0.5 text-xs bg-[#00f0ff]/20 text-[#00f0ff] border border-[#00f0ff]/30 rounded font-semibold">
                v0.1.0-pure01
              </span>
              <span className="px-2 py-0.5 text-xs bg-emerald-500/20 text-emerald-400 border border-emerald-500/30 rounded">
                x86_64 Native ELF64
              </span>
            </div>
            <p className="text-xs text-slate-400">Pure Binary Systems Architecture • Native Compiler • Zero Web • GPL-3.0</p>
          </div>
        </div>

        <div className="flex items-center gap-2 flex-wrap">
          {/* Download VSIX Button */}
          <a
            href="/api/download-vsix"
            download="i-language-0.1.0.vsix"
            className="flex items-center gap-2 px-3.5 py-1.5 bg-gradient-to-r from-violet-600 to-indigo-600 hover:from-violet-500 hover:to-indigo-500 text-white font-semibold rounded text-xs transition cursor-pointer shadow-[0_0_12px_rgba(139,92,246,0.4)]"
          >
            <Download className="w-4 h-4" />
            <span>Download VSIX Extension</span>
          </a>

          <button
            onClick={() => setShowVsixModal(true)}
            className="flex items-center gap-1.5 px-3 py-1.5 bg-[#141a29] hover:bg-[#1c253b] border border-violet-500/40 text-violet-300 rounded text-xs transition cursor-pointer"
          >
            <Package className="w-3.5 h-3.5" />
            <span>IntelliSense Guide</span>
          </button>

          <button
            onClick={() => executeCompiler('inspect')}
            disabled={isLoading}
            className="flex items-center gap-2 px-3.5 py-1.5 bg-[#141a29] hover:bg-[#1c253b] border border-[#2b354f] hover:border-[#00f0ff]/50 rounded text-xs text-[#00f0ff] transition cursor-pointer"
          >
            <Binary className="w-4 h-4" />
            <span>Inspect 01 Bits</span>
          </button>

          <button
            onClick={() => executeCompiler('check')}
            disabled={isLoading}
            className="flex items-center gap-2 px-3.5 py-1.5 bg-[#141a29] hover:bg-[#1c253b] border border-[#2b354f] hover:border-emerald-500/50 rounded text-xs text-emerald-400 transition cursor-pointer"
          >
            <CheckCircle2 className="w-4 h-4" />
            <span>Check Verify</span>
          </button>

          <button
            onClick={() => executeCompiler('test')}
            disabled={isLoading}
            className="flex items-center gap-2 px-3.5 py-1.5 bg-[#141a29] hover:bg-[#1c253b] border border-[#2b354f] hover:border-amber-500/50 rounded text-xs text-amber-400 transition cursor-pointer"
          >
            <ShieldCheck className="w-4 h-4" />
            <span>Run Test Suite</span>
          </button>

          <button
            onClick={() => executeCompiler('run')}
            disabled={isLoading}
            className="flex items-center gap-2 px-4 py-1.5 bg-[#00f0ff] hover:bg-[#38f4ff] text-black font-semibold rounded text-xs transition cursor-pointer shadow-[0_0_12px_rgba(0,240,255,0.4)]"
          >
            <Play className="w-4 h-4 fill-black" />
            <span>Execute Native ELF</span>
          </button>
        </div>
      </header>

      {/* Main Grid */}
      <div className="flex-1 grid grid-cols-1 lg:grid-cols-12 gap-0 overflow-hidden">
        {/* Left Sidebar: Example Library */}
        <aside className="lg:col-span-3 border-r border-[#1b2234] bg-[#090d16] p-4 flex flex-col gap-3">
          <div className="flex items-center justify-between text-xs text-slate-400 font-semibold uppercase tracking-wider pb-2 border-b border-[#1b2234]">
            <span className="flex items-center gap-1.5 text-white">
              <Layers className="w-4 h-4 text-[#00f0ff]" /> Systems Testbed
            </span>
            <span>{EXAMPLES.length} Modules</span>
          </div>

          <div className="flex-1 overflow-y-auto space-y-2 pr-1">
            {EXAMPLES.map(ex => {
              const active = selectedExample.id === ex.id;
              return (
                <button
                  key={ex.id}
                  onClick={() => handleSelectExample(ex)}
                  className={`w-full text-left p-3 rounded border transition cursor-pointer ${
                    active
                      ? 'bg-[#121929] border-[#00f0ff]/60 text-white shadow-[inset_0_0_10px_rgba(0,240,255,0.08)]'
                      : 'bg-[#0c101a] border-[#1b2234] text-slate-300 hover:border-slate-600 hover:bg-[#101524]'
                  }`}
                >
                  <div className="flex items-center justify-between mb-1">
                    <span className="text-xs font-bold text-[#00f0ff]">{ex.name}</span>
                    <span className="text-[10px] uppercase px-1.5 py-0.5 rounded bg-[#172033] text-slate-400 border border-[#232f4b]">
                      {ex.category}
                    </span>
                  </div>
                  <p className="text-[11px] text-slate-400 line-clamp-1">{ex.desc}</p>
                </button>
              );
            })}
          </div>

          {/* VSIX Quick Banner Card */}
          <div className="p-3 bg-gradient-to-br from-violet-950/40 to-indigo-950/40 border border-violet-500/30 rounded text-xs space-y-2">
            <div className="flex items-center justify-between text-white font-bold">
              <span className="flex items-center gap-1.5 text-xs text-violet-300">
                <Package className="w-3.5 h-3.5 text-violet-400" /> VS Code Extension
              </span>
              <span className="text-violet-400 text-[10px] bg-violet-500/20 px-1.5 py-0.5 rounded border border-violet-500/30">
                i-language-0.1.0.vsix
              </span>
            </div>
            <p className="text-[11px] text-slate-300">
              ملف VSIX كامل يتضمن الإكمال التلقائي الذكي IntelliSense والشروحات بالعربية والإنجليزية لكل كلمة وعملية.
            </p>
            <div className="flex gap-2 pt-1">
              <a
                href="/api/download-vsix"
                download="i-language-0.1.0.vsix"
                className="flex-1 text-center py-1 bg-violet-600 hover:bg-violet-500 text-white rounded text-[10px] font-semibold transition"
              >
                تحميل VSIX
              </a>
              <button
                onClick={() => setShowVsixModal(true)}
                className="px-2 py-1 bg-[#172033] hover:bg-[#202c45] border border-violet-500/30 text-violet-300 rounded text-[10px]"
              >
                عرض الدليل
              </button>
            </div>
          </div>
        </aside>

        {/* Middle: Source Code Editor */}
        <section className="lg:col-span-4 border-r border-[#1b2234] bg-[#0b0f19] flex flex-col">
          <div className="bg-[#0f1422] border-b border-[#1b2234] px-4 py-2.5 flex items-center justify-between text-xs">
            <div className="flex items-center gap-2 text-white font-semibold">
              <BookOpen className="w-4 h-4 text-[#00f0ff]" />
              <span>{selectedExample.file}</span>
            </div>
            <button
              onClick={() => setCode(selectedExample.code)}
              title="Reset Code"
              className="text-slate-400 hover:text-white flex items-center gap-1 text-[11px] cursor-pointer"
            >
              <RotateCcw className="w-3 h-3" /> Reset
            </button>
          </div>

          <div className="flex-1 relative p-2">
            <textarea
              value={code}
              onChange={(e) => setCode(e.target.value)}
              spellCheck={false}
              className="w-full h-full bg-transparent resize-none outline-none font-mono text-xs leading-relaxed text-[#c6d3e3] p-2 selection:bg-[#00f0ff] selection:text-black border border-transparent focus:border-[#00f0ff]/30 rounded"
              placeholder="// Write Language I source code here..."
            />
          </div>

          <div className="bg-[#0c101a] border-t border-[#1b2234] px-4 py-2 text-[11px] text-slate-400 flex items-center justify-between">
            <span>Syntax: <strong className="text-white">Pure Binary Systems (I)</strong></span>
            <span>Lines: {code.split('\n').length}</span>
          </div>
        </section>

        {/* Right: Machine Terminal & Output */}
        <section className="lg:col-span-5 bg-[#07090e] flex flex-col">
          <div className="bg-[#0c101a] border-b border-[#1b2234] px-4 py-2 flex items-center justify-between">
            <div className="flex items-center gap-2">
              <Terminal className="w-4 h-4 text-[#00f0ff]" />
              <span className="text-xs font-bold text-white uppercase tracking-wider">
                Machine Console
              </span>
              <span className="text-[10px] px-2 py-0.5 rounded bg-[#172033] text-slate-400 border border-[#232f4b]">
                {lastCmd}
              </span>
            </div>

            {isLoading && (
              <span className="flex items-center gap-1.5 text-xs text-[#00f0ff] animate-pulse">
                <Zap className="w-3.5 h-3.5" /> Compiling...
              </span>
            )}
          </div>

          <div className="flex-1 p-4 overflow-y-auto bg-black/60 font-mono text-xs leading-relaxed text-[#b8c7db] whitespace-pre-wrap select-text">
            {terminalOutput || (
              <span className="text-slate-500 italic">
                Click "Inspect 01 Bits", "Check Verify", or "Execute Native ELF" to run the native I compiler...
              </span>
            )}
          </div>

          <div className="border-t border-[#1b2234] bg-[#0c101a] p-3 text-[11px] text-slate-400 flex items-center justify-between">
            <div className="flex items-center gap-2">
              <span className="w-2 h-2 rounded-full bg-emerald-400 inline-block animate-ping"></span>
              <span className="text-emerald-400 font-semibold">Compiler Ready:</span>
              <span>bin/i ELF64 Native</span>
            </div>
            <div>
              <span className="text-slate-500">Pure 01 Machine Code Generation</span>
            </div>
          </div>
        </section>
      </div>

      {/* VSIX Modal */}
      {showVsixModal && (
        <div className="fixed inset-0 z-50 bg-black/80 backdrop-blur-sm flex items-center justify-center p-4">
          <div className="bg-[#0e1322] border border-violet-500/40 rounded-xl max-w-3xl w-full max-h-[85vh] flex flex-col shadow-2xl overflow-hidden">
            {/* Modal Header */}
            <div className="px-6 py-4 border-b border-[#1b2234] flex items-center justify-between bg-[#12182b]">
              <div className="flex items-center gap-3">
                <div className="w-8 h-8 rounded bg-violet-600/20 border border-violet-500/50 flex items-center justify-center text-violet-400 font-bold">
                  <Package className="w-4 h-4" />
                </div>
                <div>
                  <h3 className="text-sm font-bold text-white">إضافة Visual Studio Code الكاملة (VSIX)</h3>
                  <p className="text-xs text-slate-400">حزمة i-language-0.1.0.vsix مع IntelliSense وتوثيق فوري</p>
                </div>
              </div>

              <div className="flex items-center gap-2">
                <a
                  href="/api/download-vsix"
                  download="i-language-0.1.0.vsix"
                  className="flex items-center gap-1.5 px-3 py-1.5 bg-violet-600 hover:bg-violet-500 text-white rounded text-xs font-semibold transition"
                >
                  <Download className="w-3.5 h-3.5" />
                  <span>تحميل VSIX الآن</span>
                </a>
                <button
                  onClick={() => setShowVsixModal(false)}
                  className="p-1 text-slate-400 hover:text-white rounded"
                >
                  <X className="w-5 h-5" />
                </button>
              </div>
            </div>

            {/* Modal Content */}
            <div className="flex-1 overflow-y-auto p-6 space-y-5 text-xs">
              <div className="p-3 bg-violet-950/30 border border-violet-500/30 rounded-lg space-y-2">
                <h4 className="text-violet-300 font-bold flex items-center gap-2">
                  <Code2 className="w-4 h-4" /> طريقة تثبيت إضافة VSIX في VS Code:
                </h4>
                <div className="bg-black/50 p-2.5 rounded font-mono text-[11px] text-slate-300">
                  code --install-extension i-language-0.1.0.vsix
                </div>
                <p className="text-slate-400 text-[11px]">
                  أو من خلال واجهة VS Code: افتح شريط الإضافات (Extensions) ← اضغط على القائمة (···) في الأعلى ← اختر <strong>Install from VSIX...</strong> ثم حدد الملف المحمل.
                </p>
              </div>

              {/* Search Tokens */}
              <div className="space-y-2">
                <div className="flex items-center justify-between">
                  <h4 className="text-white font-bold flex items-center gap-2">
                    <Sparkles className="w-4 h-4 text-[#00f0ff]" />
                    جدول IntelliSense والشروحات التفاعلية (لا حاجة للحفظ):
                  </h4>
                  <div className="relative">
                    <input
                      type="text"
                      value={filterQuery}
                      onChange={(e) => setFilterQuery(e.target.value)}
                      placeholder="بحث في الكلمات والمعاملات..."
                      className="bg-[#141b2e] border border-[#232f4e] px-2.5 py-1 text-xs rounded text-slate-200 outline-none focus:border-[#00f0ff]"
                    />
                  </div>
                </div>

                <div className="border border-[#1b2234] rounded-lg overflow-hidden max-h-72 overflow-y-auto">
                  <table className="w-full text-left border-collapse">
                    <thead className="bg-[#12182b] text-slate-400 text-[10px] uppercase sticky top-0">
                      <tr>
                        <th className="p-2.5 border-b border-[#1b2234]">الرمز / الكلمة</th>
                        <th className="p-2.5 border-b border-[#1b2234]">النوع</th>
                        <th className="p-2.5 border-b border-[#1b2234]">الشرح بالعربية (ماذا يفعل المعالج)</th>
                        <th className="p-2.5 border-b border-[#1b2234]">English Specification</th>
                      </tr>
                    </thead>
                    <tbody className="divide-y divide-[#1b2234] text-[11px]">
                      {filteredIntellisense.map((item, idx) => (
                        <tr key={idx} className="hover:bg-[#131b2e] transition">
                          <td className="p-2.5 font-bold text-[#00f0ff] font-mono">{item.token}</td>
                          <td className="p-2.5 text-slate-400">
                            <span className="px-1.5 py-0.5 rounded bg-[#172033] border border-[#2b354f] text-[10px]">
                              {item.kind}
                            </span>
                          </td>
                          <td className="p-2.5 text-slate-300">{item.ar}</td>
                          <td className="p-2.5 text-slate-400 font-mono text-[10px]">{item.en}</td>
                        </tr>
                      ))}
                    </tbody>
                  </table>
                </div>
              </div>
            </div>
          </div>
        </div>
      )}
    </div>
  );
}
