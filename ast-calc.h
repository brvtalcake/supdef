/* A packrat parser generated by PackCC 2.0.3 */

#ifndef PCC_INCLUDED_AST_CALC_H
#define PCC_INCLUDED_AST_CALC_H

#define PCC_AST_NODE_CUSTOM_DATA_DEFINED /* <-- enables node custom data */

typedef struct text_data_tag { /* <-- node custom data type */
    char *text;
} pcc_ast_node_custom_data_t;

#define pcc_ast_node__create_0() pcc_ast_node__create_nullary(auxil)
#define pcc_ast_node__create_1(node) pcc_ast_node__create_unary(auxil, node)
#define pcc_ast_node__create_2(node0, node1) pcc_ast_node__create_binary(auxil, node0, node1)
#define pcc_ast_node__create_3(node0, node1, node2) pcc_ast_node__create_ternary(auxil, node0, node1, node2)
#define pcc_ast_node__create_v() pcc_ast_node__create_variadic(auxil)

#ifdef __cplusplus
extern "C" {
#endif

typedef struct pcc_ast_manager_tag pcc_ast_manager_t;
typedef struct pcc_ast_node_tag pcc_ast_node_t;

typedef enum pcc_ast_node_type_tag {
    PCC_AST_NODE_TYPE_NULLARY,
    PCC_AST_NODE_TYPE_UNARY,
    PCC_AST_NODE_TYPE_BINARY,
    PCC_AST_NODE_TYPE_TERNARY,
    PCC_AST_NODE_TYPE_VARIADIC
} pcc_ast_node_type_t;

typedef struct pcc_ast_node_single_tag {
    pcc_ast_node_t *node;
} pcc_ast_node_single_t;

typedef struct pcc_ast_node_double_tag {
    pcc_ast_node_t *node[2];
} pcc_ast_node_double_t;

typedef struct pcc_ast_node_triple_tag {
    pcc_ast_node_t *node[3];
} pcc_ast_node_triple_t;

typedef struct pcc_ast_node_array_tag {
    pcc_ast_node_t **node;
    size_t max, len;
} pcc_ast_node_array_t;

typedef union pcc_ast_node_data_tag {
    pcc_ast_node_single_t unary;
    pcc_ast_node_double_t binary;
    pcc_ast_node_triple_t ternary;
    pcc_ast_node_array_t variadic;
} pcc_ast_node_data_t;

struct pcc_ast_node_tag {
    pcc_ast_manager_t *manager;
    pcc_ast_node_t *next; /* the next element in the doubly linked list of managed nodes */
    pcc_ast_node_t *prev; /* the previous element in the doubly linked list of managed nodes */
    pcc_ast_node_t *parent;
    pcc_ast_node_type_t type;
    pcc_ast_node_data_t data;
#ifdef PCC_AST_NODE_CUSTOM_DATA_DEFINED
    pcc_ast_node_custom_data_t custom;
#endif
};

struct pcc_ast_manager_tag {
    pcc_ast_node_t *first; /* the first element in the doubly linked list of managed nodes */
#ifdef PCC_AST_MANAGER_CUSTOM_DATA_DEFINED
    pcc_ast_manager_custom_data_t custom;
#endif
};

void pcc_ast_manager__initialize(pcc_ast_manager_t *obj);
void pcc_ast_manager__finalize(pcc_ast_manager_t *obj);

void pcc_ast_node_array__initialize(pcc_ast_manager_t *mgr, pcc_ast_node_array_t *obj);
void pcc_ast_node_array__finalize(pcc_ast_manager_t *mgr, pcc_ast_node_array_t *obj);
void pcc_ast_node_array__add(pcc_ast_manager_t *mgr, pcc_ast_node_array_t *obj, pcc_ast_node_t *node);

pcc_ast_node_t *pcc_ast_node__create_nullary(pcc_ast_manager_t *mgr);
pcc_ast_node_t *pcc_ast_node__create_unary(pcc_ast_manager_t *mgr, pcc_ast_node_t *node);
pcc_ast_node_t *pcc_ast_node__create_binary(pcc_ast_manager_t *mgr, pcc_ast_node_t *node0, pcc_ast_node_t *node1);
pcc_ast_node_t *pcc_ast_node__create_ternary(pcc_ast_manager_t *mgr, pcc_ast_node_t *node0, pcc_ast_node_t *node1, pcc_ast_node_t *node2);
pcc_ast_node_t *pcc_ast_node__create_variadic(pcc_ast_manager_t *mgr);
pcc_ast_node_t *pcc_ast_node__add_child(pcc_ast_node_t *obj, pcc_ast_node_t *node); /* for variadic node only */
void pcc_ast_node__destroy(pcc_ast_node_t *obj);

#ifdef PCC_AST_MANAGER_CUSTOM_DATA_DEFINED
void pcc_ast_manager_custom_data__initialize(pcc_ast_manager_custom_data_t *obj);
void pcc_ast_manager_custom_data__finalize(pcc_ast_manager_custom_data_t *obj);
#else
#define pcc_ast_manager_custom_data__initialize(obj) ((void)0)
#define pcc_ast_manager_custom_data__finalize(obj) ((void)0)
#endif

#ifdef PCC_AST_NODE_CUSTOM_DATA_DEFINED
void pcc_ast_node_custom_data__initialize(pcc_ast_node_custom_data_t *obj);
void pcc_ast_node_custom_data__finalize(pcc_ast_node_custom_data_t *obj);
#else
#define pcc_ast_node_custom_data__initialize(obj) ((void)0)
#define pcc_ast_node_custom_data__finalize(obj) ((void)0)
#endif

#ifdef __cplusplus
}
#endif

#ifdef __cplusplus
extern "C" {
#endif

typedef struct calc_context_tag calc_context_t;

calc_context_t *calc_create(pcc_ast_manager_t *auxil);
int calc_parse(calc_context_t *ctx, pcc_ast_node_t **ret);
void calc_destroy(calc_context_t *ctx);

#ifdef __cplusplus
}
#endif

#endif /* !PCC_INCLUDED_AST_CALC_H */