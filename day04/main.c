#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define BUF_SIZE 1024

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

  int i, j;
  int ranges[2][2] = {0};
  char *str1;
  char *str2;
  char *token, *subtoken;
  char *saveptr1, *saveptr2;
  char *delim1 = ",";
  char *delim2 = "-";
  int total = 0;
  while ((read = getline(&line, &len, fp)) != -1) {
    line = strtok(line, "\n");

    for (i = 0, str1 = line;; i++, str1 = NULL) {
      token = strtok_r(str1, delim1, &saveptr1);
      if (token == NULL)
        break;

      printf("%s\n", token);

      for (j = 0, str2 = token;; j++, str2 = NULL) {
        subtoken = strtok_r(str2, delim2, &saveptr2);
        if (subtoken == NULL)
          break;

        printf("%d\n", atoi(subtoken));

        ranges[i][j] = atoi(subtoken);
      }
    }

    // if ((ranges[1][0] >= ranges[0][0] && ranges[1][1] <= ranges[0][1]) ||
    //     (ranges[1][0] <= ranges[0][0] && ranges[1][1] >= ranges[0][1])) {
    //   puts("completely overlapping\n");
    //   total++;
    // }

    if (((ranges[1][0] >= ranges[0][0] && ranges[1][0] <= ranges[0][1]) ||
         (ranges[1][1] >= ranges[0][0] && ranges[1][1] <= ranges[0][1])) ||
        ((ranges[0][0] >= ranges[1][0] && ranges[0][0] <= ranges[1][1]) ||
         (ranges[0][1] >= ranges[1][0] && ranges[0][1] <= ranges[1][1]))) {
      puts("overlapping\n");
      total++;
    }
    puts("\n");
  }

  printf("%d\n", total);

  fclose(fp);

  if (line)
    free(line);

  exit(EXIT_SUCCESS);
}
