#ifndef SDB_HPP_
#define SDB_HPP_
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/ptrace.h>
#include <elf.h>
#include <inttypes.h>
#include <set>
#include <vector>

#define MAX_TOKEN_NUM 10
#define ERROR(message) fprintf(stderr,"Error: %s\n",message); exit(EXIT_FAILURE)
#define SIGN() fprintf(stderr,"sdb> ")
#define PRINT(str) fprintf(stderr,"%s\n",str)
#define DEBUG(str) fprintf(stderr,"** %s\n",str)
#define NOT_LOADED 0
#define LOADED 1
#define RUNNING 2



class SDB{
public:
    SDB();
    ~SDB();
    void terminate();
    void help();
    void set_break(unsigned long addr);
    void load(char * file_name);
private:
    Elf64_Ehdr elf_header;
    Elf64_Shdr sect_header;
    std::set<long>addr_list;
    int state;
    pid_t pid;
};

#endif // SDB_HPP_
