#include <regex.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../lib/stack.h"

#define ARRAY_SIZE(arr) (sizeof((arr)) / sizeof((arr)[0]))
#define STACK_SIZE 128

static const char *const re_rate = "rate=([0-9]*);";
static const char *const re_conn = "[A-Z][A-Z]";

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

size_t getid(const valve_t *valves[64], size_t n_valves, char *name) {
  size_t left = 0;
  size_t right = n_valves;
  while (left < right) {
    size_t idx = (left + right) / 2;
    const valve_t *valve = valves[idx];
    int res = strncmp(name, valve->name, 3);
    if (res == 0) {
      return idx;
    } else if (res < 0) {
      right = idx;
    } else {
      left = idx + 1;
    }
  }

  printf("Not Found: %s", name);
  exit(EXIT_FAILURE);
}

typedef struct {
} state_t;

STACK(state, state_t *, STACK_SIZE)

int main(int argc, char *argv[]) {
  FILE *fp;
  char *line = NULL;
  size_t len = 0;

  char *filename = argv[1];

  fp = fopen(filename, "r");
  if (fp == NULL) {
    exit(EXIT_FAILURE);
  }

  regex_t regex_rate;
  regmatch_t pmatch_rate[2];
  regex_t regex_conn;
  regmatch_t pmatch_conn[1];

  if (regcomp(&regex_rate, re_rate, REG_NEWLINE | REG_EXTENDED)) {
    exit(EXIT_FAILURE);
  }
  if (regcomp(&regex_conn, re_conn, REG_NEWLINE | REG_EXTENDED)) {
    exit(EXIT_FAILURE);
  }

  valve_t *valves[64];
  size_t n_valves = 0;

  while (getline(&line, &len, fp) != -1) {
    if (regexec(&regex_rate, line, ARRAY_SIZE(pmatch_rate), pmatch_rate, 0)) {
      exit(EXIT_FAILURE);
    }
    if (regexec(&regex_conn, line, 1, pmatch_conn, 0)) {
      exit(EXIT_FAILURE);
    }
    char *name = calloc(3, sizeof(char));
    strncpy(name, line + pmatch_conn[0].rm_so, 2);

    valve_t *valve = valve_new();

    valve->name = name;
    valve->rate = atol(line + pmatch_rate[1].rm_so);

    char *str = line + pmatch_rate[0].rm_eo;

    while (regexec(&regex_conn, str, 1, pmatch_conn, 0) == 0) {
      name = calloc(3, sizeof(char));
      strncpy(name, str + pmatch_conn[0].rm_so, 2);
      valve_add_neighbor(valve, name);
      str = str + pmatch_conn[0].rm_eo;
    }

    valves[n_valves] = valve;
    n_valves++;
  }

  fclose(fp);

  if (line) {
    free(line);
  }

  // let's have an easy name to idx mapping
  qsort(&valves, n_valves, sizeof(valve_t *), cmpvalves);

  // FLOYD-WARSHALL
  // https://en.wikipedia.org/wiki/Floyd%E2%80%93Warshall_algorithm

  // init distances as infinity
  size_t **distances = calloc(n_valves, sizeof(size_t *));
  for (int i = 0; i < n_valves; i++) {
    distances[i] = calloc(n_valves, sizeof(size_t));
    for (int j = 0; j < n_valves; j++) {
      distances[i][j] = n_valves; // All valves should be reachable by going at
                                  // most through every other, also <<SIZE_MAX
    }
  }

  // distance to self and immediate neighbor
  for (int i = 0; i < n_valves; i++) {
    distances[i][i] = 0;
    for (int j = 0; j < valves[i]->n_neighbors; j++) {
      distances[i][getid(valves, n_valves, valves[i]->neighbors[j])] = 1;
    }
  }

  // find shortest distances
  for (int k = 0; k < n_valves; k++) {
    for (int i = 0; i < n_valves; i++) {
      for (int j = 0; j < n_valves; j++) {
        if (distances[i][j] > distances[i][k] + distances[k][j]) {
          distances[i][j] = distances[i][k] + distances[k][j];
        }
      }
    }
  }

  // for (int i = 0; i < n_valves; i++) {
  //   for (int j = 0; j < n_valves; j++) {
  //     printf("%2ld ", distances[i][j]);
  //   }
  //   printf("\n");
  // }

  // BRANCH AND BOUND https://en.wikipedia.org/wiki/Branch_and_bound
  stack_state_t *stack = stack_state_new();
  stack_state_push(stack);

  exit(EXIT_SUCCESS);

  return 0;
}
