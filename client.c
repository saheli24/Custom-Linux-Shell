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



#include "client.h"
#include "io_helpers.h"

#include <sys/socket.h>
#include <sys/select.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/prctl.h>
#include <fcntl.h>
#include <errno.h>

static int connected_clients = 0;

void client_connected_command(int sock) {
    const char *msg = "\\connected\n";
    send(sock, msg, strlen(msg), 0);
}

pid_t start_client(int port, const char *host, int input_fd) {
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        display_error("ERROR: Failed to create socket", "");
        return -1;
    }

    struct sockaddr_in addr = {0};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);

    if (inet_pton(AF_INET, host, &addr.sin_addr) <= 0) {
        display_error("ERROR: Invalid hostname", (char *)host);
        close(sock);
        return -1;
    }

    if (connect(sock, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        display_error("ERROR: Failed to connect", (char *)host);
        close(sock);
        return -1;
    }

    connected_clients++;

    pid_t pid = fork();
    if (pid < 0) {
        display_error("ERROR: Failed to fork", "");
        close(sock);
        return -1;
    }

    if (pid > 0) {
        close(sock);
        return pid;
    }

    // ---------------- CHILD PROCESS ----------------
    signal(SIGINT, SIG_IGN);
    prctl(PR_SET_PDEATHSIG, SIGTERM);

    // Redirect stdin to input_fd (the pipe read end)
    if (input_fd != STDIN_FILENO) {
        dup2(input_fd, STDIN_FILENO);
        close(input_fd);
    }

    int flags = fcntl(sock, F_GETFL, 0);
    fcntl(sock, F_SETFL, flags | O_NONBLOCK);

    while (1) {
        fd_set rfds;
        FD_ZERO(&rfds);
        FD_SET(STDIN_FILENO, &rfds);
        FD_SET(sock, &rfds);

        int maxfd = sock;
        struct timeval tv = {0, 100000};

        int r = select(maxfd + 1, &rfds, NULL, NULL, &tv);

        if (r == 0) {
            char tmp;
            int alive = recv(sock, &tmp, 1, MSG_PEEK);
            if (alive == 0) {
                close(sock);
                exit(0);
            }
            if (alive < 0 && errno != EAGAIN && errno != EWOULDBLOCK) {
                close(sock);
                exit(0);
            }
            continue;
        }

        if (r < 0) {
            close(sock);
            exit(0);
        }

        // ----------- INPUT FROM PIPE -----------
        if (FD_ISSET(STDIN_FILENO, &rfds)) {
            char buf[1024];
            int n = read(STDIN_FILENO, buf, sizeof(buf) - 1);

            if (n == 0) {
                close(sock);
                exit(0);
            }

            if (n < 0) {
                continue;
            }

            buf[n] = '\0';

            if (buf[0] == '\0') {
                close(sock);
                exit(0);
            }

            // Strip newline
            char *nl = strchr(buf, '\n');
            if (nl) *nl = '\0';

            // Trim leading/trailing whitespace
            char *start = buf;
            while (*start == ' ' || *start == '\t') start++;

            char *end = start + strlen(start) - 1;
            while (end >= start && (*end == ' ' || *end == '\t')) {
                *end = '\0';
                end--;
            }

            if (strlen(start) == 0) {
                continue;
            }

            // 128-char limit (expansion already done by parent)
            if ((int)strlen(start) > 128) {
                display_error("ERROR: Message too long", "");
                continue;
            }

            // Special command
            if (strcmp(start, "\\connected") == 0) {
                client_connected_command(sock);
                continue;
            }

            // Send with newline so server can delimit it
            char to_send[130];
            snprintf(to_send, sizeof(to_send), "%s\n", start);
            send(sock, to_send, strlen(to_send), 0);
        }

        // ----------- INPUT FROM SERVER -----------
        if (FD_ISSET(sock, &rfds)) {
            char buf[1024];
            int n = recv(sock, buf, sizeof(buf) - 1, 0);

            if (n <= 0) {
                close(sock);
                exit(0);
            }

            buf[n] = '\0';
            display_message(buf);
        }
    }

    close(sock);
    exit(0);
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
