#include "s21_grep.h"

Status status;
Flags flag;

int main(int argc, char **argv) {
  int (*function[3])(void *, ...) = {
      (int (*)(void *, ...))opting,
      (int (*)(void *, ...))compile,
      (int (*)(void *, ...))setup,
  };
  arg_data arg = {
      .count = argc,
      .vector = argv,
  };

  regex_data reg = {
      .count = 0,
      .pattern = NULL,
      .expresion = NULL,
  };

  output_data output = {
      .mode = default_mode,
      .rule = 0,
      .body = NULL,
      .length = 0,
      .num = 0,
  };

  int index = 0;

  long table[3] = {
      (long)&output,
      (long)&reg,
      (long)&arg,
  };

  while (function[index++](ARGUMENTS) == success)
    ;

  endup(&reg);

  return byte_value(status);
}

void *address(long *table) {
  static int key = 0;

  void *address = (void *)table[key];

  key = (key + 1) % arguments;
  return address;
}

int opting(arg_data *arg, regex_data *reg, output_data *output) {
  byte pattern_set = false;
  int argval = 0;

  if (arg->count < 2) {
    status.wrong_argument = true;
    return fail;
  }

  while (!byte_value(status) && argval != end) {
    argval = getopt_long(arg->count, arg->vector, optargs, longopts, NULL);
    switch (argval) {
      case '?':
        status.wrong_argument = true;
        fprintf(stderr, "%s: %s\n", arg->vector[optind], strerror(errno));
        break;
      case 'f':
        read_pattern(reg, optarg, arg);
        pattern_set = true;
        break;
      case 'e':
        reg->pattern = add_pattern(reg, optarg);
        pattern_set = true;
        break;
      case 'l':
        output->mode |= list;
        break;
      case 'c':
        output->mode |= count;
        break;
      case 'o':
        output->mode |= exact;
        break;
      case 'h':
        flag.h = true;
        break;
      case 'i':
        flag.i = true;
        break;
      case 'n':
        flag.n = true;
        break;
      case 's':
        flag.s = true;
        break;
      case 'v':
        flag.v = true;
        break;
    }
  }

  if (!byte_value(status) && !pattern_set) {
    reg->pattern = add_pattern(reg, arg->vector[optind++]);
  }

  return byte_value(status) ? fail : success;
}

char **add_pattern(regex_data *reg, char *string) {
  int count = reg->count;
  char **pattern = NULL;
  size_t length = 0;

  if (check_pattern(&length, &string, reg)) {
    return reg->pattern;
  }

  pattern = realloc(reg->pattern, sizeof(pattern) * (count + 1));
  if (!pattern) {
    status.malloc_failure = true;
    return NULL;
  }

  pattern[count] = malloc(length);
  if (!pattern[count]) {
    status.malloc_failure = true;

    for (int i = 0; i < count; ++i) {
      free(pattern[i]);
    }

    free(pattern);
    return NULL;
  }

  strncpy(pattern[count], string, length);
  ++reg->count;
  return pattern;
}

byte check_pattern(size_t *length, char **string, regex_data *reg) {
  char **pattern = reg->pattern;
  byte skip = false;

  *length = strlen(*string) + 1;

  for (int i = 0; i < reg->count && skip != true; ++i) {
    if (!strncmp(*string, pattern[i], strlen(pattern[i]))) {
      skip = true;
    }
  }

  if ((*string)[0] != '\n' && !skip) {
    (*string)[strcspn(*string, "\n")] = '\0';
  }

  return skip;
}

void read_pattern(regex_data *reg, char *filename, arg_data *arg) {
  char *line = NULL;
  size_t length = 0;
  FILE *file = fopen(filename, "r");

  if (!file) {
    fprintf(stderr, "%s: %s\n", arg->vector[0], strerror(errno));
    status.file_failure = true;
    return;
  }

  while (getline(&line, &length, file) != end) {
    reg->pattern = add_pattern(reg, line);
  }

  fclose(file);
  free(line);
  line = NULL;
}

int compile(arg_data *arg, regex_data *reg) {
  byte cflags = flag.i ? REG_EXTENDED | REG_ICASE : REG_EXTENDED;
  char buffer[err_buffer_size];
  int reply;

  reg->expresion = malloc(sizeof(regex_t) * reg->count);
  if (!reg->expresion) {
    status.malloc_failure = true;
    return fail;
  }

  for (int i = 0; i < reg->count; ++i) {
    reply = regcomp(&reg->expresion[i], reg->pattern[i], cflags);
    if (reply != success) {
      regerror(reply, &reg->expresion[i], buffer, err_buffer_size);
      fprintf(stderr, "%s: %s\n", arg->vector[0], buffer);
      status.wrong_argument = true;
      return fail;
    }
  }

  return success;
}

