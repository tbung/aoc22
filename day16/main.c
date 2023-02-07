#include <limits.h>
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
  uint16_t visited_b;
  valve_t *valve;
  size_t minutes_remaining;
  size_t pressure_released;
  size_t bound;
} state_t;

state_t *state_new() {
  state_t *state = calloc(1, sizeof(state_t));
  state->valve = NULL;
  state->minutes_remaining = 0;
  state->pressure_released = 0;
  state->bound = 0;
  state->visited_b = 0;

  return state;
}

void state_free(state_t *state) { free(state); }

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

size_t state_bound(state_t *state, valve_t **valves_by_flowrate,
                   size_t n_valves_non_zero, size_t valve_ids[MAX_VALVES]) {
  size_t bound = state->pressure_released;
  int j = 0;
  long min = state->minutes_remaining - 2;
  while ((j < n_valves_non_zero) && (min >= 0)) {
    if ((state->visited_b & (1 << j)) != 0) {
      j++;
      continue;
    }

    bound += min * valves_by_flowrate[j]->rate;
    j++;
    min -= 2;
  }
  return bound;
}

size_t branch_and_bound(valve_t *valves[VALVES_SIZE], size_t n_valves,
                        size_t valve_ids[MAX_VALVES], size_t **distances,
                        bool with_elephant) {
  valve_t **valves_by_flowrate = calloc(n_valves, sizeof(valve_t *));
  memcpy(valves_by_flowrate, valves, sizeof(valve_t *) * n_valves);
  qsort(valves_by_flowrate, n_valves, sizeof(valve_t *), cmp_idx_by_flowrate);

  size_t n_valves_non_zero = 0;
  for (int i = 0; i < n_valves; i++) {
    if (valves_by_flowrate[i]->rate > 0) {
      n_valves_non_zero++;
    }
  }

  state_t *initial = state_new();
  initial->valve = valves[valve_name_idx("AA", valve_ids)];
  initial->minutes_remaining = with_elephant ? 26 : 30;

  initial->pressure_released = 0;
  size_t best = 0; // initially: assume we cannot release any steam
  stack_state_t *stack = stack_state_new();
  stack_state_push(stack, initial);

  size_t best_per_visited[UINT16_MAX] = {0};

  while (!stack_state_empty(stack)) {
    state_t *state = stack_state_pop(stack);

    if (state->pressure_released > best) {
      best = state->pressure_released;
    }

    if (with_elephant &&
        (state->pressure_released > best_per_visited[state->visited_b])) {
      best_per_visited[state->visited_b] = state->pressure_released;
    }

    for (int i = 0; i < n_valves_non_zero; i++) {
      if ((state->visited_b & (1 << i)) != 0) {
        continue;
      }
      size_t distance = distances[valve_idx(state->valve, valve_ids)]
                                 [valve_idx(valves_by_flowrate[i], valve_ids)];
      if (distance >= state->minutes_remaining) {
        continue;
      }
      state_t *next = state_new();
      next->valve = valves_by_flowrate[i];
      next->minutes_remaining = state->minutes_remaining - distance - 1;
      next->visited_b = state->visited_b | (1 << i);
      next->pressure_released = state->pressure_released +
                                next->minutes_remaining * next->valve->rate;

      size_t bound =
          state_bound(next, valves_by_flowrate, n_valves_non_zero, valve_ids);

      if (bound <= best) {
        state_free(next);
        continue;
      }

      next->bound = bound;

      stack_state_push(stack, next);
    }

    state_free(state);
  }
  free(valves_by_flowrate);
  free(stack);

  if (with_elephant) {
    // solutions are a combination of a 'visited' vector and its best score,
    // since that uniquely(ish) identifies the state that got us there
    // -> find two best (read: best in sum) 'visited', 'best score' combinations
    // (one for me, one for elephant)
    uint16_t best_non_zero[UINT16_MAX] = {0};
    size_t n_best_non_zero = 0;

    for (uint16_t i = 0; i < UINT16_MAX; i++) {
      if (best_per_visited[i] > 0) {
        best_non_zero[n_best_non_zero] = i;
        n_best_non_zero++;
      }
    }

    for (size_t i = 0; i < n_best_non_zero; i++) {
      uint16_t idx = best_non_zero[i];
      for (size_t j = i + 1; j < n_best_non_zero; j++) {
        uint16_t jdx = best_non_zero[j];
        if ((idx & jdx) != 0) {
          continue;
        }
        size_t score = best_per_visited[idx] + best_per_visited[jdx];
        if (score > best) {
          best = score;
        }
      }
    }
  }

  return best;
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
  regfree(&regex_rate);
  regfree(&regex_conn);

  size_t **distances = floyd_warshall(valves, n_valves, valve_ids);

  // BRANCH AND BOUND https://en.wikipedia.org/wiki/Branch_and_bound
  size_t best = branch_and_bound(valves, n_valves, valve_ids, distances, false);
  printf("%ld\n", best);

  best = branch_and_bound(valves, n_valves, valve_ids, distances, true);
  printf("%ld\n", best);

  for (int i = 0; i < n_valves; i++) {
    valve_free(valves[i]);
    free(distances[i]);
  }
  free(distances);

  return 0;
}
