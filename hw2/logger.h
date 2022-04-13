#ifndef LOGGER_H
#define LOGGER_H
#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <dlfcn.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <limits.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>

#define BUFSIZE 256
#define LIBC "libc.so.6"
#define DEBUGGER() fprintf(stderr,"function pointer of %s not found.\n",__func__)

#define CHECKER(f_ptr) \
    if(f_ptr == NULL){ \
        DEBUGGER(); \
        exit(EXIT_FAILURE); \
    }

static int (*chmod_o)(const char *, mode_t) = NULL;
static int (*chown_o)(const char *,uid_t, gid_t) = NULL;
static int (*close_o)(int) = NULL;
static int (*creat_o)(const char *,mode_t) = NULL;
static int (*fclose_o)(FILE *) = NULL;
static FILE *(*fopen_o)(const char *, const char *) = NULL;
static size_t (*fread_o)(void *, size_t, size_t, FILE *) = NULL;
static size_t (*fwrite_o)(const void *, size_t, size_t, FILE *) = NULL;
static int (*open_o)(const char *, int, ...) = NULL;
static ssize_t (*read_o)(int, void *,size_t) = NULL;
static int (*remove_o)(const char *) = NULL;
static int (*rename_o)(const char *, const char *) = NULL;
static FILE *(*tmpfile_o)(void) = NULL;
static ssize_t (*write_o)(int, const void *, size_t) = NULL;

#endif
