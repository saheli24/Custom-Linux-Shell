
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







#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>


#include "pipeline.h"
#include "builtins.h"
#include "commands.h"
#include "io_helpers.h"
#include <fcntl.h>
#include "jobs.h"


// Compatibility wrapper for public pipeline tests
void execute_pipeline(char *segments[], int count) {
   // Public tests do NOT use background pipelines
   execute_pipeline_full(segments, count, 0, NULL);
}


// Trim leading/trailing whitespace
static char *trim(char *s) {
   while (*s == ' ' || *s == '\t') s++;
   if (*s == '\0') return s;


   char *end = s + strlen(s) - 1;
   while (end > s && (*end == ' ' || *end == '\t')) {
       *end = '\0';
       end--;
   }
   return s;
}


// Parse pipeline into segments
int parse_pipeline(char *line, char *segments[], int max_segments) {
   int count = 0;
   char *saveptr = NULL;
   char *part = strtok_r(line, "|", &saveptr);


   while (part != NULL) {
       if (count >= max_segments) {
           display_error("ERROR: Invalid pipeline syntax", "");
           return -1;
       }


       char *clean = trim(part);
       if (clean[0] == '\0') {
           display_error("ERROR: Invalid pipeline syntax", "");
           return -1;
       }


       segments[count++] = clean;
       part = strtok_r(NULL, "|", &saveptr);
   }


   return count;
}


// Execute a single command in a child
static void run_single_command(char *segment) {
   char *token_arr[MAX_STR_LEN] = {NULL};
   char expanded_tokens[MAX_STR_LEN][MAX_STR_LEN + 1];
   char *exec_tokens[MAX_STR_LEN];


   size_t token_count = tokenize_input(segment, token_arr);
   if (token_count == 0) _exit(0);


   for (size_t i = 0; i < token_count; i++) {
       expand_token(token_arr[i], expanded_tokens[i]);
       exec_tokens[i] = expanded_tokens[i];
   }
   exec_tokens[token_count] = NULL;


   // --- parse redirection ---
   redir_t r;
   parse_redirection(exec_tokens, &r);


   // --- apply redirection ---
   if (r.infile) {
       int fd = open(r.infile, O_RDONLY);
       if (fd < 0) _exit(1);
       dup2(fd, STDIN_FILENO);
       close(fd);
   }


   if (r.outfile) {
       int flags = O_WRONLY | O_CREAT;
       flags |= r.append ? O_APPEND : O_TRUNC;
       int fd = open(r.outfile, flags, 0644);
       if (fd < 0) _exit(1);
       dup2(fd, STDOUT_FILENO);
       close(fd);
   }


   // --- builtin support ---
   bn_ptr builtin_fn = check_builtin(exec_tokens[0]);
   if (builtin_fn) {
       builtin_fn(exec_tokens);
       _exit(0);
   }


   // --- external command ---
   execvp(exec_tokens[0], exec_tokens);
   display_error("ERROR: Unknown command: ", exec_tokens[0]);
   _exit(1);
}


// Execute full pipeline
void execute_pipeline_full(char *segments[], int count, int background, const char *job_cmd){
   if (count <= 0) return;


   int pipes[count - 1][2];
   pid_t pids[count];


   // Create all pipes
   for (int i = 0; i < count - 1; i++) {
       if (pipe(pipes[i]) < 0) {
           display_error("ERROR: pipe failed", "");
           return;
       }
   }


   // Fork children
   for (int i = 0; i < count; i++) {
       pid_t pid = fork();


       if (pid < 0) {
           display_error("ERROR: Failed to fork", "");
           return;
       }


       if (pid == 0) {
           // CHILD: its own job table
           init_jobs();


           // If not first command, connect stdin to previous pipe
           if (i > 0) {
               dup2(pipes[i - 1][0], STDIN_FILENO);
           }


           // If not last command, connect stdout to next pipe
           if (i < count - 1) {
               dup2(pipes[i][1], STDOUT_FILENO);
           }


           // Background pipelines must not read from terminal
           if (background) {
               int devnull = open("/dev/null", O_RDONLY);
               if (devnull >= 0) {
                   dup2(devnull, STDIN_FILENO);
                   close(devnull);
               }
           }


           // Close ALL pipe ends in child
           for (int j = 0; j < count - 1; j++) {
               close(pipes[j][0]);
               close(pipes[j][1]);
           }


           run_single_command(segments[i]);
       } else {
           // PARENT
           pids[i] = pid;
       }
   }


   // PARENT: close ALL pipe ends
   for (int i = 0; i < count - 1; i++) {
       close(pipes[i][0]);
       close(pipes[i][1]);
   }


   if (background) {
       // Background job: add each process to job table
       int first_job_id = -1;
       for (int i = 0; i < count; i++) {
           int jid = add_job(pids[i], job_cmd);
           if (i == 0) {
               first_job_id = jid;
           }
       }


       if (first_job_id != -1) {
           char buf[128];
           snprintf(buf, sizeof(buf), "[%d] %d\n", first_job_id, pids[0]);
           display_message(buf);
       }


       // Do NOT wait; let check_jobs() reap and print Done messages
       check_jobs();
   } else {
       // Foreground pipeline: wait for all children
       for (int i = 0; i < count; i++) {
           waitpid(pids[i], NULL, 0);
       }
       check_jobs();
   }
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
