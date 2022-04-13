#include "logger.h"

static int fd = -1;

void getAbsPath(const char* path, char *name){
    char *r_path = realpath(path,name);
    if(r_path==NULL){
        strcpy(name,"untouched");
    }
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
    memset(name,0,BUFSIZE);
    sprintf(buf,"/proc/self/fd/%d",fd);
    if(readlink(buf,name,BUFSIZE)==-1){
        dprintf(2,"readlink failed.\n");
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
            dprintf(2,"dlopen failed.\n");
            exit(EXIT_FAILURE);
        }
    }
}

int chmod(const char *path, mode_t mode){
    find_fptr((void**)&chmod_o,__func__);
    CHECKER(chmod_o);
    char r_path[BUFSIZE];
    getAbsPath(path,r_path);
    int result = chmod_o(path,mode);
    dprintf(getFD(),"[logger] %s(\"%s\", %o) = %d\n",__func__,r_path,mode,result);
    return result;
}

int chown(const char *path, uid_t owner, gid_t group){
    find_fptr((void**)&chown_o,__func__);
    CHECKER(chown_o);
    char r_path[BUFSIZE];
    getAbsPath(path,r_path);
    int result = chown_o(path,owner,group);
    dprintf(getFD(),"[logger] %s(\"%s\", %o, %o) = %d\n",__func__,r_path,owner,group,result);
    return result;
}

int close(int fildes){
    find_fptr((void**)&close_o,__func__);
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
    find_fptr((void**)&creat_o,__func__);
    CHECKER(creat_o);
    int result = creat_o(path, mode);
    char r_path[BUFSIZE];
    getAbsPath(path,r_path);
    dprintf(getFD(),"[logger] %s(\"%s\", %o) = %d\n",__func__, r_path, mode, result);
    return result;
}

int fclose(FILE *stream){
    find_fptr((void**)&fclose_o,__func__);
    CHECKER(fclose_o);
    char name[BUFSIZE];
    FILE2name(stream,name);
    int result = fclose_o(stream);
    dprintf(getFD(),"[logger] %s(\"%s\") = %d\n",__func__, name, result);
    return result;
}

FILE *fopen(const char *pathname, const char *mode){
    find_fptr((void**)&fopen_o,__func__);
    CHECKER(fopen_o);
    FILE *result = fopen_o(pathname, mode);  
    char r_path[BUFSIZE];
    getAbsPath(pathname,r_path);
    dprintf(getFD(),"[logger] %s(\"%s\", \"%s\") = %p\n", __func__,r_path, mode, result);
    return result;
}

size_t fread(void *ptr, size_t size, size_t nmemb, FILE *stream){
    find_fptr((void**)&fread_o,__func__);
    CHECKER(fread_o);
    size_t result = fread_o(ptr, size, nmemb, stream);
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

size_t fwrite(const void *ptr, size_t size, size_t nmemb, FILE *stream){
    find_fptr((void**)&fwrite_o,__func__);
    CHECKER(fwrite_o);
    size_t result = fwrite_o(ptr, size, nmemb, stream);
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

int open(const char *pathname, int flags,...){
    find_fptr((void**)&open_o,__func__);
    CHECKER(open_o);
    int result;
    mode_t mode = 0;

    if(__OPEN_NEEDS_MODE(flags)){
        va_list arg;
        va_start (arg,flags);
        mode = va_arg(arg, int);
        va_end(arg);
    }
    /*
    else{
        result = open_o(pathname,flags);
        dprintf(getFD(),"[logger] %s(\"%s\", %d) = %d\n", pathname, flags, result);
    }*/
    result = open_o(pathname,flags,mode);
    char r_path[BUFSIZE]; 
    getAbsPath(pathname,r_path);
    dprintf(getFD(),"[logger] %s(\"%s\", %o, %o) = %d\n",__func__, r_path, flags, mode, result);
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
    char r_path[BUFSIZE];
    getAbsPath(pathname, r_path);
    int result = remove_o(pathname); 

    dprintf(getFD(),"[logger] %s(\"%s\") = %d\n",__func__, r_path, result);
    return result;
}

int rename(const char *old, const char *new){
    find_fptr((void**)&rename_o,__func__);
    CHECKER(rename_o);
    char r_old[BUFSIZE];
    getAbsPath(old,r_old);
    int result = rename_o(old, new);
    char r_new[BUFSIZE];
    getAbsPath(new,r_new);

    dprintf(getFD(),"[logger] %s(\"%s\", \"%s\") = %d\n", __func__, r_old, r_new, result);

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
