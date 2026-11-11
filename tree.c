#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <math.h>
#include "parser.h"

static int isUnary(const char *s) {
    return !strcmp(s, "u-") || !strcmp(s, "u+");
}

static int isBinary(const char *s) {
    return strlen(s) == 1 && strchr("+-*/^", s[0]) != NULL;
}

static int isOperator(const char *s) {
    return isUnary(s) || isBinary(s);
}

Node *createNode(const char *value) {
    Node *node = (Node *)malloc(sizeof(Node));
    if (!node) return NULL;
    snprintf(node->value, sizeof(node->value), "%s", value);
    node->left = NULL;
    node->right = NULL;
    return node;
}

void freeTree(Node *root) {
    if (!root) return;
    freeTree(root->left);
    freeTree(root->right);
    free(root);
}

Node *buildExpressionTree(char postfix[][MAX_TOKEN], int count) {
    Node *stack[MAX_TOKENS];
    int top = -1;

    if (count <= 0 || count >= MAX_TOKENS) return NULL;

    for (int i = 0; i < count; i++) {
        if (isUnary(postfix[i])) {
            Node *operand, *node;

            if (top < 0) goto failure;
            operand = stack[top--];
            node = createNode(postfix[i]);

            if (!node) {
                freeTree(operand);
                goto failure;
            }

            node->right = operand;
            stack[++top] = node;
        } else if (isBinary(postfix[i])) {
            Node *right, *left, *node;

            if (top < 1) goto failure;
            right = stack[top--];
            left = stack[top--];
            node = createNode(postfix[i]);

            if (!node) {
                freeTree(left);
                freeTree(right);
                goto failure;
            }

            node->left = left;
            node->right = right;
            stack[++top] = node;
        } else {
            Node *node = createNode(postfix[i]);
            if (!node) goto failure;
            stack[++top] = node;
        }
    }

    if (top != 0) goto failure;
    return stack[0];

failure:
    while (top >= 0) freeTree(stack[top--]);
    return NULL;
}

void inorder(Node *root) {
    if (!root) return;

    if (isUnary(root->value)) {
        printf("(%c", root->value[1]);
        inorder(root->right);
        printf(")");
        return;
    }

    if (isBinary(root->value)) printf("(");
    inorder(root->left);
    printf("%s", root->value);
    inorder(root->right);
    if (isBinary(root->value)) printf(")");
}

void preorder(Node *root) {
    if (!root) return;
    if (isUnary(root->value)) {
        printf("%c ", root->value[1]);
        preorder(root->right);
        return;
    }
    printf("%s ", root->value);
    preorder(root->left);
    preorder(root->right);
}

void postorder(Node *root) {
    if (!root) return;
    if (isUnary(root->value)) {
        postorder(root->right);
        printf("%c ", root->value[1]);
        return;
    }
    postorder(root->left);
    postorder(root->right);
    printf("%s ", root->value);
}

void printTree(Node *root, int space) {
    if (!root) return;
    space += 7;
    printTree(root->right, space);
    printf("\n");
    for (int i = 7; i < space; i++) printf(" ");
    if (!strcmp(root->value, "u-")) printf("NEG\n");
    else if (!strcmp(root->value, "u+")) printf("POS\n");
    else printf("%s\n", root->value);
    printTree(root->left, space);
}

void printInfix(Node *root) {
    inorder(root);
}

void writeInorder(FILE *out, Node *root) {
    if (!root) return;
    if (isUnary(root->value)) {
        fprintf(out, "(%c", root->value[1]);
        writeInorder(out, root->right);
        fprintf(out, ")");
        return;
    }
    if (isBinary(root->value)) fprintf(out, "(");
    writeInorder(out, root->left);
    fprintf(out, "%s", root->value);
    writeInorder(out, root->right);
    if (isBinary(root->value)) fprintf(out, ")");
}

void writePreorder(FILE *out, Node *root) {
    if (!root) return;
    if (isUnary(root->value)) {
        fprintf(out, "%c ", root->value[1]);
        writePreorder(out, root->right);
        return;
    }
    fprintf(out, "%s ", root->value);
    writePreorder(out, root->left);
    writePreorder(out, root->right);
}

