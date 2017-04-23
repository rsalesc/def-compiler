# MATA61

A compiler for Def language (a simplified language which looks like C).

This compiler can be broken into four pieces:

- A lexer generator, inspired by Flex, which works in runtime.
- A LL(2) recursive descent parser which constructs an AST.
- A semantic phase on top of that AST.
- A code generation phase which spits MIPS code that is meant to be run in SPIM (a MIPS simulator).
