#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "variables.h"

static Variable *vars = NULL;
static size_t num_vars = 0; // num of vars stored
static size_t cap_vars = 0; //curr capacity of vars array

//search variable by name and return index
static ssize_t find_variable(char *name) {
    size_t i;
    for (i = 0; i < num_vars; i++) {
        if (strcmp(vars[i].name, name) == 0) {
            return i;
        }
    }
    return -1;
}

int set_variable(char *name, char *value) {
    ssize_t idx;
    if (!name) {
        return -1;
    }

    idx = find_variable(name);

    if (idx >= 0) {
        free(vars[idx].value); //
        const char *new_value;

        if (value == NULL) { // if value is null then set value to empty str
            new_value = "";
        } else {
            new_value = value;
        }

        vars[idx].value = strdup(new_value); //

        if (vars[idx].value == NULL) {
            return -1;
        }
        return 0;
    }

    if (num_vars == cap_vars) {
        size_t new_cap;

        if (cap_vars == 0) {
            new_cap = 4; //start capacity of 4
        } else {
            new_cap = cap_vars * 2; // double each time once full
        }
        Variable *new_vars = realloc(vars, new_cap * sizeof(Variable));
        if (!new_vars) {
            return -1;
        }
        vars = new_vars;
        cap_vars = new_cap;
    }

    vars[num_vars].name = strdup(name);

    if (value == NULL) {
        vars[num_vars].value = strdup("");
    } else {
        vars[num_vars].value = strdup(value);
    }

    if (vars[num_vars].name == NULL || vars[num_vars].value == NULL) {

        if (vars[num_vars].name != NULL) {
            free(vars[num_vars].name);
        }

        if (vars[num_vars].value != NULL) {
            free(vars[num_vars].value);
        }

        return -1;
    }

    num_vars++;
    return 0;
}

char *get_variable(char *name) {
    ssize_t idx = find_variable(name);
    if (idx >= 0) {
        return vars[idx].value;
    }
    return NULL;
}

//free the variables and the array once done
void free_all_variables(void) {
    size_t i;
    for (i = 0; i < num_vars; i++) {
        free(vars[i].name);
        free(vars[i].value);
    }
    free(vars);
    vars = NULL;
    num_vars = 0;
    cap_vars = 0;
}

