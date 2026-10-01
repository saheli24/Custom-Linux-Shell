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




#include "io_helpers.h"
#include <arpa/inet.h>
#include <sys/socket.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>
int send_one_shot(int port, const char *host, const char *msg) {
    int fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd < 0) {
        display_error("ERROR: Failed to create socket", "");
        return -1;
    }

    struct sockaddr_in addr = {0};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);

    if (inet_pton(AF_INET, host, &addr.sin_addr) <= 0) {
        display_error("ERROR: Invalid hostname", (char *)host);
        close(fd);
        return -1;
    }

    if (connect(fd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        display_error("ERROR: Failed to connect", (char *)host);
        close(fd);
        return -1;
    }

    // Reject messages longer than 128 chars (after expansion)
    if (strlen(msg) > 128) {
        display_error("ERROR: Message too long", "");
        close(fd);
        return -1;
    }

    //Send the message exactly as-is
    send(fd, msg, strlen(msg), 0);

    close(fd);
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
