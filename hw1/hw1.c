#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>
#include <pwd.h>

#define MAX_PID_SIZE 20
#define MAX_PROC_NAME_SIZE 256
#define MAX_USER_NAME_SIZE 256

struct info{
    int pid;
    char proc_name[MAX_PROC_NAME_SIZE];
    char user_name[MAX_USER_NAME_SIZE];
};

bool is_pid(const char* str){
    for(size_t i=0;i<strlen(str);++i){
        if(!isdigit(str[i])){
            return false;
        }
    }
    return true;
}

void getUserName(uid_t uid, struct info *p){
    
    struct passwd *getpwuid(), *pw_ptr;
    char numstr[10];
    if(!(pw_ptr=getpwuid(uid))){
        fprintf(stderr,"Username of uid %d not found.",uid);
        exit(EXIT_FAILURE);
    }else{
        strcpy(p->user_name,pw_ptr->pw_name);
    }
}

void readInfo(char *pid, struct info *p){
    FILE *fd;
    char dir[MAX_PID_SIZE];
    struct stat buffer;
    sprintf(dir,"%s%s","/proc/",pid);
    chdir("/proc");
    if(stat(pid,&buffer)==-1){
        fprintf(stderr,"stat pid %s failed.",pid);
        exit(EXIT_FAILURE);
    }else{
        getUserName(buffer.st_uid,p);
    }
    chdir(dir);
    if((fd=fopen("stat","r"))<0){
        //this error should be handled properly
        fprintf(stderr,"Fail to open stat file.");
        exit(EXIT_FAILURE);
    }else{
        //proc/[pid]/stat/
        //pid comm
        char p_name[MAX_PROC_NAME_SIZE];
        if(!(2==fscanf(fd,"%d %s\n",&(p->pid),p_name))){
            fprintf(stderr,"Fail to get proccess name.");
            exit(EXIT_FAILURE);
        }
        strcpy(p->proc_name,p_name+1);
        p->proc_name[strlen(p->proc_name)-1]='\0';
    }
    fclose(fd);
}

int main(int argc, char **argv){
    int opt;
    char *command = NULL;
    char *filename = NULL;
    char *type = NULL;
    while ((opt = getopt(argc,argv,"c:t:f:")) != -1){
        switch(opt){
        case 'c':
            command = optarg;
            break;
        case 't':
            type = optarg;
            break;
        case 'f':
            filename = optarg;
            break;
        case '?':
        case ':':
        default:
            fprintf(stderr,"Usage: %s [-c command] [-t type] [-f filename] name\n",argv[0]);
            exit(EXIT_FAILURE);
        }
    }
    if(optind <argc){
            fprintf(stderr,"Usage: %s [-c command] [-t type] [-f filename] name\n",argv[0]);
            exit(EXIT_FAILURE);
    }
    if(type){
        if(strcmp(type,"REG") && strcmp(type,"CHR") && strcmp(type,"DIR") && strcmp(type,"FIFO") && strcmp(type,"SOCK") && strcmp(type,"unknown")){
            fprintf(stderr,"Invalid TYPE option.\n");
            exit(EXIT_FAILURE);
        }
    }

    char h1[] = "COMMAND";
    char h2[] = "PID";
    char h3[] = "USER";
    char h4[] = "FD";
    char h5[] = "TYPE";
    char h6[] = "NODE";
    char h7[] = "NAME";
    printf("%-40s%-20s%-20s%-20s%-20s%-20s%-20s\n",h1,h2,h3,h4,h5,h6,h7);

    struct dirent *dirread;
    DIR* dir = opendir("/proc");
    //DIR* dir = opendir("./");
    while((dirread = readdir(dir))){
        // traverse pid dir only 
        if(is_pid(dirread->d_name)){
            struct info p;
            readInfo(dirread->d_name,&p);
            printf("%-40s%-20s%-20s\n",p.proc_name,dirread->d_name,p.user_name);
        }
    }
    closedir(dir);


    return 0;
}
