#include "compiler.h"
#include "helpers/vector.h"
#include <stdarg.h>
#include <stdio.h>
#include <assert.h>

#define STRUCTURE_PUSH_START_POSITION_ONE 1

static struct compile_process *current_process = NULL;
static struct node *current_function = NULL;
void asm_push(const char *ins, ...);

struct history;
struct _x86_generator_private
{
    struct x86_generator_remembered
    {
        struct history* history;
    } remembered;

} _x86_generator_private;
