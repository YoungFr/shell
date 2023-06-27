#ifndef AST_H
#define AST_H

struct node {
    char        *data;
    struct node *next;
};

typedef struct node * node_ptr;

node_ptr make_node(char *data, node_ptr next);

typedef enum {
    CMD_ATOM,
    CMD_LIST,
    CMD_BACK,
    CMD_PIPE,
    CMD_ORED,
    CMD_IRED,
} cmd_type;

struct cmd {
    cmd_type type;
};

typedef struct cmd * cmd_ptr;

struct cmd_atom {
    cmd_type type;
    node_ptr node;
};

typedef struct cmd_atom * cmd_atom_ptr;

cmd_ptr cmd_atom_new(node_ptr node);

struct cmd_list {
    cmd_type type;
    cmd_ptr  left;
    cmd_ptr right;
};

typedef struct cmd_list * cmd_list_ptr;

cmd_ptr cmd_list_new(cmd_ptr left, cmd_ptr right);

struct cmd_back {
    cmd_type type;
    cmd_ptr  back;
};

typedef struct cmd_back * cmd_back_ptr;

cmd_ptr cmd_back_new(cmd_ptr back);

struct cmd_pipe {
    cmd_type type;
    cmd_ptr  left;
    cmd_ptr right;
};

typedef struct cmd_pipe * cmd_pipe_ptr;

cmd_ptr cmd_pipe_new(cmd_ptr left, cmd_ptr right);

struct cmd_ored {
    cmd_type type;
    cmd_ptr  left;
    cmd_ptr right;
    int        fd;
};

typedef struct cmd_ored * cmd_ored_ptr;

cmd_ptr cmd_ored_new(cmd_ptr left, cmd_ptr right, int fd);

struct cmd_ired {
    cmd_type type;
    cmd_ptr  left;
    cmd_ptr right;
    int        fd;
};

typedef struct cmd_ired * cmd_ired_ptr;

cmd_ptr cmd_ired_new(cmd_ptr left, cmd_ptr right, int fd);

void cmd_print(cmd_ptr cmd);

void cmd_run(cmd_ptr cmd);

#endif // AST_H
