#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define N_STEPS_AREA 10
#define N_STEPS 1024
typedef enum { NORTH, SOUTH, WEST, EAST } direction;

bool move(direction dir, size_t pos, const long *pos2elf, long *elf2newpos,
          long *pos2newelf, size_t n_cols_max, size_t j) {
  switch (dir) {
  case NORTH:
    // north
    if ((pos2elf[pos - n_cols_max] == -1) &&
        (pos2elf[pos + 1 - n_cols_max] == -1) &&
        (pos2elf[pos - 1 - n_cols_max] == -1)) {
      if (pos2newelf[pos - n_cols_max] == -1) {
        elf2newpos[j] = pos - n_cols_max;
        pos2newelf[pos - n_cols_max] = j;
      } else {
        elf2newpos[j] = -1;
        elf2newpos[pos2newelf[pos - n_cols_max]] = -1;
      }
      return 0;
    } else {
      return 1;
    }

  case SOUTH:
    // south
    if ((pos2elf[pos + n_cols_max] == -1) &&
        (pos2elf[pos + 1 + n_cols_max] == -1) &&
        (pos2elf[pos - 1 + n_cols_max] == -1)) {
      if (pos2newelf[pos + n_cols_max] == -1) {
        elf2newpos[j] = pos + n_cols_max;
        pos2newelf[pos + n_cols_max] = j;
      } else {
        elf2newpos[j] = -1;
        elf2newpos[pos2newelf[pos + n_cols_max]] = -1;
      }
      return 0;
    } else {
      return 1;
    }

  case WEST:
    // west
    if ((pos2elf[pos - 1 + n_cols_max] == -1) && (pos2elf[pos - 1] == -1) &&
        (pos2elf[pos - 1 - n_cols_max] == -1)) {
      if (pos2newelf[pos - 1] == -1) {
        elf2newpos[j] = pos - 1;
        pos2newelf[pos - 1] = j;
      } else {
        elf2newpos[j] = -1;
        elf2newpos[pos2newelf[pos - 1]] = -1;
      }
      return 0;
    } else {
      return 1;
    }

  case EAST:
    // east
    if ((pos2elf[pos + 1 + n_cols_max] == -1) && (pos2elf[pos + 1] == -1) &&
        (pos2elf[pos + 1 - n_cols_max] == -1)) {
      if (pos2newelf[pos + 1] == -1) {
        elf2newpos[j] = pos + 1;
        pos2newelf[pos + 1] = j;
      } else {
        elf2newpos[j] = -1;
        elf2newpos[pos2newelf[pos + 1]] = -1;
      }
      return 0;
    } else {
      return 1;
    }
  default:
    exit(EXIT_FAILURE);
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

  char lines[128][128] = {0};
  size_t n_rows = 0;

  while (getline(&line, &len, fp) != -1) {
    memcpy(lines[n_rows], line, len);
    n_rows++;
  }

  size_t n_cols = strlen(lines[0]) - 1;

  size_t n_rows_max = n_rows + (long)2 * N_STEPS;
  size_t n_cols_max = n_cols + (long)2 * N_STEPS;

  size_t elf2pos[4096] = {0};
  size_t n_elves = 0;

  long *pos2elf;
  pos2elf = calloc(n_rows_max * n_cols_max, sizeof(size_t));
  memset(pos2elf, -1, n_rows_max * n_cols_max * sizeof(size_t));

  size_t start = N_STEPS + N_STEPS * n_cols_max;

  for (size_t i = 0; i < n_rows; i++) {
    for (size_t j = 0; j < n_cols; j++) {
      if (lines[i][j] == '#') {
        pos2elf[start + i * n_cols_max + j] = n_elves;
        elf2pos[n_elves] = start + i * n_cols_max + j;
        n_elves++;
      }
    }
  }

  printf("n_elves: %ld\n", n_elves);

  long *pos2newelf;
  pos2newelf = calloc(n_rows_max * n_cols_max, sizeof(size_t));

  size_t n_rounds = 0;

  for (size_t i = 0; i < N_STEPS; i++) {
    long elf2newpos[4096] = {[0 ... 4095] = -1};
    memset(pos2newelf, -1, n_rows_max * n_cols_max * sizeof(size_t));
    for (size_t j = 0; j < n_elves; j++) {
      size_t pos = elf2pos[j];

      // no elves around, don't move
      if ((pos2elf[pos + 1] == -1) && (pos2elf[pos - 1] == -1) &&
          (pos2elf[pos + n_cols_max] == -1) &&
          (pos2elf[pos - n_cols_max] == -1) &&
          (pos2elf[pos + 1 + n_cols_max] == -1) &&
          (pos2elf[pos - 1 - n_cols_max] == -1) &&
          (pos2elf[pos - 1 + n_cols_max] == -1) &&
          (pos2elf[pos + 1 - n_cols_max] == -1)) {
        continue;
      }

      for (size_t k = 0; k < 4; k++) {
        if (move((k + i) % 4, pos, pos2elf, elf2newpos, pos2newelf, n_cols_max,
                 j) == 0) {
          break;
        }
      }
    }

    bool moved = false;
    for (size_t j = 0; j < n_elves; j++) {
      if (elf2newpos[j] >= 0) {
        pos2elf[elf2pos[j]] = -1;
        elf2pos[j] = elf2newpos[j];
        pos2elf[elf2pos[j]] = j;
        moved = true;
      }
    }
    if (!moved) {
      n_rounds = i + 1;
      break;
    }

    if (i == N_STEPS_AREA - 1) {
      size_t min_x = n_cols_max;
      size_t max_x = 0;
      size_t min_y = n_rows_max;
      size_t max_y = 0;

      for (size_t k = 0; k < n_elves; k++) {
        size_t x = elf2pos[k] % n_cols_max;
        size_t y = elf2pos[k] / n_cols_max;

        if (x < min_x) {
          min_x = x;
        }
        if (x > max_x) {
          max_x = x;
        }
        if (y < min_y) {
          min_y = y;
        }
        if (y > max_y) {
          max_y = y;
        }
      }

      size_t area = (max_x - min_x + 1) * (max_y - min_y + 1);
      printf("area: %ld\n", area - n_elves);
    }
  }
  printf("n_rounds: %ld\n", n_rounds);

  fclose(fp);

  if (line)
    free(line);

  exit(EXIT_SUCCESS);

  return 0;
}
