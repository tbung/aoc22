#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
  long x;
  long y;
} coord_t;

typedef struct {
  coord_t sensor;
  long radius;
} pair_t;

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
    printf("[%ld;%ld]\n", tree->value->start, tree->value->end);
    tree_print(tree->right);
  }
}

typedef struct {
  coord_t start;
  coord_t end;
} line_t;

typedef struct linenode_t {
  long y_intercept;
  line_t *lines[16];
  size_t n_lines;
  struct linenode_t *left;
  struct linenode_t *right;
} linenode_t;

void line_print(line_t *line) {
  printf(" (%ld,%ld)->(%ld,%ld)", line->start.x, line->start.y, line->end.x,
         line->end.y);
}

void linetree_print(linenode_t *tree) {
  if (tree) {
    linetree_print(tree->left);
    printf("[%ld;%zu]", tree->y_intercept, tree->n_lines);

    for (int i = 0; i < tree->n_lines; i++) {
      line_print(tree->lines[i]);
    }
    printf("\n");
    linetree_print(tree->right);
  }
}

void linetree_insert(linenode_t **tree, long y_intercept, line_t *line) {
  linenode_t *tmp = NULL;

  if (!(*tree)) {
    tmp = calloc(1, sizeof(linenode_t));
    tmp->y_intercept = y_intercept;
    tmp->lines[0] = line;
    tmp->n_lines = 1;
    tmp->left = NULL;
    tmp->right = NULL;
    *tree = tmp;
    return;
  }

  if (y_intercept < (*tree)->y_intercept) {
    linetree_insert(&(*tree)->left, y_intercept, line);
  } else if (y_intercept > (*tree)->y_intercept) {
    linetree_insert(&(*tree)->right, y_intercept, line);
  } else {
    (*tree)->lines[(*tree)->n_lines] = line;
    (*tree)->n_lines++;
  }
}

size_t linetree_len(linenode_t **tree) {
  if (!(*tree))
    return 0;
  return (*tree)->n_lines + linetree_len(&(*tree)->left) +
         linetree_len(&(*tree)->right);
}

int linetree_get(line_t *(**lines)[], size_t *n_lines, long y_intercept,
                 linenode_t *tree) {
  // printf("linetree_get: %ld\n", y_intercept);
  // printf("linetree_get: %ld\n", tree->y_intercept);
  if (tree->y_intercept == y_intercept) {
    *lines = &tree->lines;
    *n_lines = tree->n_lines;
    return 0;
  }

  if (tree->y_intercept > y_intercept && tree->left) {
    return linetree_get(lines, n_lines, y_intercept, tree->left);
  }
  if (tree->y_intercept < y_intercept && tree->right) {
    return linetree_get(lines, n_lines, y_intercept, tree->right);
  }

  return 1;
}

long line_slope(line_t *line) {
  if (line->end.y - line->start.y > 0) {
    return 1;
  } else {
    return -1;
  }
}

long line_y_intercept(line_t *line) {
  long slope = line_slope(line);
  return line->start.y - slope * line->start.x;
}

void lines_overlap(line_t **overlap, line_t *line1, line_t *line2) {
  long x = (line1->start.x > line2->start.x) ? line1->start.x : line2->start.x;
  if ((x < line1->start.x) || (x > line1->end.x)) {
    *overlap = NULL;
    return;
  }
  long y = line_slope(line1) * x + line_y_intercept(line1);

  coord_t start = {x, y};

  x = (line1->end.x < line2->end.x) ? line1->end.x : line2->end.x;
  if ((x < line1->start.x) || (x > line1->end.x)) {
    *overlap = NULL;
    return;
  }
  y = line_slope(line1) * x + line_y_intercept(line1);

  coord_t end = {x, y};

  *overlap = calloc(1, sizeof(line_t));
  (*overlap)->start = start;
  (*overlap)->end = end;
}

void filter_overlap(line_t *lines[], size_t *n_lines, linenode_t *tree1,
                    linenode_t *tree2) {

  line_t *(*lines2)[16] = NULL;
  size_t n_lines2 = 0;

  linetree_get(&lines2, &n_lines2, tree1->y_intercept, tree2);

  for (int i = 0; i < tree1->n_lines; i++) {
    line_t *line1 = tree1->lines[i];

    for (int j = 0; j < n_lines2; j++) {
      line_t *overlap = NULL;
      lines_overlap(&overlap, line1, (*lines2)[j]);
      if (overlap) {

        // line_print(line1);
        // printf("\n");
        // line_print((*lines2)[j]);
        // printf("\n");
        // line_print(overlap);
        // printf("\n\n");

        lines[*n_lines] = overlap;
        (*n_lines)++;
      }
    }
  }

  if (tree1->right)
    filter_overlap(lines, n_lines, tree1->right, tree2);
  if (tree1->left)
    filter_overlap(lines, n_lines, tree1->left, tree2);
}

