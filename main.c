#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define MAX_LINE_LENGTH 1024

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

typedef struct command {
    int arg1;
    int arg2;
    char type;
    EditorRowListNode *ptr1;
    EditorRowListNode *ptr2;
    EditorRowListNode *prevHead;
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

typedef struct stackNode {
    int l;
    struct stackNode *prev;
    EditorRowListNode *startPtr;
    EditorRowListNode *endPtr;
} StackNode;

typedef enum {
    false, true
} bool;

Command getCommand(char *cmd);

void freeCommands();

void freeTemp();

void pushCommand(Command cmd, EditorRowListNode *p1, EditorRowListNode *p2, EditorRowListNode *p3);

EditorRowListNode *getRowAt(EditorRowList *list, int index);

char *getRowContent();

void addToStack(StackNode **stack, EditorRowListNode *start, EditorRowListNode *end, int l);

EditorRowListNode *createNewRow(char *str);

void executeChange(Command cmd);

void executePrint(Command cmd);

void deleteRows(EditorRowListNode *start, EditorRowListNode *end);

void executeDelete(Command cmd, bool isRedo);

int getCommandIndex(int prevIndex, int moves);

void executeUndoRedo(Command cmd);

void executeCommand(Command cmd);

void redoCommands(int moves);

void redoCommand(Command cmd);

void redoChange(Command *cmd);

void undoCommands(int moves);

void undoCommand(Command cmd);

void undoDelete(Command cmd);

void bindNodes(EditorRowListNode *first, EditorRowListNode *second);

void undoChange(Command cmd);

bool isEmpty(EditorRowList *list);

void initStructure();

CommandList commandList;
EditorRowList editorRowList;
StackNode *deleteStack;
StackNode *changedStack;
StackNode *tempStack;

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

void freeTempItem(){
    StackNode *itemToFree = tempStack;
    EditorRowListNode *rowToFree = tempStack->startPtr;
    EditorRowListNode *tmp;
    while(rowToFree!=tempStack->endPtr){
        tmp = rowToFree->next;
        free(rowToFree);
        rowToFree = tmp;
    }
    free(rowToFree);
    free(itemToFree);
}

void freeTemp() {
    StackNode *tmp;
    while(tempStack!=NULL){
        tmp = tempStack->prev;
        freeTempItem();
        tempStack = tmp;
    }
}

void pushCommand(Command cmd, EditorRowListNode *p1, EditorRowListNode*p2, EditorRowListNode *p3) {
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
    newCommand->command.ptr1 = p1;
    newCommand->command.ptr2 = p2;
    newCommand->command.prevHead = p3;
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

EditorRowListNode *createNewRow(char *str) {
    EditorRowListNode *newRow = (EditorRowListNode *) malloc(sizeof(EditorRowListNode));
    newRow->content = str;
    return newRow;
}

void executeChange(Command cmd) {
    EditorRowListNode *row1 = getRowAt(&editorRowList, cmd.arg1);
    EditorRowListNode *row2 = getRowAt(&editorRowList, cmd.arg2);
    EditorRowListNode *prevHead = editorRowList.head;
    addToStack(&changedStack, row1, row2, editorRowList.length);
    EditorRowListNode *start = NULL;
    EditorRowListNode *iterator = start;
    int rows = cmd.arg2 - cmd.arg1 + 1;
    for (int i = 0; i < rows; i++) {
        char *text = getRowContent();
        if(i == 0){
            start = createNewRow(text);
            iterator = start;
        } else {
            bindNodes(iterator, createNewRow(text));
            iterator = iterator->next;
        }
    }
    if(row1!=NULL){
        if(row1->prev!=NULL){
            bindNodes(row1->prev, start);
        }else {
            start->prev = NULL;
            editorRowList.tail = start;
        }
    } else {
        if(cmd.arg1 == 1){
            start->prev = NULL;
            editorRowList.tail = start;
        } else {
            bindNodes(editorRowList.head, start);
        }
    }
    if(row2!=NULL){
        if(row2->next!=NULL){
            bindNodes(iterator, row2->next);
        } else {
            iterator->next = NULL;
            editorRowList.head = iterator;
        }
    } else {
        iterator->next = NULL;
        editorRowList.head = iterator;
    }
    if(cmd.arg2>editorRowList.length) editorRowList.length = cmd.arg2;
    char point[4];
    char *outcome = fgets(point, 4, stdin);
    if (outcome == NULL) printf("Error while reading ending point!");
    pushCommand(cmd, start, iterator, prevHead);
}

void executePrint(Command cmd) {
    EditorRowListNode *row;
    int rows;
    if (cmd.arg1 == 0) {
        printf(".\n");
        row = editorRowList.tail;
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
            printf("%s", row->content);
        }
        row = row->next;
    }
}

void addToStack(StackNode **stack, EditorRowListNode *start, EditorRowListNode *end, int l){
    StackNode *newNode = malloc(sizeof(StackNode));
    newNode->l = l;
    newNode->startPtr = start;
    newNode->endPtr = end;
    if(*stack==NULL){
        *stack = newNode;
        newNode->prev = NULL;
    } else {
        newNode->prev = *stack;
        *stack = newNode;
    }
}

void deleteRows(EditorRowListNode *start, EditorRowListNode *end) {
    addToStack(&deleteStack, start, end, -1);
}

void executeDelete(Command cmd, bool isRedo) {
    if (cmd.arg1 == -1) return;
    if (cmd.arg1 > editorRowList.length) {
        if (!isRedo){
            pushCommand(cmd, NULL, NULL, NULL);
        } else {
            commandList.currentCommand->command.ptr1 = NULL;
        }
        commandList.currentCommand->command.arg1 = -1;
        return;
    }
    EditorRowListNode *row = (isRedo) ? cmd.ptr1 : getRowAt(&editorRowList, cmd.arg1);
    EditorRowListNode *prevRow = row->prev;
    if (cmd.arg1 == 1) {
        if (cmd.arg2 >= editorRowList.length) {
            deleteRows(editorRowList.tail, editorRowList.head);
            if (!isRedo){
                pushCommand(cmd, NULL, NULL, NULL);
                commandList.currentCommand->command.arg2 = editorRowList.length;
            } else {
                commandList.currentCommand->command.ptr1 = NULL;
            }
            editorRowList.head = NULL;
            editorRowList.tail = NULL;
            editorRowList.length = 0;
            return;
        }
        EditorRowListNode *row2 = (isRedo) ? cmd.ptr2 : getRowAt(&editorRowList, cmd.arg2);
        deleteRows(editorRowList.tail, row2);
        if (!isRedo){
            pushCommand(cmd, row2->next, NULL, NULL);
        } else {
            commandList.currentCommand->command.ptr1 = row2->next;
        }
        editorRowList.tail = row2->next;
        editorRowList.tail->prev = NULL;
        editorRowList.length -= cmd.arg2;
        return;
    }
    if (cmd.arg2 >= editorRowList.length) {
        prevRow->next = NULL;
        deleteRows(row, editorRowList.head);
        if (!isRedo){
            pushCommand(cmd, NULL, NULL, NULL);
        } else {
            commandList.currentCommand->command.ptr1 = NULL;
        }
        editorRowList.head = prevRow;
        if (!isRedo)commandList.currentCommand->command.arg2 = editorRowList.length;
        editorRowList.length = cmd.arg1 - 1;
        return;
    }
    int rows = cmd.arg2 - cmd.arg1 + 1;
    EditorRowListNode *row2 = (isRedo) ? cmd.ptr2 : getRowAt(&editorRowList, cmd.arg2);
    EditorRowListNode *tmp = row2->prev;
    deleteRows(row, row2);
    if (!isRedo){
        pushCommand(cmd, row2->next, NULL, NULL);
    } else {
        commandList.currentCommand->command.ptr1 = row2->next;
    }
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

void executeCommand(Command cmd) {
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
        redoCommand(commandList.currentCommand->command);
    }
}

void redoCommand(Command cmd) {
    if (cmd.type == 'c') redoChange(&cmd);
    if (cmd.type == 'd') executeDelete(cmd, true);
}

void redoChange(Command *cmd) {
    EditorRowListNode *row1 = cmd->ptr1;
    EditorRowListNode *row2 = cmd->ptr2;
    StackNode *tempNode = tempStack;
    tempStack = tempStack->prev;
    addToStack(&changedStack, row1, row2, editorRowList.length);
    if(row1!=NULL){
        if(row1->prev!=NULL){
            bindNodes(row1->prev, tempNode->startPtr);
        }else {
            tempNode->startPtr->prev = NULL;
            editorRowList.tail = tempNode->startPtr;
        }
    } else {
        if(cmd->arg1 == 1){
            tempNode->startPtr->prev = NULL;
            editorRowList.tail = tempNode->startPtr;
        } else {
            bindNodes(editorRowList.head, tempNode->startPtr);
        }
    }
    if(row2!=NULL){
        if(row2->next!=NULL){
            bindNodes(tempNode->endPtr, row2->next);
        } else {
            tempNode->endPtr->next = NULL;
            editorRowList.head = tempNode->endPtr;
        }
    } else {
        tempNode->endPtr->next = NULL;
        editorRowList.head = tempNode->endPtr;
    }
    if(cmd->arg2>editorRowList.length) editorRowList.length = cmd->arg2;
    commandList.currentCommand->command.ptr1 = tempNode->startPtr;
    commandList.currentCommand->command.ptr2 = tempNode->endPtr;
    free(tempNode);
}

void undoCommands(int moves) {
    moves *= -1;
    if (moves > commandList.currentCommandIndex) moves = commandList.currentCommandIndex;
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

void bindNodes(EditorRowListNode *first, EditorRowListNode *second) {
    if(first!=NULL) first->next = second;
    if (second != NULL)second->prev = first;
}

void undoChange(Command cmd) {
    StackNode *changedNode = changedStack;
    changedStack = changedStack->prev;
    EditorRowListNode *start = cmd.ptr1;
    EditorRowListNode *end = cmd.ptr2;
    addToStack(&tempStack, start, end, -1);
    if(cmd.arg1==1&&changedNode->startPtr==NULL){
        editorRowList.head = NULL;
        editorRowList.tail = NULL;
        editorRowList.length = 0;
    } else {
        if(changedNode->startPtr==NULL){
            editorRowList.head = start->prev;
            start->prev->next = NULL;
        } else {
            if(cmd.arg1==1){
                editorRowList.tail = changedNode->startPtr;
                editorRowList.tail->prev = NULL;
            } else {
                bindNodes(start->prev, changedNode->startPtr);
            }
            if(changedNode->endPtr!=NULL){
                bindNodes(changedNode->endPtr, end->next);
            } else {
                editorRowList.head = cmd.prevHead;
            }
            if(end->next==NULL&&changedNode->endPtr!=NULL) editorRowList.head = changedNode->endPtr;
        }
    }
    editorRowList.length = changedNode->l;
    commandList.currentCommand->command.ptr1 = changedNode->startPtr;
    commandList.currentCommand->command.ptr2 = changedNode->endPtr;
    free(changedNode);
}

void undoDelete(Command cmd) {
    if (cmd.arg1 == -1) return;
    EditorRowListNode *row =cmd.ptr1;
    int rows = cmd.arg2 - cmd.arg1 + 1;
    StackNode *deletedNode = deleteStack;
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
    commandList.currentCommand->command.ptr1 = deletedNode->startPtr;
    commandList.currentCommand->command.ptr2 = deletedNode->endPtr;
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
    changedStack = NULL;
}

