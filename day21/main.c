#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define ARRAY_SIZE 2048

typedef struct monkey_t {
  char name[5];
  enum {
    NUMBER,
    PLUS,
    MINUS,
    MULT,
    DIVIDE,
  } op;
  union {
    int64_t number;
    struct operands_t {
      struct monkey_t *left;
      struct monkey_t *right;
    } operands;
  } data;
  bool depends_on_humn;
} monkey_t;

int64_t monkey_eval(monkey_t *monkey) {
  switch (monkey->op) {
  case NUMBER:
    return monkey->data.number;
  case PLUS:
    return monkey_eval(monkey->data.operands.left) +
           monkey_eval(monkey->data.operands.right);
  case MINUS:
    return monkey_eval(monkey->data.operands.left) -
           monkey_eval(monkey->data.operands.right);
  case MULT:
    return monkey_eval(monkey->data.operands.left) *
           monkey_eval(monkey->data.operands.right);
  case DIVIDE:
    return monkey_eval(monkey->data.operands.left) /
           monkey_eval(monkey->data.operands.right);
  }
}

monkey_t *monkey_by_name(monkey_t *monkeys, size_t *n_monkeys, char *name) {
  for (size_t i = 0; i < *n_monkeys; i++) {
    if (strcmp(monkeys[i].name, name) == 0) {
      return &monkeys[i];
    }
  }
  (*n_monkeys)++;
  memcpy(monkeys[*n_monkeys - 1].name, name, 5);
  return &monkeys[*n_monkeys - 1];
}

void monkey_print(monkey_t *monkey) {
  switch (monkey->op) {
  case NUMBER:
    printf("%ld", monkey->data.number);
    break;
  case PLUS:
    printf("(");
    monkey_print(monkey->data.operands.left);
    printf(" + ");
    monkey_print(monkey->data.operands.right);
    printf(")");
    break;
  case MINUS:
    printf("(");
    monkey_print(monkey->data.operands.left);
    printf(" - ");
    monkey_print(monkey->data.operands.right);
    printf(")");
    break;
  case MULT:
    printf("(");
    monkey_print(monkey->data.operands.left);
    printf(" * ");
    monkey_print(monkey->data.operands.right);
    printf(")");
    break;
  case DIVIDE:
    printf("(");
    monkey_print(monkey->data.operands.left);
    printf(" / ");
    monkey_print(monkey->data.operands.right);
    printf(")");
    break;
  }
}

bool monkey_depends_on(monkey_t *monkey, monkey_t *humn) {
  if (monkey == humn) {
    return true;
  }
  switch (monkey->op) {
  case NUMBER:
    return false;
  case PLUS:
  case MINUS:
  case MULT:
  case DIVIDE:
    return monkey_depends_on(monkey->data.operands.left, humn) ||
           monkey_depends_on(monkey->data.operands.right, humn);
  }
}

// find humn such that monkey_eval(monkey) == value
int64_t monkey_solve(monkey_t *monkey, monkey_t *humn, int64_t value) {
  if (monkey == humn) {
    return value;
  }

  if (monkey_depends_on(monkey->data.operands.left, humn)) {
    switch (monkey->op) {
    case NUMBER:
      assert(false);
    case PLUS:
      return monkey_solve(monkey->data.operands.left, humn,
                          value - monkey_eval(monkey->data.operands.right));
    case MINUS:
      return monkey_solve(monkey->data.operands.left, humn,
                          value + monkey_eval(monkey->data.operands.right));
    case MULT:
      return monkey_solve(monkey->data.operands.left, humn,
                          value / monkey_eval(monkey->data.operands.right));
    case DIVIDE:
      return monkey_solve(monkey->data.operands.left, humn,
                          value * monkey_eval(monkey->data.operands.right));
    }
  } else {
    switch (monkey->op) {
    case NUMBER:
      assert(false);
    case PLUS:
      return monkey_solve(monkey->data.operands.right, humn,
                          value - monkey_eval(monkey->data.operands.left));
    case MINUS:
      return monkey_solve(monkey->data.operands.right, humn,
                          monkey_eval(monkey->data.operands.left) - value);
    case MULT:
      return monkey_solve(monkey->data.operands.right, humn,
                          value / monkey_eval(monkey->data.operands.left));
    case DIVIDE:
      return monkey_solve(monkey->data.operands.right, humn,
                          monkey_eval(monkey->data.operands.left) / value);
    }
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

  monkey_t monkeys[ARRAY_SIZE] = {0};
  size_t n_monkeys = 0;

  monkey_t *root = NULL;
  monkey_t *humn = NULL;

  while (getline(&line, &len, fp) != -1) {
    char *tokens[4] = {0};
    size_t n_tokens = 0;

    char *token = strtok(line, " ");

    while (token != NULL) {
      tokens[n_tokens] = token;
      n_tokens++;
      token = strtok(NULL, " ");
    }

    char *name = strsep(&tokens[0], ":");
    monkey_t *monkey = monkey_by_name(monkeys, &n_monkeys, name);

    if (strcmp(name, "root") == 0) {
      root = monkey;
    } else if (strcmp(name, "humn") == 0) {
      humn = monkey;
    }

    if (n_tokens == 2) {
      monkey->op = NUMBER;
      monkey->data.number = atol(tokens[1]);
    } else if (n_tokens == 4) {
      switch (*tokens[2]) {
      case '+':
        monkey->op = PLUS;
        break;
      case '-':
        monkey->op = MINUS;
        break;
      case '*':
        monkey->op = MULT;
        break;
      case '/':
        monkey->op = DIVIDE;
        break;
      }
      monkey->data.operands.left =
          monkey_by_name(monkeys, &n_monkeys, tokens[1]);
      monkey->data.operands.right =
          monkey_by_name(monkeys, &n_monkeys, strsep(&tokens[3], "\n"));
    }
  }

  fclose(fp);

  if (line)
    free(line);

  printf("Part 1: %ld\n", monkey_eval(root));
  root->op = MINUS;
  printf("Part 2: %ld\n", monkey_solve(root, humn, 0));

  exit(EXIT_SUCCESS);

  return 0;
}
