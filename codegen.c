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
    assert(current_function);
    struct stack_frame_element *element = stackframe_back(current_function);
    int flags = element->flags;
    stackframe_pop_expecting(current_function, expecting_stack_entity_type, expecting_stack_entity_name);
    return flags;
}

void asm_push_ins_push_with_data(const char *fmt, int stack_entity_type, const char *stack_entity_name, int flags, struct stack_frame_data *data, ...)
{
    char tmp_buf[200];
    sprintf(tmp_buf, "push %s", fmt);
    va_list args;
    va_start(args, data);
    asm_push_args(tmp_buf, args);
    va_end(args);
    flags |= STACK_FRAME_ELEMENT_FLAG_HAS_DATATYPE;
    assert(current_function);
    stackframe_push(current_function, &(struct stack_frame_element){.type = stack_entity_type, .name = stack_entity_name, .flags = flags, .data = *data});
}
void asm_push_ebp()
{
    asm_push_ins_push("ebp", STACK_FRAME_ELEMENT_TYPE_SAVED_BP, "function_entry_saved_ebp");
}

int asm_push_ins_pop_or_ignore(const char *fmt, int expecting_stack_entity_type, const char *expecting_stack_entity_name, ...)
{
    if (!stackframe_back_expect(current_function, expecting_stack_entity_type, expecting_stack_entity_name))
    {
        return STACK_FRAME_ELEMENT_FLAG_ELEMENT_NOT_FOUND;
    }
     char tmp_buf[200];
    sprintf(tmp_buf, "pop %s", fmt);
    va_list args;
    va_start(args, expecting_stack_entity_name);
    asm_push_args(tmp_buf, args);
    va_end(args);
     struct stack_frame_element *element = stackframe_back(current_function);
    int flags = element->flags;
    stackframe_pop_expecting(current_function, expecting_stack_entity_type, expecting_stack_entity_name);
    return flags;
}
void codegen_data_section_add(const char *data, ...)
{
    va_list args;
    va_start(args, data);
    char* new_data= malloc(256);
    vsprintf(new_data, data, args);
    vector_push(current_process->generator->custom_data_section, &new_data);
}

