#ifndef S21_GREP_SCHRADER
#define S21_GREP_SCHRADER

#include <errno.h>
#include <getopt.h>
#include <regex.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "schrader_common.h"

#define IS_MATCHED(r) (*(long *)r != *(long *)&(const regmatch_t){max_int, -1})
#define ARGUMENTS address(table), address(table), address(table)

typedef struct regex_data {
  regoff_t start;
  regoff_t end;
  int count;
  char **pattern;
  regex_t *expresion;
} regex_data;

typedef struct output_data {
  int mode;
  int rule;
  char *body;
  int length;
  size_t num;
  FILE *file;
  char *filename;
} output_data;

typedef struct Flags {
  byte h : 1;
  byte i : 1;
  byte n : 1;
  byte s : 1;
  byte v : 1;
} Flags;

typedef struct Data {
  arg_data arg;
  regex_data reg;
  output_data output;
} Data;

struct regmatch_readable {
  regoff_t start;
  regoff_t end;
};

enum {
  counter_limit = 10,
  arguments = 3,
  mode_count = 3,
  none = 0,
  list = 1,
  count = 2,
  exact = 4,
  prefixing = 14,
  default_mode = 8,
  file_prefix = 1,
  num_prefix = 2,
  both_prefix = 3,
  err_buffer_size = 100,
  arg_address = 0,
  reg_address = sizeof(arg_data),
  output_address = reg_address + sizeof(regex_data),
  max_int = (~((unsigned int)~0 >> 1) - 1),
};

const char optargs[] = "ce:f:hilnosv";
const struct option longopts[] = {
    {"count", no_argument, NULL, 'c'},
    {"regexp", required_argument, NULL, 'e'},
    {"file", required_argument, NULL, 'f'},
    {"no-filename", no_argument, NULL, 'h'},
    {"ingnore-case", no_argument, NULL, 'i'},
    {"files-with-matches", no_argument, NULL, 'l'},
    {"line-number", no_argument, NULL, 'n'},
    {"only-matching", no_argument, NULL, 'o'},
    {"no-messages", no_argument, NULL, 's'},
    {"invert-match", no_argument, NULL, 'v'}};

void *address(long *);
int opting(arg_data *, regex_data *, output_data *);
char **add_pattern(regex_data *, char *);
byte check_pattern(size_t *, char **, regex_data *);
void read_pattern(regex_data *, char *, arg_data *);
int compile(arg_data *, regex_data *);
int setup(arg_data *, regex_data *, output_data *);
void grep(regex_data *, output_data *);
int read_file(char **, output_data *);
byte match(char *, regex_data *);
void print(output_data *);
void print(output_data *);
void endup(regex_data *);

#endif  // S21_GREP_SCHRADER
