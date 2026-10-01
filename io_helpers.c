#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>

#include "io_helpers.h"


// ===== Output helpers =====

/* Prereq: str is a NULL terminated string
 */
void display_message(char *str) {
    write(STDOUT_FILENO, str, strnlen(str, MAX_STR_LEN));
}


/* Prereq: pre_str, str are NULL terminated string
 */
void display_error(char *pre_str, char *str) {
    write(STDERR_FILENO, pre_str, strnlen(pre_str, MAX_STR_LEN));
    write(STDERR_FILENO, str, strnlen(str, MAX_STR_LEN));
    write(STDERR_FILENO, "\n", 1);
}


/*
 * AI Assistance Disclosure (CSC209)
 *
 * Portions of this file were developed with assistance from Microsoft Copilot
 * and Anthropic Claude between March 27–29, 2026. These tools provided
 * structural suggestions, debugging guidance, and help identifying edge cases
 * during development. All AI‑generated ideas were reviewed, tested, and adapted
 * by me, and the final implementation reflects my own understanding, design
 * decisions, and verification against the CSC209 specifications.
 */



// ===== Input tokenizing =====

/* Prereq: in_ptr points to a character buffer of size > MAX_STR_LEN
 * Return: number of bytes read
 */
ssize_t get_input(char *in_ptr) {
    // NEW: allow server_poll() to run when no input is ready
    fd_set rfds;
    FD_ZERO(&rfds);
    FD_SET(STDIN_FILENO, &rfds);

    struct timeval tv;
    tv.tv_sec = 0;
    tv.tv_usec = 100000;   // 0.1 seconds

    int ready = select(STDIN_FILENO + 1, &rfds, NULL, NULL, &tv);

    if (ready == 0) {
        // No input yet → let main loop poll the server
        return -1;
    }
    if (ready < 0) {
        return -1;
    }
    int retval = read(STDIN_FILENO, in_ptr, MAX_STR_LEN+1); // Not a sanitizer issue since in_ptr is allocated as MAX_STR_LEN+1
    int read_len = retval;
    // Probably should exit if the retval is an error -- but for now, just returning.
    if (retval == -1) {
        read_len = 0;
    }
    if (read_len > MAX_STR_LEN) {
        read_len = 0;
        retval = -1;
        write(STDERR_FILENO, "ERROR: input line too long\n", strlen("ERROR: input line too long\n"));
        int junk = in_ptr[MAX_STR_LEN];
        while(junk != '\n' && (junk = getchar()) != EOF);
    }
    in_ptr[read_len] = '\0';
    return retval;
}

/* Prereq: in_ptr is a string, tokens is of size >= len(in_ptr)
 * Warning: in_ptr is modified
 * Return: number of tokens.
 */
size_t tokenize_input(char *in_ptr, char **tokens) {
    char *curr_ptr = in_ptr;
    size_t token_count = 0;

    while (*curr_ptr != '\0') {

        // skip leading whitespace
        while (*curr_ptr == ' ' || *curr_ptr == '\t' || *curr_ptr == '\n') {
            curr_ptr++;
        }

        if (*curr_ptr == '\0') {
            break;
        }

        // start of a token
        tokens[token_count++] = curr_ptr;

        // move until next whitespace or end
        while (*curr_ptr != '\0' &&
               *curr_ptr != ' ' &&
               *curr_ptr != '\t' &&
               *curr_ptr != '\n') {
            curr_ptr++;
        }

        // null‑terminate the token
        if (*curr_ptr != '\0') {
            *curr_ptr = '\0';
            curr_ptr++;
        }
    }

    tokens[token_count] = NULL;
    return token_count;
}

/*
 * AI Assistance Disclosure (CSC209)
 *
 * Portions of this file were developed with assistance from Microsoft Copilot
 * and Anthropic Claude between March 27–29, 2026. These tools provided
 * structural suggestions, debugging guidance, and help identifying edge cases
 * during development. All AI‑generated ideas were reviewed, tested, and adapted
 * by me, and the final implementation reflects my own understanding, design
 * decisions, and verification against the CSC209 specifications.
 */
