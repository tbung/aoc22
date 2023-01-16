#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define TOTAL_SPACE 70000000
#define FREE_SPACE 30000000

struct Dir {
  char name[128];
  unsigned int amount;
};

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

  struct Dir stack[1024];
  size_t stack_ptr = 0;

  struct Dir dir_list[1024];
  size_t dir_ptr = 0;

  char name[128] = {0};
  struct Dir current;

  while ((read = getline(&line, &len, fp)) != -1) {
    if (!strncmp("$ cd ", line, 5)) {
      char *token = strtok(line, "\n");
      if (!strcmp(token + 5, "..")) {
        current = stack[stack_ptr - 1];
        stack[stack_ptr - 2].amount += current.amount;
        dir_list[dir_ptr] = current;
        dir_ptr++;
        stack_ptr--;
      } else {
        struct Dir dir = {.amount = 0};
        strcpy(dir.name, token + 5);
        stack[stack_ptr] = dir;
        stack_ptr++;
      }
      continue;
    }

    stack[stack_ptr - 1].amount += atoi(strtok(line, " "));
  }

  for (int i = stack_ptr - 1; i >= 0; i--) {
    dir_list[dir_ptr] = stack[i];
    dir_ptr++;

    if (i > 0) {
      stack[i - 1].amount += stack[i].amount;
    }
  }

  unsigned int total = 0;
  unsigned int to_be_freed =
      FREE_SPACE + dir_list[dir_ptr - 1].amount - TOTAL_SPACE;

  unsigned int candidates[1024] = {0};
  size_t candidate_ptr = 0;
  for (int i = 0; i < dir_ptr; i++) {
    printf("%s (%d)\n", dir_list[i].name, dir_list[i].amount);
    // if (dir_list[i].amount < 100000) {
    //   total += dir_list[i].amount;
    // }
    if (dir_list[i].amount >= to_be_freed) {
      candidates[candidate_ptr] = dir_list[i].amount;
      candidate_ptr++;
    }
  }

  unsigned int min = candidates[candidate_ptr - 1];
  for (int i = 0; i < candidate_ptr; i++) {
    if (candidates[i] < min) {
      min = candidates[i];
    }
  }

  printf("%d\n", min);

  fclose(fp);

  if (line)
    free(line);

  exit(EXIT_SUCCESS);

  return 0;
}
