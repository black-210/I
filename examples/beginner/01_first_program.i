system HelloMachine;

require cap: host_syscall;

routine main() -> u32 {
    // Return exit code 0 to indicate normal termination
    yield 0;
}
