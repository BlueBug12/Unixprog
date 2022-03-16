#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>
#include <errno.h>
#include <pwd.h>
#include <regex.h>
#include <unordered_set>

#define MAX_PID_SIZE 20
#define MAX_NAME_SIZE 256

char *comm_regex = NULL;
char *file_regex = NULL;
char *type_filter = NULL;

struct info{
    int pid;
    char proc_name[MAX_NAME_SIZE];
    char user_name[MAX_NAME_SIZE];
    char fd[10];
    char type[30];
    int inode;
    char name[MAX_NAME_SIZE];
};

void error(const char* str, int error_no){
    fprintf(stderr,"Error: fail to %s, %s\n",str,strerror(error_no));
    exit(EXIT_FAILURE);
}

void print_pinfo(struct info *p){
    if(type_filter && strcmp(type_filter,p->type)){
        return;
    }
    if(comm_regex){
        regex_t preg;
        if(regcomp(&preg,comm_regex,REG_EXTENDED)==0){
            const size_t nmatch = 1;
            regmatch_t matchptr[nmatch];
            if(regexec(&preg,p->proc_name,nmatch,matchptr,0)==REG_NOMATCH){
                regfree(&preg);
                return;
            }
            regfree(&preg);
        }else{
            error("regcomp",errno);
        }
    }
    
    if(file_regex){
        regex_t preg;
        if(regcomp(&preg,file_regex,REG_EXTENDED)==0){
            const size_t nmatch = 1;
            regmatch_t matchptr[nmatch];
            if(regexec(&preg,p->name,nmatch,matchptr,0)==REG_NOMATCH){
                regfree(&preg);
                return;
            }
            regfree(&preg);
        }else{
            error("regcomp",errno);
        }
        
    }
    

    if(p->inode==-1 || strcmp(p->type,"unknown")==0)
        printf("%-40s%-20d%-20s%-20s%-20s%-20s%-20s\n",p->proc_name,p->pid,p->user_name,p->fd,p->type," ",p->name);
    else
        printf("%-40s%-20d%-20s%-20s%-20s%-20d%-20s\n",p->proc_name,p->pid,p->user_name,p->fd,p->type,p->inode,p->name);

}

bool is_num(const char* str){
    for(size_t i=0;i<strlen(str);++i){
        if(!isdigit(str[i])){
            return false;
        }
    }
    return true;
}

int getUserName(uid_t uid, struct info *p){
    
    struct passwd *pw_ptr;
    if(!(pw_ptr=getpwuid(uid))){
        fprintf(stderr,"Username of uid %d not found.",uid);
        //exit(EXIT_FAILURE);
        return -1;
    }else{
        strcpy(p->user_name,pw_ptr->pw_name);
        return 0;
    }
}

void getMeta(const char* p_path, const char* mode,struct info *p){
    char path[MAX_NAME_SIZE];
    //memset(path,0,MAX_NAME_SIZE);
    strcpy(path,p_path);
    strcat(path,mode);
    struct stat buffer;
    if(stat(path,&buffer)==0){
        memset(p->name,0,MAX_NAME_SIZE);
        if(readlink(path,p->name,MAX_NAME_SIZE)==-1){
            error("readlink",errno);
        }else{
            p->inode = buffer.st_ino;
            char type[30];
            switch(buffer.st_mode & S_IFMT){
                case S_IFDIR: 
                    strcpy(type,"DIR");
                    break;
                case S_IFREG: 
                    strcpy(type,"REG");
                    break;
                case S_IFCHR: 
                    strcpy(type,"CHR");
                    break;
                case S_IFIFO: 
                    sprintf(type,"%s%d%c","FIFO[",p->inode,']');
                    break;
                case S_IFSOCK: 
                    sprintf(type,"%s%d%c","socket[",p->inode,']');
                    break;
                default:
                    strcpy(type,"unknown");
                    break;
            }
            strcpy(p->type,type);
        }
    }else{
        if(errno==EACCES){
            strcat(path," (Permission denied)");
            p->inode = -1;
            strcpy(p->type,"unknown");
            strcpy(p->name,path);
        }else{
            error("stat",errno);
        }
    }
}

void readMem(char *pid, struct info *p){
    FILE *fd;
    char path[MAX_NAME_SIZE];
    sprintf(path,"%s%s%s","/proc/",pid,"/maps");

    //not sure
    strcpy(p->type,"REG");
    std::unordered_set<int>dict;
    dict.insert(p->inode);
    if((fd=fopen(path,"r"))!=NULL){
        //address perms offset dev inode pathname
        char *line = NULL;
        char *token = NULL; 
        char name[MAX_NAME_SIZE];
        size_t len = 0;
        while(getline(&line,&len,fd)!=EOF){
            if((token=strtok(line," "))!=NULL){
                token = NULL;//address
            }
            if((token=strtok(NULL," "))!=NULL){
                token = NULL;//perms
            }
            if((token=strtok(NULL," "))!=NULL){
                token = NULL;//offset
            }
            if((token=strtok(NULL," "))!=NULL){
                token = NULL;//dev
            }
            if((token=strtok(NULL," "))!=NULL){
                //inode
                //duplicated segments
                int seg_pid = atoi(token);
                if(seg_pid==0 || dict.find(seg_pid)!=dict.end()){
                    continue;
                }else{
                    p->inode = seg_pid;
                    dict.insert(seg_pid);
                }
            }
            if((token=strtok(NULL," "))!=NULL){
                //pathname 
                strcpy(name,token);
            }else{
                error("strtok",errno);
            }

            if(strlen(name)!=0){
                if(name[strlen(name)-1]=='\n'){
                    name[strlen(name)-1]='\0';
                }
            }else{
                error("unexcepted mem name",errno);
            }
            
            if(strstr(name,"(deleted)") != NULL){
                //remove "(deleted)" string
                size_t des = strlen(p->name) - 10;
                for(size_t i =strlen(name)-1;i>des;--i){
                    name[i] = '\0';
                }
                strcpy(p->fd,"DEL");
            }else{
                strcpy(p->fd,"mem");
            }
            strcpy(p->name,name);
            print_pinfo(p);
        }
        free(line);
        fclose(fd);
    }
}

