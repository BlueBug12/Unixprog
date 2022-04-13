#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <fcntl.h>

char shared_lib[32] = "./logger.so";
char *command_buf[32];

int main(int argc, char **argv){

    if(argc < 2){
        fprintf(stderr,"no command given.\n");
        exit(EXIT_FAILURE);
    }

    int opt;
    int fd = -1;
    char fd_buf[16];
    
    setenv("LD_PRELOAD",shared_lib,1);
    setenv("LOGGER","2",1);//stderr

    while((opt = getopt(argc,argv,"p:o:"))!=-1){
        switch(opt){
            case 'p':
                setenv("LD_PRELOAD",optarg,1);
                break;
            case 'o':
                fd = open(optarg, O_WRONLY | O_CREAT | O_TRUNC, S_IRWXU);
                sprintf(fd_buf,"%d",fd);
                setenv("LOGGER",fd_buf,1);
                break;
            default:
                fprintf(stderr, "usage: ./logger [-o file] [-p sopath] [--] cmd [cmd args ...]\n");
                fprintf(stderr, "-p: set the path to logger.so, default = ./logger.so\n");
                fprintf(stderr, "-o: print output to file, print to \"stderr\" if no file specified\n");
                fprintf(stderr, "--: separate the arguments for logger and for the command\n");
                exit(EXIT_FAILURE);
        }
    }
    opt = 0;
    for(int i = optind; i < argc; ++i){
        command_buf[opt++] = argv[i];
    }
    pid_t pid;
    if((pid = fork()) < 0){
        fprintf(stderr,"fork failed.\n");
        exit(EXIT_FAILURE);
    }else if(pid==0){//child process
        if(execvp(argv[optind], command_buf)==-1){
            fprintf(stderr,"WRONG\n");
            return 0;
        
        }

    }else{
        while(waitpid(pid,NULL,WNOHANG)!=-1);
        if(fd>0){
            close(fd);
        }
    }

    return 0;
}
