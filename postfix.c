#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include "parser.h"

typedef struct {
    char items[MAX_TOKENS][MAX_TOKEN];
    int top;
} TokenStack;

static void push(TokenStack *s, const char *token) {
    if (s->top < MAX_TOKENS - 1) {
        strcpy(s->items[++s->top], token);
    }
}

static void pop(TokenStack *s, char *token) {
    if (s->top < 0) {
        token[0] = '\0';
        return;
    }
    strcpy(token, s->items[s->top--]);
}

static const char *peek(TokenStack *s) {
    return s->top < 0 ? "" : s->items[s->top];
}

static int precedence(const char *op) {
    if (!strcmp(op, "^")) return 4;
    if (!strcmp(op, "u+") || !strcmp(op, "u-")) return 3;
    if (!strcmp(op, "*") || !strcmp(op, "/")) return 2;
    if (!strcmp(op, "+") || !strcmp(op, "-")) return 1;
    return 0;
}

static int isRightAssociative(const char *op) {
    return !strcmp(op, "^") || !strcmp(op, "u+") || !strcmp(op, "u-");
}

static int appendToken(char postfix[][MAX_TOKEN], int *count,
                       const char *token) {
    if (*count >= MAX_TOKENS) return 0;
    if (strlen(token) >= MAX_TOKEN) return 0;
    strcpy(postfix[(*count)++], token);
    return 1;
}

int infixToPostfix(const char *infix,
                   char postfix[][MAX_TOKEN],
                   int *postfixCount) {
    TokenStack operators;
    int i = 0, count = 0, expectOperand = 1;
    int sawToken = 0, parenDepth = 0;

    operators.top = -1;
    if (!infix || !postfix || !postfixCount) return 0;

    while (infix[i] != '\0') {
        unsigned char ch = (unsigned char)infix[i];

        if (isspace(ch)) {
            i++;
            continue;
        }

        /* Numeric literal, including decimals and scientific notation. */
        if (isdigit(ch) || (infix[i] == '.' &&
                            isdigit((unsigned char)infix[i + 1]))) {
            char token[MAX_TOKEN];
            int j = 0, digits = 0;

            if (!expectOperand) return 0;

            while (isdigit((unsigned char)infix[i])) {
                if (j >= MAX_TOKEN - 1) return 0;
                token[j++] = infix[i++];
                digits++;
            }

            if (infix[i] == '.') {
                if (j >= MAX_TOKEN - 1) return 0;
                token[j++] = infix[i++];
                while (isdigit((unsigned char)infix[i])) {
                    if (j >= MAX_TOKEN - 1) return 0;
                    token[j++] = infix[i++];
                    digits++;
                }
            }

            if (digits == 0) return 0;

            if (infix[i] == 'e' || infix[i] == 'E') {
                if (j >= MAX_TOKEN - 1) return 0;
                token[j++] = infix[i++];
                if (infix[i] == '+' || infix[i] == '-') {
                    if (j >= MAX_TOKEN - 1) return 0;
                    token[j++] = infix[i++];
                }
                int exponentDigits = 0;
                while (isdigit((unsigned char)infix[i])) {
                    if (j >= MAX_TOKEN - 1) return 0;
                    token[j++] = infix[i++];
                    exponentDigits++;
                }
                if (exponentDigits == 0) return 0;
            }

            token[j] = '\0';
            if (!appendToken(postfix, &count, token)) return 0;
            expectOperand = 0;
            sawToken = 1;
            continue;
        }

        /* Identifier: starts with a letter or underscore. */
        if (isalpha(ch) || infix[i] == '_') {
            char token[MAX_TOKEN];
            int j = 0;

            if (!expectOperand) return 0;

            while (isalnum((unsigned char)infix[i]) || infix[i] == '_') {
                if (j >= MAX_TOKEN - 1) return 0;
                token[j++] = infix[i++];
            }
            token[j] = '\0';

            if (!appendToken(postfix, &count, token)) return 0;
            expectOperand = 0;
            sawToken = 1;
            continue;
        }

        if (infix[i] == '(') {
            if (!expectOperand) return 0; /* Reject implicit multiplication. */
            push(&operators, "(");
            parenDepth++;
            i++;
            sawToken = 1;
            expectOperand = 1;
            continue;
        }

        if (infix[i] == ')') {
            if (expectOperand || parenDepth == 0) return 0;

            while (operators.top >= 0 && strcmp(peek(&operators), "(") != 0) {
                char op[MAX_TOKEN];
                pop(&operators, op);
                if (!appendToken(postfix, &count, op)) return 0;
            }

            if (operators.top < 0) return 0;
            {
                char discard[MAX_TOKEN];
                pop(&operators, discard);
            }
            parenDepth--;
            i++;
            expectOperand = 0;
            continue;
        }

        if (strchr("+-*/^", infix[i]) != NULL) {
            char current[MAX_TOKEN];

            if (expectOperand) {
                if (infix[i] != '+' && infix[i] != '-') return 0;
                current[0] = 'u';
                current[1] = infix[i];
                current[2] = '\0';
                push(&operators, current);
                i++;
                continue;
            }

            current[0] = infix[i];
            current[1] = '\0';

            while (operators.top >= 0 &&
                   strcmp(peek(&operators), "(") != 0) {
                const char *top = peek(&operators);
                int shouldPop =
                    precedence(top) > precedence(current) ||
                    (precedence(top) == precedence(current) &&
                     !isRightAssociative(current));

                if (!shouldPop) break;

                {
                    char op[MAX_TOKEN];
                    pop(&operators, op);
                    if (!appendToken(postfix, &count, op)) return 0;
                }
            }

            push(&operators, current);
            i++;
            expectOperand = 1;
            continue;
        }

        return 0; /* Unknown character. */
    }

    if (!sawToken || expectOperand || parenDepth != 0) return 0;

    while (operators.top >= 0) {
        char op[MAX_TOKEN];
        pop(&operators, op);
        if (!strcmp(op, "(")) return 0;
        if (!appendToken(postfix, &count, op)) return 0;
    }

    if (count == 0) return 0;
    *postfixCount = count;
    return 1;
}

