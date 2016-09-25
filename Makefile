dcc: dcc.c
	gcc -g -o dcc dcc.c letter_analysis.c parse_statement.c malloc.c misc.c symbol_table.c opcode.c syntactic_analysis.c instruction.c parse.c
