#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// #define MARKER_LEN 4 // part1
#define MARKER_LEN 14 // part2

int main(int argc, char *argv[]) {
  FILE *fp;
  int n_lines = 0;
  char *line = NULL;
  size_t len = 0;
  ssize_t read;

  char *filename = argv[1];

  fp = fopen(filename, "r");
  if (fp == NULL)
    exit(EXIT_FAILURE);

  char window[MARKER_LEN] = {0};

  bool uniq = true;

  char a, b;

  while ((read = getline(&line, &len, fp)) != -1) {
    for (int i = 0; i < read; i++) {
      window[i % MARKER_LEN] = line[i];

      if (i < MARKER_LEN - 1)
        continue;

      uniq = true;

      for (int j = 0; j < MARKER_LEN; j++) {
        a = window[j];
        for (int k = 1; k < MARKER_LEN; k++) {
          b = window[(j + k) % MARKER_LEN];
          uniq = uniq && (a != b);
        }
      }

      if (uniq) {
        printf("%d\n", i + 1);
        break;
      }
    }
  }

  fclose(fp);

  if (line)
    free(line);

  exit(EXIT_SUCCESS);

  return 0;
}
