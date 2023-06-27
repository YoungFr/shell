#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

#include "alloc.h"
#include "ast.h"

node_ptr make_node(char *data, node_ptr next)
{
    node_ptr t = malloc(sizeof(*t));
    t->data = data;
    t->next = next;
    return t;
}

cmd_ptr cmd_atom_new(node_ptr node)
{
    cmd_atom_ptr cmd;
    NEW(cmd);
    cmd->type = CMD_ATOM;
    cmd->node = node;
    return (cmd_ptr) cmd;
}

cmd_ptr cmd_list_new(cmd_ptr left, cmd_ptr right)
{
    cmd_list_ptr cmd;
    NEW(cmd);
    cmd->type  = CMD_LIST;
    cmd->left  = left;
    cmd->right = right;
    return (cmd_ptr) cmd;
}

cmd_ptr cmd_back_new(cmd_ptr back)
{
    cmd_back_ptr cmd;
    NEW(cmd);
    cmd->type = CMD_BACK;
    cmd->back = back;
    return (cmd_ptr) cmd;
}

cmd_ptr cmd_pipe_new(cmd_ptr left, cmd_ptr right)
{
    cmd_pipe_ptr cmd;
    NEW(cmd);
    cmd->type  = CMD_PIPE;
    cmd->left  = left;
    cmd->right = right;
    return (cmd_ptr) cmd;
}

cmd_ptr cmd_ored_new(cmd_ptr left, cmd_ptr right, int fd)
{
    cmd_ored_ptr cmd;
    NEW(cmd);
    cmd->type  = CMD_ORED;
    cmd->left  = left;
    cmd->right = right;
    cmd->fd    = fd;
    return (cmd_ptr) cmd;
}

cmd_ptr cmd_ired_new(cmd_ptr left, cmd_ptr right, int fd)
{
    cmd_ired_ptr cmd;
    NEW(cmd);
    cmd->type  = CMD_IRED;
    cmd->left  = left;
    cmd->right = right;
    cmd->fd    = fd;
    return (cmd_ptr) cmd;
}

void cmd_print(cmd_ptr cmd) {
    switch (cmd->type)
    {
        case CMD_ATOM:
        {
            cmd_atom_ptr t = (cmd_atom_ptr) cmd;
            node_ptr  node = t->node;
            if (node)
            {
                printf("\033[0;32m%s \033[0m", node->data);
                node = node->next;
            }
            while (node)
            {
                printf("\033[0;37m%s \033[0m", node->data);
                node = node->next;
            }
            break;
        }
        case CMD_LIST:
        {
            cmd_list_ptr t = (cmd_list_ptr) cmd;
            cmd_print(t->left);
            printf("\033[1;31m; \033[0m");
            cmd_print(t->right);
            break;
        }
        case CMD_BACK:
        {
            cmd_back_ptr t = (cmd_back_ptr) cmd;
            cmd_print(t->back);
            printf("\033[1;31m& \033[0m");
            break;
        }
        case CMD_PIPE:
        {
            cmd_pipe_ptr t = (cmd_pipe_ptr) cmd;
            cmd_print(t->left);
            printf("\033[1;31m| \033[0m");
            cmd_print(t->right);
            break;
        }
        case CMD_ORED:
        {
            cmd_ored_ptr t = (cmd_ored_ptr) cmd;
            cmd_print(t->left);
            printf("\033[1;31m> \033[0m");
            cmd_print(t->right);
            break;
        }
        case CMD_IRED:
        {
            cmd_ired_ptr t = (cmd_ired_ptr) cmd;
            cmd_print(t->left);
            printf("\033[1;31m< \033[0m");
            cmd_print(t->right);
            break;
        }
        default:
        {
            break;
        }
    }
}

void cmd_run(cmd_ptr cmd) {
    switch (cmd->type)
    {
        case CMD_ATOM:
        {
            char *args[10] = {0};
            int i = 0;
            node_ptr node = ((cmd_atom_ptr) cmd)->node;
            while (node)
            {
                args[i] = node->data;
                node    = node->next;
                i++;
            }
            char *root = "/bin/";
            char bin_path[50];
            strcpy(bin_path, root);
            strcat(bin_path, args[0]);
            if (execv(bin_path, args) == -1)
            {
                char *path = "/usr/bin/";
                char usr_bin_path[50];
                strcpy(usr_bin_path, path);
                strcat(usr_bin_path, args[0]);
                if (execv(usr_bin_path, args) == -1)
                {
                    fprintf(stderr,
                            "\033[1;31mCannot run command \033[0m"
                            "\033[0;32m%s\033[0m"
                            "\033[1;31m. Check your input.\033[0m"
                            "\n",
                            args[0]);
                }
            }
            break;
        }
        case CMD_LIST:
        {
            cmd_list_ptr t = (cmd_list_ptr) cmd;
            if (fork() == 0)
            {
                cmd_run(t->left);
            }
            wait(0);
            cmd_run(t->right);
            break;
        }
        case CMD_BACK:
        {
            cmd_back_ptr t = (cmd_back_ptr) cmd;
            if (fork() == 0)
            {
                cmd_run(t->back);
            }
            break;
        }
        case CMD_PIPE:
        {
            cmd_pipe_ptr t = (cmd_pipe_ptr) cmd;
            int p[2];
            pipe(p);
            if (fork() == 0)
            {
                close(p[0]);
                close(1);
                dup(p[1]);
                close(p[1]);
                cmd_run(t->left);
            }
            if (fork() == 0)
            {
                close(p[1]);
                close(0);
                dup(p[0]);
                close(p[0]);
                cmd_run(t->right);
            }
            close(p[0]);
            close(p[1]);
            wait(0);
            wait(0);
            break;
        }
        case CMD_ORED:
        {
            cmd_ored_ptr t = (cmd_ored_ptr) cmd;
            const char *ofile_path = ((cmd_atom_ptr) (t->right))->node->data;
            if (fork() == 0)
            {
                close(t->fd);
                open(ofile_path, O_CREAT | O_TRUNC | O_WRONLY);
                cmd_run(t->left);
            }
            wait(0);
            break;
        }
        case CMD_IRED:
        {
            cmd_ired_ptr t = (cmd_ired_ptr) cmd;
            const char *ifile_path = ((cmd_atom_ptr) (t->right))->node->data;
            if (fork() == 0)
            {
                close(t->fd);
                open(ifile_path, O_RDONLY);
                cmd_run(t->left);
            }
            wait(0);
            break;
        }
        default:
        {
            break;
        }
    }
    exit(EXIT_SUCCESS);
}
