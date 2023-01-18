#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
  long x;
  long y;
} coord_t;

typedef struct {
  long start;
  long end;
} range_t;

typedef struct node_t {
  range_t *value;
  struct node_t *left;
  struct node_t *right;
} node_t;

int range_cmp(range_t *left, range_t *right) {
  if (left->start == right->start)
    return 0;
  if (left->start < right->start)
    return -1;
  return 1;
}

void tree_free(node_t *tree) {
  if (!tree)
    return;
  tree_free(tree->right);
  tree_free(tree->left);
  free(tree->value);
  free(tree);
}

void tree_insert(node_t **tree, range_t *value) {
  node_t *tmp = NULL;

  if (!(*tree)) {
    tmp = calloc(1, sizeof(node_t));
    tmp->value = value;
    tmp->left = NULL;
    tmp->right = NULL;
    *tree = tmp;
    return;
  }

  range_t *tvalue = (*tree)->value;

  if (value->end < tvalue->start) {
    tree_insert(&(*tree)->left, value);
  } else if (value->start > tvalue->end) {
    tree_insert(&(*tree)->right, value);
  } else if (value->start <= tvalue->start && value->end >= tvalue->end) {
    tvalue->start = value->start;
    tvalue->end = value->end;
    free(value);
  } else if (value->start >= tvalue->start && value->end <= tvalue->end) {
    free(value);
  } else if (value->start >= tvalue->start && value->end > tvalue->end) {
    tvalue->end = value->end;
    free(value);
  } else if (value->start < tvalue->start && value->end <= tvalue->end) {
    tvalue->start = value->start;
    free(value);
  }

  if ((*tree)->right) {
    if (tvalue->end >= (*tree)->right->value->start &&
        tvalue->end < (*tree)->right->value->end) {
      tvalue->end = (*tree)->right->value->end;
      tree_free((*tree)->right);
      (*tree)->right = NULL;
    }
  }
}

size_t tree_len(node_t **tree) {
  if (!(*tree))
    return 0;
  return 1 + tree_len(&(*tree)->left) + tree_len(&(*tree)->right);
}

size_t tree_size(node_t **tree) {
  if (!(*tree))
    return 0;
  return (*tree)->value->end - (*tree)->value->start +
         tree_size(&(*tree)->left) + tree_size(&(*tree)->right);
}

void tree_print(node_t *tree) {
  if (tree) {
    tree_print(tree->left);
    printf("[%d;%d]\n", tree->value->start, tree->value->end);
    tree_print(tree->right);
  }
}

int main(int argc, char *argv[]) {
  FILE *fp;
  char *line = NULL;
  size_t len = 0;
  ssize_t read;

  char *filename = argv[1];

  long y = 2000000;
  if (strcmp(filename, "test_in") == 0)
    y = 10;

  fp = fopen(filename, "r");
  if (fp == NULL)
    exit(EXIT_FAILURE);

  char *sensor_str, *beacon_str;
  coord_t sensor, beacon;
  long radius;
  node_t *root = NULL;
  range_t *range = NULL;
  // char dbg_str[40] = {[0 ... 39] = '.'};
  while ((read = getline(&line, &len, fp)) != -1) {
    sensor_str = strtok(line, ":");
    beacon_str = strtok(NULL, ":");

    sensor_str = strchr(sensor_str, 'x') + 2;
    sensor.x = atol(sensor_str);
    sensor_str = strchr(sensor_str, 'y') + 2;
    sensor.y = atol(sensor_str);

    // printf("Sensor at (%d,%d)\n", sensor.x, sensor.y);

    beacon_str = strchr(beacon_str, 'x') + 2;
    beacon.x = atoi(beacon_str);
    beacon_str = strchr(beacon_str, 'y') + 2;
    beacon.y = atoi(beacon_str);

    // printf("Beacon at (%d,%d)\n", beacon.x, beacon.y);

    radius = labs(sensor.x - beacon.x) + labs(sensor.y - beacon.y);
    // printf("Radius: %d\n", radius);

    if (labs(sensor.y - y) > radius)
      continue;

    range = calloc(1, sizeof(range_t));
    range->start = sensor.x - (radius - labs(sensor.y - y));
    range->end = sensor.x + (radius - labs(sensor.y - y));

    tree_insert(&root, range);
    // tree_print(root);
    range = NULL;
  }

  printf("%d\n", tree_size(&root));

  tree_free(root);

  fclose(fp);

  if (line)
    free(line);

  exit(EXIT_SUCCESS);

  return 0;
}
