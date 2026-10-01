
/*
 * AI Assistance Disclosure (CSC209)
 *
 * Portions of this file were developed with assistance from Microsoft Copilot
 * between March 14–16, 2026. Copilot provided structural suggestions, debugging
 * guidance, and help identifying edge cases. I reviewed, tested, and adapted all
 * AI‑generated suggestions myself, and the final code reflects my own
 * understanding and decisions.
 */

#include <string.h>
#include <stdlib.h>
#include <limits.h>
#include <unistd.h>
#include <pwd.h>
#include <stdio.h>
#include "paths.h"

char *normalize_path(const char *input) {
    char *copy = strdup(input);
    if (!copy) return NULL;

    char *stack[PATH_MAX];
    int top = 0;

    char *token = strtok(copy, "/");
    while (token) {
        size_t len = strlen(token);

        if (strcmp(token, ".") == 0) {
            // stay
        }
        else if (strcmp(token, "..") == 0) {
            if (top > 0) top--;
        }
        else if (len >= 2 && strspn(token, ".") == len) {
            int up = (int)len - 1;
            while (up-- > 0 && top > 0) top--;
        }
        else {
            stack[top++] = token;
        }

        token = strtok(NULL, "/");
    }

    char *result = malloc(PATH_MAX);
    if (!result) {
        free(copy);
        return NULL;
    }
    result[0] = '\0';

    if (top == 0) {
        // If input was absolute, root is fine.
        // If input was relative, keep it as "." instead of collapsing to "/".
        if (input[0] == '/') {
            strcpy(result, "/");
        } else {
            strcpy(result, ".");
        }
        free(copy);
        return result;
    }

    if (input[0] == '/') {
        // absolute path
        for (int i = 0; i < top; i++) {
            strncat(result, "/", PATH_MAX - strlen(result) - 1);
            strncat(result, stack[i], PATH_MAX - strlen(result) - 1);
        }
    } else {
        // relative path
        for (int i = 0; i < top; i++) {
            if (i > 0) {
                strncat(result, "/", PATH_MAX - strlen(result) - 1);
            }
            strncat(result, stack[i], PATH_MAX - strlen(result) - 1);
        }
    }


    free(copy);
    return result;
}

// Convert relative → absolute, then normalize
char *resolve_path(const char *input) {
    char cwd[PATH_MAX];

    // Absolute path
    if (input[0] == '/') {
        return normalize_path(input);
    }

    // Relative path
    if (!getcwd(cwd, sizeof(cwd))) return NULL;

    char temp[PATH_MAX];
    temp[0] = '\0';

    // Build "cwd/input" safely
    strncat(temp, cwd, PATH_MAX - strlen(temp) - 1);
    strncat(temp, "/", PATH_MAX - strlen(temp) - 1);
    strncat(temp, input, PATH_MAX - strlen(temp) - 1);

    return normalize_path(temp);
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

