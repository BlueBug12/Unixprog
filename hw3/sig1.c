#include "libmini.h"

static void sig_quit(int signo){
    if(signo==SIGQUIT){
        char m[] = "handler catched\n";
        write(1,m,strlen(m));
    }else{
        char m[] = "No way...\n";
        write(1,m,strlen(m));
    }
}

int main(){
    sigset_t newmask, oldmask, pendmask;
    if(signal(SIGQUIT,sig_quit)==SIG_ERR){
        perror("signal error\n");    
    }
    sigemptyset(&newmask);
    sigaddset(&newmask,SIGQUIT);
    if(sigprocmask(SIG_BLOCK,&newmask,&oldmask)<0){
        perror("SIG_BLOCK error\n");
    }
    sleep(5);
    if(sigpending(&pendmask)<0){
        perror("sigpending error\n");
    }
    
    if(sigismember(&pendmask,SIGQUIT)){
        char m[] = "SIGQUIT pending\n";
        write(1,m,strlen(m));
    }
    if(sigprocmask(SIG_SETMASK,&oldmask,NULL)<0){
        perror("SIG_SETMASK error\n");
    }
    char mse[] = "SIGQUIT unblocked\n";
    write(1,mse,strlen(mse));
    sleep(10);
    char mse2[] = "del SIGQUIT signal\n";
    write(1,mse2,strlen(mse2));
    sigdelset(&newmask,SIGQUIT);
    if(sigismember(&newmask,SIGQUIT)){
        perror("sigdelset error\n");
    }
    if(sigprocmask(SIG_BLOCK,&newmask,&oldmask)<0){
        perror("SIG_BLOCK error\n");
    }

    sleep(5);
    char mse3[] = "end\n";
    write(1,mse3,strlen(mse3));
    exit(0);
}

