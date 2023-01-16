#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define ITEM_SIZE 1024
// #define N_ROUNDS 20
#define N_ROUNDS 10000

struct monkey {
  size_t items[ITEM_SIZE];
  size_t item_start;
  size_t item_end;
  char op[128];
  unsigned int divisor;
  size_t if_true;
  size_t if_false;
  unsigned long counter;
};

struct monkey monkey_new() {
  struct monkey m = {.items = {0},
                     .item_end = -1,
                     .item_start = -1,
                     .op = {0},
                     .divisor = 0,
                     .if_true = 0,
                     .if_false = 0,
                     .counter = 0};
  return m;
}

bool monkey_is_empty(struct monkey *mnk) {
  return (mnk->item_start == -1) || (mnk->item_end == -1);
}

void monkey_push_back(struct monkey *mnk, size_t item) {
  if ((mnk->item_end != -1) && ((mnk->item_end + 1) % ITEM_SIZE == mnk->item_start)) {
    printf("overflow\n");
    exit(EXIT_FAILURE);
  }

  if (mnk->item_start == -1)
    mnk->item_start = 0;

  mnk->item_end = (mnk->item_end + 1) % ITEM_SIZE;
  mnk->items[mnk->item_end] = item;
}

size_t monkey_pop_front(struct monkey *mnk) {
  if (monkey_is_empty(mnk)) {
    printf("underflow\n");
    exit(EXIT_FAILURE);
  }
  size_t item = mnk->items[mnk->item_start];
  if (mnk->item_start == mnk->item_end) {
    mnk->item_start = -1;
    mnk->item_end = -1;
  }

  mnk->item_start = (mnk->item_start + 1) % ITEM_SIZE;

  mnk->counter++;
  return item;
}

size_t monkey_do_op(struct monkey *mnk, size_t worry_lvl) {
  char str[128] = {0};
  strcpy(str, mnk->op);

  char *a, *op, *b;
  size_t aa, bb;
  a = strtok(str, " ");
  op = strtok(NULL, " ");
  b = strtok(NULL, " ");

  if (strcmp(a, "old") == 0) {
    aa = worry_lvl;
  } else {
    aa = atol(a);
  }
  if (strcmp(b, "old") == 0) {
    bb = worry_lvl;
  } else {
    bb = atol(b);
  }

  switch (op[0]) {
  case '+':
    return aa + bb;
  case '*':
    return aa * bb;
  }
}

void print_monkeys(struct monkey *monkeys, size_t n) {
  for (int i = 0; i < n; i++) {
    printf("Monkey %d: (%lu) [", i, monkeys[i].counter);
    for (int j = monkeys[i].item_start;
         j <= monkeys[i].item_end && !(monkey_is_empty(&(monkeys[i]))); j++) {
      if (j > monkeys[i].item_start)
        printf(", ");
      printf("%lu", monkeys[i].items[j]);
    }
    printf("]\n");
    // printf("  Op: %s\n", monkeys[i].op);
    // printf("  Dv: %d\n", monkeys[i].divisor);
    // printf("  It: %d\n", monkeys[i].if_true);
    // printf("  If: %d\n", monkeys[i].if_false);
  }
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

  struct monkey monkeys[10];
  size_t monkey_ptr = -1;

  char *token, *subtoken, *str;

  while ((read = getline(&line, &len, fp)) != -1) {
    if (strncmp(line, "Monkey", 6) == 0) {
      monkey_ptr++;
      monkeys[monkey_ptr] = monkey_new();
    } else if (strncmp(line, "  Starting items", 16) == 0) {
      token = strtok(line, ":");
      token = strtok(NULL, ":");
      for (str = token;; str = NULL) {
        subtoken = strtok(str, ",");
        if (subtoken == NULL)
          break;
        monkey_push_back(&(monkeys[monkey_ptr]), atoi(subtoken));
      }
    } else if (strncmp(line, "  Operation", 11) == 0) {
      token = strtok(line, ": ");
      token = strtok(NULL, "=");
      token = strtok(NULL, "=") + 1;
      token = strtok(token, "\n");
      strncpy(monkeys[monkey_ptr].op, token, 128);
    } else if (strncmp(line, "  Test", 6) == 0) {
      token = strtok(line, ": ");
      token = strtok(NULL, ": ");
      token = strtok(NULL, ": ");
      token = strtok(NULL, ": ");
      monkeys[monkey_ptr].divisor = atoi(token);
    } else if (strncmp(line, "    If true", 11) == 0) {
      token = strtok(line, ": ");
      token = strtok(NULL, ": ");
      token = strtok(NULL, ": ");
      token = strtok(NULL, ": ");
      token = strtok(NULL, ": ");
      token = strtok(NULL, ": ");
      monkeys[monkey_ptr].if_true = atoi(token);
    } else if (strncmp(line, "    If false", 12) == 0) {
      token = strtok(line, ": ");
      token = strtok(NULL, ": ");
      token = strtok(NULL, ": ");
      token = strtok(NULL, ": ");
      token = strtok(NULL, ": ");
      token = strtok(NULL, ": ");
      monkeys[monkey_ptr].if_false = atoi(token);
    }
  }

  print_monkeys(monkeys, monkey_ptr + 1);
  printf("\n");

  struct monkey *mnk, *mnk2;
  size_t worry_lvl;
  size_t mod = 1;

  for (int monkey_idx = 0; monkey_idx < monkey_ptr + 1; monkey_idx++) {
    mod *= monkeys[monkey_idx].divisor;
  }

  for (int round = 0; round < N_ROUNDS; round++) {
    for (int monkey_idx = 0; monkey_idx < monkey_ptr + 1; monkey_idx++) {
      mnk = &(monkeys[monkey_idx]);
      while (!monkey_is_empty(mnk)) {
        worry_lvl = monkey_pop_front(mnk) % mod;
        // worry_lvl = monkey_do_op(mnk, worry_lvl) / 3;
        worry_lvl = monkey_do_op(mnk, worry_lvl) % mod;

        if (worry_lvl % mnk->divisor == 0) {
          mnk2 = &monkeys[mnk->if_true];
          monkey_push_back(mnk2, worry_lvl);
        } else {
          mnk2 = &monkeys[mnk->if_false];
          monkey_push_back(mnk2, worry_lvl);
        }
      }
    }

    if (((round + 1) % 1000 == 0) || (round <= 19)) {
      printf("Round %d:\n", round + 1);
      print_monkeys(monkeys, monkey_ptr + 1);
      printf("\n");
    }
  }

  unsigned long counts[2] = {0};
  for (int i = 0; i < monkey_ptr + 1; i++) {
    if (counts[0] <= counts[1] && monkeys[i].counter > counts[0]) {
      counts[0] = monkeys[i].counter;
    } else if (counts[1] < counts[0] && monkeys[i].counter > counts[1]) {
      counts[1] = monkeys[i].counter;
    }
  }

  printf("%lu\n", counts[0] * counts[1]);
  printf("%lu\n", mod);

  fclose(fp);

  if (line)
    free(line);

  exit(EXIT_SUCCESS);

  return 0;
}
