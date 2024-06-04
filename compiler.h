#ifndef PEACHCOMPILER_H
#define PEACHCOMPILER_H

#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <linux/limits.h>
#include <assert.h>
#define FAIL_ERR(message) assert(0 == 1 && message)

#define S_EQ(str, str2) \
    (str && str2 && (strcmp(str, str2) == 0))

#define C_STACK_ALIGNMENT 16
#define STACK_PUSH_SIZE 4
#define C_ALIGN(size) (size % C_STACK_ALIGNMENT) ? size + (C_STACK_ALIGNMENT - (size % C_STACK_ALIGNMENT)) : size
#define NUMERIC_CASE \
    case '0':        \
    case '1':        \
    case '2':        \
    case '3':        \
    case '4':        \
    case '5':        \
    case '6':        \
    case '7':        \
    case '8':        \
    case '9'
#define OPERATOR_CASE_EXCLUDING_DIVISION \
    case '+':                            \
    case '-':                            \
    case '*':                            \
    case '>':                            \
    case '<':                            \
    case '^':                            \
    case '%':                            \
    case '!':                            \
    case '=':                            \
    case '~':                            \
    case '|':                            \
    case '&':                            \
    case '(':                            \
    case '[':                            \
    case ',':                            \
    case '.':                            \
    case '?'
#define SYMBOL_CASE \
    case '{':       \
    case '}':       \
    case ':':       \
    case ';':       \
    case '#':       \
    case '\\':      \
    case ')':       \
    case ']'
    enum
{
    LEXICAL_ANALYSIS_ALL_OK,
    LEXICAL_ANALYSIS_INPUT_ERROR
};
enum
{
    TOKEN_TYPE_IDENTIFIER,
    TOKEN_TYPE_KEYWORD,
    TOKEN_TYPE_OPERATOR,
    TOKEN_TYPE_SYMBOL,
    TOKEN_TYPE_NUMBER,
    TOKEN_TYPE_STRING,
    TOKEN_TYPE_COMMENT,
    TOKEN_TYPE_NEWLINE
};
enum
{
    NUMBER_TYPE_NORMAL,
    NUMBER_TYPE_LONG,
    NUMBER_TYPE_FLOAT,
    NUMBER_TYPE_DOUBLE
};
enum
{
    TOKEN_FLAG_IS_CUSTOM_OPERATOR = 0b00000001
};
struct token
{
    int type;
    int flags;
    struct pos pos;
    union
    {
        char cval;
        const char *sval;
        unsigned int inum;
        unsigned long lnum;
        unsigned long long llnum;
        void *any;
    };
struct token_number
    {
        int type;
    } num;
  bool whitespace;

    // (5+10+20)
    const char *between_brackets;

    // Anything between function call arguments. ABC(hello world)
    const char* between_arguments;
};
struct lex_process;
typedef char (*LEX_PROCESS_NEXT_CHAR)(struct lex_process *process);
typedef char (*LEX_PROCESS_PEEK_CHAR)(struct lex_process *process);
typedef void (*LEX_PROCESS_PUSH_CHAR)(struct lex_process *process, char c);

struct lex_process_functions
{
    LEX_PROCESS_NEXT_CHAR next_char;
    LEX_PROCESS_PEEK_CHAR peek_char;
    LEX_PROCESS_PUSH_CHAR push_char;
};
struct lex_process
{
    struct pos pos;
    struct vector *token_vec;
    struct compile_process *compiler;

    int current_expression_count;
    struct buffer *parentheses_buffer;

    struct buffer* argument_string_buffer;
    struct lex_process_functions *function;

    // This will be private data that the lexer does not understand
    // but the person using the lexer does understand.
    void *private;
};
enum
{
    COMPILER_FILE_COMPILED_OK,
    COMPILER_FAILED_WITH_ERRORS
};
enum
{
    COMPILE_PROCESS_EXECUTE_NASM = 0b00000001,
    COMPILE_PROCESS_EXPORT_AS_OBJECT = 0b00000010,
};
struct scope
{
    int flags;

    // void*
    struct vector *entities;

    // The total number of bytes this scope uses. Aligned to 16 bytes.
    size_t size;

    // NULL if no parent.
    struct scope *parent;
};
enum
{
    SYMBOL_TYPE_NODE,
    SYMBOL_TYPE_NATIVE_FUNCTION,
    SYMBOL_TYPE_UNKNOWN
};

struct symbol
{
    const char *name;
    int type;
    void *data;
}
struct codegen_entry_point
{
    // ID of the entry point
    int id;
};
struct code_generator
{

    struct generator_switch_stmt
    {
        struct generator_switch_stmt_entity
        {
            int id;
        } current;

        // Vector of generatr_switch_stmt_entity
        struct vector* swtiches;
    } _switch;
 struct vector *string_table;

    // vector of struct codegen_entry_point*
    struct vector *entry_points;
    // vector of struct codegen_exit_point*
    struct vector *exit_points;

    // Vector of const char* that will go in the data section
    struct vector* custom_data_section;

    // vector of struct response*
    struct vector *responses;
};
struct resolver_process;

struct generator;
struct native_function;
struct node;
struct resolver_entity;
struct datatype;
struct generator_entity_address
{
    bool is_stack;
    long offset;
    const char* address;
    const char* base_address;
};
#define GENERATOR_BEGIN_EXPRESSION(gen)
#define GENERATOR_END_EXPRESSION(gen) gen->end_exp(gen)

typedef void(*ASM_PUSH_PROTOTYPE)(const char* ins, ...);
typedef void (*NATIVE_FUNCTION_CALL)(struct generator* generator, struct native_function* func, struct vector* arguments);
typedef void(*GENERATOR_GENERATE_EXPRESSION)(struct generator* generator, struct node* node, int flags);
typedef void (*GENERATOR_ENTITY_ADDRESS)(
    struct generator* generator, struct resolver_entity* entity, 
    struct generator_entity_address* address_out);
typedef void(*GENERATOR_END_EXPRESSION)(struct generator* generator);

typedef void(*GENERATOR_FUNCTION_RETURN)(struct datatype* dtype, const char* fmt, ...);
