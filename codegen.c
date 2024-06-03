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
struct generator x86_codegen = {
    .asm_push=asm_push,
    .gen_exp=codegen_gen_exp,
    .end_exp=codegen_end_exp,
    .entity_address=codegen_entity_address,
    .ret=asm_push_ins_with_datatype,
    .private=&_x86_generator_private
};
enum
{
    CODEGEN_ENTITY_RULE_IS_STRUCT_OR_UNION_NON_POINTER = 0b00000001,
    CODEGEN_ENTITY_RULE_IS_FUNCTION_CALL = 0b00000010,
    CODEGEN_ENTITY_RULE_IS_GET_ADDRESS = 0b00000100,
    CODEGEN_ENTITY_RULE_WILL_PEEK_AT_EBX = 0b00001000,
};
enum
{
    RESPONSE_FLAG_ACKNOWLEDGED = 0b00000001,
    RESPONSE_FLAG_PUSHED_STRUCTURE = 0b00000010,
    RESPONSE_FLAG_RESOLVED_ENTITY = 0b00000100,
    RESPONSE_FLAG_UNARY_GET_ADDRESS = 0b00001000,
};
#define RESPONSE_SET(x) (&(struct response){x})
#define RESPONSE_EMPTY RESPONSE_SET()
struct response_data
{
    union
    {
        struct resolver_entity *resolved_entity;
    };
};
void codegen_response_expect()
{
    struct response *res = calloc(1, sizeof(struct response));
    vector_push(current_process->generator->responses, &res);
}
struct response_data *codegen_response_data(struct response *response)
{
    return &response->data;
}
struct response *codegen_response_pull()
{
    struct response *res = vector_back_ptr_or_null(current_process->generator->responses);
    if (res)
    {
        vector_pop(current_process->generator->responses);
    }
    return res;
}
void codegen_response_acknowledge(struct response *response_in)
{
// it will store the vectoe
    struct response *res= vector_back_ptr_or_null(struct response *response_in)
    if(res)
    {
        res->flags |= response_in->flags;
        if (response_in->data.resolved_entity)
        {
            res->data.resolved_entity = response_in->data.resolved_entity;
        }
    }
}
bool codegen_response_acknowledged(struct response *res)
{
    return res && res->flags && RESPONSE_FLAG_ACKNOWLEDGED;
}
bool codegen_response_has_entity(struct response *res)
{
    return codegen_response_acknowledged(res) && res->flags & RESPONSE_FLAG_RESOLVED_ENTITY && res->data.resolved_entity;
}
struct history_exp
{
    const char *logical_start_op;
    char logical_end_label[20];
    char logical_end_label_positive[20];
};
struct history
{
    int flags;
    union
    {
        struct history_exp exp;
    };
};
struct history
{
    int flags;
    union
    {
        struct history_exp exp;
    };
};
static struct history *history_down(struct history *history, int flags)
{
    struct history *new_history = calloc(1, sizeof(struct history));
    memcpy(new_history, history, sizeof(struct history));
    new_history->flags = flags;
    return new_history;
}
// generate the history  nodes
void codegen_generate_exp_node(struct node *node, struct history *history);
// generate th registers
const char *codegen_sub_register(const char *original_register, size_t size);
//generate the entity access for the function call
void codegen_generate_entity_access_for_function_call(struct resolver_result *result, struct resolver_entity *entity);
void codegen_generate_structure_push(struct resolver_entity *entity, struct history *history, int start_pos);
bool codegen_resolve_node_for_value(struct node *node, struct history *history);
bool asm_datatype_back(struct datatype *dtype_out);
void codegen_generate_entity_access_for_unary_get_address(struct resolver_result *result, struct resolver_entity *entity);
void codegen_generate_expressionable(struct node *node, struct history *history);
int codegen_label_count();
void codegen_generate_body(struct node *node, struct history *history);
int codegen_remove_uninheritable_flags(int flags);
void codegen_generate_assignment_part(struct node *node, const char *op, struct history *history);
void codegen_finish_scope()
{
    resolver_default_finish_scope(current_process->resolver);
}
struct node *codegen_node_next()
{
    return vector_peek_ptr(current_process->node_tree_vec);
}
void codegen_entity_address(struct generator* generator, struct resolver_entity* entity, struct generator_entity_address* address_out)
{
    struct resolver_default_entity_data* data = codegen_entity_private(entity);
    address_out->address = data->address;
    address_out->base_address = data->base_address;
    address_out->is_stack = data->flags & RESOLVER_DEFAULT_ENTITY_FLAG_IS_LOCAL_STACK;
    address_out->offset = data->offset;
}
void asm_push_args(const char *ins, va_list args)
{
    va_list args2;
    va_copy(args2, args);
    vfprintf(stdout, ins, args);
    fprintf(stdout, "\n");
    if (current_process->ofile)
    {
        vfprintf(current_process->ofile, ins, args2);
        fprintf(current_process->ofile, "\n");
    }
}
void asm_push(const char *ins, ...)
{
    va_list args;
    va_start(args, ins);
    asm_push_args(ins, args);
    va_end(args);
}

void asm_push_ins_with_datatype(struct datatype* dtype, const char* fmt, ...)
{
     char tmp_buf[200];
    sprintf(tmp_buf, "push %s", fmt);
    va_list args;
    va_start(args, fmt);
    asm_push_args(tmp_buf, args);
    va_end(args);

    assert(current_function);
    stackframe_push(current_function, &(struct stack_frame_element){.type = STACK_FRAME_ELEMENT_TYPE_PUSHED_VALUE, .name="result_value", .flags=STACK_FRAME_ELEMENT_FLAG_HAS_DATATYPE, .data.dtype=*dtype});
}

void asm_push_no_nl(const char *ins, ...)
{
    va_list args;
    va_start(args, ins);
    vfprintf(stdout, ins, args);
    va_end(args);

    if (current_process->ofile)
    {
        va_list args;
        va_start(args, ins);
        vfprintf(current_process->ofile, ins, args);
        va_end(args);
    }
}
void asm_push_ins_push(const char *fmt, int stack_entity_type, const char *stack_entity_name, ...)
{
    char tmp_buf[200];
    sprintf(tmp_buf, "push %s", fmt);
    va_list args;
     va_start(args, stack_entity_name);
    asm_push_args(tmp_buf, args);
    va_end(args);

    assert(current_function);
    stackframe_push(current_function, &(struct stack_frame_element){.type = stack_entity_type, .name = stack_entity_name});
}
void asm_push_ins_push_with_flags(const char *fmt, int stack_entity_type, const char *stack_entity_name, int flags, ...)
{
    char tmp_buf[200];
    sprintf(tmp_buf, "push %s", fmt);
    va_list args;
    va_start(args, flags);
    asm_push_args(tmp_buf, args);
    va_end(args);
    assert(current_function);
    stackframe_push(current_function, &(struct stack_frame_element){.flags = flags, .type = stack_entity_type, .name = stack_entity_name});
}
int asm_push_ins_pop(const char *fmt, int expecting_stack_entity_type, const char *expecting_stack_entity_name, ...)
{
    char tmp_buf[200];
    sprintf(tmp_buf, "pop %s", fmt);
    va_list args;
    va_start(args, expecting_stack_entity_name);
    asm_push_args(tmp_buf, args);
    va_end(args);

