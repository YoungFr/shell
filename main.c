#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>
#include <string.h>

#include "ast.h"

cmd_ptr root = 0;
extern int yyparse();

int main()
{
    while (1)
    {
        printf(">>> ");
        yyparse(); // analyzes the input from the user
        if (root == NULL)
        {
            continue;
        }
        printf("\033[1;36mcommand(s): \033[0m");
        cmd_print(root);
        printf("\n");
        if (root->type == CMD_ATOM)
        {
            cmd_atom_ptr acmd = (cmd_atom_ptr) root;
            node_ptr     node = acmd->node;
            if (!strcmp(node->data, "exit"))    // input "exit" cmd
            {
                break;
            }
            else if (!strcmp(node->data, "cd")) // input "cd" cmd
            {
                node = node->next;
                if (node)
                {
                    char cd_path[20];
                    strcpy(cd_path, node->data);
                    if (chdir(cd_path) < 0)
                    {
                        fprintf(stderr,
                                "\033[1;31mCannot cd \033[0m\033[0;37m%s\033[0m\033[1;31m.\033[0m\n",
                                cd_path);
                    }
                }
            }
            else // other cmds
            {
                if (fork() == 0)
                {
                    cmd_run(root);
                }
                wait(0);
            }
        }
        else // compound commands
        {
            if (fork() == 0)
            {
                cmd_run(root);
            }
            wait(0);
        }
    }
    return 0;
}
