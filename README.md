# dcc
Daiki C Compiler

### Notice
This is for my understandings of interpreter-compiler.

Currently the usage is limited.



## Specification

### Supported types
Types below are supported.

& is not supported stil. (this will be supported as unary operation below)

- Int

- Char (string)

### Passed statements
statements below were properly translated to assembler with simple processing.
like

```c
int a;
a = 0;
while (a < 5) {
      a = a + 1;
}
```

- whlie, do-while

- for

- if, else if, else

- pre & post position incre & decrement

- switch, case, default (nest capable)

- continue

- break

- function, prototype, call it (with arguments)

- local & global (array) variable declaration (with initialization)


### Passed preprocessor directives

- \#define (const and function) recursive only for const type

- \#include (possible to file nesting compile)

- \#if, #elif, #else, #ifdef, #ifndef and #endif


### Passed expressions

- binary operations (+ - * / %) without pointer addressing

- unary operations (+ - * ! ++ --)

- binary bit operations (& | ^ << >>)

- unary bit operation (~)

- binary boolean operations (== != <= => >= =< && ||)

- compound operators (+= -= *= /= %= &= |= ^= <<= >>=)

- comment (// /**/)