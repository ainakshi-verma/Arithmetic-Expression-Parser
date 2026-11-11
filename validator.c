#include "parser.h"

/*
 * Validation and conversion use the same parser.
 * This prevents validation and postfix conversion from disagreeing.
 */
int validateExpression(const char *infix) {
    char postfix[MAX_TOKENS][MAX_TOKEN];
    int count = 0;
    return infixToPostfix(infix, postfix, &count);
}
