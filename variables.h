#ifndef __VARIABLES_H__
#define __VARIABLES_H__
#include <stddef.h>
#include <stdint.h>

typedef struct {
    char *name;
    char *value;
} Variable;

int set_variable(char *name, char *value);
char *get_variable(char *name);
void free_all_variables(void);

#endif

