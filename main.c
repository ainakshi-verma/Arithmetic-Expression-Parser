#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "parser.h"

static void clearInputLine(void) {
    int c;
    while ((c = getchar()) != '\n' && c != EOF) {}
}

static void showMenu(void) {
    printf("\n");
    printf("====================================================\n");
    printf("       ADVANCED ARITHMETIC EXPRESSION PARSER\n");
    printf("====================================================\n");
    printf(" 1. Enter a new expression\n");
    printf(" 2. Validate current expression\n");
    printf(" 3. Convert infix to postfix\n");
    printf(" 4. Generate expression tree\n");
    printf(" 5. Display parse tree\n");
    printf(" 6. Display tree traversals\n");
    printf(" 7. Evaluate expression\n");
    printf(" 8. Simplify expression\n");
    printf(" 9. Display expression statistics\n");
    printf("10. Reconstruct infix expression\n");
    printf("11. Save results to a text file\n");
    printf("12. Exit\n");
    printf("----------------------------------------------------\n");
}

static int prepareTree(const char *infix,
                       char postfix[][MAX_TOKEN],
                       int *postfixCount,
                       Node **root) {
    if (!validateExpression(infix)) {
        printf("\nError: Invalid arithmetic expression.\n");
        return 0;
    }

    if (!infixToPostfix(infix, postfix, postfixCount)) {
        printf("\nError: Could not convert expression.\n");
        return 0;
    }

    freeTree(*root);
    *root = buildExpressionTree(postfix, *postfixCount);

    if (!*root) {
        printf("\nError: Could not construct expression tree.\n");
        return 0;
    }
    return 1;
}

static void saveResults(const char *infix,
                        char postfix[][MAX_TOKEN],
                        int count,
                        Node *root) {
    FILE *file = fopen("parser_results.txt", "w");
    if (!file) {
        printf("\nCould not create parser_results.txt.\n");
        return;
    }

    fprintf(file, "ADVANCED ARITHMETIC EXPRESSION PARSER\n");
    fprintf(file, "=====================================\n\n");
    fprintf(file, "Original infix expression:\n%s\n", infix);
    fprintf(file, "Postfix expression:\n");
    for (int i = 0; i < count; i++)
        fprintf(file, "%s ", postfix[i]);

    fprintf(file, "\n\nFully parenthesized infix expression:\n");
    writeInfix(file, root);

    fprintf(file, "\n\nInorder traversal:\n");
    writeInorder(file, root);

    fprintf(file, "\n\nPreorder traversal:\n");
    writePreorder(file, root);

    fprintf(file, "\n\nPostorder traversal:\n");
    writePostorder(file, root);

    fprintf(file, "\n\nParse tree (sideways):\n");
    writeTree(file, root, 0);

    fprintf(file, "\n\nExpression statistics:\n");
    writeStatistics(file, root);

    fclose(file);
    printf("\nResults saved to parser_results.txt\n");
}

