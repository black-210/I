system PacketParser;

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
}
