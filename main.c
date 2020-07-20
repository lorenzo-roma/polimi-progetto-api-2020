#include <stdio.h>
#define MAX_LINE_LENGTH 1024

void getCommand(char *cmd){
    printf("Insert command\n");
    fgets(cmd, MAX_LINE_LENGTH, stdin);
}
int main() {

    char cmdRaw[MAX_LINE_LENGTH];
    for (int i = 0; i < 3; ++i) {
        getCommand(cmdRaw);
        
        printf("I have read: %s\n", cmdRaw);
    }

    return 0;
}

