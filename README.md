# dcc
Daiki C Compiler

### Notice
This is for my understandings of interpreter-compiler.

Currently the usage is limited, but you can use example/ directory for testing.

```sh
$ git clone https://github.com/ami-GS/dcc
$ cd dcc
$ make
$ dcc example/fib.c -srmo
$ dcc example/sort.c -srmo
$ dcc example/list.c -srmo
$ dcc example/test.c -srmo
```

## Options
options below can be used with command
- -r : run the code
- -i : leave '.i' file after precompile
- -s : show assembly generated
- -m : show move of assembly
- -p : step execution by assembly level
- -t : show tree parsed from expression/statement
- -o : optimize code. remove/marge nonsense code

## TODO
These are TODO list specilly I would like to implement.

- Use syntax tree for parse statement.

- Enable global variable initialization. In assembler generated by gcc, it seems to be initialized before associated functions are called.

- convert to binary

- Write linker

## Specification

- Stack based (no registers)

- My own designed assemblers

### Supported types
Types below with pointer are supported.

- Int, Short, Float, Double, Char(string) and these pointer

- Struct

### Passed statements
statements below were properly translated to assembler with simple processing.
like

```c
int a = 0;
while (a < 5) {a += 1;}
for (a = 0; i < 5 ; a+=2) {a -= 2;}
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

- binary operations (+, -, *, /, %)

- unary operations (+, -, *, !, ++, --)

- binary bit operations (&, |, ^, <<, >>)

- unary bit operation (~)

- addressing ([], ., ->)

- binary boolean operations (==, !=, <=, =>, >=, =<, &&, ||)

- compound operators (+=, -=, *=, /=, %=, &=, |=, ^=, <<=, >>=)

- comment (//, /**/)