#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#define MAX_LINE_LENGTH 1024


typedef struct command {
    int arg1;
    int arg2;
    char type;
} Command;

typedef struct commandNode {
    Command command;
    struct commandNode* next;
    struct commandNode* prev;
} CommandListNode;

typedef struct commandList {
    CommandListNode* items;
    int length;
} CommandList;

CommandList commandList;

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

void pushCommand(Command cmd)
{
    CommandListNode* newCommand = (CommandListNode*)malloc(sizeof(CommandListNode));
    newCommand->command = cmd;
    newCommand->prev = (*(&commandList.items));
    newCommand->next = NULL;
    if ((*(&commandList.items)) != NULL) (*(&commandList.items))->next = newCommand;
    (*(&commandList.items)) = newCommand;
}

void printCommand(Command cmd){
    printf("Printing cmd\n");
    printf("cmd.arg1 = %d\n", cmd.arg1);
    printf("cmd.arg2 = %d\n", cmd.arg2);
    printf("cmd.type = %c\n\n", cmd.type);
}

typedef struct editorNode {
    char* content;
    struct editorNode* next;
    struct editorNode* prev;
} EditorRowListNode;

typedef struct editorRowList {
    EditorRowListNode* tail;
    EditorRowListNode* head;
    int length;
} EditorRowList;

EditorRowList editorRowList;

void pushRow(EditorRowListNode** list, char* c)
{
    EditorRowListNode* newRow = (EditorRowListNode*)malloc(sizeof(EditorRowListNode));
    newRow->content = c;
    newRow->prev = (*list);
    newRow->next = NULL;
    if ((*list) != NULL) (*list)->next = newRow;
    if(editorRowList.head == NULL) editorRowList.head = newRow;
    (*list) = newRow;
}

void printList(CommandListNode* node)
{
    CommandListNode* last;
    while (node != NULL) {
        printCommand(node->command);
        last = node;
        node = node->next;
    }
}

EditorRowListNode* getRowAt(int index){
    EditorRowListNode* ptr = editorRowList.head;
    for(int i = 1; i<index; i++){
        ptr = ptr->next;
    }
    return ptr;
}

char* getRowContent(){
    return "ciao\n";
}

void executeChange(Command cmd){
    EditorRowListNode* row = getRowAt(cmd.arg1);
    int rows = cmd.arg2 - cmd.arg1 + 1;
    for(int i = 0; i<rows; i++){
        char* text = getRowContent();
        if(row==NULL){
            pushRow(&editorRowList.tail, text);
            row = editorRowList.tail;
        }else{
            //replaceText(row, text);
        }
        row = row->next;
    }
}

void executePrint(Command cmd){
    EditorRowListNode* row = getRowAt(cmd.arg1);
    int rows = cmd.arg2 - cmd.arg1 + 1;
    for(int i = 0; i<rows; i++){
        if(row==NULL){
            printf(".\n");
            continue;
        }else{
            printf("%s",row->content);
        }
        row = row->next;
    }
}

void executeCommand(Command cmd){
    switch (cmd.type) {
        case 'c':
            executeChange(cmd);
            break;
        case 'd':
            break;
        case 'u':
            break;
        case 'r':
            break;
        case 'p':
            executePrint(cmd);
            break;
        case 'q':
            exit(1);
    }
}

int main() {
    char cmdRaw[MAX_LINE_LENGTH+1];
    commandList.items = NULL;
    commandList.length = 0;
    editorRowList.head = NULL;
    editorRowList.tail = NULL;
    editorRowList.length = 0;

    for (int i=0; i<4; i++){
        Command command = getCommand(cmdRaw);
        pushCommand(command);
        executeCommand(command);
    }
    //printf("%lu",sizeof(cmd.arg1));
    //do{
     //   execute(getCommand(cmdRaw));
    //}while(1);
    return 0;
}

