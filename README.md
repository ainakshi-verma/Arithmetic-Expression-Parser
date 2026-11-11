# Advanced Arithmetic Expression Parser

A menu-driven mini-project in C that parses arithmetic expressions, converts infix expressions to postfix notation, constructs a binary expression tree, evaluates expressions, simplifies selected algebraic patterns, and reports tree statistics.

## Features

- Infix-to-postfix conversion using a stack (Shunting Yard algorithm)
- Binary expression tree construction from postfix notation
- Unary plus and unary minus
- Decimal and scientific-notation numeric literals
- Identifiers / variables
- Parentheses and operator precedence
- Inorder, preorder and postorder traversals
- Sideways terminal parse-tree visualization
- Numerical expression evaluation
- Variable values requested during evaluation and cached for the current expression
- Basic symbolic simplification and constant folding
- Expression reconstruction from the tree
- Tree statistics
- Export of results to `parser_results.txt`
- Basic syntax validation and error handling

## Supported operators

| Operator | Meaning |
|---|---|
| `+` | Addition / unary plus |
| `-` | Subtraction / unary minus |
| `*` | Multiplication |
| `/` | Division |
| `^` | Exponentiation |

Exponentiation is right-associative. Unary signs are handled with conventional precedence, so `-2^2` is interpreted as `-(2^2)`.

## Files

- `main.c` — menu and application control
- `parser.h` — shared structures and function declarations
- `postfix.c` — token parsing and infix-to-postfix conversion
- `tree.c` — tree construction, traversals, visualization, evaluation, simplification and statistics
- `validator.c` — expression validation wrapper

## Requirements

- GCC compiler
- C11-compatible environment
- Math library (`libm`)

## Compile

### Windows (MinGW GCC)

```bash
gcc -std=c11 -Wall -Wextra main.c postfix.c tree.c validator.c -o parser.exe -lm
```

Run:

```bash
.\parser.exe
```

### Linux / macOS

```bash
gcc -std=c11 -Wall -Wextra main.c postfix.c tree.c validator.c -o parser -lm
```

Run:

```bash
./parser
```

## Example expressions

```text
A+B*C
(10+20)*5-15/3
-2^2
A*(-B)
12.5+3.2
2.5e3/5
```

For expressions with variables, choose **Evaluate expression** and enter the requested values when prompted.

## Simplification examples

The simplifier includes selected rules such as:

- `A+0 -> A`
- `A-0 -> A`
- `A*1 -> A`
- `A*0 -> 0`
- `A/1 -> A`
- `A^1 -> A`
- `A^0 -> 1`

It also folds constant-only subtrees, for example `2+3*4 -> 14`.

This is a basic educational simplifier, not a complete computer algebra system.

## Complexity

Let `n` be the number of tokens and `h` the height of the expression tree.

| Operation | Time | Auxiliary space |
|---|---:|---:|
| Infix-to-postfix conversion | O(n) | O(n) |
| Tree construction | O(n) | O(n) |
| Tree traversal | O(n) | O(h) recursion stack |
| Tree statistics | O(n) | O(h) recursion stack |
| Tree storage | — | O(n) |

## Limitations

- No implicit multiplication: write `2*(A+B)`, not `2(A+B)`.
- No functions such as `sin(x)`, `sqrt(x)` or `log(x)`.
- No comparison or logical operators.
- Variables are assigned numeric values during evaluation.
- Simplification implements selected algebraic identities only.
- Unary and exponentiation precedence follows the documented convention.

## Future enhancements

- Function calls and built-in mathematical functions
- Graphical tree visualization
- Symbolic differentiation
- More advanced algebraic simplification
- Unit tests and automated regression tests
