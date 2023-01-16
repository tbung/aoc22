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

  unsigned int elve_calories[BUF_SIZE] = {0};
  size_t elve_id = 0;

  while ((read = getline(&line, &len, fp)) != -1) {
    if (elve_id > (BUF_SIZE - 1)) {
      perror("elve buffer to small\n");
      printf("%d\n", elve_id);
      exit(EXIT_FAILURE);
    }

    if (strcmp(line, "\n") == 0) {
      elve_id++;
      continue;
    }

    elve_calories[elve_id] += atoi(line);
  }

  // int max_cal = elve_calories[0];
  unsigned int top_three[3] = {0};
  int smallest = 0;
  size_t smallest_j = 0;
  for (int i = 0; i <= elve_id; i++) {
    // if (elve_calories[i] > max_cal){
    //   max_cal = elve_calories[i];
    // }
    // printf("%d, %d, %d\n", top_three[0], top_three[1], top_three[2]);
    // printf("Cur val: %d\n\n", elve_calories[i]);

    smallest = top_three[0];
    smallest_j = 0;
    for (int j = 0; j < 3; j++) {
      if (top_three[j] < smallest) {
        smallest = top_three[j];
        smallest_j = j;
      }
    }

    if (smallest < elve_calories[i]) {
      top_three[smallest_j] = elve_calories[i];
    }
  }

  printf("%d\n", top_three[0] + top_three[1] + top_three[2]);
  // printf("%d, %d, %d\n", top_three[0], top_three[1], top_three[2]);

  fclose(fp);

  if (line)
    free(line);

  exit(EXIT_SUCCESS);

  return 0;
}
