#ifndef SDB_HPP_
#define SDB_HPP_
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/ptrace.h>
#include <sys/wait.h>
#include <sys/user.h>
#include <sys/stat.h>
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
    void set_break(unsigned long addr);
    void cont();
    void del(int id);
    void disasm(unsigned long addr, size_t len);
    void dump(unsigned long addr);
    void quit();
    void get(char * reg_name);
    void getregs();
    void help();
    void list();
    void load(char * file_name);
    void run();
    void set(char * reg_name, unsigned long val);
    void si();
    void start();
    bool in_text(unsigned long addr);
    void get_code(char* file_name);
    void check_process();

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
