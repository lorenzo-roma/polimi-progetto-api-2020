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
    struct commandNode *next;
    struct commandNode *prev;
} CommandListNode;

typedef struct commandList {
    CommandListNode *currentCommand;
    int currentCommandIndex;
    CommandListNode *items;
    int length;
} CommandList;

typedef struct editorNode {
    char *content;
    struct editorNode *next;
    struct editorNode *prev;
} EditorRowListNode;

typedef struct editorRowList {
    EditorRowListNode *tail;
    EditorRowListNode *head;
    int length;
} EditorRowList;

Command getCommand(char *cmd);

void freeCommands();

void freeTemp();

void pushCommand(Command cmd);

void printCommand(Command cmd);

void pushRow(EditorRowList *list, char *c);

EditorRowListNode *popRow(EditorRowList *list);

void printList(CommandListNode *node);

EditorRowListNode *getRowAt(int index);

char *getRowContent();

void replaceText(EditorRowListNode *row, char *txt);

void executeChange(Command cmd);

void executePrint(Command cmd);

void deleteRows(EditorRowListNode *start, int rows);

void executeDelete(Command cmd);

void executeUndo(Command cmd);

void executeCommand(Command cmd);

void undoCommands(int moves);

void undoCommand(Command cmd);

void undoChange(Command cmd);

void undoDelete(Command cmd);

void initStructure();

CommandList commandList;
EditorRowList editorRowList;
EditorRowList changedRowList;
EditorRowList deletedRowList;
EditorRowList tempRowList;

int main() {
    char cmdRaw[MAX_LINE_LENGTH + 1];
    initStructure();
    while (1) {
        Command command = getCommand(cmdRaw);
        executeCommand(command);
    }
}


Command getCommand(char *cmd) {
    //printf("Insert command\n");
    char *outcome = fgets(cmd, MAX_LINE_LENGTH, stdin);
    if (outcome == NULL) printf("Error while reading command!");
    if (strcmp(cmd, "q\n") == 0) return (Command) {.arg1=-1, .arg2=-1, .type='q'};
    char *ptr;
    int arg1 = (int) strtol(cmd, &ptr, 10);
    int arg2 = -1;
    char type;
    if (*ptr == 'u' || *ptr == 'r') {
        type = *ptr;
    } else {
        arg2 = (int) strtol(ptr + 1, &ptr, 10);
        type = *ptr;
    }
    return (Command) {.arg1 = arg1, .arg2 = arg2, .type = type};
}

void freeCommands(){
    commandList.items = commandList.currentCommand;
    CommandListNode *cmd = commandList.currentCommand->next;
    CommandListNode *toFree;
    do{
        toFree = cmd;
        cmd = cmd->next;
        free(toFree);
        commandList.length--;
    } while (cmd!=NULL);
}

void freeTemp(){
    if(tempRowList.tail!=NULL){
        EditorRowListNode *tmp = tempRowList.tail;
        EditorRowListNode *toFree;
        do{
            toFree = tmp;
            tmp = tmp->next;
            free(toFree);
        } while (tmp != NULL);
    }
    tempRowList.head = NULL;
    tempRowList.tail = NULL;
}

void pushCommand(Command cmd) {
    if(commandList.currentCommand!=NULL){
        if(commandList.currentCommand->next!=NULL){
            freeCommands();
            freeTemp();
        }
    }
    CommandListNode *newCommand = (CommandListNode *) malloc(sizeof(CommandListNode));
    newCommand->command = cmd;
    newCommand->prev = commandList.items;
    newCommand->next = NULL;
    if (commandList.items != NULL) (commandList.items)->next = newCommand;
    (commandList.items) = newCommand;
    commandList.length++;
    commandList.currentCommandIndex++;
    commandList.currentCommand = newCommand;
}

void printCommand(Command cmd) {
    printf("Printing cmd\n");
    printf("cmd.arg1 = %d\n", cmd.arg1);
    printf("cmd.arg2 = %d\n", cmd.arg2);
    printf("cmd.type = %c\n\n", cmd.type);
}

