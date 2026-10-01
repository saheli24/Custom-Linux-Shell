//mysh.c
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
#include <sys/types.h>
#include <unistd.h>
#include "builtins.h"
#include "io_helpers.h"
#include "variables.h"
#include "commands.h"
#include <sys/wait.h>
#include <errno.h>
#include "pipeline.h"
#include "jobs.h"
#include <fcntl.h>
#include <signal.h>
#include "server.h"
#include "client.h"

int send_one_shot(int port, const char *host, const char *msg);



static void sigint_handler(int sig) {
   (void)sig;
   write(STDOUT_FILENO, "\n", 1);
   display_message("mysh$ ");
}


static void sigchld_handler(int sig) {
   (void)sig;
   check_jobs();
}






// You can remove __attribute__((unused)) once argc and argv are used.
int main(__attribute__((unused)) int argc,
        __attribute__((unused)) char* argv[]) {
   char *prompt = "mysh$ "; // TODO Step 1, Uncomment this.
   init_jobs();   // Phase 3: initialize job table
    init_server();
   signal(SIGINT, sigint_handler);
   signal(SIGCHLD, sigchld_handler);


   char input_buf[MAX_STR_LEN + 1];
   char *token_arr[MAX_STR_LEN] = {NULL};


   while (1) {
       // Prompt and input tokenization
       // TODO Step 2:
       // Display the prompt via the display_message function.
       server_poll();
       display_message(prompt);
       check_jobs();   // Phase 3: print DONE messages

       int ret;
       while ((ret = get_input(input_buf)) == -1) {
           server_poll();
           check_jobs();
       }

       server_poll();
       check_jobs();
       if (ret == 0) {
           break;
       }
       if (ret == -1) {
           continue;
       }



       char *line = strtok(input_buf, "\n");


       while (line != NULL) {
           int skip_line = 0;
           // Phase 2: handle pipelines first
           if (strchr(line, '|') != NULL) {
               // Detect background '&' at end of line
               int background = 0;
               char *end = line + strlen(line) - 1;


               // Skip trailing whitespace
               while (end >= line && (*end == ' ' || *end == '\t')) {
                   *end = '\0';
                   end--;
               }


               // If last non-space char is '&', mark background and strip it
               if (end >= line && *end == '&') {
                   background = 1;
                   *end = '\0';


                   // Strip any whitespace before '&'
                   end--;
                   while (end >= line && (*end == ' ' || *end == '\t')) {
                       *end = '\0';
                       end--;
                   }
               }


               char *segments[32];
               int count = parse_pipeline(line, segments, 32);
               if (count > 0) {
                   execute_pipeline_full(segments, count, background, line);
                   check_jobs();
               }
               line = strtok(NULL, "\n");
               continue;
           }


           if (*line != '\0') {


               size_t token_count = tokenize_input(line, token_arr);


               if (token_count > 0) {
                   if (token_count == 1 && strcmp(token_arr[0], "exit") == 0) {
                       kill_all_jobs();
                       free_all_variables();
                       return 0;
                   }


                   char *eq_pos = strchr(token_arr[0], '=');
                   if (token_count == 1 && eq_pos && eq_pos != token_arr[0]) {


                       size_t name_len = eq_pos - token_arr[0];
                       char name[128];
                       if (name_len > 127) {
                           name_len = 127;
                       }
                       strncpy(name, token_arr[0], name_len);
                       name[name_len] = '\0';


                       // EXPAND the RHS before storing
                       char expanded_value[MAX_STR_LEN + 1];
                       expand_token(eq_pos + 1, expanded_value);


                       // Case 1: RHS came from expansion AND starts with '=' → invalid
                       if (strchr(eq_pos + 1, '$') != NULL && expanded_value[0] == '=') {
                           display_error("ERROR: Invalid assignment: ", token_arr[0]);
                           skip_line = 1;
                           break;
                       }


                       // Case 2: RHS is literal AND is exactly "=" or "=="
                       if (strchr(eq_pos + 1, '$') == NULL &&
                           (strcmp(expanded_value, "=") == 0 || strcmp(expanded_value, "==") == 0)) {
                           display_error("ERROR: Invalid assignment: ", token_arr[0]);
                           skip_line = 1;
                           break;
                           }




                           // Validate variable name: must start with letter or underscore
                           if (!((name[0] >= 'A' && name[0] <= 'Z') ||
                                 (name[0] >= 'a' && name[0] <= 'z') ||
                                 name[0] == '_')) {
                               display_error("ERROR: Invalid variable name", name);
                               skip_line = 1;
                               break;
                                 }
                       // Reject variable names starting with '$'
                       if (name[0] == '$') {
                           display_error("ERROR: Invalid variable name", name);
                           skip_line = 1;
                           break;
                       }




                           // Remaining chars must be letters, digits, underscore
                           for (size_t k = 1; name[k] != '\0'; k++) {
                               if (!((name[k] >= 'A' && name[k] <= 'Z') ||
                                     (name[k] >= 'a' && name[k] <= 'z') ||
                                     (name[k] >= '0' && name[k] <= '9') ||
                                     name[k] == '_')) {
                                   display_error("ERROR: Invalid variable name", name);
                                   skip_line = 1;
                                   break;
                                     }
                           }




                       if (set_variable(name, expanded_value) == -1) {
                           display_error("ERROR: Failed to set variable", name);
                       }


                   } else {


                       char expanded_tokens[MAX_STR_LEN][MAX_STR_LEN + 1];
                       char *exec_tokens[MAX_STR_LEN];
                       size_t i;


                       for (i = 0; i < MAX_STR_LEN; i++) {
                           expanded_tokens[i][0] = '\0';
                       }


                       for (i = 0; i < token_count; i++) {
                           expand_token(token_arr[i], expanded_tokens[i]);
                           exec_tokens[i] = expanded_tokens[i];
                       }
                       exec_tokens[token_count] = NULL;


                       // Phase 3: detect background job BEFORE anything else
                       int background = 0;


                       if (token_count > 0 && strcmp(exec_tokens[token_count - 1], "&") == 0) {
                           background = 1;
                           exec_tokens[token_count - 1] = NULL;   // remove &
                           token_count--;
                       }

                       // -------------------- start-server --------------------
                       if (strcmp(exec_tokens[0], "start-server") == 0) {
                           if (token_count < 2) {
                               display_error("ERROR: No port provided", "");
                           } else {
                               start_server(atoi(exec_tokens[1]));
                           }
                           goto next_command;
                       }

                       // -------------------- close-server --------------------
                       if (strcmp(exec_tokens[0], "close-server") == 0) {
                           close_server();
                           goto next_command;
                       }

                       // -------------------- send --------------------
                       if (strcmp(exec_tokens[0], "send") == 0) {
                           if (token_count < 2) {
                               display_error("ERROR: No port provided", "send");
                               goto next_command;
                           }
                           if (token_count < 3) {
                               display_error("ERROR: No hostname provided", "send");
                               goto next_command;
                           }

                           int port = atoi(exec_tokens[1]);
                           const char *host = exec_tokens[2];

                           char msg[MAX_STR_LEN + 1] = "";
                           msg[0] = '\0';
                           for (size_t k = 3; k < token_count; k++) {
                               if (msg[0] != '\0') {
                                   strncat(msg, " ", MAX_STR_LEN - strlen(msg) - 1);
                               }
                               strncat(msg, exec_tokens[k], MAX_STR_LEN - strlen(msg) - 1);
                           }


                           for (size_t k = 3; k < token_count; k++) {
                               if (strchr(token_arr[k], '$') != NULL && strlen(exec_tokens[k]) == 128) {
                                   display_error("ERROR: Message too long", "");
                                   goto next_command;
                               }
                           }

                           if (strlen(msg) > 128) {
                               display_error("ERROR: Message too long", "");
                               goto next_command;
                           }

                           send_one_shot(port, host, msg);
                           server_poll();      // <-- process server messages immediately
                           check_jobs();
                           goto next_command;
                       }

                       // -------------------- start-client --------------------
                       if (strcmp(exec_tokens[0], "start-client") == 0) {
                           if (token_count < 2) {
                               display_error("ERROR: No port provided", "start-client");
                               goto next_command;
                           }
                           if (token_count < 3) {
                               display_error("ERROR: No hostname provided", "start-client");
                               goto next_command;
                           }

                           int port = atoi(exec_tokens[1]);
                           const char *host = exec_tokens[2];

                           // Create pipe: parent expands input, child reads from pipe
                           int pipefd[2];
                           if (pipe(pipefd) < 0) {
                               display_error("ERROR: pipe failed", "");
                               goto next_command;
                           }

                           pid_t client_pid = start_client(port, host, pipefd[0]);
                           close(pipefd[0]);  // parent doesn't need read end

                           if (client_pid > 0) {
                               signal(SIGCHLD, SIG_DFL);

                               // Parent reads stdin, expands, checks length, writes to child
                               char raw[MAX_STR_LEN + 1];
                               while (1) {
                                   int r;
                                   while ((r = get_input(raw)) == -1) {
                                       server_poll();
                                   }
                                   if (r == 0) break;  // EOF

                                   if (raw[0] == '\0') break;  // null byte

                                   // Strip newline
                                   char *nl = strchr(raw, '\n');
                                   if (nl) *nl = '\0';

                                   // Expand variables
                                   char expanded[MAX_STR_LEN + 1];
                                   expand_token(raw, expanded);

                                   // Check length after expansion
                                   if (strlen(expanded) >= 128) {
                                       display_error("ERROR: Message too long", "");
                                       continue;
                                   }

                                   // Write expanded line to child
                                   write(pipefd[1], expanded, strlen(expanded));
                                   write(pipefd[1], "\n", 1);
                               }

                               close(pipefd[1]);
                               int status;
                               waitpid(client_pid, &status, 0);
                               signal(SIGCHLD, sigchld_handler);
                           } else {
                               close(pipefd[1]);
                           }
                           goto next_command;
                       }

                       bn_ptr builtin_fn = check_builtin(exec_tokens[0]);
                       if (builtin_fn) {


                           if (!background) {


                               for (int k = 1; exec_tokens[k]; k++) {
                                   if (strcmp(exec_tokens[k], "<") == 0 ||
                                       strcmp(exec_tokens[k], ">") == 0 ||
                                       strcmp(exec_tokens[k], ">>") == 0) {
                                       display_error("ERROR: Redirection not allowed for builtin", exec_tokens[0]);
                                       skip_line = 1;
                                       break;
                                       }
                               }


                               // Builtins that MUST run in the shell
                               if (strcmp(exec_tokens[0], "cd") == 0 ||
                                   strcmp(exec_tokens[0], "exit") == 0 ||
                                   strcmp(exec_tokens[0], "ps") == 0) {


                                   if (builtin_fn(exec_tokens) == -1) {
                                       display_error("ERROR: Builtin failed: ", exec_tokens[0]);
                                   }


                                   } else {
                                       // Foreground builtin that must be interruptible → fork
                                       pid_t pid = fork();
                                       if (pid < 0) {
                                           display_error("ERROR: Failed to fork", "");
                                       } else if (pid == 0) {
                                           // Child: this process has its own job table
                                           init_jobs();


                                           // Foreground builtins die on Ctrl+C
                                           signal(SIGINT, SIG_DFL);
                                           builtin_fn(exec_tokens);
                                           _exit(0);
                                       } else {
                                           int status;
                                           waitpid(pid, &status, 0);
                                           check_jobs();
                                       }
                                   }


                           } else {
                               // Background builtin → must fork
                               pid_t pid = fork();
                               if (pid < 0) {
                                   display_error("ERROR: Failed to fork", "");
                               } else if (pid == 0) {
                                   // Child: its own job table
                                   init_jobs();


                                   // Background builtins ignore Ctrl+C
                                   signal(SIGINT, SIG_IGN);


                                   int devnull = open("/dev/null", O_RDONLY);
                                   dup2(devnull, STDIN_FILENO);
                                   close(devnull);


                                   builtin_fn(exec_tokens);
                                   _exit(0);


                               } else {
                                   // Parent: add job + print job id
                                   char cmd_str[MAX_CMD_LEN] = "";
                                   for (size_t k = 0; exec_tokens[k] != NULL; k++) {
                                       strcat(cmd_str, exec_tokens[k]);
                                       if (exec_tokens[k + 1]) strcat(cmd_str, " ");
                                   }


                                   int job_id = add_job(pid, cmd_str);


                                   char buf[128];
                                   snprintf(buf, sizeof(buf), "[%d] %d\n", job_id, pid);
                                   display_message(buf);


                                   check_jobs();
                               }
                           }
                       } else {
                           // External command
                           pid_t pid = fork();
                           if (pid < 0) {
                               display_error("ERROR: Failed to fork", "");
                           } else if (pid == 0) {
                               // Child: its own job table
                               init_jobs();


                               // --- Phase 5: parse redirection ---
                               redir_t r;
                               parse_redirection(exec_tokens, &r);


                               // --- Phase 5: apply redirection ---
                               if (r.infile) {
                                   int fd = open(r.infile, O_RDONLY);
                                   if (fd < 0) {
                                       display_error("ERROR: Cannot open file", r.infile);
                                       _exit(1);
                                   }
                                   dup2(fd, STDIN_FILENO);
                                   close(fd);
                               }


                               if (r.outfile) {
                                   int flags = O_WRONLY | O_CREAT;
                                   flags |= r.append ? O_APPEND : O_TRUNC;


                                   int fd = open(r.outfile, flags, 0644);
                                   if (fd < 0) {
                                       display_error("ERROR: Cannot open file", r.outfile);
                                       _exit(1);
                                   }
                                   dup2(fd, STDOUT_FILENO);
                                   close(fd);
                               }


                               // Background job behavior
                               if (background) {
                                   signal(SIGINT, SIG_IGN);


                                   int devnull = open("/dev/null", O_RDONLY);
                                   dup2(devnull, STDIN_FILENO);
                                   close(devnull);


                               } else {
                                   signal(SIGINT, SIG_DFL);
                               }


                               execvp(exec_tokens[0], exec_tokens);
                               display_error("ERROR: Unknown command: ", exec_tokens[0]);
                               _exit(1);
                           } else {
                               if (background) {
                                   // Build command string for job table
                                   char cmd_str[MAX_CMD_LEN] = "";
                                   for (size_t k = 0; exec_tokens[k] != NULL; k++) {
                                       strcat(cmd_str, exec_tokens[k]);
                                       if (exec_tokens[k + 1]) strcat(cmd_str, " ");
                                   }


                                   int job_id = add_job(pid, cmd_str);


                                   char buf[128];
                                   snprintf(buf, sizeof(buf), "[%d] %d\n", job_id, pid);
                                   display_message(buf);


                                   check_jobs();


                               } else {
                                   int status;
                                   waitpid(pid, &status, 0);
                                   check_jobs();
                               }
                           }
                       }
                   }
               }
           }

           next_command:
           server_poll();     // <-- flush server messages after ANY command
           check_jobs();
           if (skip_line) {
               line = strtok(NULL, "\n");
               continue;
           }


           line = strtok(NULL, "\n");
       }
   }


   free_all_variables();
   return 0;
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