int main(void) {
    char infix[MAX_SIZE] = "";
    char postfix[MAX_TOKENS][MAX_TOKEN];
    int postfixCount = 0;
    int expressionReady = 0;
    int treeReady = 0;
    int choice;
    Node *root = NULL;
    Variable variables[MAX_VARIABLES];
    int variableCount = 0;

    while (1) {
        showMenu();
        printf("Enter your choice: ");

        if (scanf("%d", &choice) != 1) {
            printf("\nPlease enter a number from 1 to 12.\n");
            clearInputLine();
            continue;
        }
        clearInputLine();

        switch (choice) {
            case 1:
                printf("\nEnter arithmetic expression: ");
                if (!fgets(infix, sizeof(infix), stdin)) {
                    printf("Input error.\n");
                    break;
                }
                infix[strcspn(infix, "\n")] = '\0';

                freeTree(root);
                root = NULL;
                treeReady = 0;
                expressionReady = validateExpression(infix);
                postfixCount = 0;
                variableCount = 0;

                if (expressionReady) {
                    printf("\nExpression accepted successfully.\n");
                } else {
                    printf("\nInvalid expression. Please check its syntax.\n");
                }
                break;

            case 2:
                if (!strlen(infix)) {
                    printf("\nEnter an expression first.\n");
                } else if (validateExpression(infix)) {
                    printf("\nExpression is syntactically valid.\n");
                    expressionReady = 1;
                } else {
                    printf("\nExpression is invalid.\n");
                    expressionReady = 0;
                }
                break;

            case 3:
                if (!expressionReady) {
                    printf("\nEnter a valid expression first.\n");
                    break;
                }
                if (!infixToPostfix(infix, postfix, &postfixCount)) {
                    printf("\nPostfix conversion failed.\n");
                    break;
                }
                printf("\nPostfix expression: ");
                for (int i = 0; i < postfixCount; i++)
                    printf("%s ", postfix[i]);
                printf("\n");
                break;

            case 4:
                if (!expressionReady) {
                    printf("\nEnter a valid expression first.\n");
                    break;
                }
                if (prepareTree(infix, postfix, &postfixCount, &root)) {
                    treeReady = 1;
                    printf("\nExpression tree generated successfully.\n");
                } else {
                    treeReady = 0;
                }
                break;

            case 5:
                if (!treeReady) {
                    printf("\nGenerate the expression tree first.\n");
                    break;
                }
                printf("\nPARSE TREE (sideways; root is toward the left)\n");
                printTree(root, 0);
                printf("\n");
                break;

            case 6:
                if (!treeReady) {
                    printf("\nGenerate the expression tree first.\n");
                    break;
                }
                printf("\nInorder: ");
                inorder(root);
                printf("\nPreorder: ");
                preorder(root);
                printf("\nPostorder: ");
                postorder(root);
                printf("\n");
                break;

            case 7: {
                double result;
                char error[160] = "";
                if (!treeReady) {
                    if (!expressionReady ||
                        !prepareTree(infix, postfix, &postfixCount, &root)) {
                        break;
                    }
                    treeReady = 1;
                }
                if (evaluateTree(root, variables, &variableCount,
                                 &result, error, sizeof(error))) {
                    printf("\nEvaluation result: %.12g\n", result);
                } else {
                    printf("\nEvaluation error: %s\n", error);
                }
                break;
            }

            case 8:
                if (!treeReady) {
                    if (!expressionReady ||
                        !prepareTree(infix, postfix, &postfixCount, &root)) {
                        break;
                    }
                    treeReady = 1;
                }
                root = simplifyTree(root);
                if (!root) {
                    printf("\nSimplification could not be completed.\n");
                    treeReady = 0;
                } else {
                    printf("\nSimplified expression: ");
                    printInfix(root);
                    printf("\nUpdated parse tree:\n");
                    printTree(root, 0);
                }
                break;

            case 9:
                if (!treeReady) {
                    if (!expressionReady ||
                        !prepareTree(infix, postfix, &postfixCount, &root)) {
                        break;
                    }
                    treeReady = 1;
                }
                printf("\nEXPRESSION STATISTICS\n");
                printStatistics(root);
                break;

            case 10:
                if (!treeReady) {
                    if (!expressionReady ||
                        !prepareTree(infix, postfix, &postfixCount, &root)) {
                        break;
                    }
                    treeReady = 1;
                }
                printf("\nReconstructed infix expression: ");
                printInfix(root);
                printf("\n");
                break;

            case 11:
                if (!treeReady) {
                    if (!expressionReady ||
                        !prepareTree(infix, postfix, &postfixCount, &root)) {
                        break;
                    }
                    treeReady = 1;
                }
                saveResults(infix, postfix, postfixCount, root);
                break;

            case 12:
                freeTree(root);
                printf("\nThank you for using the Arithmetic Expression Parser!\n");
                return 0;

            default:
                printf("\nInvalid choice. Enter a number from 1 to 12.\n");
        }
    }

    freeTree(root);
    return 0;
}