int setup(arg_data *arg, regex_data *reg, output_data *output) {
  byte files = arg->count - optind;

  for (int i = 0; i < mode_count; ++i) {
    if (output->mode & bit(i)) {
      output->mode = bit(i);
      break;
    }
  }

  if (flag.v) {
    output->mode &= ~exact;
  }

  if (output->mode & prefixing) {
    if (!flag.h && files > 1) {
      output->rule |= bit(0);
    }
    if (output->mode != count && flag.n) {
      output->rule |= bit(1);
    }
  }
  if (files) {
    for (int i = 0; i < files; ++i) {
      output->filename = arg->vector[optind];
      output->file = fopen(output->filename, "r");
      output->num = 0;
      if (output->file) {
        grep(reg, output);
        fclose(output->file);
      } else if (!flag.s) {
        fprintf(stderr, "%s: %s: %s\n", arg->vector[0], output->filename,
                strerror(errno));
      }
      ++optind;
    }
  } else {
    output->file = stdin;
    output->num = 0;
    grep(reg, output);
  }
  return end;
}

void grep(regex_data *reg, output_data *output) {
  char *line = NULL;
  switch (output->mode) {
    case count: {
      size_t counter = 0;
      char counter_str[counter_limit];
      while (read_file(&line, output) != end) {
        if (match(line, reg) != flag.v) {
          ++counter;
        }
        free(line);
        line = NULL;
      }
      snprintf(counter_str, counter_limit, "%ld", counter);
      output->body = counter_str;
      print(output);
    } break;
    case exact: {
      int offset;
      while (read_file(&line, output) != end) {
        ++output->num;
        offset = 0;
        if (match(line, reg) != flag.v) {
          do {
            output->length = reg->end - reg->start;
            output->body = &line[offset + reg->start];
            print(output);
            offset += reg->end;
          } while (match(&line[offset], reg));
        }
        free(line);
        line = NULL;
      }
      putc('\n', output->file);
    } break;
    case list: {
      while (read_file(&line, output) != end) {
        if (match(line, reg) != flag.v) {
          printf("%s\n", output->filename);
          break;
        }
        free(line);
        line = NULL;
      }
    } break;
    case none:
      break;
    default: {
      while (read_file(&line, output) != end) {
        ++output->num;
        if (match(line, reg) != flag.v) {
          output->body = line;
          print(output);
        }
        free(line);
        line = NULL;
      }
    } break;
  }
  if (line) {
    free(line);
    line = NULL;
  }
}

int read_file(char **line, output_data *output) {
  size_t capacity = 0;
  output->length = getline(line, &capacity, output->file);
  return output->length;
}

byte match(char *line, regex_data *reg) {
  struct regmatch_readable closest = {
      .start = max_int,
      .end = -1,
  };

  byte matched = true;

  for (int i = 0; i < reg->count; ++i) {
    if (!regexec(&reg->expresion[i], line, 1, (regmatch_t *)reg, 0)) {
      if (reg->start < closest.start) {
        closest.start = reg->start;
        closest.end = reg->end;
      }
    }
  }

  if (as(long) closest != as(long)(const regmatch_t){max_int, -1}) {
    reg->start = closest.start;
    reg->end = closest.end;
  } else {
    matched = false;
  }

  return matched;
}

void print(output_data *out) {
  out->body[strcspn(out->body, "\r\n")] = '\0';

  switch (out->rule) {
    case both_prefix:
      printf("%s:%ld:%.*s\n", out->filename, out->num, out->length, out->body);
      break;
    case file_prefix:
      printf("%s:%.*s\n", out->filename, out->length, out->body);
      break;
    case num_prefix:
      printf("%ld:%.*s\n", out->num, out->length, out->body);
      break;
    default:
      printf("%.*s\n", out->length, out->body);
      break;
  }
}

void endup(regex_data *reg) {
  if (reg->pattern) {
    for (int i = 0; i < reg->count; ++i) {
      free(reg->pattern[i]);
      regfree(&reg->expresion[i]);
    }
  }

  free(reg->pattern);
  free(reg->expresion);
}
