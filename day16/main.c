#include <regex.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define ARRAY_SIZE(arr) (sizeof((arr)) / sizeof((arr)[0]))

static const char *const re_rate = "rate=([0-9]*);";
static const char *const re_conn = "[A-Z][A-Z]";

typedef struct {
  int rate;
  char *connections[];
} valve;

int main(int argc, char *argv[]) {
  FILE *fp;
  char *line = NULL;
  size_t len = 0;

  char *filename = argv[1];

  fp = fopen(filename, "r");
  if (fp == NULL) {
    exit(EXIT_FAILURE);
  }

  regex_t regex_rate;
  regmatch_t pmatch_rate[2];
  regex_t regex_conn;
  regmatch_t pmatch_conn[1];

  if (regcomp(&regex_rate, re_rate, REG_NEWLINE | REG_EXTENDED)) {
    exit(EXIT_FAILURE);
  }
  if (regcomp(&regex_conn, re_conn, REG_NEWLINE | REG_EXTENDED)) {
    exit(EXIT_FAILURE);
  }

  while (getline(&line, &len, fp) != -1) {
    if (regexec(&regex_rate, line, ARRAY_SIZE(pmatch_rate), pmatch_rate, 0)) {
      exit(EXIT_FAILURE);
    }
    printf("%s", line);

    printf("%.*s\n", pmatch_rate[0].rm_eo - pmatch_rate[0].rm_so,
           line + pmatch_rate[0].rm_so);
    printf("%ld\n", atol(line + pmatch_rate[1].rm_so));

    char *str = line + pmatch_rate[0].rm_eo;

    for (int i = 0;; i++) {
      if (regexec(&regex_conn, str, 1, pmatch_conn, 0)) {
        break;
      }
      if (i > 0) {
        printf(", ");
      }
      printf("%.*s", pmatch_conn[0].rm_eo - pmatch_conn[0].rm_so,
             str + pmatch_conn[0].rm_so);
      str = str + pmatch_conn[0].rm_eo;
    }

    printf("\n");
  }

  fclose(fp);

  if (line) {
    free(line);
  }

  exit(EXIT_SUCCESS);

  return 0;
}
