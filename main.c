#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define MAX_LINE_LENGTH 1024

//new_structure branch

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
    CommandListNode *head;
    CommandListNode *tail;
    int length;
} CommandList;

typedef struct versionListNode {
    char *content;
    int version;
    struct versionListNode *prev;
} VersionListNode;



typedef struct editorNode {
    VersionListNode *versionStackTail;
    VersionListNode *versionStackHead;
    struct editorNode *next;
    struct editorNode *prev;
} EditorRowListNode;

typedef struct editorRowList {
    EditorRowListNode *tail;
    EditorRowListNode *head;
    int length;
} EditorRowList;

typedef struct deletedNode {
    struct deletedNode *prev;
    EditorRowListNode *startPtr;
    EditorRowListNode *endPtr;
} DeletedNode;

typedef enum {
    false, true
} bool;

Command getCommand(char *cmd);

void freeCommands();

VersionListNode *checkToFreeVersion(VersionListNode *version, VersionListNode **head);

EditorRowListNode *checkToFreeRow(EditorRowListNode *row);

void freeTemp();

void pushCommand(Command cmd);

void pushRow(EditorRowList *list, EditorRowListNode *newRow);

EditorRowListNode *popRow(EditorRowList *list);

EditorRowListNode *getRowAt(EditorRowList *list, int index);

char *getRowContent();

VersionListNode *createNewVersion(char *str);

EditorRowListNode *createNewRow(char *str);

void addVersion(EditorRowListNode *row, char *str);

void executeChange(Command cmd);

void executePrint(Command cmd);

void deleteRows(int startIndex, int endIndex);

void executeDelete(Command cmd, bool isRedo);

int getCommandIndex(int prevIndex, int moves);

void executeUndoRedo(Command cmd);

void debugCheckList(EditorRowList list);

void debugCheckLists();

void executeCommand(Command cmd);

void redoCommands(int moves);

void redoCommand(Command cmd);

void redoChange();

void undoCommands(int moves);

void undoCommand(Command cmd);

void undoDelete(Command cmd);

void bindNodes(EditorRowListNode *first, EditorRowListNode *second);

void undoChange();

bool isEmpty(EditorRowList *list);

void printCorrectVersion(EditorRowListNode *row);

void initStructure();

CommandList commandList;
EditorRowList editorRowList;
DeletedNode *deleteStack;
int globalVersion;

int main() {
    char cmdRaw[MAX_LINE_LENGTH + 1];
    initStructure();
    while (1) {
        Command command = getCommand(cmdRaw);
        executeCommand(command);
    }
}

