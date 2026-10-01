
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





#ifndef __PIPELINE_H__
#define __PIPELINE_H__


int parse_pipeline(char *line, char *segments[], int max_segments);
// Full M4 version
void execute_pipeline_full(char *segments[], int count, int background, const char *job_cmd);


// Compatibility version for public tests
void execute_pipeline(char *segments[], int count);


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
