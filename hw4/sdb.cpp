#include "sdb.hpp"

SDB::SDB(){
    state = NOT_LOADED;
    code_text = NULL;
    code_size = 0;
    pid = 0;
}
SDB::~SDB(){}


void SDB::quit(){
    if(pid>0){
        kill(pid, SIGTERM);
    }
    if(code_text){
        free(code_text);
    }
    exit(0);
}

bool SDB::in_text(const unsigned long addr){
    return sect_header.sh_addr <= addr && addr < sect_header.sh_addr + sect_header.sh_size;   
}

void SDB::get_code(char * file_name){
    struct stat st;
    stat(file_name, &st);
    code_size = st.st_size;
    code_text = (char*)malloc(sizeof(char)*code_size);
    FILE * fp = fopen(file_name,"r");
    fread(code_text,sizeof(char),code_size,fp);
    fclose(fp);
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
        strcpy(program_name,file_name);
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
    if(ptrace(PTRACE_CONT,pid,0,0) < 0){
        ERROR("ptrace: PTRACE_CONT failed");
    }
    check_process();
}

void SDB::check_process(){
    int status;
    if(waitpid(pid, &status, 0) < 0){
        ERROR("waitpid failed");
    }
    //check the child process exit normally
    if(WIFEXITED(status)){
        char buf[128];
        if(WIFSIGNALED(status)){
            sprintf(buf,"child process %d terminated by signal (code %d)", pid, WTERMSIG(status));
        }else{
            sprintf(buf,"child process %d terminated normally (code %d)", pid, status);
        } 
        DEBUG(buf);
        pid = 0;
        state = LOADED;
    }else if(WIFSTOPPED(status)){
        if(WSTOPSIG(status)==SIGTRAP){            
            struct user_regs_struct regs;
            if(ptrace(PTRACE_GETREGS, pid, 0, &regs)!=0){
                ERROR("ptrace: PTRACE_GETREGS failed");
            }
            std::map<unsigned long, long>::iterator it;
            if((it=addr_list.find(regs.rip-1))!=addr_list.end()){
                fprintf(stderr,"** breakpoint at @");
                disasm(it->first,1);
                //recover original code
                if(ptrace(PTRACE_POKETEXT, pid, it->first, it->second) < 0){
                    ERROR("ptrace: PTRACE_POKETEXT failed");
                }
                //recover program counter
                --regs.rip;
                if(ptrace(PTRACE_SETREGS, pid, 0, &regs) != 0){
                    ERROR("ptrace: PTRACE_SETREGS failed");
                }
            }else{
                //ERROR("hit no addr??");
            }
        }    
    }else{
        char buf[128];
        sprintf(buf,"child process %d terminated by signal (code %d)", pid, WTERMSIG(status));
        DEBUG(buf);
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
            fprintf(stderr,"      %0lx: ", insn[i].address);
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

void SDB::del(int id){
    if(state!=RUNNING){
        DEBUG("state must be RUNNING");
        return;
    }
    if(id < 0 || (size_t)id >= addr_list.size()){
        DEBUG("the address is out of the range of the text segment");
    }else{
        auto it = addr_list.begin();
        std::advance(it,id);
        if(ptrace(PTRACE_POKETEXT, pid, it->first, it->second)!=0){
            ERROR("ptrace: PTRACE_POKETEXT failed");
        }
        addr_list.erase(it);
        char buf[32];
        sprintf(buf, "breakpoint %d deleted.", id);
        DEBUG(buf);
    }
}

void SDB::dump(unsigned long addr){
    if(state!=RUNNING){
        DEBUG("state must be RUNNING");
        return;
    }
    if(!in_text(addr)){
        DEBUG("the address is out of the range of the text segment");
        return;
    }
    
    unsigned char data[16];
    unsigned long text;
    for(int i =0;i<DUMP_BYTES/16;++i){
        fprintf(stderr,"      %0lx: ",addr);
        text = ptrace(PTRACE_PEEKTEXT,pid,addr,0);
        for(int j=0;j<8;++j){
            data[j] = ((char *)&text)[j];
        }
        text = ptrace(PTRACE_PEEKTEXT,pid,addr+8,0);
        for(int j=0;j<8;++j){
            data[j+8] = ((char *)&text)[j];
        }
        for(int j = 0;j < 16;++j){
            fprintf(stderr,"%02hhx ", data[j]);
        }
        fprintf(stderr," |");

        for(int j = 0;j < 16;++j){
            char c = data[j];
            if(isprint(c) != 0){
                fprintf(stderr,"%c",c);
            }else{
                fprintf(stderr,".");
            }
        }
        fprintf(stderr,"|\n");
        addr+=16;
    }
}

void SDB::get(char * reg_name){
    if(state!=RUNNING){
        DEBUG("state must be RUNNING");
        return;
    }

    struct user_regs_struct regs;
    unsigned long value;
    if(ptrace(PTRACE_GETREGS, pid, 0, &regs)<0){
        ERROR("ptrace: PTRACE_GETREGS failed");
    }
    if(strcmp(reg_name,"rax")==0){
        value = regs.rax;
    }else if(strcmp(reg_name,"rbx")==0){
        value = regs.rbx;
    }else if(strcmp(reg_name,"rcx")==0){
        value = regs.rcx;
    }else if(strcmp(reg_name,"rdx")==0){
        value = regs.rdx;
    }else if(strcmp(reg_name,"r8")==0){
        value = regs.r8;
    }else if(strcmp(reg_name,"r9")==0){
        value = regs.r9;
    }else if(strcmp(reg_name,"r10")==0){
        value = regs.r10;
    }else if(strcmp(reg_name,"r11")==0){
        value = regs.r11;
    }else if(strcmp(reg_name,"r12")==0){
        value = regs.r12;
    }else if(strcmp(reg_name,"r13")==0){
        value = regs.r13;
    }else if(strcmp(reg_name,"r14")==0){
        value = regs.r14;
    }else if(strcmp(reg_name,"r15")==0){
        value = regs.r15;
    }else if(strcmp(reg_name,"rdi")==0){
        value = regs.rdi;
    }else if(strcmp(reg_name,"rbp")==0){
        value = regs.rbp;
    }else if(strcmp(reg_name,"rsp")==0){
        value = regs.rsp;
    }else if(strcmp(reg_name,"rip")==0){
        value = regs.rip;
    }else if(strcmp(reg_name,"flags")==0){
        value = regs.eflags;
    }else{
        //DEBUG("no reg name matched");
        //do nothing??
        return;
    }
    fprintf(stderr,"%s = %lu (0x%lx)\n", reg_name, value, value);
}

void SDB::run(){
    if(state == RUNNING){
        char buf[128];
        sprintf(buf,"program %s is already running", program_name);
        DEBUG(buf);
        cont();
    }else if(state == LOADED){
        start();
        cont();
    }else{
        DEBUG("state must be LOADED or RUNNING");
    }
}

void SDB::set(char * reg_name, unsigned long val){
    struct user_regs_struct regs;
    if(ptrace(PTRACE_GETREGS, pid, 0, &regs)!=0){
        ERROR("ptrace: PTRACE_GETREGS failed");
    }
    if(strcmp(reg_name,"rax")==0){
        regs.rax = val;
    }else if(strcmp(reg_name,"rbx")==0){
        regs.rbx = val;
    }else if(strcmp(reg_name,"rcx")==0){
        regs.rcx = val;
    }else if(strcmp(reg_name,"rdx")==0){
        regs.rdx = val;
    }else if(strcmp(reg_name,"r8")==0){
        regs.r8 = val;
    }else if(strcmp(reg_name,"r9")==0){
        regs.r9 = val;
    }else if(strcmp(reg_name,"r10")==0){
        regs.r10 = val;
    }else if(strcmp(reg_name,"r11")==0){
        regs.r11 = val;
    }else if(strcmp(reg_name,"r12")==0){
        regs.r12 = val;
    }else if(strcmp(reg_name,"r13")==0){
        regs.r13 = val;
    }else if(strcmp(reg_name,"r14")==0){
        regs.r14 = val;
    }else if(strcmp(reg_name,"r15")==0){
        regs.r15 = val;
    }else if(strcmp(reg_name,"rdi")==0){
        regs.rdi = val;
    }else if(strcmp(reg_name,"rbp")==0){
        regs.rbp = val;
    }else if(strcmp(reg_name,"rsp")==0){
        regs.rsp = val;
    }else if(strcmp(reg_name,"rip")==0){
        regs.rip = val;
    }else if(strcmp(reg_name,"flags")==0){
        regs.eflags = val;
    }else{
        //DEBUG("no reg name matched");
        //do nothing??
        return;
    }
    if(ptrace(PTRACE_SETREGS, pid, 0, &regs) != 0){
        ERROR("ptrace: PTRACE_SETREGS failed");
    }
}

void SDB::si(){
    if(state!=RUNNING){
        DEBUG("state must be RUNNING");
        return;
    }
    if(ptrace(PTRACE_SINGLESTEP, pid, 0, 0) < 0){
        ERROR("ptrace: PTRACE_SINGLESTEP failed");
    }
    check_process();
}