Command getCommand(char *cmd) {
    char *outcome = fgets(cmd, 100, stdin);
    if (strcmp(cmd, "q\n") == 0) return (Command) {.arg1=-1, .arg2=-1, .type='q'};
    if (outcome == NULL) {
        printf("Error while reading command! |%s|", cmd);
        exit(1);
    }
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

void freeCommands() {
    CommandListNode *cmd;
    if (commandList.currentCommand != NULL) {
        commandList.head = commandList.currentCommand;
        cmd = commandList.currentCommand->next;
    } else {
        cmd = commandList.tail;
    }
    CommandListNode *toFree;
    do {
        toFree = cmd;
        cmd = cmd->next;
        free(toFree);
        commandList.length--;
    } while (cmd != NULL);
    if (commandList.currentCommand == NULL) {
        commandList.tail = NULL;
        commandList.head = NULL;
    }
}

VersionListNode *checkToFreeVersion(VersionListNode *version, VersionListNode **head) {
    VersionListNode *prev = version->prev;
    free(version->content);
    free(version);
    *head = prev;

    return prev;
}

EditorRowListNode *checkToFreeRow(EditorRowListNode *row) {
    EditorRowListNode *prev = row->prev;
    VersionListNode *versionToFree = row->versionStackHead;
    while (versionToFree != NULL&&versionToFree->version>=globalVersion) versionToFree = checkToFreeVersion(versionToFree, &row->versionStackHead);
    if (row->versionStackHead == NULL) {
        free(row);
        editorRowList.head = prev;
        if(editorRowList.head!=NULL){
            editorRowList.head->next = NULL;
        } else {
            editorRowList.tail = NULL;
        }
        editorRowList.length--;
    }
    return prev;
}

void freeTemp() {
    EditorRowListNode *row = editorRowList.head;
    int length = editorRowList.length;
    for (int i = 0; i < length; i++) {
        row = checkToFreeRow(row);
    }
}

void pushCommand(Command cmd) {
    globalVersion++;
    if (commandList.length > 0) {
        if (commandList.currentCommand == NULL) {
            freeCommands();
            freeTemp();
        } else if (commandList.currentCommand->next != NULL) {
            freeCommands();
            freeTemp();
        }
    }
    CommandListNode *newCommand = (CommandListNode *) malloc(sizeof(CommandListNode));
    newCommand->command = cmd;
    newCommand->prev = commandList.head;
    newCommand->next = NULL;
    if (commandList.head != NULL) {
        (commandList.head)->next = newCommand;
    } else {
        commandList.tail = newCommand;
    }
    (commandList.head) = newCommand;
    commandList.length++;
    commandList.currentCommandIndex++;
    commandList.currentCommand = newCommand;
}

void pushRow(EditorRowList *list, EditorRowListNode *newRow) {
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
    if (list->length == 1) {
        list->head = NULL;
        list->tail = NULL;
    } else {
        list->head = list->head->prev;
        list->head->next = NULL;
    }
    list->length--;
    return res;
}

EditorRowListNode *getRowAt(EditorRowList *list, int index) {
    if (index < 1) return NULL;
    if (index > list->length) return NULL;
    int distFromHead = list->length - index;
    if (distFromHead < index) {
        EditorRowListNode *ptr = list->head;
        for (int i = 0; i < distFromHead; i++) {
            ptr = ptr->prev;
        }
        return ptr;
    } else {
        EditorRowListNode *ptr = list->tail;
        for (int i = 1; i < index; i++) {
            ptr = ptr->next;
        }
        return ptr;
    }
}

char *getRowContent() {
    char row[1025];
    int c;
    size_t len = 0;
    while ((c = getchar_unlocked()) != EOF && c != '\n') {
        row[len++] = c;
    }
    row[len++] = '\n';
    row[len++] = '\0';

    char *res = (char *) malloc(len);
    memcpy(res, row, len);
    return res;
}

VersionListNode *createNewVersion(char *str) {
    VersionListNode *versionListNode = (VersionListNode *) malloc(sizeof(VersionListNode));
    versionListNode->content = str;
    versionListNode->version = globalVersion;
    versionListNode->prev = NULL;
    return versionListNode;
}

EditorRowListNode *createNewRow(char *str) {
    EditorRowListNode *newRow = (EditorRowListNode *) malloc(sizeof(EditorRowListNode));
    newRow->versionStackHead = NULL;
    addVersion(newRow, str);
    return newRow;
}

void addVersion(EditorRowListNode *row, char *str) {
    if (row->versionStackHead == NULL) {
        row->versionStackHead = createNewVersion(str);
        row->versionStackTail = row->versionStackHead;
    } else {
        VersionListNode *newVersion = createNewVersion(str);
        newVersion->prev = row->versionStackHead;
        row->versionStackHead = newVersion;
    }
}

void executeChange(Command cmd) {
    pushCommand(cmd);
    EditorRowListNode *row = getRowAt(&editorRowList, cmd.arg1);
    int rows = cmd.arg2 - cmd.arg1 + 1;
    for (int i = 0; i < rows; i++) {
        char *text = getRowContent();
        if (row == NULL) {
            pushRow(&editorRowList, createNewRow(text));
            row = editorRowList.head;
        } else {
            addVersion(row, text);
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
        printf(".\n");
        row = getRowAt(&editorRowList, cmd.arg1 + 1);
        rows = cmd.arg2 - cmd.arg1;
    } else {
        row = getRowAt(&editorRowList, cmd.arg1);
        rows = cmd.arg2 - cmd.arg1 + 1;
    }
    for (int i = 0; i < rows; i++) {
        if (row == NULL) {
            printf(".\n");
            continue;
        } else {
            printCorrectVersion(row);
        }
        row = row->next;
    }
}

void printCorrectVersion(EditorRowListNode *row){
    if(row->versionStackTail->version>globalVersion){
        printf(".\n");
        return;
    }
    VersionListNode *versionToPrint = row->versionStackHead;
    while (versionToPrint->version > globalVersion) {
        versionToPrint = versionToPrint->prev;
        if (versionToPrint == NULL) {
            printf(".\n");
            return;
        }
    }
    printf("%s", versionToPrint->content);
}

void addToDeletedStack(EditorRowListNode *start, EditorRowListNode *end){
    DeletedNode *newNode = malloc(sizeof(DeletedNode));
    newNode->startPtr = start;
    newNode->endPtr = end;
    if(deleteStack==NULL){
        deleteStack = newNode;
        newNode->prev = NULL;
    } else {
        newNode->prev = deleteStack;
        deleteStack = newNode;
    }
}

void deleteRows(int startIndex, int endIndex) {
    EditorRowListNode *start = getRowAt(&editorRowList, startIndex);
    EditorRowListNode *end = getRowAt(&editorRowList, endIndex);
    addToDeletedStack(start, end);
}

void executeDelete(Command cmd, bool isRedo) {
    if (cmd.arg1 == -1) return;
    if (!isRedo) pushCommand(cmd);
    if (cmd.arg1 > editorRowList.length) {
        commandList.currentCommand->command.arg1 = -1;
        return;
    }
    EditorRowListNode *row = getRowAt(&editorRowList, cmd.arg1);
    EditorRowListNode *prevRow = row->prev;
    if (cmd.arg1 == 1) {
        if (cmd.arg2 >= editorRowList.length) {
            deleteRows(1, editorRowList.length);
            if (!isRedo)commandList.currentCommand->command.arg2 = editorRowList.length;
            editorRowList.head = NULL;
            editorRowList.tail = NULL;
            editorRowList.length = 0;
            return;
        }
        EditorRowListNode *newTail = getRowAt(&editorRowList, cmd.arg2 + 1);
        deleteRows(1, cmd.arg2);
        editorRowList.tail = newTail;
        editorRowList.tail->prev = NULL;
        editorRowList.length -= cmd.arg2;
        return;
    }
    if (cmd.arg2 >= editorRowList.length) {
        prevRow->next = NULL;
        deleteRows(cmd.arg1, editorRowList.length);
        editorRowList.head = prevRow;
        if (!isRedo)commandList.currentCommand->command.arg2 = editorRowList.length;
        editorRowList.length = cmd.arg1 - 1;
        return;
    }
    int rows = cmd.arg2 - cmd.arg1 + 1;
    EditorRowListNode *tmp = getRowAt(&editorRowList, cmd.arg2 + 1);
    deleteRows(cmd.arg1, cmd.arg2);
    bindNodes(prevRow, tmp);
    editorRowList.length -= rows;
}

int getCommandIndex(int prevIndex, int moves) {
    if (moves == 0) return prevIndex;
    if (moves > 0) {
        return (prevIndex + moves < commandList.length) ? prevIndex + moves : commandList.length;
    }
    if (moves < 0) {
        return (prevIndex + moves > 0) ? prevIndex + moves : 0;
    }
}

void executeUndoRedo(Command cmd) {
    int startingCommand = commandList.currentCommandIndex;
    int finalCommand = startingCommand;
    finalCommand = (cmd.type == 'u') ? getCommandIndex(finalCommand, -cmd.arg1) : getCommandIndex(finalCommand,
                                                                                                  cmd.arg1);
    Command command;
    char raw[MAX_LINE_LENGTH + 1];
    do {
        command = getCommand(raw);
        if (command.type == 'u') finalCommand = getCommandIndex(finalCommand, -command.arg1);
        if (command.type == 'r') finalCommand = getCommandIndex(finalCommand, command.arg1);
    } while (command.type == 'u' || command.type == 'r');
    int moves = finalCommand - startingCommand;
    if (moves < 0) undoCommands(moves);
    if (moves > 0) redoCommands(moves);
    executeCommand(command);
}

void debugCheckList(EditorRowList list) {
    EditorRowListNode *r = list.tail;
    if (list.length == 0 && list.tail == NULL && list.head == NULL) return;
    for (int i = 1; i < list.length; i++) {
        r = r->next;
    }
    if (r->next != NULL && r != list.head) {
        printf("Error");
    }
    for (int i = 1; i < list.length; i++) {
        r = r->prev;
    }
    if (r->prev != NULL && r != list.tail) {
        printf("Error");
    }
}

void debugCheckLists() {
    debugCheckList(editorRowList);
}

void executeCommand(Command cmd) {
    //debugCheckLists();
    switch (cmd.type) {
        case 'c':
            executeChange(cmd);
            break;
        case 'd':
            executeDelete(cmd, false);
            break;
        case 'u':
            executeUndoRedo(cmd);
            break;
        case 'r':
            executeUndoRedo(cmd);
            break;
        case 'p':
            executePrint(cmd);
            break;
        case 'q':
            exit(0);
    }
}

void redoCommands(int moves) {
    int availableCommands = commandList.length - commandList.currentCommandIndex;
    if (moves > availableCommands) moves = availableCommands;
    for (int i = 0; i < moves; i++) {
        if (commandList.currentCommand != NULL) {
            commandList.currentCommand = commandList.currentCommand->next;
        } else {
            commandList.currentCommand = commandList.tail;
        }
        commandList.currentCommandIndex++;
        globalVersion++;
        redoCommand(commandList.currentCommand->command);
    }
}

void redoCommand(Command cmd) {
    if (cmd.type == 'c') redoChange(cmd);
    if (cmd.type == 'd') executeDelete(cmd, true);
}

void redoChange() {}

void undoCommands(int moves) {
    moves *= -1;
    if (moves > commandList.currentCommandIndex) moves = commandList.currentCommandIndex;
    for (int i = 0; i < moves; i++) {
        undoCommand(commandList.currentCommand->command);
        commandList.currentCommand = commandList.currentCommand->prev;
        commandList.currentCommandIndex--;
        globalVersion--;
    }
}

void undoCommand(Command cmd) {
    if (cmd.type == 'c') undoChange(cmd);
    if (cmd.type == 'd') undoDelete(cmd);
}

void bindNodes(EditorRowListNode *first, EditorRowListNode *second) {
    first->next = second;
    if (second != NULL)second->prev = first;
}

void undoChange() {}

void undoDelete(Command cmd) {
    if (cmd.arg1 == -1) return;
    EditorRowListNode *row = getRowAt(&editorRowList, cmd.arg1);
    int rows = cmd.arg2 - cmd.arg1 + 1;
    DeletedNode *deletedNode = deleteStack;
    deleteStack = deleteStack->prev;
    if (cmd.arg1 == 1) {
        if(isEmpty(&editorRowList)){
            editorRowList.tail = deletedNode->startPtr;
            editorRowList.head = deletedNode->endPtr;
            editorRowList.tail->prev = NULL;
            editorRowList.head->next = NULL;
        } else {
            bindNodes(deletedNode->endPtr, row);
            editorRowList.tail = deletedNode->startPtr;
            editorRowList.tail->prev = NULL;
        }
    } else {
        if (row != NULL) {
            bindNodes(row->prev, deletedNode->startPtr);
            bindNodes(deletedNode->endPtr, row);
        } else {
            bindNodes(editorRowList.head, deletedNode->startPtr);
            editorRowList.head = deletedNode->endPtr;
            editorRowList.head->next = NULL;
        }
    }
    editorRowList.length += rows;
    free(deletedNode);
}

bool isEmpty(EditorRowList *list) {
    return (list->length == 0) ? true : false;
}

void initStructure() {
    commandList.head = NULL;
    commandList.length = 0;
    commandList.currentCommand = NULL;
    commandList.currentCommandIndex = 0;
    editorRowList.head = NULL;
    editorRowList.tail = NULL;
    editorRowList.length = 0;
    deleteStack = NULL;
    globalVersion = 0;
}

