#ifndef STACK_H
#define STACK_H

#define STACK(name, type, size)                                                \
  typedef struct {                                                             \
    type data[size];                                                           \
    size_t read;                                                               \
    size_t write;                                                              \
  } stack_##name##_t;                                                          \
  stack_##name##_t *stack_##name##_new() {                                     \
    stack_##name##_t *_stack = calloc(1, sizeof(stack_##name##_t));                     \
    _stack->read = 0;                                                          \
    _stack->write = 0;                                                         \
    return _stack;                                                             \
  }                                                                            \
  void stack_##name##_push(stack_##name##_t *stack, type item) {               \
    if ((stack->write + 1 == stack->read) ||                                   \
        (stack->read == 0 && stack->write + 1 == size)) {                      \
      perror("Overflow\n");                                                    \
      exit(EXIT_FAILURE);                                                      \
    }                                                                          \
    stack->data[stack->write] = item;                                          \
    stack->write++;                                                            \
    if (stack->write >= size)                                                  \
      stack->write = 0;                                                        \
  }                                                                            \
  type stack_##name##_pop(stack_##name##_t *stack) {                           \
    if (stack->read == stack->write) {                                         \
      perror("Overflow\n");                                                    \
      exit(EXIT_FAILURE);                                                      \
    }                                                                          \
    type res = stack->data[stack->read];                                       \
    stack->read++;                                                             \
    if (stack->read >= size)                                                   \
      stack->read = 0;                                                         \
    return res;                                                                \
  };                                                                           \
  bool stack_##name##_empty(stack_##name##_t *stack) {                         \
    return stack->read == stack->write;                                        \
  }

#endif
