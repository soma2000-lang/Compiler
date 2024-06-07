
#include <stdio.h>
#include "helpers/vector.h"
#include "compiler.h"


#define LEX_GETC_IF(buffer, c, exp)     \
    for (c = peekc(); exp; c = peekc()) \
    {                                   \
        buffer_write(buffer, c);        \
        nextc();                        \
    }
    struct token *read_next_token();
    bool lex_is_in_expression();

    static struct lex_process *lex_process;
    static struct token tmp_token;
    char lex_get_escaped_char(char c);
    static char peekc()
    {
        return lex_process->function->peek_char(lex_process);
    }

    static char nextc()
{
    char c = lex_process->function->next_char(lex_process);
    if (lex_is_in_expression())
