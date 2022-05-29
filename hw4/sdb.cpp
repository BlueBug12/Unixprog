#include "sdb.hpp"

SDB::SDB(){
    state = NOT_LOADED;
    pid = 0;
    program_name = NULL;
}
SDB::~SDB(){}


void SDB::terminate(){
    exit(0);
}

bool SDB::in_text(const unsigned long addr){
    return sect_header.sh_addr <= addr && addr < sect_header.sh_addr + sect_header.sh_size;   
}

void SDB::get_code(char * file_name){
    std::ifstream fin(file_name, std::ios::in|std::ios::binary);
    fin.seekg(0,fin.end);
    code_size = fin.tellg();
    fin.seekg(0,fin.beg);
    code_text = (char*)malloc(sizeof(char)*code_size);
    fin.read(code_text,code_size);
    fin.close();
}

void SDB::help(){
    PRINT("- break {instruction-address}: add a break point");
    PRINT("- cont: continue execution");
    PRINT("- delete {break-point-id}: remove a break point");
    PRINT("- disasm addr: disassemble instructions in a file or a memory region");
    PRINT("- dump addr: dump memory content");
    PRINT("- exit: terminate the debugger");
    PRINT("- get reg: get a single value from a register");    PRINT("- getregs: show registers");
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

void SDB::start(){
    if(state!=LOADED){
        DEBUG("state must be LOADED");
    return;
}
    if((pid = fork())<0){
        ERROR("fork child-process failed");
        return;   
    }
    else if(pid == 0){
        if(ptrace(PTRACE_TRACEME, 0, 0, 0)){
            ERROR("ptrace: TRACEME failed.");
        }
        char *argv_child[] = {program_name,NULL};
        if(execvp(program_name,argv_child)){
            ERROR("execvp failed.");
        }
    }else{
        int status;
        waitpid(pid, &status, 0);
        if(ptrace(PTRACE_SETOPTIONS, pid, 0, PTRACE_O_EXITKILL)){
            ERROR("ptrace: PTRACE_SETOPTIONS failed.");
        }
        char buf[32];
        sprintf(buf,"pid %d",pid);
        DEBUG(buf);
        state = RUNNING;
    }
}

void SDB::list(){
    int counter = 0;
    for(auto it = addr_list.begin();it!=addr_list.end();++it){
        fprintf(stderr,"  %d: %0lx\n", counter++, it->first);
    }
}

void SDB::set_break(unsigned long addr){
    if(state!=RUNNING){
        DEBUG("state must be RUNNING");
        return;
    }
    if(!in_text(addr)){
        DEBUG("the address is out of the range of the text segment");
        return;
    }
    if(addr_list.find(addr)!=addr_list.end()){
        return;
    }
    long code = ptrace(PTRACE_PEEKTEXT, pid, addr, NULL);
    addr_list[addr] = code;
    if(ptrace(PTRACE_POKETEXT, pid, addr, (code & 0xffffffffffffff00) | 0xcc)!=0){
        ERROR("ptrace: PTRACE_POKETEXT failed");
    }
}

//https://unix.stackexchange.com/questions/190178/reading-the-contents-of-an-elf-file-programmatically
void SDB::load(char * file_name){
    if(state!=NOT_LOADED){
        DEBUG("state must be NOT LOADED");
    }else{
        FILE *fp = fopen(file_name,"rb");
        char buf[128];
        if(fp==NULL){
            sprintf(buf, "can not open %s", file_name);
            DEBUG(buf);
            return;
        }

        fread(&elf_header, 1, sizeof(elf_header), fp);
        if(memcmp(elf_header.e_ident, ELFMAG, SELFMAG) != 0){
            sprintf(buf,"%s is not a valid ELF file.", file_name);
            DEBUG(buf);
            return;
        }
        fseek(fp, elf_header.e_shoff + elf_header.e_shstrndx * sizeof(Elf64_Shdr), SEEK_SET);
        fread(&sect_header, 1, sizeof(Elf64_Shdr), fp);
        char * sect_name = (char *)malloc(sizeof(char)*sect_header.sh_size);
        fseek(fp,sect_header.sh_offset, SEEK_SET);
        fread(sect_name, 1, sect_header.sh_size, fp);

        //Elf64_Shdr temp;
        fseek(fp, elf_header.e_shoff, SEEK_SET);
        for(int i =0;i<elf_header.e_shnum; ++i){
            fread(&(sect_header),1 , sizeof(Elf64_Shdr), fp);
            //find the text scetion
            if(strcmp(sect_name+sect_header.sh_name,".text")==0){
                fprintf(stderr, "from %0lx to %0lx\n", sect_header.sh_addr, sect_header.sh_addr+sect_header.sh_size);
                break;    
            }
        }
        state = LOADED;
        sprintf(buf,"program \'%s\' loaded. entry point 0x%0lx",file_name, elf_header.e_entry);
        DEBUG(buf);
        fclose(fp);
        get_code(file_name);
    }
}

void SDB::cont(){
    if(state!=RUNNING){
        DEBUG("state must be RUNNING");
        return;
    }

}

void SDB::getregs(){
    if(state!=RUNNING){
        DEBUG("state must be RUNNING");
        return;
    }
    struct user_regs_struct regs;
    if(ptrace(PTRACE_GETREGS, pid, 0, &regs)!=0){
        ERROR("ptrace: PTRACE_GETREGS failed");
    }
    fprintf(stderr,"RAX %-16llx RBX %-16llx RCX %-16llx RDX %-16llx\n", regs.rax, regs.rbx, regs.rcx, regs.rdx);
    fprintf(stderr,"R8  %-16llx R9  %-16llx R10 %-16llx R11 %-16llx\n", regs.r8,  regs.r9,  regs.r10, regs.r11);
    fprintf(stderr,"R12 %-16llx R13 %-16llx R14 %-16llx R15 %-16llx\n", regs.r12, regs.r13, regs.r14, regs.r15);
    fprintf(stderr,"RDI %-16llx RSI %-16llx RBP %-16llx RSP %-16llx\n", regs.rdi, regs.rsi, regs.rbp, regs.rsp);
    fprintf(stderr,"RIP %-16llx FLAGS %-16llx\n", regs.rip, regs.eflags);
}

void SDB::disasm(unsigned long addr, size_t len){
    if(state!=RUNNING){
        DEBUG("state must be RUNNING");
        return;
    }
    if(!in_text(addr)){
        DEBUG("the address is out of the range of the text segment");
        return;
    }
    
    csh handle;
    cs_insn *insn;
    size_t count;

    unsigned long offset = sect_header.sh_offset + addr - sect_header.sh_addr;
    char * cur_code = code_text + offset;

    if(cs_open(CS_ARCH_X86, CS_MODE_64, &handle)!=CS_ERR_OK){
        ERROR("cs_open failed");
    }

    if((count=cs_disasm(handle,(uint8_t*)cur_code,code_size,addr,len, &insn))>0){
        for(size_t i=0;i<count;++i){
            if(insn[i].address >= sect_header.sh_addr+sect_header.sh_size){
                DEBUG("the address is out of the range of the text segment");
                break;
            }
            fprintf(stderr,"0x%0lx: ", insn[i].address);
            for (size_t j = 0; j < 10; j++) {
                if (j < insn[i].size) {
                    fprintf(stderr,"%02hhx ", insn[i].bytes[j]);
                }else{
                    fprintf(stderr,"   ");
                }
            }
            fprintf(stderr,"%-10s%-10s\n", insn[i].mnemonic, insn[i].op_str);
        }
    }else{
        DEBUG("the address is out of the range of the text segment");
    }
    cs_close(&handle);
}
