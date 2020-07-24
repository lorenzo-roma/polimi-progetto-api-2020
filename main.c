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
    CommandListNode *items;
    int length;
} CommandList;

CommandList commandList;

Command getCommand(char *cmd) {
    printf("Insert command\n");
    fgets(cmd, MAX_LINE_LENGTH, stdin);
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

void pushCommand(Command cmd) {
    CommandListNode *newCommand = (CommandListNode *) malloc(sizeof(CommandListNode));
    newCommand->command = cmd;
    newCommand->prev = (*(&commandList.items));
    newCommand->next = NULL;
    if ((*(&commandList.items)) != NULL) (*(&commandList.items))->next = newCommand;
    (*(&commandList.items)) = newCommand;
}

void printCommand(Command cmd) {
    printf("Printing cmd\n");
    printf("cmd.arg1 = %d\n", cmd.arg1);
    printf("cmd.arg2 = %d\n", cmd.arg2);
    printf("cmd.type = %c\n\n", cmd.type);
}

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

EditorRowList editorRowList;
EditorRowList changedRowList;
EditorRowList deletedRowList;

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

void printList(CommandListNode *node) {
    CommandListNode *last;
    while (node != NULL) {
        printCommand(node->command);
        last = node;
        node = node->next;
    }
}

EditorRowListNode *getRowAt(int index) {
    EditorRowListNode *ptr = editorRowList.tail;
    for (int i = 1; i < index; i++) {
        ptr = ptr->next;
    }
    return ptr;
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
}

void executePrint(Command cmd) {
    EditorRowListNode *row = getRowAt(cmd.arg1);
    int rows = cmd.arg2 - cmd.arg1 + 1;
    for (int i = 0; i < rows; i++) {
        if (row == NULL) {
            printf(".\n");
            continue;
        } else {
            printf("%s", row->content);
        }
        row = row->next;
    }
}

void deleteRows(EditorRowListNode *start, int rows){
    start->prev = NULL;
    EditorRowListNode *tmp = start;
    for(int i =1; i<rows; i++) tmp = tmp->next;
    tmp->next = NULL;
    if(deletedRowList.head != NULL){
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
        deleteRows(row, editorRowList.length-cmd.arg1+1);
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

void executeCommand(Command cmd) {
    switch (cmd.type) {
        case 'c':
            executeChange(cmd);
            break;
        case 'd':
            executeDelete(cmd);
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

void initStructure() {
    commandList.items = NULL;
    commandList.length = 0;
    editorRowList.head = NULL;
    editorRowList.tail = NULL;
    editorRowList.length = 0;
    changedRowList.head = NULL;
    changedRowList.tail = NULL;
    changedRowList.length = 0;
    deletedRowList.head = NULL;
    deletedRowList.tail = NULL;
    deletedRowList.length = 0;
}

int main() {
    char cmdRaw[MAX_LINE_LENGTH + 1];
    initStructure();

    for (int i = 0; i < 99; i++) {
        Command command = getCommand(cmdRaw);
        executeCommand(command);
    }
    //printf("%lu",sizeof(cmd.arg1));
    //do{
    //   execute(getCommand(cmdRaw));
    //}while(1);
    return 0;
}

