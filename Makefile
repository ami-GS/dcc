dcc: dcc.c
	gcc -g -o dcc dcc.c letter_analysis.c parse_statement.c malloc.c misc.c symbol_table.c syntactic_analysis.c instruction.c parse.c preprocessor.c emulate_cpu.c -lm

.PHONY: clean test
clean:
	rm -f dcc
test:
	@./dcc example/sort.c -srmo
	@./dcc example/fib.c -srmo
	@./dcc example/list.c -srmo
	@./dcc example/operator.c -srmo
	@./dcc example/arithmetic.c -srmo
	@./dcc example/type.c -srmo
