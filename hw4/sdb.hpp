#ifndef SDB_HPP_
#define SDB_HPP_
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/ptrace.h>
#include <sys/wait.h>
#include <sys/user.h>
#include <elf.h>
#include <inttypes.h>
#include <capstone/capstone.h>
#include <map>
#include <vector>
#include <fstream>
#include <iterator>

#define MAX_TOKEN_NUM 10
#define ERROR(message) fprintf(stderr,"Error: %s\n",message); exit(EXIT_FAILURE)
#define SIGN() fprintf(stderr,"sdb> ")
#define PRINT(str) fprintf(stderr,"%s\n",str)
#define DEBUG(str) fprintf(stderr,"** %s\n",str)
#define NOT_LOADED 0
#define LOADED 1
#define RUNNING 2
#define ASM_LINES 10
#define DUMP_BYTES 80

/*
 * echo $LD_LIBRARY_PATH
 * LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/usr/lib
 * export LD_LIBRARY_PATH
 * */

class SDB{
public:
    SDB();
    ~SDB();
    void terminate();
    void get_code(char* file_name);
    void help();
    void set_break(unsigned long addr);
    void load(char * file_name);
    void start();
    void list();
    void cont();
    void getregs();
    void disasm(unsigned long addr, size_t len);
    void del(int id);
    void dump(unsigned long long addr);
    bool in_text(unsigned long addr);

private:
    Elf64_Ehdr elf_header;
    Elf64_Shdr sect_header;
    std::map<unsigned long, long>addr_list;
    int state;
    size_t code_size;
    pid_t pid;
    char program_name[64];
    char * code_text;
};

#endif // SDB_HPP_
