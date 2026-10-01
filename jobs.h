/*
* AI Assistance Disclosure (CSC209)
 *
 * Portions of this file were developed with assistance from Microsoft Copilot
 * between March 14–16, 2026. Copilot provided structural suggestions, debugging
 * guidance, and help identifying edge cases. I reviewed, tested, and adapted all
 * AI‑generated suggestions myself, and the final code reflects my own
 * understanding and decisions.
 */

#ifndef JOBS_H
#define JOBS_H

#include <sys/types.h>

#define MAX_JOBS 128
#define MAX_CMD_LEN 256

typedef struct {
    pid_t pid;
    char cmd[MAX_CMD_LEN];
    int job_id;
    int active;   // 1 = running, 0 = done
} job_t;

void init_jobs();
int add_job(pid_t pid, const char *cmd);
void check_jobs();        // reap finished jobs + print DONE messages
void print_ps();          // builtin ps
void mark_job_done(pid_t pid);
void kill_all_jobs(void);

#endif

/*
 * AI Assistance Disclosure (CSC209)
 *
 * Portions of this file were developed with assistance from Microsoft Copilot
 * between March 14–16, 2026. Copilot provided structural suggestions, debugging
 * guidance, and help identifying edge cases. I reviewed, tested, and adapted all
 * AI‑generated suggestions myself, and the final code reflects my own
 * understanding and decisions.
 */
