
typedef enum {
    SYNTAX_ERROR,
    BUFFER_OVERFLOW,
    BUFFER_UNDERFLOW,
    VARIABLE_DUPLICATION,
    FUNCTION_DUPLICATION,
} dcc_error;

static const ERROR_STRING[] = {
    "Syntax error",
    "Buffer overflow",
    "Buffer underflow",
    "Variable name is duplicating",
    "Function name is duplicating",
};
