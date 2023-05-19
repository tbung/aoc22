#include <assert.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* #include "inputstr.h" */

#define BOARD_SIZE 256

enum direction { RIGHT, DOWN, LEFT, UP };

typedef struct {
  int x;
  int y;
} delta_t;

const delta_t deltas[] = {{1, 0}, {0, 1}, {-1, 0}, {0, -1}};

void board_print(char board[BOARD_SIZE][BOARD_SIZE]) {
  for (size_t i = 0; i < BOARD_SIZE; i++) {
    if (board[i][0] == 0) {
      break;
    }
    for (size_t j = 0; j < BOARD_SIZE; j++) {
      if (board[i][j] == 0) {
        break;
      }
      printf("%c", board[i][j]);
    }
    printf("\n");
  }
}

void move_plane(char board[BOARD_SIZE][BOARD_SIZE], size_t n_rows, int *x_coord,
                int *y_coord, enum direction dir, int distance) {
  size_t x_init;
  size_t y_init;

  for (int i = 0; i < distance; i++) {
    x_init = *x_coord;
    y_init = *y_coord;

    *x_coord += deltas[dir].x;
    *y_coord += deltas[dir].y;
    if (board[*y_coord][*x_coord] == '\0' || board[*y_coord][*x_coord] == ' ' ||
        *x_coord >= strlen(board[*y_coord]) || *y_coord >= n_rows) {
      switch (dir) {
      case RIGHT:
        for (*x_coord = 0; board[*y_coord][*x_coord] == ' '; (*x_coord)++)
          ;
        break;
      case DOWN:
        for (*y_coord = 0; *x_coord >= strlen(board[*y_coord]) ||
                           (board[*y_coord][*x_coord] == ' ');
             (*y_coord)++)
          ;
        break;
      case LEFT:
        for (*x_coord = strlen(board[*y_coord]) - 1;
             *x_coord >= strlen(board[*y_coord]) ||
             board[*y_coord][*x_coord] == ' ';
             (*x_coord)--)
          ;
        break;
      case UP:
        for (*y_coord = n_rows - 1; *x_coord >= strlen(board[*y_coord]) ||
                                    (board[*y_coord][*x_coord] == ' ');
             (*y_coord)--)
          ;
        break;
      }
    }

    if (board[*y_coord][*x_coord] == '#') {
      *x_coord = x_init;
      *y_coord = y_init;
      return;
    }

    if (*x_coord >= strlen(board[*y_coord])) {
      printf("x_coord out of bounds: %d >= %ld @ y = %d\n", *x_coord,
             strlen(board[*y_coord]), *y_coord);
      printf("\tx_init = %ld, y_init = %ld\n", x_init, y_init);
      printf("\tdir = %u\n", dir);
      exit(EXIT_FAILURE);
    };
    if (*y_coord >= n_rows) {
      printf("y_coord out of bounds: %d >= %ld @ x = %d\n", *y_coord, n_rows,
             *x_coord);
      printf("\tx_init = %ld, y_init = %ld\n", x_init, y_init);
      printf("\tdir = %u\n", dir);
      exit(EXIT_FAILURE);
    };
    if (board[*y_coord][*x_coord] != '.') {
      printf("invalid final board field: %c (%x)\n", board[*y_coord][*x_coord],
             board[*y_coord][*x_coord]);
      printf("\tx_coord = %d, y_coord = %d\n", *x_coord, *y_coord);
      printf("\tx_init = %ld, y_init = %ld\n", x_init, y_init);
      printf("\tdir = %u\n", dir);
      exit(EXIT_FAILURE);
    };
  }
}

int64_t trace(char board[BOARD_SIZE][BOARD_SIZE], size_t n_rows,
              char *instructions) {
  int x_coord = 0;
  int y_coord = 0;
  enum direction dir = RIGHT;

  while (board[y_coord][x_coord] != '.') {
    x_coord++;
  }

  char *inst_ptr = instructions;
  while (*inst_ptr != '\0' && *inst_ptr != '\n') {

    if (*inst_ptr == 'R') {
      dir = (dir + 1) % 4;
      inst_ptr++;
    } else if (*inst_ptr == 'L') {
      dir = (dir + 3) % 4;
      inst_ptr++;
    } else if (isdigit(*inst_ptr)) {
      move_plane(board, n_rows, &x_coord, &y_coord, dir,
                 strtol(inst_ptr, &inst_ptr, 10));
    } else {
      printf("Invalid instruction: %c\n", *inst_ptr);
      exit(EXIT_FAILURE);
    }
  }

  return (y_coord + 1) * 1000 + (x_coord + 1) * 4 + dir;
}

int main(int argc, char *argv[]) {
  FILE *fp;
  char *line = NULL;
  size_t len = 0;

  char *filename = argv[1];

  fp = fopen(filename, "r");
  if (fp == NULL)
    exit(EXIT_FAILURE);

  char board[BOARD_SIZE][BOARD_SIZE] = {0};
  size_t n_rows = 0;

  while (getline(&line, &len, fp) != -1) {
    if (line[0] == '\n') {
      break;
    }
    memcpy(board[n_rows], line, len);
    board[n_rows][strlen(line) - 1] = 0;
    n_rows++;
  }
  getline(&line, &len, fp);

  fclose(fp);

  /* board_print(board); */
  printf("Part 1: %ld\n", trace(board, n_rows, line));

  if (line)
    free(line);

  exit(EXIT_SUCCESS);

  return 0;
}
