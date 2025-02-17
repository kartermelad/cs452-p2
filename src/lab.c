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

char *get_prompt(const char *env) {
    char *prompt = getenv(env);
    if (prompt == NULL) {
        prompt = strdup("shell>");
    } else {
        prompt = strdup(prompt);
    }
    return prompt;
}

int change_dir(char **dir) {
    if (dir[1] == NULL) {
        char *home = getenv("HOME");
        if (home == NULL) {
            struct passwd *pw = getpwuid(getuid());
            if (pw == NULL) {
                fprintf(stderr, "cd: HOME environment variable not set and could not determine home directory\n");
                return -1;
            }
            home = pw->pw_dir;
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

char **cmd_parse(char const *line) {
    int bufsize = 64, position = 0;
    char **tokens = malloc(bufsize * sizeof(char*));
    char *line_copy = strdup(line);
    char *token;
    if (!tokens || !line_copy) {
        fprintf(stderr, "cmd_parse: allocation error\n");
        exit(EXIT_FAILURE);
    }
    token = strtok(line_copy, " \t\r\n\a");
    while (token != NULL) {
        tokens[position] = strdup(token);
        position++;
        if (position >= bufsize) {
            bufsize += 64;
            tokens = realloc(tokens, bufsize * sizeof(char*));
            if (!tokens) {
                fprintf(stderr, "cmd_parse: allocation error\n");
                exit(EXIT_FAILURE);
            }
        }
        token = strtok(NULL, " \t\r\n\a");
    }
    tokens[position] = NULL;
    free(line_copy);
    return tokens;
}

void cmd_free(char ** line) {
    for (int i = 0; line[i] != NULL; i++) {
        free(line[i]);
    }
    free(line);
}

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

void sh_init(struct shell *sh) {
    sh->prompt = NULL;
    sh->shell_terminal = STDIN_FILENO;
    sh->shell_pgid = getpid();
}

void sh_destroy(struct shell *sh) {
    if (sh->prompt && strcmp(sh->prompt, "shell>") != 0) {
        free(sh->prompt);
        sh->prompt = NULL;
    }
}

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