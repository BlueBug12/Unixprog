#ifndef SDB_HPP_
#define SDB_HPP_
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_TOKEN_NUM 10
#define ERROR(message) fprintf(stderr,"Error: %s\n",message); exit(EXIT_FAILURE)
#define SIGN() fprintf(stderr,"sdb> ")
#define PRINT(str) fprintf(stderr,"%s\n",str)

using namespace std;

void terminate();
void help();

#endif // SDB_HPP_
