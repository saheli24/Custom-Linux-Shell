#include <string.h>
#include "variables.h"
#include "io_helpers.h"
#include "commands.h"
#include <fcntl.h>


void expand_token(char *token, char *destination) {
    size_t out_idx = 0; // index into destination
    size_t in_idx = 0; // index into token
    size_t i;

    while (token[in_idx] != '\0' && out_idx < MAX_STR_LEN) {

        if (token[in_idx] == '$') {

            if (token[in_idx + 1] == '\0' ||
                token[in_idx + 1] == ' '  ||
                token[in_idx + 1] == '\t' ||
                token[in_idx + 1] == '\n') {

                in_idx++;
                continue;
                }

            if (token[in_idx + 1] == '$') {
                destination[out_idx++] = '$';
                in_idx += 2;
                continue;
            }

            in_idx++;

            // First character must be letter or underscore
            if (!((token[in_idx] >= 'A' && token[in_idx] <= 'Z') ||
                  (token[in_idx] >= 'a' && token[in_idx] <= 'z') ||
                  token[in_idx] == '_')) {

                // Not a valid variable start: treat '$' as literal
                destination[out_idx++] = '$';
                continue;
            }

            // Scan the rest of the variable name: letters, digits, underscore
            size_t start = in_idx;
            in_idx++;

            while ((token[in_idx] >= 'A' && token[in_idx] <= 'Z') ||
                   (token[in_idx] >= 'a' && token[in_idx] <= 'z') ||
                   (token[in_idx] >= '0' && token[in_idx] <= '9') ||
                   token[in_idx] == '_') {
                in_idx++;
            }

            size_t len = in_idx - start;

            char var_name[128] = {0};
            if (len > 127) {
                len = 127;
            }
            strncpy(var_name, token + start, len);
            var_name[len] = '\0';

            const char *val = get_variable(var_name);
            if (!val) {
                val = "";
            }

            for (i = 0; val[i] != '\0' && out_idx < MAX_STR_LEN; i++) {
                destination[out_idx++] = val[i];
            }

            continue;
        }

        // Normal character
        destination[out_idx++] = token[in_idx++];
    }

    destination[out_idx] = '\0';
}

/*
 * AI Assistance Disclosure (CSC209)
 *
 * Portions of this file were developed with assistance from Microsoft Copilot
 * between March 14–16, 2026. Copilot provided structural suggestions, debugging
 * guidance, and help identifying edge cases. I reviewed, tested, and adapted all
 * AI‑generated suggestions myself, and the final code reflects my own
 * understanding and decisions.
 */


int parse_redirection(char *exec_tokens[], redir_t *r) {
    r->infile = NULL;
    r->outfile = NULL;
    r->append = 0;

    int write_index = 0;

    for (int i = 0; exec_tokens[i] != NULL; i++) {
        if (strcmp(exec_tokens[i], "<") == 0) {
            r->infile = exec_tokens[i+1];
            i++;
        }
        else if (strcmp(exec_tokens[i], ">") == 0) {
            r->outfile = exec_tokens[i+1];
            r->append = 0;
            i++;
        }
        else if (strcmp(exec_tokens[i], ">>") == 0) {
            r->outfile = exec_tokens[i+1];
            r->append = 1;
            i++;
        }
        else {
            exec_tokens[write_index++] = exec_tokens[i];
        }
    }

    exec_tokens[write_index] = NULL;
    return 0;
}

/*
 * AI Assistance Disclosure (CSC209)
 *
 * Portions of this file were developed with assistance from Microsoft Copilot
 * between March 14–16, 2026. Copilot provided structural suggestions, debugging
 * guidance, and help identifying edge cases. I reviewed, tested, and adapted all
 * AI‑generated suggestions myself, and the final code reflects my own
 * understanding and decisions.
 */
