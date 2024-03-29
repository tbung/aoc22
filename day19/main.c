#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/param.h>

#include "../lib/stack.h"

#define MAX_BLUEPRINTS 32

bool is_integer(char *str) {
  char *c = str;
  while (*c != '\0') {
    if (*c < '0' || *c > '9') {
      return false;
    }
    c++;
  }
  return true;
}

typedef struct {
  size_t ore;
  size_t clay;
  size_t obsidian;
  size_t geode;
} resources_t;

typedef struct {
  int id;
  int quality;
  resources_t ore_bot;
  resources_t clay_bot;
  resources_t obsidian_bot;
  resources_t geode_bot;
} blueprint_t;

blueprint_t *blueprint_parse(char *line) {
  blueprint_t *blueprint = malloc(sizeof(blueprint_t));
  memset(blueprint, 0, sizeof(blueprint_t));

  char *token = strtok(line, " ");
  char *endptr;
  long count;
  int i = 0;

  int numbers[7] = {0};

  while (token != NULL) {
    count = strtol(token, &endptr, 10);
    if (endptr == token) {
    } else {
      numbers[i] = count;
      i++;
    }
    token = strtok(NULL, " ");
  }

  blueprint->id = numbers[0];
  blueprint->ore_bot.ore = numbers[1];
  blueprint->clay_bot.ore = numbers[2];
  blueprint->obsidian_bot.ore = numbers[3];
  blueprint->obsidian_bot.clay = numbers[4];
  blueprint->geode_bot.ore = numbers[5];
  blueprint->geode_bot.obsidian = numbers[6];

  return blueprint;
}

typedef struct {
  size_t ore;
  size_t clay;
  size_t obsidian;
  size_t geode;
} bots_t;

typedef struct inventory_t {
  resources_t resources;
  bots_t bots;
  size_t time;
  bool disallowed_ore;
  bool disallowed_clay;
  bool disallowed_obsidian;
  bool disallowed_geode;
  size_t bound;
} inventory_t;

inventory_t *inventory_new() {
  inventory_t *inventory = malloc(sizeof(inventory_t));
  memset(inventory, 0, sizeof(inventory_t));
  return inventory;
}

bool can_build_bot(resources_t *resources, resources_t *recipe) {
  return ((resources->ore >= recipe->ore) &&
          (resources->clay >= recipe->clay) &&
          (resources->obsidian >= recipe->obsidian) &&
          (resources->geode >= recipe->geode));
}

void build_bot(size_t *bot, resources_t *resources, resources_t *recipe) {
  resources->ore -= recipe->ore;
  resources->clay -= recipe->clay;
  resources->obsidian -= recipe->obsidian;
  resources->geode -= recipe->geode;
  *bot += 1;
}

void inventory_step(inventory_t *initial, inventory_t *next) {
  next->resources.ore += initial->bots.ore;
  next->resources.clay += initial->bots.clay;
  next->resources.obsidian += initial->bots.obsidian;
  next->resources.geode += initial->bots.geode;
  next->time -= 1;
}

STACK(inventory, inventory_t *, 64)

size_t inventory_bound(inventory_t *inventory) {
  size_t bound = inventory->bots.geode * inventory->time +
                 inventory->resources.geode +
                 (inventory->time * (inventory->time - 1)) / 2;

  return bound;
}