void pushRow(EditorRowList *list, char *c) {
    EditorRowListNode *newRow = (EditorRowListNode *) malloc(sizeof(EditorRowListNode));
    newRow->content = c;
    if (list->head != NULL) {
        newRow->prev = list->head;
    } else {
        newRow->prev = NULL;
    }
    newRow->next = NULL;
    if (list->head != NULL) list->head->next = newRow;
    if (list->tail == NULL) list->tail = newRow;
    list->head = newRow;
    list->length++;
}

EditorRowListNode *popRow(EditorRowList *list) {
    if (list->length == 0) printf("Error, trying to pop empty stack!");
    EditorRowListNode *res = list->head;
    list->head = list->head->prev;
    list->head->next = NULL;
    list->length--;
    return res;
}

void printList(CommandListNode *node) {
    CommandListNode *last;
    while (node != NULL) {
        printCommand(node->command);
        last = node;
        node = node->prev;
    }
}

EditorRowListNode *getRowAt(int index) {
    if (index > editorRowList.length) return NULL;
    int distFromHead = editorRowList.length - index;
    if (distFromHead < index) {
        EditorRowListNode *ptr = editorRowList.head;
        for (int i = 0; i < distFromHead; i++) {
            ptr = ptr->prev;
        }
        return ptr;
    } else {
        EditorRowListNode *ptr = editorRowList.tail;
        for (int i = 1; i < index; i++) {
            ptr = ptr->next;
        }
        return ptr;
    }
}

char *getRowContent() {
    char *row = NULL;
    int c;
    size_t size = 0, len = 0;
    while ((c = getchar()) != EOF && c != '\n') {
        if (len + 1 >= size) {
            if (size == 1000) {
                size = 1025;
            } else {
                size += 200;
            }
            row = realloc(row, size);
        }
        row[len++] = c;
    }
    if (row != NULL) {
        row[len++] = '\n';
        row[len] = '\0';
    }
    return row;
}

void replaceText(EditorRowListNode *row, char *txt) {
    pushRow(&changedRowList, row->content);
    row->content = txt;
}

void executeChange(Command cmd) {
    pushCommand(cmd);
    EditorRowListNode *row = getRowAt(cmd.arg1);
    int rows = cmd.arg2 - cmd.arg1 + 1;
    for (int i = 0; i < rows; i++) {
        char *text = getRowContent();
        if (row == NULL) {
            pushRow(&changedRowList, NULL);
            pushRow(&editorRowList, text);
            row = editorRowList.head;
        } else {
            replaceText(row, text);
        }
        row = row->next;
    }
    char point[4];
    char *outcome = fgets(point, 4, stdin);
    if (outcome == NULL) printf("Error while reading ending point!");
}

void executePrint(Command cmd) {
    EditorRowListNode *row;
    int rows;
    if (cmd.arg1 == 0) {
        printf(".\r\n");
        row = getRowAt(cmd.arg1 + 1);
        rows = cmd.arg2 - cmd.arg1;
    } else {
        row = getRowAt(cmd.arg1);
        rows = cmd.arg2 - cmd.arg1 + 1;
    }
    for (int i = 0; i < rows; i++) {
        if (row == NULL) {
            printf(".\r\n");
            continue;
        } else {
            printf("%s", row->content);
        }
        row = row->next;
    }
}

void deleteRows(EditorRowListNode *start, int rows) {
    start->prev = NULL;
    EditorRowListNode *tmp = start;
    for (int i = 1; i < rows; i++) tmp = tmp->next;
    tmp->next = NULL;
    if (deletedRowList.head != NULL) {
        deletedRowList.head->next = start;
    } else {
        deletedRowList.tail = start;
    }
    deletedRowList.head = tmp;
}

