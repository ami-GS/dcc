# dcc
Daiki C Compiler

### Notice
This is for my understandings of interpreter-compiler.

Currently the usage is limited.


## Supported types
Types below are supported.

pointer * is not supported stil.

- Int

- Char (string)

## Passed statements
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

- switch, case, default

- continue

- break

- function, prototype, call it (with arguments)

- local & global (array) variable declaration (with initialization)