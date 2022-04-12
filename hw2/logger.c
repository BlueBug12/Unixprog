#include "logger.h"

static int fd = -1;

char* getAbsPath(const char* path){
    char *r_path = realpath(path,NULL);
    if(r_path==NULL){
        char str[9] = "untouched";
        r_path = (char *)malloc(sizeof(char)*strlen(str));
        strcpy(r_path,str);
    }
    return r_path;
}

int getFD(){
    if(fd<0){
        fd = atoi(getenv("LOGGER"));
    }
    return fd;
}

void FD2name(int fd, char *name){
    char buf[BUFSIZE];
    memset(buf,0,BUFSIZE);
    sprintf(buf,"/proc/self/fd/%d",fd);
    if(readlink(buf,name,BUFSIZE)==-1){
        fprintf(stderr,"readlink failed.\n");
        //exit(EXIT_FAILURE);
    }
}

void FILE2name(FILE* stream, char* name){
    int fd = fileno(stream);
    FD2name(fd,name);
}

void find_fptr(void** f, const char* f_name){
    if(*f == NULL){
        void *handle = dlopen(LIBC,RTLD_LAZY);
        if(handle != NULL){
            *f = dlsym(handle,f_name);
        }else{
            fprintf(stderr,"dlopen failed.\n");
            exit(EXIT_FAILURE);
        }
    }
}

int chmod(const char *path, mode_t mode){
    find_fptr((void**)&chmod_o,"chmod");
    CHECKER(chmod_o);
    int result = chmod_o(path,mode);
    char *r_path = getAbsPath(path);
    dprintf(getFD(),"[logger] %s(\"%s\", %o) = %d\n",__func__,r_path,mode,result);
    free(r_path);
    return result;
}

int chown(const char *path, uid_t owner, gid_t group){
    find_fptr((void**)&chown_o,"chown");
    CHECKER(chown_o);
    int result = chown_o(path,owner,group);
    char *r_path = getAbsPath(path);
    dprintf(getFD(),"[logger] %s(\"%s\", %d, %d) = %d\n",__func__,r_path,owner,group,result);
    free(r_path);
    return result;
}

int close(int fildes){
    find_fptr((void**)&close_o,"close");
    CHECKER(close_o);
    char name[BUFSIZE];
    FD2name(fildes,name);
    int result = 0;
    if(fildes!=2){
        result = close_o(fildes);
    }
    dprintf(getFD(),"[logger] %s(\"%s\") = %d\n",__func__,name,result);
}

int creat(const char *path, mode_t mode){
    find_fptr((void**)&creat_o,"creat");
    CHECKER(creat_o);
    int result = creat_o(path, mode);
    char *r_path = getAbsPath(path);
    dprintf(getFD(),"[logger] %s(\"%s\", %o) = %d\n",__func__, r_path, mode, result);
    free(r_path);
    return result;
}

int fclose(FILE *stream){
    find_fptr((void**)&fclose_o,__func__);
    CHECKER(fclose_o);
    int result = fclose_o(stream);
    char name[BUFSIZE];
    FILE2name(stream,name);
    dprintf(getFD(),"[logger] %s(\"%s\") = %d\n",__func__, name, result);
    return result;
}

FILE *fopen(const char *pathname, const char *mode){
    find_fptr((void**)&fopen_o,__func__);
    CHECKER(fopen_o);
    FILE *result = fopen_o(pathname, mode);
    char *r_path = getAbsPath(pathname);
    dprintf(getFD(),"[logger] %s(\"%s\", %o) = %p\n", __func__, mode, result);
    free(r_path);
    return result;
}

