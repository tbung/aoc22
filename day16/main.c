#include <regex.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../lib/stack.h"
#include "valve.h"

#define ARRAY_SIZE(arr) (sizeof((arr)) / sizeof((arr)[0]))
#define STACK_SIZE 1024L * 1024L
#define VALVES_SIZE 64

static const char *const re_rate = "rate=([0-9]*);";
static const char *const re_conn = "[A-Z][A-Z]";

size_t **floyd_warshall(valve_t *valves[], size_t n_valves,
                        size_t valve_ids[MAX_VALVES]) {
  // FLOYD-WARSHALL
  // https://en.wikipedia.org/wiki/Floyd%E2%80%93Warshall_algorithm
  size_t **distances = calloc(n_valves, sizeof(size_t *));

  // init distances as infinity
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
      distances[i][valve_name_idx(valves[i]->neighbors[j], valve_ids)] = 1;
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

  return distances;
}

typedef struct state_t {
  bool visited[VALVES_SIZE];
  valve_t *valve;
  size_t minutes_remaining;
  size_t pressure_released;
  size_t bound;
  struct state_t *parent;
} state_t;

state_t *state_new() {
  state_t *state = calloc(1, sizeof(state_t));
  state->valve = NULL;
  state->minutes_remaining = 0;
  state->pressure_released = 0;
  state->bound = 0;
  state->parent = NULL;

  return state;
}

STACK(state, state_t *, STACK_SIZE)

int cmp_idx_by_flowrate(const void *valve_a, const void *valve_b) {
  size_t rate_a;
  size_t rate_b;
  rate_a = (*((valve_t **)valve_a))->rate;
  rate_b = (*((valve_t **)valve_b))->rate;
  if (rate_a == rate_b) {
    return 0;
  }
  if (rate_a > rate_b) {
    return -1;
  }
  return 1;
}

int cmpstate(const void *state_a, const void *state_b) {
  size_t bound_a;
  size_t bound_b;
  bound_a = (*((state_t **)state_a))->bound;
  bound_b = (*((state_t **)state_b))->bound;
  if (bound_a == bound_b) {
    return 0;
  }
  if (bound_a > bound_b) {
    return -1;
  }
  return 1;
}

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

  valve_t *valves[VALVES_SIZE] = {0};
  size_t valve_ids[MAX_VALVES] = {0};
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

    valve_ids[valve_id(valve)] = n_valves;
    valves[n_valves] = valve;
    n_valves++;
  }

  fclose(fp);

  if (line) {
    free(line);
  }

  size_t **distances = floyd_warshall(valves, n_valves, valve_ids);

  // BRANCH AND BOUND https://en.wikipedia.org/wiki/Branch_and_bound
  valve_t **valves_by_flowrate = calloc(n_valves, sizeof(valve_t *));
  memcpy(valves_by_flowrate, valves, sizeof(valve_t *) * n_valves);
  qsort(valves_by_flowrate, n_valves, sizeof(valve_t *), cmp_idx_by_flowrate);

  state_t *initial = state_new();
  initial->valve = valves[valve_name_idx("AA", valve_ids)];
  initial->visited[valve_name_idx("AA", valve_ids)] = true;
  initial->minutes_remaining = 30;
  initial->pressure_released = 0;
  initial->parent = NULL;
  state_t *best = initial; // initially: assume we cannot release any steam
  stack_state_t *stack = stack_state_new();
  stack_state_push(stack, initial);

  while (!stack_state_empty(stack)) {
    state_t *state = stack_state_pop(stack);
    // printf("%ld\n", state->pressure_released);

    if (state->pressure_released > best->pressure_released) {
      best = state;
    }

    for (int i = 0; i < n_valves; i++) {
      if ((valves[i]->rate == 0) || (state->visited[i])) {
        continue;
      }
      size_t distance = distances[valve_idx(state->valve, valve_ids)][i];
      if (distance >= state->minutes_remaining) {
        continue;
      }
      state_t *next = state_new();
      next->valve = valves[i];
      next->minutes_remaining = state->minutes_remaining - distance - 1;
      memcpy(next->visited, state->visited, n_valves * sizeof(bool));
      next->visited[i] = true;
      next->pressure_released = state->pressure_released +
                                next->minutes_remaining * next->valve->rate;
      next->parent = state;

      size_t bound = next->pressure_released;
      int j = 0;
      long min = next->minutes_remaining - 2;
      while ((j < n_valves) && (min >= 0)) {
        if (next->visited[valve_idx(valves_by_flowrate[j], valve_ids)]) {
          j++;
          continue;
        }

        bound += min * valves_by_flowrate[j]->rate;
        j++;
        min -= 2;
      }

      if (bound <= best->pressure_released) {
        continue;
      }

      next->bound = bound;

      stack_state_push(stack, next);
    }
  }

  printf("%ld\n", best->pressure_released);

  return 0;
}
