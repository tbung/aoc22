#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../lib/stack.h"

#define GRID_SIZE 64 * 256
#define STACK_SIZE 1024
#define START 'E'
#define STARTVAL 'z'
#define END 'S'
#define ENDVAL 'a'

STACK(, size_t, STACK_SIZE)

void print_grid(char *grid, size_t row_ptr, size_t col_ptr) {
  for (int i = 0; i < row_ptr; i++) {
    for (int j = 0; j < col_ptr; j++) {
      printf("%c", grid[i * col_ptr + j]);
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

  char grid[GRID_SIZE] = {0};
  size_t row_ptr = 0;
  size_t col_ptr = 0;
  while ((read = getline(&line, &len, fp)) != -1) {
    col_ptr = read - 1; // remove newline
    strncpy(grid + (col_ptr * row_ptr), line, col_ptr);
    row_ptr++;
  }

  size_t idx = 0;
  long start_idx = -1, end_idx = -1;

  while ((start_idx == -1) || (end_idx == -1)) {
    if (grid[idx] == START) {
      start_idx = idx;
    }

    if (grid[idx] == END) {
      end_idx = idx;
    }
    idx++;
  }

  stack__t *stack = stack__new();
  // cstack_t *stack = &_stack;
  stack__push(stack, start_idx);

  size_t current, i, j, adj;
  bool visited[GRID_SIZE] = {false};
  visited[start_idx] = true;
  int parent[GRID_SIZE] = {[0 ... GRID_SIZE - 1] = -1};
  long inspecting, inspadj;

  while (!stack__empty(stack)) {
    current = stack__pop(stack);
    // if (current == end_idx)
    //   break;

    inspecting = grid[current];
    if ((inspecting == ENDVAL) || (inspecting == END)) {
      end_idx = current;
      break;
    }

    if (inspecting == START)
      inspecting = STARTVAL;

    j = current % col_ptr;
    i = current / col_ptr;

    adj = (i - 1) * col_ptr + j;
    if ((i > 0)) {
      if (!(visited[adj])) {
        inspadj = grid[adj];
        if (inspadj == END)
          inspadj = ENDVAL;
        // if ((inspadj <= (inspecting + 1))) {
        if ((inspadj >= (inspecting - 1))) {
          stack__push(stack, adj);
          visited[adj] = true;
          parent[adj] = current;
        }
      }
    }

    adj = i * col_ptr + j - 1;
    if ((j > 0)) {
      if (!(visited[adj])) {
        inspadj = grid[adj];
        if (inspadj == END)
          inspadj = ENDVAL;
        if ((inspadj >= (inspecting - 1))) {
          stack__push(stack, adj);
          visited[adj] = true;
          parent[adj] = current;
        }
      }
    }

    adj = (i + 1) * col_ptr + j;
    if ((i < row_ptr - 1)) {
      if (!(visited[adj])) {
        inspadj = grid[adj];
        if (inspadj == END)
          inspadj = ENDVAL;
        if ((inspadj >= (inspecting - 1))) {
          stack__push(stack, adj);
          visited[adj] = true;
          parent[adj] = current;
        }
      }
    }

    adj = i * col_ptr + j + 1;
    if ((j < col_ptr - 1)) {
      if (!(visited[adj])) {
        inspadj = grid[adj];
        if (inspadj == END)
          inspadj = ENDVAL;
        if ((inspadj >= (inspecting - 1))) {
          stack__push(stack, adj);
          visited[adj] = true;
          parent[adj] = current;
        }
      }
    }
  }

  size_t counter = 0;
  current = end_idx;
  size_t prev;
  char dbg_grid[GRID_SIZE] = {[0 ... GRID_SIZE - 1] = '.'};
  dbg_grid[current] = 'E';

  while (parent[current] != -1) {
    prev = current;
    current = parent[current];

    j = prev % col_ptr;
    i = prev / col_ptr;

    if (current == (i + 1) * col_ptr + j) {
      dbg_grid[current] = '^';
    } else if (current == (i - 1) * col_ptr + j) {
      dbg_grid[current] = 'v';
    } else if (current == i * col_ptr + (j - 1)) {
      dbg_grid[current] = '>';
    } else if (current == i * col_ptr + (j + 1)) {
      dbg_grid[current] = '<';
    }
    counter++;
  }

  print_grid(dbg_grid, row_ptr, col_ptr);

  printf("%zu\n", counter);

  fclose(fp);

  if (line)
    free(line);

  exit(EXIT_SUCCESS);

  return 0;
}
