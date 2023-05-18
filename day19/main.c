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

void print_resources(resources_t *resources) {
  printf("\t\tore: %d\n", resources->ore);
  printf("\t\tclay: %d\n", resources->clay);
  printf("\t\tobsidian: %d\n", resources->obsidian);
  printf("\t\tgeode: %d\n", resources->geode);
}

typedef struct {
  int id;
  int quality;
  resources_t ore_bot;
  resources_t clay_bot;
  resources_t obsidian_bot;
  resources_t geode_bot;
} blueprint_t;

void print_blueprint(blueprint_t *blueprint) {
  printf("Blueprint %d:\n", blueprint->id);
  printf("\tore_bot:\n");
  print_resources(&blueprint->ore_bot);
  printf("\tclay_bot:\n");
  print_resources(&blueprint->clay_bot);
  printf("\tobsidian_bot:\n");
  print_resources(&blueprint->obsidian_bot);
  printf("\tgeode_bot:\n");
  print_resources(&blueprint->geode_bot);
}

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
  struct inventory_t *parent;
} inventory_t;

inventory_t *inventory_new() {
  inventory_t *inventory = malloc(sizeof(inventory_t));
  memset(inventory, 0, sizeof(inventory_t));
  return inventory;
}

void print_inventory(inventory_t *inventory) {
  printf("Inventory:\n");
  printf("\tresources:\n");
  print_resources(&inventory->resources);
  printf("\tbots:\n");
  printf("\t\tore: %d\n", inventory->bots.ore);
  printf("\t\tclay: %d\n", inventory->bots.clay);
  printf("\t\tobsidian: %d\n", inventory->bots.obsidian);
  printf("\t\tgeode: %d\n", inventory->bots.geode);
  printf("\ttime: %d\n", inventory->time);
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

STACK(inventory, inventory_t *, 1024)

size_t inventory_bound(inventory_t *inventory) {
  size_t bound =
      inventory->bots.geode * inventory->time + inventory->resources.geode;

  for (size_t i = inventory->time; i > 0; i--) {
    bound += i;
  }

  return bound;
}

inventory_t *branch_and_bound(blueprint_t *blueprint) {
  inventory_t *initial = inventory_new();
  initial->bots.ore = 1;
  initial->time = 24;
  stack_inventory_t *stack = stack_inventory_new();
  stack_inventory_push(stack, initial);

  const size_t max_ore_bots =
      MAX(blueprint->clay_bot.ore,
          MAX(blueprint->obsidian_bot.ore, blueprint->geode_bot.ore));
  const size_t max_clay_bots = blueprint->obsidian_bot.clay;
  const size_t max_obsidian_bots = blueprint->geode_bot.obsidian;

  inventory_t *best = initial;

  while (!stack_inventory_empty(stack)) {
    inventory_t *inventory = stack_inventory_pop(stack);

    /* printf("\n"); */
    /* print_inventory(inventory); */

    if (inventory->resources.geode > best->resources.geode) {
      best = inventory;
    }

    if (inventory->time == 0) {
      /* free(inventory); */
      continue;
    }

    bool disallowed_ore = 0;
    bool disallowed_clay = 0;
    bool disallowed_obsidian = 0;
    bool disallowed_geode = 0;

    if (can_build_bot(&inventory->resources, &blueprint->geode_bot) &&
        !inventory->disallowed_geode) {
      disallowed_geode = true;
      inventory_t *next = inventory_new();
      memcpy(next, inventory, sizeof(inventory_t));
      next->parent = inventory;
      build_bot(&next->bots.geode, &next->resources, &blueprint->geode_bot);

      inventory_step(inventory, next);
      size_t bound = inventory_bound(next);
      if (bound <= best->resources.geode) {
        free(next);
        /* free(inventory); */
        continue;
      }
      stack_inventory_push(stack, next);
      /* free(inventory); */
      continue;
    }

    if (can_build_bot(&inventory->resources, &blueprint->ore_bot) &&
        inventory->bots.ore < max_ore_bots && !inventory->disallowed_ore) {
      disallowed_ore = true;
      inventory_t *next = inventory_new();
      memcpy(next, inventory, sizeof(inventory_t));
      next->parent = inventory;
      build_bot(&next->bots.ore, &next->resources, &blueprint->ore_bot);

      inventory_step(inventory, next);
      size_t bound = inventory_bound(next);
      if (bound <= best->resources.geode) {
        free(next);
        /* free(inventory); */
        continue;
      }
      stack_inventory_push(stack, next);
    }

    if (can_build_bot(&inventory->resources, &blueprint->clay_bot) &&
        inventory->bots.clay < max_clay_bots && !inventory->disallowed_clay) {
      disallowed_clay = true;
      inventory_t *next = inventory_new();
      memcpy(next, inventory, sizeof(inventory_t));
      next->parent = inventory;
      build_bot(&next->bots.clay, &next->resources, &blueprint->clay_bot);

      inventory_step(inventory, next);
      size_t bound = inventory_bound(next);
      if (bound <= best->resources.geode) {
        free(next);
        /* free(inventory); */
        continue;
      }
      stack_inventory_push(stack, next);
    }

    if (can_build_bot(&inventory->resources, &blueprint->obsidian_bot) &&
        inventory->bots.obsidian < max_obsidian_bots &&
        !inventory->disallowed_obsidian) {
      disallowed_obsidian = true;
      inventory_t *next = inventory_new();
      memcpy(next, inventory, sizeof(inventory_t));
      next->parent = inventory;
      build_bot(&next->bots.obsidian, &next->resources,
                &blueprint->obsidian_bot);

      inventory_step(inventory, next);
      size_t bound = inventory_bound(next);
      if (bound <= best->resources.geode) {
        free(next);
        /* free(inventory); */
        continue;
      }
      stack_inventory_push(stack, next);
    }

    inventory_t *next = inventory_new();
    memcpy(next, inventory, sizeof(inventory_t));
    next->parent = inventory;

    inventory_step(inventory, next);
    size_t bound = inventory_bound(next);
    if (bound <= best->resources.geode) {
      free(next);
      /* free(inventory); */
      continue;
    }
    next->disallowed_ore = disallowed_ore;
    next->disallowed_clay = disallowed_clay;
    next->disallowed_obsidian = disallowed_obsidian;
    next->disallowed_geode = disallowed_geode;

    stack_inventory_push(stack, next);

    /* free(inventory); */
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

  for (size_t i = 0; i < n_blueprints; i++) {
    /* print_blueprint(blueprints[i]); */
    inventory_t *inventory = branch_and_bound(blueprints[i]);
    size_t best = inventory->resources.geode;
    blueprints[i]->quality = blueprints[i]->id * best;
    /* while (inventory) { */
    /*   print_inventory(inventory); */
    /* free(inventory); */
    /*   inventory = inventory->parent; */
    /* } */
    /* printf("%zu\n", best); */
  }

  size_t total = 0;
  for (size_t i = 0; i < n_blueprints; i++) {
    total += blueprints[i]->quality;
    free(blueprints[i]);
  }
  printf("%zu\n", total);

  fclose(fp);

  if (line) {
    free(line);
  }

  exit(EXIT_SUCCESS);

  return 0;
}
