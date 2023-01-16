#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int cmpfunc(const void *a, const void *b) {
  // int x = (((int *)a)[0] - ((int *)b)[0]);
  if (((int *)a)[0] == ((int *)b)[0]) {
    return (((int *)a)[1] > ((int *)b)[1]   ? +1
            : ((int *)a)[1] < ((int *)b)[1] ? -1
                                            : 0);
  } else {
    // return x;
    return (((int *)a)[0] > ((int *)b)[0] ? +1 : -1);
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

  int tail[1024 * 10][2] = {0};
  size_t tail_ptr = 0;

  long head[2] = {0};

  char *token;
  char direction;
  int count;

  while ((read = getline(&line, &len, fp)) != -1) {
    token = strtok(line, " ");
    direction = token[0];
    token = strtok(NULL, " ");
    count = atoi(token);

    for (int i = 0; i < count; i++) {
      switch (direction) {
      case 'R':
        head[0] += 1;
        break;
      case 'L':
        head[0] -= 1;
        break;
      case 'U':
        head[1] += 1;
        break;
      case 'D':
        head[1] -= 1;
        break;
      }
      if (abs(head[0] - tail[tail_ptr][0]) >= 2) {
        tail[tail_ptr + 1][0] = tail[tail_ptr][0];
        tail[tail_ptr + 1][1] = tail[tail_ptr][1];
        if (head[0] > tail[tail_ptr][0]) {
          tail[tail_ptr + 1][0] += 1;
        } else {
          tail[tail_ptr + 1][0] -= 1;
        }
        if (head[1] > tail[tail_ptr][1]) {
          tail[tail_ptr + 1][1] += 1;
        }
        if (head[1] < tail[tail_ptr][1]) {
          tail[tail_ptr + 1][1] -= 1;
        }
        tail_ptr += 1;
      }
      if (abs(head[1] - tail[tail_ptr][1]) >= 2) {
        tail[tail_ptr + 1][0] = tail[tail_ptr][0];
        tail[tail_ptr + 1][1] = tail[tail_ptr][1];
        if (head[1] > tail[tail_ptr][1]) {
          tail[tail_ptr + 1][1] += 1;
        } else {
          tail[tail_ptr + 1][1] -= 1;
        }
        if (head[0] > tail[tail_ptr][0]) {
          tail[tail_ptr + 1][0] += 1;
        }
        if (head[0] < tail[tail_ptr][0]) {
          tail[tail_ptr + 1][0] -= 1;
        }
        tail_ptr += 1;
      }

      // for (int y = 5; y >= 0; y--) {
      //   for (int x = 0; x <= 5; x++) {
      //     if ((head[0] == x) && (head[1] == y)) {
      //       printf("H");
      //     } else if ((tail[tail_ptr][0] == x) && (tail[tail_ptr][1] == y)) {
      //       printf("T");
      //     } else {
      //       printf(".");
      //     }
      //   }
      //   printf("\n");
      // }
      // printf("\n");
    }
  }

  size_t total = 0;
  qsort(tail, tail_ptr + 1, sizeof(tail[0]), cmpfunc);
  for (int i = 0; i <= tail_ptr; i++) {
    printf("%d,%d\n", tail[i][0], tail[i][1]);

    if (i > 0) {
      if ((tail[i][0] == tail[i - 1][0]) && (tail[i][1] == tail[i - 1][1]))
        continue;
    }
    total += 1;
  }

  printf("%d\n", total);

  fclose(fp);

  if (line)
    free(line);

  exit(EXIT_SUCCESS);

  return 0;
}
