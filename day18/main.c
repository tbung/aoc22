#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../lib/stack.h"

#define ARRAY_SIZE(arr) (sizeof((arr)) / sizeof((arr)[0]))
#define MAX_COORD 32

typedef struct {
  int x;
  int y;
  int z;
} coord_t;

const coord_t adjacent[] = {
    {-1, 0, 0}, {1, 0, 0}, {0, -1, 0}, {0, 1, 0}, {0, 0, -1}, {0, 0, 1},
};

coord_t coord_add(coord_t a, coord_t b) {
  a.x += b.x;
  a.y += b.y;
  a.z += b.z;

  return a;
}

typedef struct {
  coord_t *data;
  size_t size;
  size_t capacity;
} list_t;

list_t *list_new() {
  list_t *list = calloc(1, sizeof(list_t));
  list->size = 0;
  list->capacity = 2;
  list->data = calloc(list->capacity, sizeof(coord_t));

  return list;
}

void list_push(list_t *list, coord_t item) {

  if (list->size == list->capacity) {
    list->capacity *= 2;
    list->data = reallocarray(list->data, list->capacity, sizeof(coord_t));
    memset(list->data + list->size, 0, list->size);
  }

  list->data[list->size] = item;
  list->size++;
}

QUEUE(coord, coord_t, MAX_COORD *MAX_COORD *MAX_COORD)

int main(int argc, char *argv[]) {
  FILE *fp;
  char *line = NULL;
  size_t len = 0;

  char *filename = argv[1];

  fp = fopen(filename, "r");
  if (fp == NULL)
    exit(EXIT_FAILURE);

  bool box[MAX_COORD][MAX_COORD][MAX_COORD] = {0};
  list_t *coords = list_new();

  while (getline(&line, &len, fp) != -1) {
    char *strx, *stry, *strz;
    int x, y, z;

    strx = strtok(line, ",");
    stry = strtok(NULL, ",");
    strz = strtok(NULL, ",");

    x = atoi(strx);
    y = atoi(stry);
    z = atoi(strz);

    box[x][y][z] = true;
    coord_t coord = {x, y, z};
    list_push(coords, coord);
  }

  fclose(fp);

  if (line)
    free(line);

  size_t res = 0;
  for (int i = 0; i < coords->size; i++) {
    coord_t c = coords->data[i];
    for (int j = 0; j < ARRAY_SIZE(adjacent); j++) {
      coord_t a = coord_add(c, adjacent[j]);
      if ((a.x >= MAX_COORD) || (a.x < 0) || (a.y >= MAX_COORD) || (a.y < 0) ||
          (a.z >= MAX_COORD) || (a.z < 0)) {
        res++;
        continue;
      }
      if (!box[a.x][a.y][a.z])
        res++;
    }
  }

  printf("%ld\n", res);

  res = 0;
  queue_coord_t *queue = queue_coord_new();
  coord_t initial = {0, 0, 0};
  queue_coord_push(queue, initial);
  bool visited[MAX_COORD][MAX_COORD][MAX_COORD] = {0};
  while (!queue_coord_empty(queue)) {
    coord_t c = queue_coord_pop(queue);
    if (visited[c.x][c.y][c.z]) {
      continue;
    }
    visited[c.x][c.y][c.z] = true;
    for (int j = 0; j < ARRAY_SIZE(adjacent); j++) {
      coord_t a = coord_add(c, adjacent[j]);
      if ((a.x >= MAX_COORD) || (a.x < 0) || (a.y >= MAX_COORD) || (a.y < 0) ||
          (a.z >= MAX_COORD) || (a.z < 0)) {
        continue;
      }
      if (visited[a.x][a.y][a.z]) {
        continue;
      }
      if (box[a.x][a.y][a.z]) {
        res++;
      } else {
        queue_coord_push(queue, a);
      }
    }
  }

  printf("%ld\n", res);

  free(coords->data);
  free(coords);
  free(queue->data);
  free(queue);
  return 0;
}
