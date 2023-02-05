#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_VALVES 26l * 26l

typedef struct {
  char *name;
  size_t rate;
  char **neighbors;
  size_t n_neighbors;
  size_t capacity;
} valve_t;

valve_t *valve_new() {
  valve_t *valve = calloc(1, sizeof(valve_t));
  valve->name = calloc(3, sizeof(char));
  valve->capacity = 1;
  valve->n_neighbors = 0;
  valve->neighbors = calloc(valve->capacity, sizeof(char *));
  valve->rate = 0;

  return valve;
}

void valve_add_neighbor(valve_t *valve, char *neighbor) {
  valve->neighbors[valve->n_neighbors] = neighbor;
  valve->n_neighbors++;

  if (valve->n_neighbors == valve->capacity) {
    valve->capacity *= 2;
    valve->neighbors =
        reallocarray(valve->neighbors, valve->capacity, sizeof(char *));
    if (valve->neighbors == NULL) {
      exit(EXIT_FAILURE);
    }
  }
}

void valve_print(valve_t *valve) {
  printf("Name: %s\n", valve->name);
  printf("Rate: %ld\n", valve->rate);
  for (int i = 0; i < valve->n_neighbors; i++) {
    if (i > 0) {
      printf(", ");
    }
    printf("%s", valve->neighbors[i]);
  }
  printf("\n");
}

int cmpvalves(const void *valve1, const void *valve2) {
  return strncmp(((*(valve_t **)valve1))->name, (*(valve_t **)valve2)->name, 2);
}

size_t valve_name_to_id(const char *name) {
  size_t i = name[0] - 'A';
  size_t j = name[1] - 'A';

  return i * 26 + j;
}

size_t valve_id(const valve_t *valve) { return valve_name_to_id(valve->name); }

size_t valve_idx(const valve_t *valve, const size_t valve_ids[MAX_VALVES]) {
  return valve_ids[valve_id(valve)];
}
size_t valve_name_idx(const char *name, const size_t valve_ids[MAX_VALVES]) {
  return valve_ids[valve_name_to_id(name)];
}
