%{
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "ast.h"
#include "alloc.h"

int  yylex();
void yyerror(const char* msg);

cmd_ptr cmd;
extern cmd_ptr root;
%}

%union {
    int       num;
    char      *id;
    cmd_ptr   cmd;
    node_ptr node;
}

%token<id> T_ARG

%type<cmd>  command basic_command
%type<num>  line
%type<node> args

%left '|' ';'
%left '>'
%left '<'
%left '&'

%%

line            : 	command '\n'		{ root = $1; $$ = 0; return 0; }
				|  	'\n'       			{ root = 0; $$ = 0; return 0;  }
                ;
                
command	        :  	basic_command			{ $$ = $1; }
				|  	command ';' command  	{ $$ = cmd_list_new($1, $3); }
				|  	command '&'				{ $$ = cmd_back_new($1); }
				|  	command '|' command 	{ $$ = cmd_pipe_new($1, $3); }
				|  	command '>' command		{ $$ = cmd_ored_new($1, $3, 1); }
				|  	command '<' command		{ $$ = cmd_ired_new($1, $3, 0); }
				;
		
basic_command 	:  	T_ARG args				{ node_ptr t = make_node($1, $2); $$ = cmd_atom_new(t); };
		
args			:  	T_ARG args	    		{ $$ = make_node($1, $2); }
				|                           { $$ = (node_ptr) 0; }
				;

%%

void yyerror(const char *msg) {
    printf("%s\n",msg);
    exit(1);
}
