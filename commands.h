#ifndef __COMMANDS_H__
#define __COMMANDS_H__
typedef struct {
    char *infile;
    char *outfile;
    int append;   // 0 = truncate, 1 = append
} redir_t;


void expand_token(char *token, char *destination);
int parse_redirection(char *exec_tokens[], redir_t *r);

#endif
