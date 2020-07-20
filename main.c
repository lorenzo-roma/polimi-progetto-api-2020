#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#define MAX_LINE_LENGTH 1024

typedef struct command {
    int arg1;
    int arg2;
    char type;
} Command;

void printCmd(Command cmd){
    printf("cmd.arg1 = %d\n", cmd.arg1);
    printf("cmd.arg2 = %d\n", cmd.arg2);
    printf("cmd.type = %c\n", cmd.type);
}

Command getCommand(char *cmd){
    printf("Insert command\n");
    fgets(cmd, MAX_LINE_LENGTH, stdin);
    if(strcmp(cmd, "q\n")==0) return (Command){.arg1=-1, .arg2=-1, .type='q'};
    char *ptr;
    int arg1 = (int) strtol(cmd, &ptr, 10);
    int arg2 = -1;
    char type;
    if(*ptr=='u'||*ptr=='r') {
        type = *ptr;
    }else{
        arg2 = (int) strtol(ptr+1, &ptr, 10);
        type = *ptr;
    }
    return (Command){.arg1 = arg1, .arg2 = arg2, .type = type};
}

void execute(Command cmd){
    printCmd(cmd);
    switch (cmd.type) {
        case 'c':
            break;
        case 'd':
            break;
        case 'u':
            break;
        case 'r':
            break;
        case 'p':
            break;
        case 'q':
            exit(1);
    }
}

int main() {
    char cmdRaw[MAX_LINE_LENGTH];
    //printf("%lu",sizeof(cmd.arg1));
    //do{
        execute(getCommand(cmdRaw));
    //}while(1);
    return 0;
}

