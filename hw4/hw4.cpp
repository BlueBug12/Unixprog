#include "sdb.hpp"

int main(int argc, char *argv[]){
    char script_nmae[64];
    char program_name[64];
    FILE * fp = NULL;
    size_t len = 0;
    char * line = NULL;
    if(argc == 3){
        if(strcmp(argv[1],"-s")!=0){
            ERROR("unexpected flag.");
        }else{
            strcpy(script_nmae,argv[2]);
            if((fp=fopen(argv[2],"r"))==NULL){
                ERROR("can not open script file.");
            }
        }
    }else if(argc == 2){
        strcpy(program_name,argv[1]);
    }else if(argc == 1){
        //do nothing
    }else{
        ERROR("wrong parameter format.");
    }

    while(1){
        if(fp){//read command in script
            if(getline(&line,&len,fp)==-1){
                terminate();                
            }    
        }else{//read command from stdin
            SIGN();
            getline(&line,&len,stdin);
        }

        char * token = NULL;
        char * cmd_tokens[MAX_TOKEN_NUM];
        size_t token_num = 0;
        //remove newline character
        if(strlen(line) > 0 && line[strlen(line)-1]=='\n'){
            line[strlen(line)-1] = '\0';
        }
        token = strtok(line," ");
        while(token!=NULL){
            cmd_tokens[token_num++]=token;    
            token = strtok(NULL," ");
        }
        /*
        fprintf(stderr,"token len= %zu\n",token_num);
        for(size_t i=0;i<token_num;++i){
            fprintf(stderr,"%s\n",cmd_tokens[i]);
        }*/
        if(token_num==0){
            ERROR("no command token received.");
        }
        if(strcmp(cmd_tokens[0],"break")==0 || strcmp(cmd_tokens[0],"b") == 0){
            
        }else if(strcmp(cmd_tokens[0],"cont")    == 0 || strcmp(cmd_tokens[0],"c") == 0){
        }else if(strcmp(cmd_tokens[0],"delete")  == 0){
        }else if(strcmp(cmd_tokens[0],"disasm")  == 0 || strcmp(cmd_tokens[0],"d") == 0){
        }else if(strcmp(cmd_tokens[0],"dump")    == 0 || strcmp(cmd_tokens[0],"x") == 0){
        }else if(strcmp(cmd_tokens[0],"exit")    == 0 || strcmp(cmd_tokens[0],"q") == 0){
        }else if(strcmp(cmd_tokens[0],"get")     == 0 || strcmp(cmd_tokens[0],"g") == 0){
        }else if(strcmp(cmd_tokens[0],"getregs") == 0){
        }else if(strcmp(cmd_tokens[0],"help")    == 0 || strcmp(cmd_tokens[0],"h") == 0){
            help();
        }else if(strcmp(cmd_tokens[0],"list")    == 0 || strcmp(cmd_tokens[0],"l") == 0){
        }else if(strcmp(cmd_tokens[0],"load")    == 0){
        }else if(strcmp(cmd_tokens[0],"run")     == 0 || strcmp(cmd_tokens[0],"r") == 0){
        }else if(strcmp(cmd_tokens[0],"vmmap")   == 0 || strcmp(cmd_tokens[0],"m") == 0){
        }else if(strcmp(cmd_tokens[0],"set")     == 0 || strcmp(cmd_tokens[0],"s") == 0){
        }else if(strcmp(cmd_tokens[0],"si")      == 0){
        }else if(strcmp(cmd_tokens[0],"start")   == 0){
               
        }else{
            //PRINT("Unknown command.");
        }
    }
    return 0;
}
