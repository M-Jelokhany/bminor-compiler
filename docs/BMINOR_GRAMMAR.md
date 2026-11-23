# B-Minor Grammar (Phase 1)

This document is a typed version of the grammar supplied with the project. It uses EBNF.

## EBNF notation

- `A B` means that `A` is followed by `B`.
- `A | B` means either `A` or `B` can be used.
- `[ A ]` means that `A` is optional.
- `{ A }` means that `A` can repeat zero or more times.
- Text inside double quotes is a terminal symbol of the language.
- Names such as `expression` and `statement` refer to grammar rules.

## 1. Programs and declarations

```ebnf
program                 = { declaration } ;
declaration             = variableDeclaration
                        | functionPrototype
                        | functionDefinition ;

variableDeclaration    = IDENTIFIER, ":", type, [ "=", initializer ], ";" ;

functionPrototype      = IDENTIFIER, ":", "function", returnType,
                         "(", [ parameterList ], ")", ";" ;

functionDefinition     = IDENTIFIER, ":", "function", returnType,
                         "(", [ parameterList ], ")", "=", block ;
```

## 2. Types and initializers

```ebnf
type                    = basicType | arrayType ;
basicType               = "integer" | "boolean" | "char" | "string" ;
returnType              = basicType | "void" ;

arrayType               = "array", "[", INTEGER_LITERAL, "]", basicType ;

initializer             = expression | arrayInitializer ;
arrayInitializer        = "{", expression, { ",", expression }, "}" ;
```

## 3. Parameters

```ebnf
parameterList           = parameter, { ",", parameter } ;
parameter               = IDENTIFIER, ":", parameterType ;
parameterType           = basicType | arrayParameterType ;
arrayParameterType      = "array", "[", "]", basicType ;
```

An array declaration has a size, for example `array[5]integer`. An array parameter uses empty brackets, for example `array[]integer`.

## 4. Expressions and precedence

The rules below are ordered from lower precedence to higher precedence. Assignment and power are right-associative in this grammar; the other binary levels are parsed from left to right.

```ebnf
expression              = assignmentExpression ;
assignmentExpression    = logicalOrExpression, [ "=", assignmentExpression ] ;
logicalOrExpression     = logicalAndExpression, { "||", logicalAndExpression } ;
logicalAndExpression    = equalityExpression, { "&&", equalityExpression } ;
equalityExpression      = relationalExpression,
                          { ( "==" | "!=" ), relationalExpression } ;
relationalExpression    = additiveExpression,
                          { ( "<" | "<=" | ">" | ">=" ), additiveExpression } ;
additiveExpression      = multiplicativeExpression,
                          { ( "+" | "-" ), multiplicativeExpression } ;
multiplicativeExpression = powerExpression,
                           { ( "*" | "/" | "%" ), powerExpression } ;
powerExpression         = unaryExpression, [ "^", powerExpression ] ;
unaryExpression         = { "-" | "!" }, postfixExpression ;
postfixExpression       = accessExpression, [ "++" | "--" ] ;

accessExpression        = primaryExpression,
                          { "[", expression, "]"
                          | "(", [ argumentList ], ")" } ;
argumentList            = expression, { ",", expression } ;

primaryExpression       = IDENTIFIER
                        | INTEGER_LITERAL
                        | BOOLEAN_LITERAL
                        | STRING_LITERAL
                        | CHARACTER_LITERAL
                        | "(", expression, ")" ;
```

## 5. Blocks and statements

```ebnf
block                   = "{", { blockItem }, "}" ;
blockItem               = variableDeclaration | statement ;

statement               = expressionStatement
                        | returnStatement
                        | printStatement
                        | ifStatement
                        | forStatement
                        | block ;

expressionStatement     = expression, ";" ;
returnStatement         = "return", [ expression ], ";" ;
printStatement          = "print", expression, { ",", expression }, ";" ;

ifStatement             = "if", "(", expression, ")", statement,
                          [ "else", statement ] ;

forStatement            = "for", "(", [ expression ], ";",
                          [ expression ], ";", [ expression ], ")", statement ;
```

## 6. Lexical categories

The lexer uses the categories already present in the student's starter code and handwritten notes:

| Category | Rule / examples |
|---|---|
| Identifier | `[a-zA-Z_][a-zA-Z0-9_]*` |
| Integer literal | `[0-9]+` |
| Boolean literal | `true`, `false` |
| Keywords | `array`, `boolean`, `char`, `else`, `for`, `function`, `if`, `integer`, `print`, `return`, `string`, `void`, `while` |
| String literal | Double-quoted text; backslash escape pairs are read as part of the literal |
| Character literal | Single-quoted one-character value or one escape pair |
| Arithmetic operators | `+ - * / % ^` |
| Increment/decrement | `++ --` |
| Relational/equality operators | `< <= > >= == !=` |
| Logical operators | `&& || !` |
| Assignment operator | `=` |
| Delimiters | `( ) [ ] { } ; , :` |
| Comments | `// ...` and `/* ... */` |
| Whitespace | spaces, tabs, CR and LF; skipped by the parser but retained from the original source during highlighting |

The lexer applies longest match to operators (for example, `<=` is one token) and checks keywords after reading a complete identifier (so `ifx` remains an identifier).

## 7. Parser decisions and scope

1. The parser is a hand-written recursive-descent parser. It follows the top-down approach described in the project instructions.
2. The expression grammar has no left recursion. Repetition is implemented with loops, while right-associative rules use recursive calls.
3. A declaration begins with `IDENTIFIER :`. The next token determines whether it is a variable declaration or a function declaration (`function`).
4. The grammar in the student's handwritten notes does not include `while`, `do-while` or `switch` statements. The lexer still recognizes `while` as a keyword because it appears in the student's lexical notes, but this parser reports it as an unsupported statement.
5. `for` loop clauses are expressions. A variable declaration is not accepted directly in the `for` initializer because the supplied grammar defines that position as an optional expression.
6. The parser builds AST nodes with source line, column and offset. The `typeAnnotation` field is initially empty for a later semantic-analysis phase.
7. Semantic checks such as variable declaration lookup, type compatibility and return-type validation are not part of this parser implementation.

## 8. Error recovery

When a syntax error occurs, the parser reports the current token's file, line and column. It then skips tokens until a synchronization point such as `;`, `}`, `if`, `for`, `return`, `print`, or the beginning of the next declaration. This is a simple panic-mode recovery strategy.
