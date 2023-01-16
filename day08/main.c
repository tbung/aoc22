#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define GRID_SIZE 100

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

  unsigned int grid[GRID_SIZE * GRID_SIZE] = {0};
  int i = 0;
  int j = 0;

  unsigned int grid_size = GRID_SIZE;

  while ((read = getline(&line, &len, fp)) != -1) {
    if (read > 0)
      grid_size = read - 1;
    for (j = 0; j < grid_size; j++) {
      // printf("%d\n", line[j] - '0');
      grid[i * grid_size + j] = line[j] - '0';
    }
    i++;
  }

  bool visible = true;
  unsigned int current, x, idx;
  // unsigned int total = 0;
  unsigned int tree_score = 1;
  unsigned int max = 0;
  for (i = 0; i < grid_size; i++) {
    for (j = 0; j < grid_size; j++) {
      if ((i == 0) || (j == 0) || (i == grid_size - 1) || (j == grid_size - 1))
        continue;

      if (j > 0)
        printf(" ");

      current = grid[i * grid_size + j];
      // printf("%d", current);
      tree_score = 1;

      // visible = true;
      // for (int x = 0; x < i; x++) {
      //   visible = visible && (grid[x * grid_size + j] < current);
      // }
      // if (visible) {
      //   total++;
      //   continue;
      // }
      //
      // visible = true;
      // for (int x = i + 1; x < grid_size; x++) {
      //   visible = visible && (grid[x * grid_size + j] < current);
      // }
      // if (visible) {
      //   total++;
      //   continue;
      // }
      //
      // visible = true;
      // for (int y = 0; y < j; y++) {
      //   visible = visible && (grid[i * grid_size + y] < current);
      // }
      // if (visible) {
      //   total++;
      //   continue;
      // }
      //
      // visible = true;
      // for (int y = j + 1; y < grid_size; y++) {
      //   visible = visible && (grid[i * grid_size + y] < current);
      // }
      // if (visible) {
      //   total++;
      //   continue;
      // }

      for (x = 1; i > x; x++) {
        idx = (i - x) * grid_size + j;
        if (grid[idx] >= current)
          break;
      }
      printf("%d,", x);
      tree_score *= x;
      for (x = 1; i + x < grid_size - 1; x++) {
        idx = (i + x) * grid_size + j;
        if (grid[idx] >= current)
          break;
      }
      printf("%d,", x);
      tree_score *= x;
      for (x = 1; j > x; x++) {
        idx = i * grid_size + j - x;
        if (grid[idx] >= current)
          break;
      }
      printf("%d,", x);
      tree_score *= x;
      for (x = 1; j + x < grid_size - 1; x++) {
        idx = i * grid_size + j + x;
        if (grid[idx] >= current)
          break;
      }
      printf("%d,", x);
      tree_score *= x;

      printf("%d,%d", current, tree_score);

      if (tree_score > max)
        max = tree_score;
    }
    // printf(" %d\n", total);
    printf("\n");
  }

  printf("%d\n", max);
  fclose(fp);

  if (line)
    free(line);

  exit(EXIT_SUCCESS);

  return 0;
}