void lines_intersect(coord_t **intersection, line_t *line1, line_t *line2) {
  long x = (line_y_intercept(line1) - line_y_intercept(line2)) /
           (line_slope(line2) - line_slope(line1));
  // printf("lines_intersect: %ld\n", x);
  long y1 = line_slope(line1) * x + line_y_intercept(line1);
  long y2 = line_slope(line2) * x + line_y_intercept(line2);
  // printf("lines_intersect: %ld\n", y1);
  // printf("lines_intersect: %ld\n", y2);

  if ((y1 != y2) || (x < line1->start.x) || (x > line1->end.x) ||
      (x < line2->start.x) || (x > line2->end.x)) {
    intersection = NULL;
    return;
  }

  (*intersection) = calloc(1, sizeof(coord_t));
  (*intersection)->x = x;
  (*intersection)->y = y1;
}

void filter_intersect(coord_t intersections[], size_t *n_intersections,
                      line_t *lines1[], size_t n_lines1, line_t *lines2[],
                      size_t n_lines2) {
  for (int i = 0; i < n_lines1; i++) {
    for (int j = 0; j < n_lines2; j++) {
      coord_t *intersection = NULL;
      // line_print(lines1[i]);
      // printf("\n");
      // line_print(lines2[j]);
      // printf("\n");
      lines_intersect(&intersection, lines1[i], lines2[j]);
      if (intersection) {
        // printf("(%ld,%ld)\n", intersection->x, intersection->y);
        intersections[*n_intersections] = *intersection;
        (*n_intersections)++;
      }
      // printf("\n\n");
    }
  }
}

bool is_covered(pair_t pairs[], size_t n_pairs, coord_t point) {
  bool covered = false;
  for (int i = 0; i < n_pairs; i++) {
    if ((labs(pairs[i].sensor.x - point.x) +
         labs(pairs[i].sensor.y - point.y)) <= pairs[i].radius)
      covered = true;
  }

  return covered;
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

  linenode_t *left_to_top = NULL;
  linenode_t *right_to_top = NULL;
  linenode_t *left_to_bottom = NULL;
  linenode_t *right_to_bottom = NULL;
  line_t *curline = NULL;

  pair_t pairs[64] = {[0 ... 63] = {.sensor = {0, 0}, .radius = 0}};
  size_t pairs_idx = 0;
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
    pairs[pairs_idx].sensor = sensor;
    pairs[pairs_idx].radius = radius;
    pairs_idx++;

    // left -> top
    curline = calloc(1, sizeof(line_t));
    curline->start.x = sensor.x - radius - 1;
    curline->start.y = sensor.y;
    curline->end.x = sensor.x;
    curline->end.y = sensor.y - radius - 1;
    linetree_insert(&left_to_top, line_y_intercept(curline), curline);

    // top -> right
    curline = calloc(1, sizeof(line_t));
    curline->start.x = sensor.x;
    curline->start.y = sensor.y - radius - 1;
    curline->end.x = sensor.x + radius + 1;
    curline->end.y = sensor.y;
    linetree_insert(&right_to_top, line_y_intercept(curline), curline);

    // left -> bottom
    curline = calloc(1, sizeof(line_t));
    curline->start.x = sensor.x - radius - 1;
    curline->start.y = sensor.y;
    curline->end.x = sensor.x;
    curline->end.y = sensor.y + radius + 1;
    linetree_insert(&left_to_bottom, line_y_intercept(curline), curline);

    // bottom -> right
    curline = calloc(1, sizeof(line_t));
    curline->start.x = sensor.x;
    curline->start.y = sensor.y + radius + 1;
    curline->end.x = sensor.x + radius + 1;
    curline->end.y = sensor.y;
    linetree_insert(&right_to_bottom, line_y_intercept(curline), curline);

    if (labs(sensor.y - y) > radius)
      continue;

    range = calloc(1, sizeof(range_t));
    range->start = sensor.x - (radius - labs(sensor.y - y));
    range->end = sensor.x + (radius - labs(sensor.y - y));

    tree_insert(&root, range);
  }

  printf("%zu\n", tree_size(&root));

  tree_free(root);

  line_t *lines_upslope[16] = {0};
  size_t n_lines_upslope = 0;
  filter_overlap(lines_upslope, &n_lines_upslope, left_to_top, right_to_bottom);
  line_t *lines_downslope[16] = {0};
  size_t n_lines_downslope = 0;
  filter_overlap(lines_downslope, &n_lines_downslope, right_to_top,
                 left_to_bottom);

  coord_t intersections[32] = {0};
  size_t n_intersections = 0;
  filter_intersect(intersections, &n_intersections, lines_upslope,
                   n_lines_upslope, lines_downslope, n_lines_downslope);
  // HACK: These should be made unique
  for (int i = 0; i < n_intersections; i++) {
    if (is_covered(pairs, pairs_idx, intersections[i]))
      continue;
    printf("%ld\n", intersections[i].x * 4000000 + intersections[i].y);
  }

  fclose(fp);

  if (line)
    free(line);

  exit(EXIT_SUCCESS);

  return 0;
}