void writePostorder(FILE *out, Node *root) {
    if (!root) return;
    if (isUnary(root->value)) {
        writePostorder(out, root->right);
        fprintf(out, "%c ", root->value[1]);
        return;
    }
    writePostorder(out, root->left);
    writePostorder(out, root->right);
    fprintf(out, "%s ", root->value);
}

void writeTree(FILE *out, Node *root, int space) {
    if (!root) return;
    space += 7;
    writeTree(out, root->right, space);
    fprintf(out, "\n");
    for (int i = 7; i < space; i++) fputc(' ', out);
    if (!strcmp(root->value, "u-")) fprintf(out, "NEG\n");
    else if (!strcmp(root->value, "u+")) fprintf(out, "POS\n");
    else fprintf(out, "%s\n", root->value);
    writeTree(out, root->left, space);
}

void writeInfix(FILE *out, Node *root) {
    writeInorder(out, root);
}

/* Variable lookup and expression evaluation. */
static int findVariable(Variable vars[], int count, const char *name) {
    for (int i = 0; i < count; i++)
        if (!strcmp(vars[i].name, name)) return i;
    return -1;
}

static int isNumber(const char *s, double *value) {
    char *end;
    double v = strtod(s, &end);
    if (s[0] == '\0' || *end != '\0') return 0;
    *value = v;
    return 1;
}

static int evalNode(Node *root, Variable vars[], int *varCount,
                    double *result, char *error, size_t errorSize) {
    double left, right, number;

    if (!root) {
        snprintf(error, errorSize, "Invalid expression tree.");
        return 0;
    }

    if (!isOperator(root->value)) {
        if (isNumber(root->value, &number)) {
            *result = number;
            return 1;
        }

        int index = findVariable(vars, *varCount, root->value);
        if (index >= 0) {
            *result = vars[index].value;
            return 1;
        }

        if (*varCount >= MAX_VARIABLES) {
            snprintf(error, errorSize, "Too many variables.");
            return 0;
        }

        printf("Enter value for %s: ", root->value);
        if (scanf("%lf", &number) != 1) {
            int c;
            while ((c = getchar()) != '\n' && c != EOF) {}
            snprintf(error, errorSize, "Invalid variable value.");
            return 0;
        }
        vars[*varCount].value = number;
        snprintf(vars[*varCount].name, MAX_TOKEN, "%s", root->value);
        (*varCount)++;
        *result = number;
        return 1;
    }

    if (isUnary(root->value)) {
        if (!evalNode(root->right, vars, varCount, &right,
                      error, errorSize)) return 0;
        *result = root->value[1] == '-' ? -right : right;
        return 1;
    }

    if (!evalNode(root->left, vars, varCount, &left,
                  error, errorSize)) return 0;
    if (!evalNode(root->right, vars, varCount, &right,
                  error, errorSize)) return 0;

    switch (root->value[0]) {
        case '+': *result = left + right; break;
        case '-': *result = left - right; break;
        case '*': *result = left * right; break;
        case '/':
            if (fabs(right) < 1e-12) {
                snprintf(error, errorSize, "Division by zero.");
                return 0;
            }
            *result = left / right;
            break;
        case '^':
            *result = pow(left, right);
            if (!isfinite(*result)) {
                snprintf(error, errorSize, "Invalid or out-of-range power.");
                return 0;
            }
            break;
        default:
            snprintf(error, errorSize, "Unknown operator.");
            return 0;
    }

    if (!isfinite(*result)) {
        snprintf(error, errorSize, "Result is outside the supported range.");
        return 0;
    }
    return 1;
}

int evaluateTree(Node *root, Variable vars[], int *varCount,
                 double *result, char *error, size_t errorSize) {
    if (!root || !varCount || !result) return 0;
    return evalNode(root, vars, varCount, result, error, errorSize);
}

/* Simplification helpers. */
static int numericValue(Node *node, double *value) {
    char *end;
    if (!node || node->left || node->right) return 0;
    *value = strtod(node->value, &end);
    return node->value[0] != '\0' && *end == '\0';
}

static Node *makeNumber(double value) {
    char buffer[MAX_TOKEN];
    snprintf(buffer, sizeof(buffer), "%.12g", value);
    return createNode(buffer);
}