void readFd(char *pid, struct info *p){
    char path[MAX_NAME_SIZE];
    sprintf(path,"%s%s%s","/proc/",pid,"/fd");

    struct dirent *dirread;
    DIR* dir = opendir(path);
    if(dir==NULL){
        strcpy(p->fd,"NOFD");
        strcpy(p->type,"");
        p->inode = -1;
        strcpy(p->name,strcat(path," (Permission denied)"));
        print_pinfo(p);
    }else{
        if(chdir(path)==-1){
            error("chdir",errno);
        }
        
        while((dirread=readdir(dir))!=NULL){
            if(is_num(dirread->d_name)){   
                memset(p->name,0,MAX_NAME_SIZE);
                if(readlink(dirread->d_name,p->name,MAX_NAME_SIZE)==-1){
                    //if(errno!=ENOENT)
                    error("readlink",errno);
                }else{
                    if(p->name[strlen(p->name)-1]=='\n')
                        p->name[strlen(p->name)-1] = '\0';
                    
                    struct stat buffer;
                    if(stat(dirread->d_name,&buffer)<0){
                        error("stat",errno);
                    }else{
                        p->inode = buffer.st_ino;
                        strcpy(p->fd,dirread->d_name);
                        switch(buffer.st_mode & S_IFMT){
                            case S_IFDIR: 
                                strcpy(p->type,"DIR");
                                break;
                            case S_IFREG: 
                                strcpy(p->type,"REG");
                                break;
                            case S_IFCHR: 
                                strcpy(p->type,"CHR");
                                break;
                            case S_IFIFO: 
                                strcpy(p->type,"FIFO");
                                sprintf(p->name,"%s%d%c","pipe[",p->inode,']');
                                break;
                            case S_IFSOCK: 
                                strcpy(p->type,"SOCK");
                                sprintf(p->name,"%s%d%c","socket[",p->inode,']');
                                break;
                            default:
                                strcpy(p->name,"");
                                strcpy(p->type,"unknown");
                                break;
                        }

                        if(access(dirread->d_name,R_OK & W_OK)==0){
                            strcat(p->fd,"u");
                        }else if(access(dirread->d_name,R_OK)==0){
                            strcat(p->fd,"r");
                        }else if(access(dirread->d_name,W_OK)==0){
                            strcat(p->fd,"w");
                        }else{
                            error("unexceped W/R permission",errno);
                        }

                        if(strstr(p->name,"(deleted)") != NULL){
                            //remove "(deleted)" string
                            size_t des = strlen(p->name) - 10;
                            for(size_t i =strlen(p->name)-1;i>des;--i){
                                p->name[i] = '\0';
                            }
                            strcpy(p->type,"unknown");
                            p->inode = -1;
                        }

                        print_pinfo(p);
                    }
                }
            }
        }
    }

}

void readInfo(char *pid, struct info *p){
    FILE *fd;
    char dir[MAX_NAME_SIZE];
    struct stat buffer;
    sprintf(dir,"%s%s%c","/proc/",pid,'/');
    chdir("/proc");
    if(stat(pid,&buffer)==-1){
        error("stat",errno);
    }else{
        getUserName(buffer.st_uid,p);
    }
    chdir(dir);
    if((fd=fopen("stat","r"))==NULL){
        //this error should be handled properly
        error("fopen",errno);
    }else{
        //proc/[pid]/stat/
        //pid comm
        char p_name[MAX_NAME_SIZE];
        if(!(2==fscanf(fd,"%d %s\n",&(p->pid),p_name))){
            error("fscanf",errno);
        }
        strcpy(p->proc_name,p_name+1);
        p->proc_name[strlen(p->proc_name)-1]='\0';
    }
    strcpy(p->fd,"cwd");
    getMeta(dir,p->fd,p);
    print_pinfo(p);

    getMeta(dir,"root",p);
    strcpy(p->fd,"rtd");
    print_pinfo(p);

    getMeta(dir,"exe",p);
    strcpy(p->fd,"txt");
    print_pinfo(p);
    
    readMem(pid,p);
    readFd(pid,p);
    fclose(fd);
}

int main(int argc, char **argv){
    int opt;
    while ((opt = getopt(argc,argv,"c:t:f:")) != -1){
        switch(opt){
        case 'c':
            comm_regex = optarg;
            break;
        case 't':
            type_filter = optarg;
            break;
        case 'f':
            file_regex = optarg;
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
    if(type_filter){
        if(strcmp(type_filter,"REG") && strcmp(type_filter,"CHR") && strcmp(type_filter,"DIR") && strcmp(type_filter,"FIFO") && strcmp(type_filter,"SOCK") && strcmp(type_filter,"unknown")){
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
        if(is_num(dirread->d_name)){
            struct info p;
            readInfo(dirread->d_name,&p);
        }
    }
    closedir(dir);


    return 0;
}
