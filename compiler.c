#include "compiler.h"
#include <stdarg.h>
#include <stdlib.h>

struct lex_process_functions compiler_lex_functions = {
    .next_char=compile_process_next_char,
    .peek_char=compile_process_peek_char,
    .push_char=compile_process_push_char
};
void compiler_node_error(struct node* node, const char* msg, ...)
{
    va_list args;
    va_start(args, msg);
    vfprintf(stderr, msg, args);
    va_end(args);

    fprintf(stderr, " on line %i, col %i in file %s\n", node->pos.line, node->pos.col, node->pos.filename);
    exit(-1);
}
void compiler_error(struct compile_process* compiler, const char* msg, ...)
{
    va_list args;
    va_start(args, msg);
    vfprintf(stderr, msg, args);
    va_end(args);
    fprintf(stderr, " on line %i, col %i in file %s\n", compiler->pos.line, compiler->pos.col, compiler->pos.filename);
    exit(-1);
}
void compiler_warning(struct compile_process* compiler, const char* msg, ...)
{
    va_list args;
    va_start(args, msg);
    vfprintf(stderr, msg, args);
    va_end(args);
    fprintf(stderr, " on line %i, col %i in file %s\n", compiler->pos.line, compiler->pos.col, compiler->pos.filename);
}
struct compile_process* compile_include_for_include_dir(const char* include_dir, const char* filename, struct compile_process* parent_process)
{
    char tmp_filename[512];
    sprintf(tmp_filename, "%s/%s", include_dir, filename);
    if (file_exists(tmp_filename))
    {
        filename = tmp_filename;
    }
      struct compile_process* process = compile_process_create(filename, NULL, parent_process->flags, parent_process);
    if (!process)
    {
        return NULL;
    }
     struct lex_process* lex_process = lex_process_create(process, &compiler_lex_functions, NULL);
    if (!lex_process)
    {
        return NULL;
    }
    if (lex(lex_process) != LEXICAL_ANALYSIS_ALL_OK)
    {
        return NULL;
    }
    process->token_vec_original = lex_process_tokens(lex_process);
    if (preprocessor_run(process) < 0)
    {
        return NULL;
    }

    return process;
}
struct compile_process* compile_include(const char* filename, struct compile_process* parent_process)
{
    struct compile_process* new_process = NULL;
    const char* include_dir = compiler_include_dir_begin(parent_process);
    while(include_dir && !new_process)
    {
        new_process = compile_include_for_include_dir(include_dir, filename, parent_process);
        include_dir = compiler_include_dir_next(parent_process);
    }

    return new_process;
}
int compile_file(const char* filename, const char* out_filename, int flags)
{
    struct compile_process* process = compile_process_create(filename, out_filename, flags, NULL);
    if (!process)
        return COMPILER_FAILED_WITH_ERRORS;

    // Preform lexical analysis
    struct lex_process* lex_process = lex_process_create(process, &compiler_lex_functions, NULL);
    if (!lex_process)
    {
        return COMPILER_FAILED_WITH_ERRORS;
    }

    if (lex(lex_process) != LEXICAL_ANALYSIS_ALL_OK)
    {
        return COMPILER_FAILED_WITH_ERRORS;
    }
    process->token_vec_original = lex_process_tokens(lex_process);
    if (preprocessor_run(process) != 0)
    {
        return COMPILER_FAILED_WITH_ERRORS;
    }
      if (parse(process) != PARSE_ALL_OK)
    {
        return COMPILER_FAILED_WITH_ERRORS;
    }
     if (validate(process) != VALIDATION_ALL_OK)
    {
        return COMPILER_FAILED_WITH_ERRORS;
    }
      if (codegen(process) != CODEGEN_ALL_OK)
    {
        return COMPILER_FAILED_WITH_ERRORS;
    }
    
    // Preform code generation..

    fclose(process->ofile);
    return COMPILER_FILE_COMPILED_OK;
}
struct native_function_callbacks
{
    NATIVE_FUNCTION_CALL call;
};

struct native_function
{
    const char* name;
    struct native_function_callbacks callbacks;
};
struct symbol* native_create_function(struct compile_process* compiler, const char* name,
struct native_function_callbacks* callbacks);

struct native_function* native_function_get(struct compile_process* compiler, const char* name);

struct preprocessor;
struct preprocessor_definition;
struct preprocessor_function_argument;
struct preprocessor_included_file;
typedef void (*PREPROCESSOR_STATIC_INCLUDE_HANDLER_POST_CREATION)(struct preprocessor* preprocessor, struct preprocessor_included_file* included_file);

enum
{
    PREPROCESSOR_DEFINITION_STANDARD,
    PREPROCESSOR_DEFINITION_MACRO_FUNCTION,
    PREPROCESSOR_DEFINITION_NATIVE_CALLBACK,
    PREPROCESSOR_DEFINITION_TYPEDEF
};
struct preprocessor;
struct preprocessor_definition;
struct preprocessor_function_argument
{
    // Tokens for this argument struct token
    struct vector* tokens;
};
struct preprocessor_function_arguments
{
    // Vector of struct preprocessor_function_argument
    struct vector* arguments;
};
typedef int (*PREPROCESSOR_DEFINITION_NATIVE_CALL_EVALUATE)(struct preprocessor_definition* definition, struct preprocessor_function_arguments* arguments);
typedef struct vector* (*PREPROCESSOR_DEFINITION_NATIVE_CALL_VALUE)(struct preprocessor_definition* definition, struct preprocessor_function_arguments* arguments);