system MinimalBoot;

// Capability to access bare-metal CPU instructions and hardware ports
require cap: hardware_io;
require cap: kernel_ring0;

kernel_entry boot_start() -> void {
    // Disable external interrupts during early stage
    @cli();

    // Send ASCII 'O' and 'K' to serial COM1 port 0x3F8
    @out8(0x3F8, 0x4F); // 'O'
    @out8(0x3F8, 0x4B); // 'K'
    @out8(0x3F8, 0x0A); // '\n'

    // Halt processor until next hardware interrupt
    halt;
}