void executeDelete(Command cmd) {
    pushCommand(cmd);
    if (cmd.arg1 > editorRowList.length) return;
    EditorRowListNode *row = getRowAt(cmd.arg1);
    EditorRowListNode *prevRow = row->prev;
    if (cmd.arg1 == 1) {
        if (cmd.arg2 >= editorRowList.length) {
            //ottimizzabile
            deleteRows(editorRowList.tail, editorRowList.length);
            editorRowList.head = NULL;
            editorRowList.tail = NULL;
            editorRowList.length = 0;
            return;
        }
        EditorRowListNode *tmp = editorRowList.tail;
        editorRowList.tail = getRowAt(cmd.arg2 + 1);
        deleteRows(tmp, cmd.arg2);
        editorRowList.tail->prev = NULL;
        editorRowList.length -= cmd.arg2;
        return;
    }
    if (cmd.arg2 >= editorRowList.length) {
        prevRow->next = NULL;
        editorRowList.head = getRowAt(cmd.arg1 - 1);
        deleteRows(row, editorRowList.length - cmd.arg1 + 1);
        editorRowList.head->next = NULL;
        editorRowList.length = cmd.arg1 - 1;
        return;
    }
    int rows = cmd.arg2 - cmd.arg1 + 1;
    EditorRowListNode *tmp = row;
    for (int i = 0; i < rows; i++) {
        row = row->next;
    }
    prevRow->next = row;
    deleteRows(tmp, rows);
    editorRowList.length -= rows;
}

void executeUndo(Command cmd) {
    int moves = -cmd.arg1;
    Command command;
    char cmdRaw[MAX_LINE_LENGTH + 1];
    do {
        command = getCommand(cmdRaw);
        if (command.type == 'u') moves -= command.arg1;
        if (command.type == 'r') moves += command.arg1;
    } while (command.type == 'u' || command.type == 'r');
    if (moves < 0) undoCommands(moves);
    //if(moves>0) redoCommands(moves);
    executeCommand(command);
}

void executeCommand(Command cmd) {
    switch (cmd.type) {
        case 'c':
            executeChange(cmd);
            break;
        case 'd':
            executeDelete(cmd);
            break;
        case 'u':
            executeUndo(cmd);
            break;
        case 'r':
            break;
        case 'p':
            executePrint(cmd);
            break;
        case 'q':
            exit(0);
    }
}

void undoCommands(int moves) {
    moves *=-1;
    for (int i = 0; i < moves; i++) {
        undoCommand(commandList.currentCommand->command);
        commandList.currentCommand = commandList.currentCommand->prev;
        commandList.currentCommandIndex--;
    }
}

void undoCommand(Command cmd) {
    if (cmd.type == 'c') undoChange(cmd);
    if (cmd.type == 'd') undoDelete(cmd);
}

void undoChange(Command cmd) {
    EditorRowListNode *row = getRowAt(cmd.arg2);
    EditorRowListNode *toFree;
    int rows = cmd.arg2 - cmd.arg1 + 1;
    for (int i = 0; i < rows; i++) {
        pushRow(&tempRowList, row->content);
        EditorRowListNode *replace = popRow(&changedRowList);
        if(replace->content==NULL){
            toFree = row;
            row = row->prev;
            row->next = NULL;
            free(toFree);
            editorRowList.head = row;
            editorRowList.length--;
        } else {
            row->content = replace->content;
            row = row->prev;
        }
    }
}

void undoDelete(Command cmd) {}

void initStructure() {
    commandList.items = NULL;
    commandList.length = 0;
    commandList.currentCommand = NULL;
    commandList.currentCommandIndex = 0;
    editorRowList.head = NULL;
    editorRowList.tail = NULL;
    editorRowList.length = 0;
    changedRowList.head = NULL;
    changedRowList.tail = NULL;
    changedRowList.length = 0;
    deletedRowList.head = NULL;
    deletedRowList.tail = NULL;
    deletedRowList.length = 0;
    tempRowList.head = NULL;
    tempRowList.tail = NULL;
    tempRowList.length = 0;
}

