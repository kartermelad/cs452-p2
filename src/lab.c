#include "../src/lab.h"
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <readline/readline.h>
#include <pwd.h>
#include <readline/history.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <limits.h>
#include <signal.h>

// Function to get the shell prompt from an environment variable
char *get_prompt(const char *env) {
    char *prompt = getenv(env);
    if (prompt == NULL) {
        prompt = strdup("shell>"); // Default prompt
    } else {
        prompt = strdup(prompt); // Custom prompt
    }
    return prompt;
}

// Function to change the current working directory
int change_dir(char **dir) {
    if (dir[1] == NULL) {
        char *home = getenv("HOME");
        if (home == NULL) {
            struct passwd *pw = getpwuid(getuid());
            if (pw == NULL) {
                fprintf(stderr, "cd: HOME environment variable not set and could not determine home directory\n");
                return -1;
            }
            home = pw->pw_dir; // Get home directory from passwd struct
        }
        if (chdir(home) != 0) {
            perror("cd");
            return -1;
        }
    } else {
        if (chdir(dir[1]) != 0) {
            perror("cd");
            return -1;
        }
    }
    return 0;
}

// Function to parse a command line into tokens
char **cmd_parse(char const *line) {
    int bufsize = 64, position = 0;
    char **tokens = malloc(bufsize * sizeof(char*));
    char *line_copy = strdup(line);
    int in_quotes = 0;

    if (!tokens || !line_copy) {
        fprintf(stderr, "cmd_parse: allocation error\n");
        exit(EXIT_FAILURE);
    }
    char *start = line_copy;
    for (char *p = line_copy; *p; p++) {
        if (*p == '"') {
            in_quotes = !in_quotes; // Toggle in_quotes flag
        } else if (isspace(*p) && !in_quotes) {
            *p = '\0';
            if (start != p) {
                tokens[position++] = strdup(start);
                if (position >= bufsize) {
                    bufsize += 64;
                    tokens = realloc(tokens, bufsize * sizeof(char*));
                    if (!tokens) {
                        fprintf(stderr, "cmd_parse: allocation error\n");
                        exit(EXIT_FAILURE);
                    }
                }
            }
            start = p + 1;
        }
    }
    if (start != line_copy + strlen(line_copy)) {
        tokens[position++] = strdup(start);
    }
    tokens[position] = NULL;
    free(line_copy);
    return tokens;
}

// Function to free the memory allocated for command tokens
void cmd_free(char ** line) {
    for (int i = 0; line[i] != NULL; i++) {
        free(line[i]);
    }
    free(line);
}

// Function to trim leading and trailing whitespace from a string
char *trim_white(char *line) {
    char *end;
    while (isspace((unsigned char)*line)) {
        line++;
    }
    if (*line == 0) {
        return line;
    }
    end = line + strlen(line) - 1;
    while (end > line && isspace((unsigned char)*end)) {
        end--;
    }
    end[1] = '\0';
    return line;
}

// Function to handle built-in commands
bool do_builtin(struct shell *sh, char **argv) {
    if (strcmp(argv[0], "cd") == 0) {
        return change_dir(argv) == 0;
    } else if (strcmp(argv[0], "exit") == 0) {
        sh_destroy(sh);
        exit(0);
    } else if (strcmp(argv[0], "history") == 0) {
        HIST_ENTRY **hist_list = history_list();
        if (hist_list) {
            for (int i = 0; hist_list[i]; i++) {
                printf("%d: %s\n", i + history_base, hist_list[i]->line);
            }
        }
        return true;
    }
    return false;
}

// Function to initialize the shell structure
void sh_init(struct shell *sh) {
    sh->shell_terminal = STDIN_FILENO;
    sh->shell_is_interactive = isatty(sh->shell_terminal);
    if (sh->shell_is_interactive) {
        while (tcgetpgrp(sh->shell_terminal) != (sh->shell_pgid = getpgrp()))
            kill(-sh->shell_pgid, SIGTTIN);
    }

    signal(SIGINT, SIG_IGN);
    signal(SIGQUIT, SIG_IGN);
    signal(SIGTSTP, SIG_IGN);
    signal(SIGTTIN, SIG_IGN);
    signal(SIGTTOU, SIG_IGN);

    sh->shell_pgid = getpid();
    if (setpgid(sh->shell_pgid, sh->shell_pgid) < 0) {
        perror("Couldn't put the shell in its own process group");
        exit(1);
    }
    tcsetpgrp(sh->shell_terminal, sh->shell_pgid);
    tcgetattr(sh->shell_terminal, &sh->shell_tmodes);

    sh->prompt = get_prompt("MY_PROMPT");
}

// Function to destroy the shell structure and free resources
void sh_destroy(struct shell *sh) {
    tcsetpgrp(sh->shell_terminal, sh->shell_pgid);
    tcsetattr(sh->shell_terminal, TCSADRAIN, &sh->shell_tmodes);

    signal(SIGINT, SIG_DFL);
    signal(SIGQUIT, SIG_DFL);
    signal(SIGTSTP, SIG_DFL);
    signal(SIGTTIN, SIG_DFL);
    signal(SIGTTOU, SIG_DFL);

    free(sh->prompt);
    kill(getpid(), SIGTERM);
}

// Function to parse command-line arguments
void parse_args(int argc, char **argv) {
    int opt;
    while ((opt = getopt(argc, argv, "v")) != -1) {
        switch (opt) {
            case 'v':
                printf("Shell version: %d.%d\n", lab_VERSION_MAJOR, lab_VERSION_MINOR);
                return;
            default:
                fprintf(stderr, "Usage: %s [-v]\n", argv[0]);
                exit(EXIT_FAILURE);
        }
    }
}