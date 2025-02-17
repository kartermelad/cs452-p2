#include "../src/lab.h"
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <readline/readline.h>
#include <readline/history.h>

char *get_prompt(const char *env) {
    char *prompt = getenv(env);
    if (prompt == NULL) {
        prompt = "shell> ";
    }
    return prompt;
}

int change_dir(char **dir) {

}

char **cmd_parse(char const *line) {

}

void cmd_free(char ** line) {

}

char *trim_white(char *line) {

}

bool do_builtin(struct shell *sh, char **argv) {

}

void sh_init(struct shell *sh) {

}

void sh_destroy(struct shell *sh) {

}

void parse_args(int argc, char **argv) {
    int opt;
    while ((opt = getopt(argc, argv, "v")) != -1) {
        switch (opt) {
            case 'v':
                printf("Shell version: %d.%d\n", lab_VERSION_MAJOR, lab_VERSION_MINOR);
                exit(0);
            default:
                fprintf(stderr, "Usage: %s [-v]\n", argv[0]);
                exit(EXIT_FAILURE);
        }
    }
}