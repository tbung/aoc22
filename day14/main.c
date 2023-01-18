#include <limits.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define DEBUG 0
#define GRID_WIDTH 1024

typedef enum { AIR, ROCK, SAND } block_t;

typedef struct {
  int x;
  int y;
} coord_t;

typedef struct path_t {
  coord_t *coord;
  struct path_t *next;
} path_t;

void grid_print(block_t grid[GRID_WIDTH][GRID_WIDTH], coord_t *offset,
                coord_t *max) {
  for (int y = 0; y <= max->y - offset->y; y++) {
    printf("%2d ", y);
    for (int x = 0; x <= max->x - offset->x; x++) {
      switch (grid[x][y]) {
      case AIR:
        printf(".");
        break;
      case ROCK:
        printf("#");
        break;
      case SAND:
        printf("o");
        break;
      }
    }
    printf("\n");
  }
}

coord_t *coord_new() { return calloc(1, sizeof(coord_t)); }

path_t *path_new() {
  path_t *new = calloc(1, sizeof(path_t));
  new->coord = coord_new();
  new->next = NULL;

  return new;
}

void path_free(path_t *path) {
  path_t *current = path;

  while (current) {
    path_t *next = current->next;
    free(current->coord);
    free(current);
    current = next;
  }
}

path_t *path_parse(char *string, coord_t *offset, coord_t *max) {
  path_t *root;
  path_t *prev = NULL;
  path_t *current;

  char *str1, *token, *subtoken;
  char *saveptr;
  int x, y;

  for (str1 = string;; str1 = NULL) {
    token = strtok_r(str1, " -> ", &saveptr);
    if (token == NULL)
      break;

    current = path_new();

    x = atoi(strtok(token, ","));
    y = atoi(strtok(NULL, ","));

    current->coord->x = x;
    current->coord->y = y;

    if (x < offset->x)
      offset->x = x;
    if (x > max->x)
      max->x = x;
    if (y > max->y)
      max->y = y;

    if (prev == NULL) {
      root = current;
    } else {
      prev->next = current;
    }

    prev = current;
  }

  return root;
}

void path_print(path_t *path) {
  for (path_t *current = path; current != NULL; current = current->next) {
    if (current != path)
      printf("->");

    printf("(%d,%d)", current->coord->x, current->coord->y);
  }
  printf("\n");
}

coord_t *absolute_to_relative(coord_t *coord, coord_t *offset) {
  coord_t *rel = coord_new();
  rel->x = coord->x - offset->x;
  rel->y = coord->y - offset->y;
  return rel;
}

int main(int argc, char *argv[]) {
  FILE *fp;
  char *line = NULL;
  size_t len = 0;
  ssize_t read;

  char *filename = argv[1];

  fp = fopen(filename, "r");
  if (fp == NULL)
    exit(EXIT_FAILURE);

  coord_t offset = {.x = INT_MAX, .y = 0};
  coord_t max = {.x = 0, .y = 0};

  path_t *paths[256];
  size_t paths_idx = 0;

  while ((read = getline(&line, &len, fp)) != -1) {
    line[strcspn(line, "\n")] = 0;
    // Each line is one path
    // For each coordinate we parse we might have to update offset and max
    paths[paths_idx] = path_parse(line, &offset, &max);
    paths_idx++;
  }

  fclose(fp);

  if (line)
    free(line);

  block_t grid[GRID_WIDTH][GRID_WIDTH] = {AIR};
  max.y = max.y + 2;
  offset.x = offset.x - max.y;
  for (int x = 0; x < GRID_WIDTH; x++) {
    grid[x][max.y] = ROCK;
  }
  max.x = max.x + max.y;

#if DEBUG
  for (int i = 0; i < paths_idx; i++) {
    path_print(paths[i]);
  }

  printf("Offset: (%d,%d)\n", offset.x, offset.y);
  printf("Max: (%d,%d)\n", max.x, max.y);
#endif

  for (int i = 0; i < paths_idx; i++) {
    for (path_t *current = paths[i]; current->next != NULL;
         current = current->next) {
      int cur_x, next_x, cur_y, next_y;

      cur_x = current->coord->x;
      next_x = current->next->coord->x;
      if (cur_x > next_x) {
        int tmp = cur_x;
        cur_x = next_x;
        next_x = tmp;
      }

      for (int x = cur_x; x <= next_x; x++) {
        grid[x - offset.x][current->coord->y] = ROCK;
      }

      cur_y = current->coord->y;
      next_y = current->next->coord->y;
      if (cur_y > next_y) {
        int tmp = cur_y;
        cur_y = next_y;
        next_y = tmp;
      }

      for (int y = cur_y; y <= next_y; y++) {
        grid[current->coord->x - offset.x][y] = ROCK;
      }
    }
  }

  for (int i = 0; i < paths_idx; i++) {
    path_free(paths[i]);
  }

  coord_t source = {.x = 500 - offset.x, .y = 0};
  coord_t sand = source;
  block_t below = AIR, left = AIR, right = AIR;
  size_t count = 0;

  while (true) {
    // if (sand.y > max.y - offset.y)
    //   break;

    const int x = sand.x;
    const int y = sand.y;
    below = grid[x][y + 1];
    left = grid[x - 1][y + 1];
    right = grid[x + 1][y + 1];

    if (below == AIR) {
      sand.y++;
    } else if (left == AIR) {
      sand.x--;
      sand.y++;
    } else if (right == AIR) {
      sand.x++;
      sand.y++;
    } else {
      count++;
      grid[x][y] = SAND;
      if (sand.x == source.x && sand.y == source.y) {
        grid[sand.x][sand.y] = SAND;
        break;
      }
      sand = source;
#if DEBUG
      grid_print(grid, &offset, &max);
      printf("\n");
#endif
    }
  }

  printf("%d\n", count);

  exit(EXIT_SUCCESS);

  return 0;
}
