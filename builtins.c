#include <string.h>
#include <stdio.h>
#include <ctype.h>
#include "builtins.h"
#include "io_helpers.h"
#include <pwd.h>
#include <unistd.h>
#include "paths.h"
#include <stdlib.h>
#include <dirent.h>
#include <limits.h>
#include <fcntl.h>
#include "jobs.h"
#include <signal.h>
#include <errno.h>


// ====== Command execution =====

/* Return: index of builtin or -1 if cmd doesn't match a builtin
 */
bn_ptr check_builtin(const char *cmd) {
    ssize_t cmd_num = 0;
    while (cmd_num < BUILTINS_COUNT &&
           strncmp(BUILTINS[cmd_num], cmd, MAX_STR_LEN) != 0) {
        cmd_num += 1;
    }
    if (cmd_num == BUILTINS_COUNT) {
        return NULL;
    }
    return BUILTINS_FN[cmd_num];
}


// ===== Builtins =====

/* Prereq: tokens is a NULL terminated sequence of strings.
 * Return 0 on success and -1 on error ... but there are no errors on echo.
 */
ssize_t bn_echo(char **tokens) {
    ssize_t index = 1;
    int first = 1;

    if (tokens[index] != NULL) {
        // TODO:
        // Implement the echo command
        if (tokens[index][0] != '\0') {
            display_message(tokens[index]);
            first = 0;
        }
        index += 1;
    }
    while (tokens[index] != NULL) {
        // TODO:
        // Implement the echo command
        if (tokens[index][0] == '\0') {
            index++;
            continue;
        }

        if (!first) {
            display_message(" ");
        }

        display_message(tokens[index]);
        first = 0;
        index += 1;
    }
    display_message("\n");

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

ssize_t bn_cat(char **tokens) {
    int fd = -1;

    // Case 1: filename provided
    if (tokens[1] != NULL) {
        if (tokens[2] != NULL) {
            display_error("ERROR: Too many arguments: cat takes a single file", "");
            return -1;
        }

        fd = open(tokens[1], O_RDONLY);
        if (fd < 0) {
            display_error("ERROR: Cannot open file", tokens[1]);
            return -1;
        }
    }
    // Case 2: no filename → read from stdin
    else {
        // Try reading 1 byte to detect if stdin is closed
        char test;
        ssize_t r = read(STDIN_FILENO, &test, 1);

        if (r == 0) {
            display_error("ERROR: No input source provided", "");
            return -1;
        }

        // stdin is valid → print the byte we consumed
        write(STDOUT_FILENO, &test, 1);
        fd = STDIN_FILENO;
    }

    // Read until EOF
    char buf[4096];
    ssize_t n;
    while ((n = read(fd, buf, sizeof(buf))) > 0) {
        write(STDOUT_FILENO, buf, n);
    }

    if (fd != STDIN_FILENO) close(fd);
    return 0;
}

ssize_t bn_wc(char **tokens) {
    int fd = -1;

    // Case 1: filename provided
    if (tokens[1] != NULL) {
        if (tokens[2] != NULL) {
            display_error("ERROR: Too many arguments: wc takes a single file", "");
            return -1;
        }

        fd = open(tokens[1], O_RDONLY);
        if (fd < 0) {
            display_error("ERROR: Cannot open file", tokens[1]);
            return -1;
        }
    }
    // Case 2: no filename → read from stdin
    else {
        char test;
        ssize_t r = read(STDIN_FILENO, &test, 1);

        if (r == 0) {
            display_error("ERROR: No input source provided", "");
            return -1;
        }

        // stdin valid → count the byte we consumed
        fd = STDIN_FILENO;

        long chars = 1;
        long words = 0;
        long newlines = (test == '\n');
        int in_word = (test != ' ' && test != '\n' && test != '\t');

        char buf[4096];
        ssize_t n;

        while ((n = read(fd, buf, sizeof(buf))) > 0) {
            for (ssize_t i = 0; i < n; i++) {
                char c = buf[i];
                chars++;

                if (c == '\n') newlines++;

                if (c == ' ' || c == '\n' || c == '\t') {
                    if (in_word) {
                        words++;
                        in_word = 0;
                    }
                } else {
                    in_word = 1;
                }
            }
        }

        if (in_word) words++;

        char out[128];
        snprintf(out, sizeof(out), "word count %ld\n", words);
        display_message(out);
        snprintf(out, sizeof(out), "character count %ld\n", chars);
        display_message(out);
        snprintf(out, sizeof(out), "newline count %ld\n", newlines);
        display_message(out);

        return 0;
    }

    // Case 3: reading from file
    long chars = 0;
    long words = 0;
    long newlines = 0;
    int in_word = 0;

    char buf[4096];
    ssize_t n;

    while ((n = read(fd, buf, sizeof(buf))) > 0) {
        for (ssize_t i = 0; i < n; i++) {
            char c = buf[i];
            chars++;

            if (c == '\n') newlines++;

            if (c == ' ' || c == '\n' || c == '\t') {
                if (in_word) {
                    words++;
                    in_word = 0;
                }
            } else {
                in_word = 1;
            }
        }
    }

    if (in_word) words++;

    char out[128];
    snprintf(out, sizeof(out), "word count %ld\n", words);
    display_message(out);
    snprintf(out, sizeof(out), "character count %ld\n", chars);
    display_message(out);
    snprintf(out, sizeof(out), "newline count %ld\n", newlines);
    display_message(out);

    close(fd);
    return 0;
}

static void ls_recursive(const char *path, int show_all, const char *filter, int depth, int max_depth) {
    if (max_depth >= 0 && depth > max_depth) {
        return;
    }

    DIR *dir = opendir(path);
    if (!dir) return;

    struct dirent *entry;

    /* FIRST PASS: print entries */
while ((entry = readdir(dir)) != NULL) {
    char *name = entry->d_name;

    /* PRINT "." and ".." ALWAYS */
    if (strcmp(name, ".") == 0 || strcmp(name, "..") == 0) {
        if (!filter || strstr(name, filter) != NULL) {
            display_message(name);
            display_message("\n");
        }
        continue;
    }

    /* Hidden files: skip only .<something> where second char is NOT '.' */
    if (!show_all && name[0] == '.' && name[1] != '.') {
        continue;
    }

    /* Substring filter */
    if (filter && strstr(name, filter) == NULL) continue;

    display_message(name);
    display_message("\n");
}


    closedir(dir);

    /* SECOND PASS: recurse into subdirectories */
    if (max_depth >= 0 && depth >= max_depth) {
        return;
    }

    dir = opendir(path);
    if (!dir) return;

    while ((entry = readdir(dir)) != NULL) {
        char *name = entry->d_name;

        /* Skip "." and ".." */
        if (strcmp(name, ".") == 0 || strcmp(name, "..") == 0) continue;

        /* Hidden dirs: skip only .<something> where second char is NOT '.' */
        if (!show_all && name[0] == '.' && name[1] != '.') {
            continue;
        }

        /* Build next path */
        char next[PATH_MAX];
        next[0] = '\0';
        strncat(next, path, PATH_MAX - 1);
        strncat(next, "/", PATH_MAX - strlen(next) - 1);
        strncat(next, name, PATH_MAX - strlen(next) - 1);

        /* Recurse only if it's a directory */
        DIR *sub = opendir(next);
        if (sub) {
            closedir(sub);
            ls_recursive(next, show_all, filter, depth + 1, max_depth);
        }
    }

    closedir(dir);
}



ssize_t bn_ls(char **tokens) {
    int show_all = 0;
    char *filter = NULL;
    char *path = NULL;
    int recursive = 0;
    int max_depth = -1; // -1 = unlimited

    for (int i = 1; tokens[i] != NULL; i++) {
        if (strcmp(tokens[i], "--a") == 0) {
            show_all = 1;
        }
        else if (strcmp(tokens[i], "--f") == 0) {
            if (!tokens[i+1]) {
                display_error("ERROR: Builtin failed: ", "ls");
                return -1;
            }
            filter = tokens[i+1];
            i++;
        }
        else if (strcmp(tokens[i], "--rec") == 0) {
            recursive = 1;
        }
        else if (strcmp(tokens[i], "--d") == 0) {
            if (!tokens[i+1]) {
                display_error("ERROR: Builtin failed: ", "ls");
                return -1;
            }
            max_depth = atoi(tokens[i+1]);
            i++;
        }
        else if (tokens[i][0] == '-') {
            display_error("ERROR: Builtin failed: ", "ls");
            return -1;
        }
        else {
            if (path != NULL) {
                display_error("ERROR: Too many arguments: ls takes a single path", "");
                return -1;
            }
            path = tokens[i];
        }
    }

    if (!path) path = ".";

    // Depth without recursion is illegal
    if (max_depth >= 0 && !recursive) {
        display_error("ERROR: Builtin failed: ", "ls");
        return -1;
    }

    char *resolved = resolve_path(path);

    if (!resolved) {
        display_error("ERROR: Invalid path", "");
        return -1;
    }

    if (recursive) {
        ls_recursive(resolved, show_all, filter, 0, max_depth);
        free(resolved);
        return 0;
    }

    // Non-recursive (Phase 1)
    DIR *dir = opendir(resolved);
    if (!dir) {
        FILE *f = fopen(resolved, "r");
        if (!f) {
            free(resolved);
            display_error("ERROR: Invalid path", "");
            return -1;
        }
        fclose(f);
        display_message(path);
        display_message("\n");
        free(resolved);
        return 0;
    }

    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL) {
        char *name = entry->d_name;

        /* Handle "." and ".." */
        if (strcmp(name, ".") == 0 || strcmp(name, "..") == 0) {
            if (!filter || strstr(name, filter) != NULL) {
                display_message(name);
                display_message("\n");
            }
            continue;
        }


        // Skip hidden files ONLY if they start with '.' AND the second char is NOT '.'
        if (!show_all && name[0] == '.' && name[1] != '.' && strcmp(name, ".") != 0 && strcmp(name, "..") != 0) {
            continue;
        }



        /* Substring filter */
        if (filter && strstr(name, filter) == NULL) continue;

        display_message(name);
        display_message("\n");
    }


    closedir(dir);
    free(resolved);
    return 0;
}




