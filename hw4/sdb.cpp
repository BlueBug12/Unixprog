#include "sdb.hpp"

SDB::SDB(){
    state = NOT_LOADED;
    pid = -1;
}
SDB::~SDB(){}


void SDB::terminate(){
    exit(0);
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
void SDB::set_break(unsigned long addr){
    if(state!=RUNNING){
        DEBUG("state must be RUNNING");
    }else{
        long word = ptrace(PTRACE_PEEKTEXT, pid, addr, NULL);
        if(word==-1){
            DEBUG("the address is out of the range of the text segment");
        }
        fprintf(stderr,"addr = %lu\n", addr);
        fprintf(stderr,"word = %ld\n", word);
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
                break;    
            }
        }
        sprintf(buf,"program \'%s\' loaded. entry point %08lx",file_name, elf_header.e_entry);
        DEBUG(buf);
    }
}
