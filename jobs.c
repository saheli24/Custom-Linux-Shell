/*
 * AI Assistance Disclosure (CSC209)
 *
 * Portions of this file were developed with assistance from Microsoft Copilot
 * between March 14–16, 2026. Copilot provided structural suggestions, debugging
 * guidance, and help identifying edge cases. I reviewed, tested, and adapted all
 * AI‑generated suggestions myself, and the final code reflects my own
 * understanding and decisions.
 */

#include "jobs.h"
#include "io_helpers.h"
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>
#include <stdio.h>
#include <signal.h>   // add at top if missing

static job_t jobs[MAX_JOBS];
static int next_job_id = 1;

void init_jobs() {
    for (int i = 0; i < MAX_JOBS; i++) {
        jobs[i].active = 0;
    }
    next_job_id = 1;
}

int add_job(pid_t pid, const char *cmd) {
    for (int i = 0; i < MAX_JOBS; i++) {
        if (!jobs[i].active) {
            jobs[i].pid = pid;
            jobs[i].job_id = next_job_id++;
            jobs[i].active = 1;
            strncpy(jobs[i].cmd, cmd, MAX_CMD_LEN - 1);
            jobs[i].cmd[MAX_CMD_LEN - 1] = '\0';
            return jobs[i].job_id;
        }
    }
    return -1;
}

void mark_job_done(pid_t pid) {
    for (int i = 0; i < MAX_JOBS; i++) {
        if (jobs[i].active && jobs[i].pid == pid) {
            jobs[i].active = 0;

            char buf[512];
            snprintf(buf, sizeof(buf),
                     "[%d]+ Done %s\n",
                     jobs[i].job_id,
                     jobs[i].cmd);
            display_message(buf);

            // Reset job counter if no active jobs remain
            int any_active = 0;
            for (int j = 0; j < MAX_JOBS; j++) {
                if (jobs[j].active) {
                    any_active = 1;
                    break;
                }
            }
            if (!any_active) {
                next_job_id = 1;
            }

            return;
        }
    }
}


void check_jobs() {
    int status;
    pid_t pid;

    while ((pid = waitpid(-1, &status, WNOHANG)) > 0) {
        mark_job_done(pid);
    }
}

void print_ps() {
    for (int i = 0; i < MAX_JOBS; i++) {
        if (jobs[i].active) {
            char buf[512];
            snprintf(buf, sizeof(buf),
                     "[%d] %d %s\n",
                     jobs[i].job_id,
                     jobs[i].pid,
                     jobs[i].cmd);
            display_message(buf);
        }
    }
}


void kill_all_jobs(void) {
    for (int i = 0; i < MAX_JOBS; i++) {
        if (jobs[i].active) {
            kill(jobs[i].pid, SIGKILL);
            waitpid(jobs[i].pid, NULL, 0);
            jobs[i].active = 0;
        }
    }
    next_job_id = 1;
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
