#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define ARRAY_SIZE 4096 + 1024
#define ENCRYPTION_KEY 811589153

typedef struct list_t {
  int64_t number;
  struct list_t *next;
  struct list_t *previous;
} list_t;

list_t *list_new(int64_t number) {
  list_t *list = malloc(sizeof(list_t));
  list->number = number;
  list->next = list;
  list->previous = list;
  return list;
}

void list_free(list_t *list) {
  list_t *next = list->next;
  while (next != list) {
    list_t *tmp = next->next;
    free(next);
    next = tmp;
  }
  free(list);
}

void print_list(list_t *list) {
  list_t *next = list->next;
  printf("%ld", list->number);
  while (next != list) {
    printf(", %ld", next->number);
    next = next->next;
  }
  printf("\n");
}

int64_t list_get(list_t *list, size_t n, size_t index) {
  list_t *current = list;
  index = index % n;
  bool invert = false;
  if (invert > n / 2) {
    invert = !invert;
    index = n - index;
  }
  if (invert) {
    for (size_t i = index; i > 0; i--) {
      current = current->previous;
    }
  } else {
    for (size_t i = 0; i < index; i++) {
      current = current->next;
    }
  }
  return current->number;
}

void mix(list_t *original[ARRAY_SIZE], size_t n_original, size_t n_iter) {
  list_t *current;
  list_t *next;
  list_t *previous;
#ifdef DEBUG
  list_t *start = original[0];

  print_list(start);
#endif

  for (size_t iter = 0; iter < n_iter; iter++) {

    for (size_t i = 0; i < n_original; i++) {

      current = original[i];

#ifdef DEBUG
      printf("current: %ld\n", current->number);

      if (start == current) {
        start = start->next;
      }
#endif

      next = current->next;
      previous = current->previous;

      // take out current
      next->previous = previous;
      previous->next = next;

      // insert current
      int64_t steps = labs(current->number) % (n_original - 1);

      bool invert = current->number < 0;
      if (steps > (n_original - 1) / 2) {
        invert = !invert;
        steps = (n_original - 1) - steps;
      }

#ifdef DEBUG
      printf("steps: %ld\n", steps);
#endif

      if (invert) {
        for (int j = steps; j > 0; j--) {
          previous = previous->previous;
        }
        next = previous->next;
      } else {
        for (int j = 0; j < steps; j++) {
          next = next->next;
        }
        previous = next->previous;
      }
      previous->next = current;
      next->previous = current;
      current->next = next;
      current->previous = previous;

#ifdef DEBUG
      print_list(start);
      printf("\n");
#endif
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

  list_t *original[ARRAY_SIZE] = {0};
  list_t _original[ARRAY_SIZE] = {0};
  size_t n_original = 0;

  list_t *original_enc[ARRAY_SIZE] = {0};
  list_t _original_enc[ARRAY_SIZE] = {0};
  size_t n_original_enc = 0;

  list_t *start = NULL;
  list_t *start_enc = NULL;

  while (getline(&line, &len, fp) != -1) {
    _original[n_original].number = atoll(line);
    original[n_original] = &_original[n_original];

    if (n_original > 0) {
      original[n_original - 1]->next = original[n_original];
      original[n_original]->previous = original[n_original - 1];
    }

    if (original[n_original]->number == 0) {
      start = original[n_original];
    }

    n_original++;

    _original_enc[n_original_enc].number = atoll(line) * ENCRYPTION_KEY;
    original_enc[n_original_enc] = &_original_enc[n_original_enc];

    if (n_original_enc > 0) {
      original_enc[n_original_enc - 1]->next = original_enc[n_original_enc];
      original_enc[n_original_enc]->previous = original_enc[n_original_enc - 1];
    }

    if (original_enc[n_original_enc]->number == 0) {
      start_enc = original_enc[n_original_enc];
    }

    n_original_enc++;
  }

  original[0]->previous = original[n_original - 1];
  original[n_original - 1]->next = original[0];

  original_enc[0]->previous = original_enc[n_original_enc - 1];
  original_enc[n_original_enc - 1]->next = original_enc[0];

  fclose(fp);

  if (line)
    free(line);

  mix(original, n_original, 1);
  int64_t one = list_get(start, n_original, 1000);
  int64_t two = list_get(start, n_original, 2000);
  int64_t three = list_get(start, n_original, 3000);

#ifdef DEBUG
  printf("1000th: %ld\n", one);
  printf("2000th: %ld\n", two);
  printf("3000th: %ld\n", three);
  printf("\n");
#endif

  printf("Part 1: %ld\n", one + two + three);

  mix(original_enc, n_original_enc, 10);
  one = list_get(start_enc, n_original_enc, 1000);
  two = list_get(start_enc, n_original_enc, 2000);
  three = list_get(start_enc, n_original_enc, 3000);

#ifdef DEBUG
  printf("1000th: %ld\n", one);
  printf("2000th: %ld\n", two);
  printf("3000th: %ld\n", three);
  printf("\n");
#endif

  printf("Part 2: %ld\n", one + two + three);

  exit(EXIT_SUCCESS);

  return 0;
}
