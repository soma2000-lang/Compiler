#include "compiler.h"
#include "helpers/vector.h"
#include <stdlib.h>

struct fixup_system* fixup_sys_new()
{
    struct fixup_system* system = calloc(1, sizeof(struct fixup_system));
    system->fixups = vector_create(sizeof(struct fixup));
    return system;
}
struct fixup_config* fixup_config(struct fixup* fixup)
{
    return &fixup->config;
}
void fixup_free(struct fixup* fixup)
{
    fixup->config.end(fixup);
    free(fixup);
}
void fixup_start_iteration(struct fixup_system* system)
{
    vector_set_peek_pointer(system->fixups, 0);
}
struct fixup* fixup_next(struct fixup_system* system)
{
    return vector_peek_ptr(system->fixups);
}
void fixup_sys_free(struct fixup_system* system)
{
    fixup_sys_fixups_free(system);
    vector_free(system->fixups);
    free(system);
}
int fixup_sys_unresolved_fixups_count(struct fixup_system* system)
{
    size_t c = 0;
    fixup_start_iteration(system);
    struct fixup* fixup = fixup_next(system);
    while(fixup)
    {
        if (fixup->flags & FIXUP_FLAG_RESOLVED)
        {
            fixup = fixup_next(system);
            continue;
        }
        c++;
        fixup = fixup_next(system);
    }

    return c;
}

struct fixup* fixup_register(struct fixup_system* system, struct fixup_config* config)
{
    struct fixup* fixup = calloc(1, sizeof(struct fixup));
    memcpy(&fixup->config, config, sizeof(struct fixup_config));
    fixup->system = system;
    vector_push(system->fixups, fixup);
    return fixup;
}
bool fixup_resolve(struct fixup* fixup)
{
    if (fixup_config(fixup)->fix(fixup))
    {
        fixup->flags |= FIXUP_FLAG_RESOLVED;
        return true;
    }

    return false;
}
void* fixup_private(struct fixup* fixup)
{
    return fixup_config(fixup)->private;
}
 else
    {
        expect_sym(';');
    }
    void parse_symbol()
{
    if (token_next_is_symbol('{'))
    {
        size_t variable_size = 0;
         size_t variable_size = 0;
        struct history *history = history_begin(HISTORY_FLAG_IS_GLOBAL_SCOPE);
        parse_body(&variable_size, history);
        struct node *body_node = node_pop();

        node_push(body_node);
    }
    else if (token_next_is_symbol(':'))
    {
        parse_label(history_begin(0));
        return;
    }

    compiler_error(current_process, "Invalid symbol was provided");
}
void parse_statement(struct history *history)
{
    if (token_peek_next()->type == TOKEN_TYPE_KEYWORD)
    {
        parse_keyword(history);
        return;
    }
     parse_expressionable_root(history);
    if (token_peek_next()->type == TOKEN_TYPE_SYMBOL && !token_is_symbol(token_peek_next(), ';'))
    {
        parse_symbol();
        return;
    }

    // All statements end with semicolons;
    expect_sym(';');
}
void parser_append_size_for_node_struct_union(struct history *history, size_t *_variable_size, struct node *node)
{
    *_variable_size += variable_size(node);
    if (node->var.type.flags & DATATYPE_FLAG_IS_POINTER)
    {
        return;
    }

    struct node *largest_var_node = variable_struct_or_union_body_node(node)->body.largest_var_node;
    if (largest_var_node)
    {
        *_variable_size += align_value(*_variable_size, largest_var_node->var.type.size);
    }
}
void parser_append_size_for_node(struct history *history, size_t *_variable_size, struct node *node);
void parser_append_size_for_variable_list(struct history *history, size_t *variable_size, struct vector *vec)
{
    vector_set_peek_pointer(vec, 0);
    struct node *node = vector_peek_ptr(vec);
    while (node)
    {
        parser_append_size_for_node(history, variable_size, node);
        node = vector_peek_ptr(vec);
    }
}

void parser_append_size_for_node(struct history *history, size_t *_variable_size, struct node *node)
{
    if (!node)
    {
        return;
    }
 if (node->type == NODE_TYPE_VARIABLE)
    {
        if (node_is_struct_or_union_variable(node))
        {
            parser_append_size_for_node_struct_union(history, _variable_size, node);
            return;
        }
        *_variable_size += variable_size(node);
    }
    else if (node->type == NODE_TYPE_VARIABLE_LIST)
    {
        parser_append_size_for_variable_list(history, _variable_size, node->var_list.list);
    }
}
void parser_finalize_body(struct history *history, struct node *body_node, struct vector *body_vec, size_t *_variable_size, struct node *largest_align_eligible_var_node, struct node *largest_possible_var_node)
{
    if (history->flags & HISTORY_FLAG_INSIDE_UNION)
    {
        if (largest_possible_var_node)
        {
            *_variable_size = variable_size(largest_possible_var_node);
        }
    }
      int padding = compute_sum_padding(body_vec);
    *_variable_size += padding;

    if (largest_align_eligible_var_node)
    {
        *_variable_size = align_value(*_variable_size, largest_align_eligible_var_node->var.type.size);
    }


