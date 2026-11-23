# B-Minor Lexical Rules

This file records the token rules implemented by `Lexer.cpp`. The rules follow the student's handwritten lexer notes and the existing starter implementation.

## Regular-expression style rules

The notation below is descriptive. `\` is an escape character and `ASCII` refers to printable ASCII characters.

| Token | Rule |
|---|---|
| Identifier | `[A-Za-z_][A-Za-z0-9_]*` |
| Integer literal | `[0-9]+` |
| Boolean literal | `true` or `false` |
| String literal | `"` followed by zero or more non-quote, non-newline characters or escaped printable characters, followed by `"` |
| Character literal | `'` followed by exactly one ordinary ASCII character or one escaped character, followed by `'` |
| Line comment | `//` followed by characters until CR or LF |
| Block comment | `/*` followed by characters until the first closing `*/` |
| Whitespace | one or more spaces, tabs, CR or LF characters |

The starter lexer limits identifiers and string contents to 256 characters. A character literal must contain exactly one character (an escape pair counts as one character).

## Keywords

```text
array boolean char else for function if integer print return string void while
```

The words `true` and `false` are Boolean literals, not keywords.

## Operators

The lexer checks two-character operators before one-character operators. This implements longest match.

```text
Two-character:  ++  --  <=  >=  ==  !=  &&  ||
One-character:  +  -  *  /  %  ^  <  >  !  =
```

## Delimiters

```text
(  )  [  ]  {  }  ;  ,  :
```

## Priority and longest match

1. Whitespace is consumed and ignored by the token stream.
2. Identifiers are read completely before they are checked against the keyword list. Thus `return` is a keyword, but `returnValue` is an identifier.
3. A comment beginning with `//` or `/*` is checked before `/` is returned as an arithmetic operator.
4. Two-character operators are checked before one-character operators. Thus `<=`, `++` and `&&` each produce one token.
5. If no rule matches the current character, the lexer emits one `Invalid` token, records its source position and continues from the next character.

## Source positions

Every token stores the source file name, line, column and character offset at which it begins. The offset is also used by the syntax highlighter to copy the original text between tokens, preserving whitespace and line breaks.

## Notes about escape sequences

The existing starter tests include examples such as `\n`, `\0`, `\'`, `\\` and `\q`. The current lexer treats a backslash followed by a printable ASCII character as one escaped character; it does not restrict escapes to a smaller named list. This behavior is intentionally kept compatible with the student's original tests. If the instructor later provides a stricter escape list, the validation can be tightened in `Lexer.cpp`.
