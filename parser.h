#ifndef PARSER_H
#define PARSER_H

#include <stdio.h>

#define MAX_SIZE 500
#define MAX_TOKEN 64
#define MAX_TOKENS 500
#define MAX_VARIABLES 100

typedef struct Node {
    char value[MAX_TOKEN];
    struct Node *left;
    struct Node *right;
} Node;

typedef struct {
    char name[MAX_TOKEN];
    double value;
} Variable;

int infixToPostfix(const char *infix,
                   char postfix[][MAX_TOKEN],
                   int *postfixCount);

int validateExpression(const char *infix);

Node *createNode(const char *value);
Node *buildExpressionTree(char postfix[][MAX_TOKEN], int count);
void freeTree(Node *root);

void inorder(Node *root);
void preorder(Node *root);
void postorder(Node *root);
void printTree(Node *root, int space);
void printInfix(Node *root);

void writeInorder(FILE *out, Node *root);
void writePreorder(FILE *out, Node *root);
void writePostorder(FILE *out, Node *root);
void writeTree(FILE *out, Node *root, int space);
void writeInfix(FILE *out, Node *root);

int evaluateTree(Node *root, Variable vars[], int *varCount,
                 double *result, char *error, size_t errorSize);

Node *simplifyTree(Node *root);

void printStatistics(Node *root);
void writeStatistics(FILE *out, Node *root);

#endif