Node *simplifyTree(Node *root) {
    double a, b, result;

    if (!root) return NULL;

    root->left = simplifyTree(root->left);
    root->right = simplifyTree(root->right);

    if (isUnary(root->value) && numericValue(root->right, &a)) {
        result = root->value[1] == '-' ? -a : a;
        Node *replacement = makeNumber(result);
        if (!replacement) return root;
        freeTree(root);
        return replacement;
    }

    if (!isBinary(root->value)) return root;

    if (numericValue(root->left, &a) &&
        numericValue(root->right, &b)) {
        switch (root->value[0]) {
            case '+': result = a + b; break;
            case '-': result = a - b; break;
            case '*': result = a * b; break;
            case '/':
                if (fabs(b) < 1e-12) return root;
                result = a / b;
                break;
            case '^':
                result = pow(a, b);
                if (!isfinite(result)) return root;
                break;
            default: return root;
        }
        if (!isfinite(result)) return root;
        Node *replacement = makeNumber(result);
        if (!replacement) return root;
        freeTree(root);
        return replacement;
    }

    /* Algebraic identities, keeping symbolic expressions intact. */
    if (!strcmp(root->value, "+")) {
        if (numericValue(root->left, &a) && fabs(a) < 1e-12) {
            Node *r = root->right;
            root->right = NULL;
            freeTree(root);
            return r;
        }
        if (numericValue(root->right, &b) && fabs(b) < 1e-12) {
            Node *l = root->left;
            root->left = NULL;
            freeTree(root);
            return l;
        }
    }

    if (!strcmp(root->value, "-") &&
        numericValue(root->right, &b) && fabs(b) < 1e-12) {
        Node *l = root->left;
        root->left = NULL;
        freeTree(root);
        return l;
    }

    if (!strcmp(root->value, "*")) {
        if ((numericValue(root->left, &a) && fabs(a) < 1e-12) ||
            (numericValue(root->right, &b) && fabs(b) < 1e-12)) {
            freeTree(root);
            return createNode("0");
        }
        if (numericValue(root->left, &a) && fabs(a - 1.0) < 1e-12) {
            Node *r = root->right;
            root->right = NULL;
            freeTree(root);
            return r;
        }
        if (numericValue(root->right, &b) && fabs(b - 1.0) < 1e-12) {
            Node *l = root->left;
            root->left = NULL;
            freeTree(root);
            return l;
        }
    }

    if (!strcmp(root->value, "/") &&
        numericValue(root->right, &b) && fabs(b - 1.0) < 1e-12) {
        Node *l = root->left;
        root->left = NULL;
        freeTree(root);
        return l;
    }

    if (!strcmp(root->value, "^")) {
        if (numericValue(root->right, &b) && fabs(b) < 1e-12) {
            freeTree(root);
            return createNode("1");
        }
        if (numericValue(root->right, &b) && fabs(b - 1.0) < 1e-12) {
            Node *l = root->left;
            root->left = NULL;
            freeTree(root);
            return l;
        }
    }

    return root;
}

/* Tree statistics. */
static void collectStats(Node *root, int depth, int *nodes,
                         int *operands, int *operators, int *maxDepth) {
    if (!root) return;
    (*nodes)++;
    if (isOperator(root->value)) (*operators)++;
    else (*operands)++;
    if (depth > *maxDepth) *maxDepth = depth;
    collectStats(root->left, depth + 1, nodes, operands,
                 operators, maxDepth);
    collectStats(root->right, depth + 1, nodes, operands,
                 operators, maxDepth);
}

void writeStatistics(FILE *out, Node *root) {
    int nodes = 0, operands = 0, operators = 0, maxDepth = 0;
    if (!root) {
        fprintf(out, "No expression tree available.\n");
        return;
    }
    collectStats(root, 0, &nodes, &operands, &operators, &maxDepth);
    fprintf(out, "Total nodes: %d\n", nodes);
    fprintf(out, "Operands: %d\n", operands);
    fprintf(out, "Operators: %d\n", operators);
    fprintf(out, "Tree height (edges): %d\n", maxDepth);
}

void printStatistics(Node *root) {
    writeStatistics(stdout, root);
}
