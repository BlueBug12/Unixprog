#include "sdb.hpp"
void terminate(){
    exit(0);
}

void help(){
    PRINT("- break {instruction-address}: add a break point");
    PRINT("- cont: continue execution");
    PRINT("- delete {break-point-id}: remove a break point");
    PRINT("- disasm addr: disassemble instructions in a file or a memory region");
    PRINT("- dump addr: dump memory content");
    PRINT("- exit: terminate the debugger");
    PRINT("- get reg: get a single value from a register");
    PRINT("- getregs: show registers");
    PRINT("- help: show this message");
    PRINT("- list: list break points");
    PRINT("- load {path/to/a/program}: load a program");
    PRINT("- run: run the program");
    PRINT("- vmmap: show memory layout");
    PRINT("- set reg val: get a single value to a register");
    PRINT("- si: step into instruction");
    PRINT("- start: start the program and stop at the first instruction");
}
