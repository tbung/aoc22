#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char *strrev(char *str) {
  char *p1, *p2;

  if (!str || !*str)
    return str;
  for (p1 = str, p2 = str + strlen(str) - 1; p2 > p1; ++p1, --p2) {
    *p1 ^= *p2;
    *p2 ^= *p1;
    *p1 ^= *p2;
  }
  return str;
}

void print_stacks(int stack_idx, char stacks[128][128], int stack_ptrs[128]) {
  int i, j;
  for (i = 0; i < stack_idx; i++) {
    for (j = 0; j < stack_ptrs[i]; j++) {
      if (j > 0)
        printf(", ");
      printf("%c", stacks[i][j]);
    }
    printf("\n");
  }
}

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

  int i, j, stack_idx, n_crates, from_crate, to_crate;
  char stacks[128][128] = {0};
  int stack_ptrs[128] = {0};
  char *str1;
  char *str2;
  char *token, *subtoken;
  char *saveptr1, *saveptr2;
  char *delim1 = " ";
  int total = 0;

  bool parse_as_stack = true;
  while ((read = getline(&line, &len, fp)) != -1) {
    if (!strcmp(line, "\n")) {
      for (i = 0; i < stack_idx; i++)
        strrev(stacks[i]);
      print_stacks(stack_idx, stacks, stack_ptrs);
      parse_as_stack = false;
      continue;
    }

    if (parse_as_stack) {
      for (i = 0, stack_idx = 0; i < read; i += 4, stack_idx++) {
        if (line[i] != '[')
          continue;

        stacks[stack_idx][stack_ptrs[stack_idx]] = line[i + 1];
        stack_ptrs[stack_idx] += 1;
      }
    } else {
      strtok(line, delim1);
      token = strtok(NULL, delim1);
      n_crates = atoi(token);

      strtok(NULL, delim1);
      token = strtok(NULL, delim1);
      from_crate = atoi(token) - 1;

      strtok(NULL, delim1);
      token = strtok(NULL, delim1);
      to_crate = atoi(token) - 1;
      printf("move %d crates from stack %d to stack %d\n", n_crates, from_crate,
             to_crate);

      // for (i = 0; i < n_crates; i++) {
      //   stacks[to_crate][stack_ptrs[to_crate]] =
      //       stacks[from_crate][stack_ptrs[from_crate] - 1];
      //   stacks[from_crate][stack_ptrs[from_crate] - 1] = 0;
      //   stack_ptrs[to_crate] += 1;
      //   stack_ptrs[from_crate] -= 1;
      // }

      for (i = 0; i < n_crates; i++) {
        stacks[to_crate][stack_ptrs[to_crate]] =
            stacks[from_crate][stack_ptrs[from_crate] - n_crates + i];
        stacks[from_crate][stack_ptrs[from_crate] - n_crates + i] = 0;
        stack_ptrs[to_crate] += 1;
      }
      stack_ptrs[from_crate] -= n_crates;

      print_stacks(stack_idx, stacks, stack_ptrs);
    }
  }

  for (i = 0; i < stack_idx; i++) {
    printf("%c", stacks[i][stack_ptrs[i] - 1]);
  }
  printf("\n");

  fclose(fp);

  if (line)
    free(line);

  exit(EXIT_SUCCESS);
}
