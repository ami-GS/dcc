# dcc
Daiki C Compiler

### Notice
This is for my understandings of interpreter-compiler.
Currently the usage is limited.


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

- whlie

- do-whlie

- for

- if

- pre & post position incre & decrement

- switch, case, default

- continue

- break

- call function (without arguments)