#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// compare function, compares two elements
int compare(const void *num1, const void *num2) {
  if (*(char *)num1 > *(char *)num2)
    return 1;
  else
    return -1;
}

char find_dupe(char *line, size_t len) {
  size_t compsize = len / 2 * sizeof(char);
  char *comp1 = malloc(compsize);
  char *comp2 = malloc(compsize);

  strncpy(comp1, line, len / 2);
  strncpy(comp2, line + len / 2, len / 2);

  qsort(comp1, len / 2, sizeof(char), compare);
  qsort(comp2, len / 2, sizeof(char), compare);

  // printf("%s\n", line);
  // printf("%s\n", comp1);
  // printf("%s\n\n", comp2);

  char c1 = 0;
  char c2 = 0;

  for (int i = 0; i < len / 2; i++) {
    if (comp1[i] == c1)
      continue;
    c1 = comp1[i];

    // printf("Checking %c\n", c1);

    c2 = 0;
    for (int j = 0; j < len / 2; j++) {

      if (comp2[j] == c2)
        continue;
      c2 = comp2[j];
      // printf("Against %c\n", c2);

      if (c1 < c2) {
        // strings are sorted, so there is no chance of a match from here ->
        // break inner loop
        // printf("Breaking\n");
        break;
      } else if (c1 > c2) {
        // chars are not equal, move along
        // printf("Continuing\n");
        continue;
      } else {
        // found match, should only ever be one
        // printf("Matching\n");
        free(comp1);
        free(comp2);
        return c1;
      }
    }
  }

  printf("No match found: %s %s\n", comp1, comp2);
  free(comp1);
  free(comp2);
  return -1;
}

char find_batch(char *group[]) {
  int len0 = strlen(group[0]);
  int len1 = strlen(group[1]);
  int len2 = strlen(group[2]);

  qsort(group[0], len0, sizeof(char), compare);
  qsort(group[1], len1, sizeof(char), compare);
  qsort(group[2], len2, sizeof(char), compare);

  char c0 = 0;
  char c1 = 0;
  char c2 = 0;

  for (int i = 0; i < len0; i++) {
    if (group[0][i] == c0)
      continue;
    c0 = group[0][i];
    if (!isalnum(c0))
      continue;
    c1 = 0;
    for (int j = 0; j < len1; j++) {
      if (group[1][j] == c1)
        continue;
      c1 = group[1][j];
      if (!isalnum(c1))
        continue;
      if (c0 == c1) {
        c2 = 0;
        for (int k = 0; k < len2; k++) {
          if (group[2][k] == c2)
            continue;
          c2 = group[2][k];
          if (!isalnum(c2))
            continue;

          if (c1 == c2) {
            return c1;
          } else if (c1 > c2) {
            continue;
          } else {
            break;
          }
        }
      } else if (c0 > c1) {
        continue;
      } else {
        break;
      }
    }
  }
  printf("No match found: %s %s %s\n", group[0], group[1], group[2]);
  return -1;
}

int score(char c) {
  if (islower(c)) {
    return c - 'a' + 1;
  } else {
    return c - 'A' + 27;
  }
}

int main(int argc, char *argv[]) {
  FILE *fp;
  int n_lines = 0;
  char *line = NULL;
  size_t len = 0;
  ssize_t read;

  char *filename = argv[1];

  fp = fopen(filename, "r");
  if (fp == NULL)
    exit(EXIT_FAILURE);

  char c;
  int total = 0;
  char *group[3] = {malloc(1024 * sizeof(char)), malloc(1024 * sizeof(char)),
                    malloc(1024 * sizeof(char))};
  int counter = 0;

  while ((read = getline(&line, &len, fp)) != -1) {
    // c = find_dupe(line, read);
    // if (c == -1)
    //   continue;
    // printf("%c (%d)\n", c, score(c));
    // total += score(c);
    strcpy(group[counter % 3], line);

    if (counter % 3 == 2) {
      c = find_batch(group);
      printf("%c (%d)\n", c, score(c));
      total += score(c);
    }

    counter++;
  }

  printf("%d\n", total);

  fclose(fp);

  if (line)
    free(line);

  exit(EXIT_SUCCESS);
}