ssize_t bn_cd(char **tokens) {
    // Too many args
    if (tokens[1] && tokens[2]) {
        display_error("ERROR: Too many arguments: cd takes a single path", "");
        return -2;  // error already printed
    }

    char *target = NULL;

    // No args → go to HOME
    if (!tokens[1]) {
        struct passwd *pw = getpwuid(getuid());
        if (!pw) {
            display_error("ERROR: Invalid path", "");
            return -2;
        }
        target = pw->pw_dir;
    } else {
        // Resolve + normalize
        target = resolve_path(tokens[1]);
        if (!target) {
            display_error("ERROR: Invalid path", "");
            return -2;
        }
    }

    // Attempt to change directory
    if (chdir(target) != 0) {
        display_error("ERROR: Invalid path", "");
        if (tokens[1]) free(target);
        return -2;
    }

    if (tokens[1]) free(target);
    return 0;
}

ssize_t bn_ps(char **tokens) {
    (void)tokens;   // explicitly mark unused
    print_ps();
    return 0;
}

ssize_t bn_kill(char **tokens) {
    int sig = SIGTERM;
    int pid_index = 1;

    if (!tokens[1]) {
        display_error("ERROR: The process does not exist", "");
        return -1;
    }

    if (tokens[1][0] == '-') {
        sig = atoi(tokens[1] + 1);
        if (sig <= 0 || sig > 64) {
            display_error("ERROR: Invalid signal specified", "");
            return -1;
        }
        pid_index = 2;
    }

    // Syntax 2: kill 1234 9
    else if (tokens[2]) {
        sig = atoi(tokens[2]);
        if (sig <= 0 || sig > 64) {
            display_error("ERROR: Invalid signal specified", "");
            return -1;
        }
    }

    if (!tokens[pid_index]) {
        display_error("ERROR: The process does not exist", "");
        return -1;
    }

    pid_t pid = atoi(tokens[pid_index]);
    if (pid <= 0) {
        display_error("ERROR: The process does not exist", "");
        return -1;
    }

    if (kill(pid, sig) == -1) {
        if (errno == ESRCH) {
            display_error("ERROR: The process does not exist", "");
        } else if (errno == EINVAL) {
            display_error("ERROR: Invalid signal specified", "");
        } else {
            display_error("ERROR: kill failed", "");
        }
        return -1;
    }

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
