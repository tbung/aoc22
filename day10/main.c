#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int crt_x(size_t counter) { return counter % 40; }
int crt_y(size_t counter) { return floor((float)counter / 40); }

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

  int x = 1;
  size_t counter = 1;
  int total = 0;
  char crt[40 * 6] = {'.'};
  int v;

  // printf("(c=%d, x=%d, t=%d)\n", counter, x, total);
  while ((read = getline(&line, &len, fp)) != -1) {
    // printf("(c=%d, x=%d, t=%d)\n", counter, x, crt_x(counter - 1));
    // for (int j = 0; j < 40; j++) {
    //   printf("%c", crt[crt_y(counter - 1) * 40 + j]);
    // }
    // printf("\n");
    if ((counter + 20) % 40 == 0) {
      total += x * counter;
    }

    if (strncmp(line, "addx", 4) == 0) {
      strtok(line, " ");
      v = atoi(strtok(NULL, " "));
      counter++;
      if ((counter + 20) % 40 == 0) {
        total += x * counter;
      }
      if ((x - 1 <= crt_x(counter - 1)) && (crt_x(counter - 1) <= x + 1)) {
        crt[counter - 1] = '#';
      } else {
        crt[counter - 1] = '.';
      }
      // printf("(c=%d, x=%d, t=%d)\n", counter, x, crt_x(counter - 1));
      // for (int j = 0; j < 40; j++) {
      //   printf("%c", crt[crt_y(counter - 1) * 40 + j]);
      // }
      // printf("\n");
      x += v;
      counter++;
    } else { // noop
      counter++;
    }

    // printf("(c=%d, x=%d, t=%d)\n", counter, x, total);
    if ((x - 1 <= crt_x(counter - 1)) && (crt_x(counter - 1) <= x + 1)) {
      crt[counter - 1] = '#';
    } else {
      crt[counter - 1] = '.';
    }
  }

  printf("(c=%d, x=%d, t=%d)\n", counter, x, total);

  for (int i = 0; i < 6; i++) {
    for (int j = 0; j < 40; j++) {
      printf("%c", crt[i * 40 + j]);
    }
    printf("\n");
  }

  fclose(fp);

  if (line)
    free(line);

  exit(EXIT_SUCCESS);

  return 0;
}