void codegen_stack_add_no_compile_time_stack_frame_restore(size_t stack_size)
{
    if (stack_size != 0)
    {
        asm_push("add esp, %lld", stack_size);
    }
}
void codegen_stack_sub_with_name(size_t stack_size, const char *name)
{
    if (stack_size != 0)
    {
        stackframe_sub(current_function, STACK_FRAME_ELEMENT_TYPE_UNKNOWN, name, stack_size);
        asm_push("sub esp, %lld", stack_size);
    }
}
void codegen_stack_sub(size_t stack_size)
{
    codegen_stack_sub_with_name(stack_size, "literal_stack_change");
}
void codegen_stack_add_with_name(size_t stack_size, const char *name)
{
    if (stack_size != 0)
    {
        stackframe_add(current_function, STACK_FRAME_ELEMENT_TYPE_UNKNOWN, name, stack_size);
        asm_push("add esp, %lld", stack_size);
    }
}
struct resolver_entity *codegen_new_scope_entity(struct node *var_node, int offset, int flags)
{
    return resolver_default_new_scope_entity(current_process->resolver, var_node, offset, flags);
}
const char *codegen_get_label_for_string(const char *str)
{
    const char *result = NULL;
    struct code_generator *generator = current_process->generator;
    vector_set_peek_pointer(generator->string_table, 0);
     struct string_table_element *current = vector_peek_ptr(generator->string_table);
    while (current)
    {
        if (S_EQ(current->str, str))
        {
            result = current->label;
            break;
        }

        current = vector_peek_ptr(generator->string_table);
    }

    return result;
}
const char *codegen_register_string(const char *str)
{
    const char *label = codegen_get_label_for_string(str);
    if (label)
    {
        // We already registered this string, just return the label to the string memory.
        return label;
    }
const char *codegen_register_string(const char *str)
{
    const char *label = codegen_get_label_for_string(str);
    if (label)
    {
        // We already registered this string, just return the label to the string memory.
        return label;
    }
struct code_generator *codegenerator_new(struct compile_process *process)
{
    struct code_generator *generator = calloc(1, sizeof(struct code_generator));
    generator->string_table = vector_create(sizeof(struct string_table_element *));
    generator->entry_points = vector_create(sizeof(struct codegen_entry_point *));
    generator->exit_points = vector_create(sizeof(struct codegen_exit_point *));
    generator->responses = vector_create(sizeof(struct response *));
    generator->_switch.swtiches = vector_create(sizeof(struct generator_switch_stmt_entity));
    generator->custom_data_section = vector_create(sizeof(const char*));
    return generator;
}
void codegen_register_exit_point(int exit_point_id)
{
    struct code_generator *gen = current_process->generator;
    struct codegen_exit_point *exit_point = calloc(1, sizeof(struct codegen_exit_point));
    exit_point->id = exit_point_id;
    vector_push(gen->exit_points, &exit_point);
}
struct codegen_exit_point *codegen_current_exit_point()
{
    struct code_generator *gen = current_process->generator;
    return vector_back_ptr_or_null(gen->exit_points);
}
int codegen_label_count()
{
    static int count = 0;
    count++;
    return count;
}
void codegen_begin_exit_point()
{
    int exit_point_id = codegen_label_count();
    codegen_register_exit_point(exit_point_id);
}
void codegen_goto_exit_point(struct node *node)
{
    struct code_generator *gen = current_process->generator;
    struct codegen_exit_point *exit_point = codegen_current_exit_point();
    asm_push("jmp .exit_point_%i", exit_point->id);
}
void codegen_register_entry_point(int entry_point_id)
{
    struct code_generator *gen = current_process->generator;
    struct codegen_entry_point *entry_point = calloc(1, sizeof(struct codegen_entry_point));
    entry_point->id = entry_point_id;
    vector_push(gen->entry_points, &entry_point);
}
void codegen_begin_entry_point()
{
    int entry_point_id = codegen_label_count();
    codegen_register_entry_point(entry_point_id);
    asm_push(".entry_point_%i:", entry_point_id);
}
void codegen_end_entry_point()
{
    struct code_generator *gen = current_process->generator;
    struct codegen_entry_point *entry_point = codegen_current_entry_point();
    assert(entry_point);
    free(entry_point);
    vector_pop(gen->entry_points);
}
void codegen_begin_entry_exit_point()
{
    codegen_begin_entry_point();
    codegen_begin_exit_point();
}
void codegen_end_entry_exit_point()
{
    codegen_end_entry_point();
    codegen_end_exit_point();
}
void codegen_begin_switch_statement()
{
    struct code_generator *generator = current_process->generator;
    struct generator_switch_stmt *switch_stmt_data = &generator->_switch;
    vector_push(switch_stmt_data->swtiches, &switch_stmt_data->current);
    memset(&switch_stmt_data->current, 0, sizeof(struct generator_switch_stmt_entity));
    int switch_stmt_id = codegen_label_count();
    asm_push(".switch_stmt_%i:", switch_stmt_id);
    switch_stmt_data->current.id = switch_stmt_id;
}
void codegen_end_switch_statement()
{
    struct code_generator *generator = current_process->generator;
    struct generator_switch_stmt *switch_stmt_data = &generator->_switch;
    asm_push(".switch_stmt_%i_end:", switch_stmt_data->current.id);
    // Lets restore the older switch statement
    memcpy(&switch_stmt_data->current, vector_back(switch_stmt_data->swtiches), sizeof(struct generator_switch_stmt_entity));
    vector_pop(switch_stmt_data->swtiches);
}
int codegen_switch_id()
{
    struct code_generator *generator = current_process->generator;
    struct generator_switch_stmt *switch_stmt_data = &generator->_switch;
    return switch_stmt_data->current.id;
}
void codegen_begin_case_statement(int index)
{
    struct code_generator *generator = current_process->generator;
    struct generator_switch_stmt *switch_stmt_data = &generator->_switch;
    asm_push(".switch_stmt_%i_case_%i:", switch_stmt_data->current.id, index);
}
void codegen_end_case_statement()
{
    // Do nothing.
}
static const char *asm_keyword_for_size(size_t size, char *tmp_buf)
{
    const char *keyword = NULL;
    switch (size)
    {
    case DATA_SIZE_BYTE:
        keyword = "db";
        break;
    case DATA_SIZE_WORD:
        keyword = "dw";
        break;
    case DATA_SIZE_DWORD:
        keyword = "dd";
        break; case DATA_SIZE_DDWORD:
        keyword = "dq";
        break;

    default:
        sprintf(tmp_buf, "times %lu db ", (unsigned long)size);
        return tmp_buf;
    }

    strcpy(tmp_buf, keyword);
    return tmp_buf;
}
void codegen_generate_global_variable_for_struct(struct node *node)
{
    if (node->var.val != NULL)
    {
        compiler_error(current_process, "We dont yet support values for structures");
        return;
    }

    char tmp_buf[256];
    asm_push("%s: %s 0", node->var.name, asm_keyword_for_size(variable_size(node), tmp_buf));
}
void codegen_generate_global_variable_for_union(struct node *node)
{
    if (node->var.val != NULL)
    {
        compiler_error(current_process, "We dont yet support values for unions");
        return;
    }

    char tmp_buf[256];
    asm_push("%s: %s 0", node->var.name, asm_keyword_for_size(variable_size(node), tmp_buf));
}
void codegen_generate_variable_for_array(struct node *node)
{
    if (node->var.val != NULL)
    {
        compiler_error(current_process, "We don't support values for arrays yet");
        return;
    }

    char tmp_buf[256];
    asm_push("%s: %s 0", node->var.name, asm_keyword_for_size(variable_size(node), tmp_buf));
}
void codegen_generate_global_variable(struct node *node)
{
    asm_push("; %s %s", node->var.type.type_str, node->var.name);
    if (node->var.type.flags & DATATYPE_FLAG_IS_ARRAY)
    {
        codegen_generate_variable_for_array(node);
        codegen_new_scope_entity(node, 0, 0);
        return;
    }
    switch (node->var.type.type)
    {
    case DATA_TYPE_VOID:
    case DATA_TYPE_CHAR:
    case DATA_TYPE_SHORT:
    case DATA_TYPE_INTEGER:
    case DATA_TYPE_LONG:
        codegen_generate_global_variable_for_primitive(node);
        break;

    case DATA_TYPE_STRUCT:
        codegen_generate_global_variable_for_struct(node);
        break;
          case DATA_TYPE_UNION:
        codegen_generate_global_variable_for_union(node);
        break;
    case DATA_TYPE_DOUBLE:
    case DATA_TYPE_FLOAT:
        compiler_error(current_process, "Doubles and floats are not supported in our subset of C\n");
        break;
    }

    assert(node->type == NODE_TYPE_VARIABLE);
    codegen_new_scope_entity(node, 0, 0);
}
void codegen_generate_struct(struct node *node)
{
    if (node->flags & NODE_FLAG_HAS_VARIABLE_COMBINED)
    {
        codegen_generate_global_variable(node->_struct.var);
    }
}
void codegen_generate_union(struct node *node)
{
    if (node->flags & NODE_FLAG_HAS_VARIABLE_COMBINED)
    {
        codegen_generate_global_variable(node->_union.var);
    }
}
void codegen_generate_global_variable_list(struct node *var_list_node)
{
    assert(var_list_node->type == NODE_TYPE_VARIABLE_LIST);
    vector_set_peek_pointer(var_list_node->var_list.list, 0);
    struct node *var_node = vector_peek_ptr(var_list_node->var_list.list);
    while (var_node)
    {
        codegen_generate_global_variable(var_node);
        var_node = vector_peek_ptr(var_list_node->var_list.list);
    }
}
void codegen_generate_data_section_part(struct node *node)
{
    switch (node->type)
    {
    case NODE_TYPE_VARIABLE:
        codegen_generate_global_variable(node);
        break;

    case NODE_TYPE_VARIABLE_LIST:
        codegen_generate_global_variable_list(node);
        break;
    case NODE_TYPE_STRUCT:
        codegen_generate_struct(node);
        break;

    case NODE_TYPE_UNION:
        codegen_generate_union(node);
        break;

    default:
        break;
    }
}s
void codegen_generate_data_section()
{
    asm_push("section .data");
    struct node *node = codegen_node_next();
    while (node)
    {
        codegen_generate_data_section_part(node);
        node = codegen_node_next();
    }
}
struct resolver_entity *codegen_register_function(struct node *func_node, int flags)
{
    return resolver_default_register_function(current_process->resolver, func_node, flags);
}
void codegen_generate_function_prototype(struct node *node)
{
    codegen_register_function(node, 0);
    asm_push("extern %s", node->func.name);
}
void codegen_generate_function_arguments(struct vector *argument_vector)
{
    vector_set_peek_pointer(argument_vector, 0);
    struct node *current = vector_peek_ptr(argument_vector);
    while (current)
    {
        codegen_new_scope_entity(current, current->var.aoffset, RESOLVER_DEFAULT_ENTITY_FLAG_IS_LOCAL_STACK);
        current = vector_peek_ptr(argument_vector);
    }
}
void codegen_generate_number_node(struct node *node, struct history *history)
{
    asm_push_ins_push_with_data("dword %i", STACK_FRAME_ELEMENT_TYPE_PUSHED_VALUE, "result_value", STACK_FRAME_ELEMENT_FLAG_IS_NUMERICAL, &(struct stack_frame_data){.dtype = datatype_for_numeric()}, node->llnum);
}
bool codegen_is_exp_root_for_flags(int flags)
{
    return !(flags & EXPRESSION_IS_NOT_ROOT_NODE);
}
void codegen_reduce_register(const char *reg, size_t size, bool is_signed)
{
    if (size != DATA_SIZE_DWORD && size > 0)
    {
        const char *ins = "movsx";
        if (!is_signed)
        {
            ins = "movzx";
        }
        asm_push("%s eax, %s", ins, codegen_sub_register("eax", size));
    }
}
void codegen_gen_mem_access_get_address(struct node *node, int flags, struct resolver_entity *entity)
{
    asm_push("lea ebx, [%s]", codegen_entity_private(entity)->address);
    asm_push_ins_push_with_flags("ebx", STACK_FRAME_ELEMENT_TYPE_PUSHED_VALUE, "result_value", STACK_FRAME_ELEMENT_FLAG_IS_PUSHED_ADDRESS);
}
void codegen_generate_structure_push_or_return(struct resolver_entity *entity, struct history *history, int start_pos)
{
    codegen_generate_structure_push(entity, history, start_pos);
}
void codegen_gen_mem_access(struct node *node, int flags, struct resolver_entity *entity)
{
    if (flags & EXPRESSION_GET_ADDRESS)
    {
        codegen_gen_mem_access_get_address(node, flags, entity);
        return;
    }

    if (datatype_is_struct_or_union_non_pointer(&entity->dtype))
    {
        codegen_gen_mem_access_get_address(node, 0, entity);
        asm_push_ins_pop("ebx", STACK_FRAME_ELEMENT_TYPE_PUSHED_VALUE, "result_value");
        codegen_generate_structure_push_or_return(entity, history_begin(0), 0);
    }
      else if (datatype_element_size(&entity->dtype) != DATA_SIZE_DWORD)
    {
        asm_push("mov eax, [%s]", codegen_entity_private(entity)->address);
        codegen_reduce_register("eax", datatype_element_size(&entity->dtype), entity->dtype.flags & DATATYPE_FLAG_IS_SIGNED);
        asm_push_ins_push_with_data("eax", STACK_FRAME_ELEMENT_TYPE_PUSHED_VALUE, "result_value", 0, &(struct stack_frame_data){.dtype = entity->dtype});
    }
    else
    {
        // We can push this straight to the stack
        asm_push_ins_push_with_data("dword [%s]", STACK_FRAME_ELEMENT_TYPE_PUSHED_VALUE, "result_value", 0, &(struct stack_frame_data){.dtype = entity->dtype}, codegen_entity_private(entity)->address);
    }
}
void codegen_generate_variable_access_for_entity(struct node *node, struct resolver_entity *entity, struct history *history)
{
    codegen_gen_mem_access(node, history->flags, entity);
}
void codegen_generate_unary_address(struct node *node, struct history *history)
{
    int flags = history->flags;
    codegen_generate_expressionable(node->unary.operand, history_down(history, flags | EXPRESSION_GET_ADDRESS));
    codegen_response_acknowledge(&(struct response){.flags = RESPONSE_FLAG_UNARY_GET_ADDRESS});
}
void codegen_generate_unary_indirection(struct node *node, struct history *history)
{
    const char *reg_to_use = "ebx";
    int flags = history->flags;
    codegen_response_expect();
    codegen_generate_expressionable(node->unary.operand, history_down(history, flags | EXPRESSION_GET_ADDRESS | EXPRESSION_INDIRECTION));
    struct response *res = codegen_response_pull();
    assert(codegen_response_has_entity(res));
    struct datatype operand_datatype;
    assert(asm_datatype_back(&operand_datatype));
    asm_push_ins_pop(reg_to_use, STACK_FRAME_ELEMENT_TYPE_PUSHED_VALUE, "result_value");

 int depth = node->unary.indirection.depth;
    int real_depth = depth;
    if (!(history->flags & EXPRESSION_GET_ADDRESS))
    {
        depth++;
    }

    for (int i = 0; i < depth; i++)
    {
        asm_push("mov %s, [%s]", reg_to_use, reg_to_use);
    }
     if (real_depth == res->data.resolved_entity->dtype.pointer_depth)
    {
        codegen_reduce_register(reg_to_use, datatype_size_no_ptr(&operand_datatype), operand_datatype.flags & DATATYPE_FLAG_IS_SIGNED);
    }
    else if (S_EQ(node->unary.op, "~"))
    {
        asm_push("not eax");
        asm_push_ins_push_with_data("eax", STACK_FRAME_ELEMENT_TYPE_PUSHED_VALUE, "result_value", 0, &(struct stack_frame_data){.dtype = last_dtype});
    }
    else if (S_EQ(node->unary.op, "*"))
    {
        codegen_generate_unary_indirection(node, history);
    }
       else if (S_EQ(node->unary.op, "++"))
    {
        if (node->unary.flags & UNARY_FLAG_IS_LEFT_OPERANDED_UNARY)
        {
            // a++
            asm_push_ins_push_with_data("eax", STACK_FRAME_ELEMENT_TYPE_PUSHED_VALUE, "result_value", 0, &(struct stack_frame_data){.dtype = last_dtype});
            asm_push("inc eax");
            asm_push_ins_push_with_data("eax", STACK_FRAME_ELEMENT_TYPE_PUSHED_VALUE, "result_value", 0, &(struct stack_frame_data){.dtype = last_dtype});
            codegen_generate_assignment_part(node->unary.operand, "=", history);
        }
        else
        {
            // ++a
            asm_push("inc eax");
            asm_push_ins_push_with_data("eax", STACK_FRAME_ELEMENT_TYPE_PUSHED_VALUE, "result_value", 0, &(struct stack_frame_data){.dtype = last_dtype});
            codegen_generate_assignment_part(node->unary.operand, "=", history);
            asm_push_ins_push_with_data("eax", STACK_FRAME_ELEMENT_TYPE_PUSHED_VALUE, "result_value", 0, &(struct stack_frame_data){.dtype = last_dtype});
        }
    }