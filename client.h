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



#ifndef CLIENT_H
#define CLIENT_H

#include <sys/types.h>

pid_t start_client(int port, const char *host, int input_fd);
void client_connected_command(int sock);

#endif


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
