#include "../lib/stack.h"
#include <assert.h>
#include <ctype.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
  size_t x;
  size_t y;
} coord_t;

coord_t *coord_new() {
  coord_t *state = calloc(1, sizeof(coord_t));
  return state;
}

void print_board(char board[256][256], size_t n_rows, size_t n_cols) {
  for (size_t i = 0; i < n_rows; i++) {
    for (size_t j = 0; j < n_cols; j++) {
      printf("%c", board[i][j]);
    }
    printf("\n");
  }
}

void print_debug(char board[256][256], size_t n_rows, size_t n_cols,
                 size_t time, bool visited[256][256]) {

  printf("time: %zu\n", time);
  char board_copy[256][256] = {0};
  memcpy(board_copy, board, 256 * 256 * sizeof(board[0][0]));

  for (size_t x = 1; x < n_cols - 1; x++) {
    for (size_t y = 1; y < n_rows - 1; y++) {
      board_copy[y][x] = '.';
    }
  }

  for (size_t x = 1; x < n_cols - 1; x++) {
    for (size_t y = 1; y < n_rows - 1; y++) {
      // positive x direction (minus wall)
      size_t n = n_cols - 2;
      size_t u = x - 1;
      size_t s = (u + n * (u / n) + n - (time % n)) % n + 1;
      if (board[y][s] == '>') {
        if (board_copy[y][x] == '.') {
          board_copy[y][x] = '>';
        } else if (isdigit(board_copy[y][x])) {
          board_copy[y][x] = atoi(&board_copy[y][x]) + 1 + '0';
        } else {
          board_copy[y][x] = '2';
        }
      }

      // negative x direction (minus wall)
      n = n_cols - 2;
      u = n - x;
      s = n - ((u + n * (u / n) + n - (time % n)) % n);
      if (board[y][s] == '<') {
        if (board_copy[y][x] == '.') {
          board_copy[y][x] = '<';
        } else if (isdigit(board_copy[y][x])) {
          board_copy[y][x] = atoi(&board_copy[y][x]) + 1 + '0';
        } else {
          board_copy[y][x] = '2';
        }
      }

      // positive y direction (minus wall)
      n = n_rows - 2;
      u = y - 1;
      s = (u + n * (u / n) + n - (time % n)) % n + 1;
      if (board[s][x] == 'v') {
        if (board_copy[y][x] == '.') {
          board_copy[y][x] = 'v';
        } else if (isdigit(board_copy[y][x])) {
          board_copy[y][x] = atoi(&board_copy[y][x]) + 1 + '0';
        } else {
          board_copy[y][x] = '2';
        }
      }

      // negative y direction (minus wall)
      n = n_rows - 2;
      u = n - y;
      s = n - ((u + n * (u / n) + n - (time % n)) % n);
      if (board[s][x] == '^') {
        if (board_copy[y][x] == '.') {
          board_copy[y][x] = '^';
        } else if (isdigit(board_copy[y][x])) {
          board_copy[y][x] = atoi(&board_copy[y][x]) + 1 + '0';
        } else {
          board_copy[y][x] = '2';
        }
      }
    }
  }
  for (size_t x = 0; x < n_cols; x++) {
    for (size_t y = 0; y < n_rows; y++) {
      if (visited[y][x]) {
        if (board_copy[y][x] != '.') {
          printf("Error: visited cell not empty: %c\n", board_copy[y][x]);
          /* exit(1); */
          board_copy[y][x] = 'X';
        } else {
          board_copy[y][x] = 'E';
        }
      }
    }
  }

  print_board(board_copy, n_rows, n_cols);
  printf("\n");
}

bool can_move(coord_t *coord, size_t time, char board[256][256], size_t n_rows,
              size_t n_cols) {
  if ((coord->x < 0 || coord->y < 0) ||
      (coord->x >= n_cols || coord->y >= n_rows) ||
      (board[coord->y][coord->x] == '#')) {
    return false;
  }

  // positive x direction (minus wall)
  size_t n = n_cols - 2;
  size_t u = coord->x - 1;
  size_t s = (u + n * (u / n) + n - (time % n)) % n + 1;
  if (board[coord->y][s] == '>') {
    return false;
  }

  // negative x direction (minus wall)
  n = n_cols - 2;
  u = n - coord->x;
  s = n - ((u + n * (u / n) + n - (time % n)) % n);
  if (board[coord->y][s] == '<') {
    return false;
  }

  // positive y direction (minus wall)
  n = n_rows - 2;
  u = coord->y - 1;
  s = (u + n * (u / n) + n - (time % n)) % n + 1;
  if (board[s][coord->x] == 'v') {
    return false;
  }

  // negative y direction (minus wall)
  n = n_rows - 2;
  u = n - coord->y;
  s = n - ((u + n * (u / n) + n - (time % n)) % n);
  if (board[s][coord->x] == '^') {
    return false;
  }

  return true;
}

