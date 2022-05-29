#include "sdb.hpp"

char* parse_cmd(char * line, std::vector<char *>& cmd_tokens){
    cmd_tokens.clear();
    char * cmd = strtok(line," ");
    if(cmd == NULL){
        ERROR("token length = 0");
    }
    char * token = NULL;
    while((token = strtok(NULL," "))!=NULL){
        cmd_tokens.push_back(token);
    }
    return cmd;
}

int main(int argc, char *argv[]){
    FILE * fp = NULL;
    char * line = NULL;
    size_t len;
    std::vector<char *>cmd_tokens;

    SDB sdb;
    if(argc == 3){
        if(strcmp(argv[1],"-s")!=0){
            ERROR("unexpected flag.");
        }else{
            if((fp=fopen(argv[2],"r"))==NULL){
                ERROR("can not open script file.");
            }
        }
    }else if(argc == 2){
        sdb.load(argv[1]);
    }else if(argc == 1){
        //do nothing
    }else{
        ERROR("wrong parameter format.");
    }
    while(1){
        if(fp){//read command in script
            if(getline(&line,&len,fp)==-1){
                sdb.terminate();                
            }    
        }else{//read command from stdin
            SIGN();
            getline(&line,&len,stdin);
        }

        //remove newline character
        if(strlen(line) > 0 && line[strlen(line)-1]=='\n'){
            line[strlen(line)-1] = '\0';
        }
        char * cmd = parse_cmd(line,cmd_tokens);
        /*
        fprintf(stderr,"token len= %zu\n",token_num);
        for(size_t i=0;i<token_num;++i){
            fprintf(stderr,"%s\n",cmd_tokens[i]);
        }*/
        if(strcmp(cmd,"break")==0 || strcmp(cmd,"b") == 0){
            if(cmd_tokens.size() >= 1){
                unsigned long addr = strtoul(cmd_tokens[0],NULL,16);
                sdb.set_break(addr);
            }else{
                DEBUG("no addr is given.");
            }
        }else if(strcmp(cmd,"cont")    == 0 || strcmp(cmd,"c") == 0){
            sdb.cont();
        }else if(strcmp(cmd,"delete")  == 0){
            if(cmd_tokens.size()>=1){
                sdb.del(atoi(cmd_tokens[0]));
            }else{
                DEBUG("no addr is given.");
            }
        }else if(strcmp(cmd,"disasm")  == 0 || strcmp(cmd,"d") == 0){
            if(cmd_tokens.size()>=1){
                unsigned long addr = strtoul(cmd_tokens[0],NULL,16);
                sdb.disasm(addr,ASM_LINES);
            }else{
                DEBUG("no addr is given.");
            }
        }else if(strcmp(cmd,"dump")    == 0 || strcmp(cmd,"x") == 0){
            if(cmd_tokens.size()>=1){
                unsigned long long addr = strtol(cmd_tokens[0],NULL,16);
                //unsigned long addr = strtoul(cmd_tokens[0],NULL,16);
                sdb.dump(addr);
            }else{
                DEBUG("no addr is given.");
            }
        }else if(strcmp(cmd,"exit")    == 0 || strcmp(cmd,"q") == 0){
        }else if(strcmp(cmd,"get")     == 0 || strcmp(cmd,"g") == 0){
        }else if(strcmp(cmd,"getregs") == 0){
            sdb.getregs();
        }else if(strcmp(cmd,"help")    == 0 || strcmp(cmd,"h") == 0){
            sdb.help();
        }else if(strcmp(cmd,"list")    == 0 || strcmp(cmd,"l") == 0){
            sdb.list();
        }else if(strcmp(cmd,"load")    == 0){
            if(cmd_tokens.size() >= 1)
                sdb.load(cmd_tokens[0]);
            else
                DEBUG("no program path is given");
        }else if(strcmp(cmd,"run")     == 0 || strcmp(cmd,"r") == 0){
        }else if(strcmp(cmd,"vmmap")   == 0 || strcmp(cmd,"m") == 0){
        }else if(strcmp(cmd,"set")     == 0 || strcmp(cmd,"s") == 0){
        }else if(strcmp(cmd,"si")      == 0){
        }else if(strcmp(cmd,"start")   == 0){
            sdb.start();       
        }else{
            //PRINT("Unknown command.");
        }
    }
    return 0;
}