size_t fread(void *ptr, size_t size, size_t nmemb, FILE *stream){
    find_fptr((void**)&fread_o,__func__);
    CHECKER(fread_o);
    size_t result = fread(ptr, size, nmemb, stream);
    char name[BUFSIZE];
    FILE2name(stream,name);
    dprintf(getFD(),"[logger] %s(\"",__func__);
    char * str = (char *)ptr;
    for(size_t i=0;i<nmemb && i<32 && i < result;++i){
        if(isprint(str[i])){
            dprintf(getFD(),"%c",str[i]);
        }else{
            dprintf(getFD(),".");
        }
    }
    dprintf(getFD(),"\", %zu, %zu, \"%s\") = %zu\n", size, nmemb, name, result);
    return result;
}
/*
size_t fwrite(const void *ptr, size_t size, size_t nmemb, FILE *stream){
    find_fptr((void**)&fwrite_o,"fwrite");
    CHECKER(fwrite_o);
    size_t result = fwrite(ptr, size, nmemb, stream);
    char name[BUFSIZE];
    FILE2name(stream,name);
    dprintf(getFD(),"[logger] %s(\"",__func__);
    char * str = (char *)ptr;
    for(size_t i=0;i<nmemb && i<32 && i < result;++i){
        if(isprint(str[i])){
            dprintf(getFD(),"%c",str[i]);
        }else{
            dprintf(getFD(),".");
        }
    }
    dprintf(getFD(),"\", %zu, %zu, \"%s\") = %zu\n", size, nmemb, name, result);
    return result;
}*/
int open(const char *pathname, int flags,...){
    find_fptr((void**)&open_o,__func__);
    CHECKER(open_o);
    int result;
    mode_t mode;
    char *r_path = getAbsPath(pathname);
    if(__OPEN_NEEDS_MODE(flags)){
        va_list arg;
        va_start (arg,flags);
        mode = va_arg(arg, int);
        va_end(arg);
        result = open_o(pathname,flags,mode);
        dprintf(getFD(),"[logger] %s(\"%s\", %d, %o) = %d\n", pathname, flags, mode, result);
    }else{
        result = open_o(pathname,flags);
        dprintf(getFD(),"[logger] %s(\"%s\", %d) = %d\n", pathname, flags, result);
    }
    free(r_path);
    return result;
}
ssize_t read(int fildes, void *buf, size_t nbyte){
    find_fptr((void**)&read_o,__func__);
    CHECKER(read_o);
    ssize_t result = read_o(fildes, buf, nbyte);
    char name[BUFSIZE];
    FD2name(fildes,name);
    dprintf(getFD(),"[logger] %s(\"%s\", \"",__func__,name);
    char * str = (char *)buf;
    for(size_t i=0;i<nbyte && i<32 && i < result;++i){
        if(isprint(str[i])){
            dprintf(getFD(),"%c",str[i]);
        }else{
            dprintf(getFD(),".");
        }
    }
    dprintf(getFD(),"\", %zu) = %zd\n", nbyte, result);    
    return result;
}


int remove(const char *pathname){
    find_fptr((void**)&remove_o,__func__);
    CHECKER(remove_o);
    int result = remove_o(pathname);
    char * r_path = getAbsPath(pathname);

    dprintf(getFD(),"[logger] %s(\"%s\") = %d\n",__func__, r_path, result);
    free(r_path);
    return result;
}

int rename(const char *old, const char *new){
    find_fptr((void**)&rename_o,__func__);
    CHECKER(rename_o);
    int result = rename_o(old, new);
    char *r_old = getAbsPath(old);
    char *r_new = getAbsPath(new);

    dprintf(getFD(),"[logger] %s(\"%s\", \"%s\") = %d\n", __func__, r_old, r_new, result);

    free(r_old);
    free(r_new);
    return result;
}

FILE *tmpfile(void){
    find_fptr((void**)&tmpfile_o,__func__);
    CHECKER(tmpfile_o);
    FILE * result = tmpfile_o();
    dprintf(getFD(),"[logger] %s() = %p\n", __func__, result);
    return result;
}

ssize_t write(int fildes, const void *buf, size_t nbyte){
    find_fptr((void**)&write_o,__func__);
    CHECKER(write_o);
    ssize_t result = write_o(fildes, buf, nbyte);
    char name[BUFSIZE];
    FD2name(fildes,name);
    dprintf(getFD(),"[logger] %s(\"%s\", \"",__func__, name);
    char * str = (char *)buf;
    for(size_t i=0;i<nbyte && i<32 && i < result;++i){
        if(isprint(str[i])){
            dprintf(getFD(),"%c",str[i]);
        }else{
            dprintf(getFD(),".");
        }
    }
    dprintf(getFD(),"\", %zu) = %zd\n", nbyte, result);
    return result;
}