size_t get_time(coord_t *start, coord_t *end, char board[256][256],
                size_t n_rows, size_t n_cols, size_t start_time) {
  coord_t **states;
  states = calloc(256 * 256, sizeof(coord_t *));
  states[0] = start;
  size_t n_states = 1;
  coord_t **next_states;
  next_states = calloc(256 * 256, sizeof(coord_t *));
  size_t n_next_states = 0;
  size_t time = start_time;
  bool visited[256][256] = {0};
  visited[0][1] = true;

  while (true) {
    /* print_debug(board, n_rows, n_cols, time, visited); */
    n_next_states = 0;
    memset(visited, 0, sizeof(visited));
    for (size_t i = 0; i < n_states; i++) {
      coord_t *state = states[i];

      if (state->x == end->x && state->y == end->y) {
        return time;
      }

      // up
      if (state->x >= 0 && state->x < 256 && state->y - 1 >= 0 &&
          state->y - 1 < 256 && !visited[state->y - 1][state->x] &&
          can_move(&(coord_t){.x = state->x, .y = state->y - 1}, time + 1,
                   board, n_rows, n_cols)) {
        coord_t *new_state = coord_new();
        new_state->x = state->x;
        new_state->y = state->y - 1;
        next_states[n_next_states++] = new_state;
        visited[state->y - 1][state->x] = true;
      }

      // down
      if (state->x >= 0 && state->x < 256 && state->y + 1 >= 0 &&
          state->y + 1 < 256 && !visited[state->y + 1][state->x] &&
          can_move(&(coord_t){.x = state->x, .y = state->y + 1}, time + 1,
                   board, n_rows, n_cols)) {
        coord_t *new_state = coord_new();
        new_state->x = state->x;
        new_state->y = state->y + 1;
        next_states[n_next_states++] = new_state;
        visited[state->y + 1][state->x] = true;
      }

      // left
      if (state->x - 1 >= 0 && state->x - 1 < 256 && state->y >= 0 &&
          state->y < 256 && !visited[state->y][state->x - 1] &&
          can_move(&(coord_t){.x = state->x - 1, .y = state->y}, time + 1,
                   board, n_rows, n_cols)) {
        coord_t *new_state = coord_new();
        new_state->x = state->x - 1;
        new_state->y = state->y;
        next_states[n_next_states++] = new_state;
        visited[state->y][state->x - 1] = true;
      }

      // right
      if (state->x + 1 >= 0 && state->x + 1 < 256 && state->y >= 0 &&
          state->y < 256 && !visited[state->y][state->x + 1] &&
          can_move(&(coord_t){.x = state->x + 1, .y = state->y}, time + 1,
                   board, n_rows, n_cols)) {
        coord_t *new_state = coord_new();
        new_state->x = state->x + 1;
        new_state->y = state->y;
        next_states[n_next_states++] = new_state;
        visited[state->y][state->x + 1] = true;
      }

      // wait
      if (state->x >= 0 && state->x < 256 && state->y >= 0 && state->y < 256 &&
          !visited[state->y][state->x] &&
          can_move(state, time + 1, board, n_rows, n_cols)) {
        next_states[n_next_states++] = state;
        visited[state->y][state->x] = true;
      }
    }
    memcpy(states, next_states, sizeof(coord_t *) * n_next_states);
    n_states = n_next_states;
    if (n_states == 0) {
      printf("no path found\n");
      exit(1);
    }
    time += 1;
  }
}

int main(int argc, char *argv[]) {
  FILE *fp;
  char *line = NULL;
  size_t len = 0;

  char *filename = argv[1];

  fp = fopen(filename, "r");
  if (fp == NULL)
    exit(EXIT_FAILURE);

  char board[256][256] = {0};

  size_t n_rows = 0;

  while (getline(&line, &len, fp) != -1) {
    memcpy(board[n_rows], line, len);
    board[n_rows][strlen(line) - 1] = 0; // remove newline
    n_rows++;
  }

  size_t n_cols = strlen(board[0]);

  coord_t start = {1, 0};
  coord_t end = {n_cols - 2, n_rows - 1};

  size_t time = get_time(&start, &end, board, n_rows, n_cols, 0);
  printf("Part 1: %zu\n", time);
  time = get_time(&end, &start, board, n_rows, n_cols, time);
  time = get_time(&start, &end, board, n_rows, n_cols, time);
  printf("Part 2: %zu\n", time);

  fclose(fp);

  if (line)
    free(line);

  exit(EXIT_SUCCESS);

  return 0;
}