size_t branch_and_bound(blueprint_t *blueprint, size_t time) {
  inventory_t *initial = inventory_new();
  initial->bots.ore = 1;
  initial->time = time;
  stack_inventory_t *stack = stack_inventory_new();
  stack_inventory_push(stack, initial);

  const size_t max_ore_bots =
      MAX(blueprint->clay_bot.ore,
          MAX(blueprint->obsidian_bot.ore, blueprint->geode_bot.ore));
  const size_t max_clay_bots = blueprint->obsidian_bot.clay;
  const size_t max_obsidian_bots = blueprint->geode_bot.obsidian;

  size_t best = 0;
  inventory_t *inventory = inventory_new();

  while (!stack_inventory_empty(stack)) {
    inventory_t *top = stack_inventory_pop(stack);
    memcpy(inventory, top, sizeof(inventory_t));

    if (inventory->resources.geode > best) {
      best = inventory->resources.geode;
    }

    if (inventory->time == 0) {
      continue;
    }

    bool disallowed_ore = 0;
    bool disallowed_clay = 0;
    bool disallowed_obsidian = 0;
    bool disallowed_geode = 0;

    if (can_build_bot(&inventory->resources, &blueprint->geode_bot) &&
        !inventory->disallowed_geode) {
      disallowed_geode = true;

      inventory_t *next = stack->data[stack->top];
      if (next == NULL) {
        next = inventory_new();
        stack->data[stack->top] = next;
      }

      if (next != top) {
        memcpy(next, inventory, sizeof(inventory_t));
      }
      build_bot(&next->bots.geode, &next->resources, &blueprint->geode_bot);

      inventory_step(inventory, next);
      size_t bound = inventory_bound(next);
      if (bound <= best) {
        continue;
      }
      next->bound = bound;
      stack->top++;
      continue;
    }
    if (can_build_bot(&inventory->resources, &blueprint->obsidian_bot) &&
        inventory->bots.obsidian < max_obsidian_bots &&
        !inventory->disallowed_obsidian) {
      disallowed_obsidian = true;
      inventory_t *next = stack->data[stack->top];
      if (next == NULL) {
        next = inventory_new();
        stack->data[stack->top] = next;
      }
      if (next != top) {
        memcpy(next, inventory, sizeof(inventory_t));
      }
      build_bot(&next->bots.obsidian, &next->resources,
                &blueprint->obsidian_bot);

      inventory_step(inventory, next);
      size_t bound = inventory_bound(next);
      if (bound <= best) {
        continue;
      }
      next->bound = bound;
      stack->top++;
      continue;
    }

    if (can_build_bot(&inventory->resources, &blueprint->ore_bot) &&
        inventory->bots.ore < max_ore_bots && !inventory->disallowed_ore) {
      disallowed_ore = true;
      inventory_t *next = stack->data[stack->top];
      if (next == NULL) {
        next = inventory_new();
        stack->data[stack->top] = next;
      }
      if (next != top) {
        memcpy(next, inventory, sizeof(inventory_t));
      }
      build_bot(&next->bots.ore, &next->resources, &blueprint->ore_bot);

      inventory_step(inventory, next);
      size_t bound = inventory_bound(next);
      if (bound <= best) {
        continue;
      }
      next->bound = bound;
      stack->top++;
    }

    if (can_build_bot(&inventory->resources, &blueprint->clay_bot) &&
        inventory->bots.clay < max_clay_bots && !inventory->disallowed_clay) {
      disallowed_clay = true;
      inventory_t *next = stack->data[stack->top];
      if (next == NULL) {
        next = inventory_new();
        stack->data[stack->top] = next;
      }
      if (next != top) {
        memcpy(next, inventory, sizeof(inventory_t));
      }
      build_bot(&next->bots.clay, &next->resources, &blueprint->clay_bot);

      inventory_step(inventory, next);
      size_t bound = inventory_bound(next);
      if (bound <= best) {
        continue;
      }
      next->bound = bound;
      stack->top++;
    }

    inventory_t *next = stack->data[stack->top];
    if (next == NULL) {
      next = inventory_new();
      stack->data[stack->top] = next;
    }
    if (next != top) {
      memcpy(next, inventory, sizeof(inventory_t));
    }
    inventory_step(inventory, next);
    size_t bound = inventory_bound(next);
    if (bound <= best) {
      continue;
    }
    next->disallowed_ore = disallowed_ore;
    next->disallowed_clay = disallowed_clay;
    next->disallowed_obsidian = disallowed_obsidian;
    next->disallowed_geode = disallowed_geode;

    next->bound = bound;
    stack->top++;
  }
  free(stack);

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

  blueprint_t *blueprints[MAX_BLUEPRINTS] = {0};
  size_t n_blueprints = 0;

  while (getline(&line, &len, fp) != -1) {
    blueprints[n_blueprints] = blueprint_parse(line);
    n_blueprints++;
  }

  size_t total = 0;
  for (size_t i = 0; i < n_blueprints; i++) {
    size_t best = branch_and_bound(blueprints[i], 24);
    total += blueprints[i]->id * best;
  }
  printf("Part 1: %zu\n", total);

  total = 1;
  for (size_t i = 0; i < MIN(3, n_blueprints); i++) {
    size_t best = branch_and_bound(blueprints[i], 32);
    total *= best;
  }
  printf("Part 2: %zu\n", total);

  for (size_t i = 0; i < n_blueprints; i++) {
    free(blueprints[i]);
  }

  fclose(fp);

  if (line) {
    free(line);
  }

  exit(EXIT_SUCCESS);

  return 0;
}
