system SerialUart;

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

routine uart_write_byte(base_port: u16, byte_val: u8) -> void {
    // Wait for transmit buffer empty: line_status bit 5 (0x20)
    // Send byte to base port
    @out8(base_port, byte_val);
}

routine main() -> u32 {
    uart_init(0x3F8);
    uart_write_byte(0x3F8, 0x48); // 'H'
    uart_write_byte(0x3F8, 0x49); // 'I'
    yield 0;
}
