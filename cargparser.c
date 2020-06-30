#include "cargparser.h"

#include <stdarg.h>
#include <stdio.h>
#include <string.h>

// Display cosmetics

#define ANSI_COLOR_RED "\x1b[31m"
#define ANSI_COLOR_YELLOW "\x1b[33m"
#define ANSI_COLOR_RESET "\x1b[0m"
#define ANSI_FONT_BOLD "\x1b[1m"

#define print(name, color)                      \
    static void p##name(const char *msg, ...) { \
        printf(ANSI_COLOR_##color);             \
        va_list args;                           \
        va_start(args, msg);                    \
        vprintf(msg, args);                     \
        va_end(args);                           \
        printf(ANSI_COLOR_RESET);               \
    }

print(red, RED) print(ylw, YELLOW)
#undef print

#define display(name, color, text)                \
    static void p##name(const char *msg, ...) {   \
        printf(ANSI_FONT_BOLD);                   \
        printf(ANSI_COLOR_##color "\n" text " "); \
        printf(ANSI_COLOR_RESET);                 \
        va_list args;                             \
        va_start(args, msg);                      \
        vprintf(msg, args);                       \
        va_end(args);                             \
    }

    display(err, RED, "[Error]") display(warn, YELLOW, "[Warning]")

#undef display

    // Argument format
    // Either
    // shorthand:   "-" [A-Za-z] "=" <value>
    // or full:     "--" [AZaz]+ " " <value>
    // Arguments with no values will exclude the "=" part
    // Values must not start with "-" or "--" to distinguish
    // them from argument keywords.

    typedef struct {
    char shorthand_char;  // This character is used for both
                          // parsing a '-' type argument and
                          // as an identifier of this argument in the list,
                          // so make sure it is unique.
    const char *full_string;
    bool value_required;  // whether this argument requires a value
    bool is_optional;     // whether this argument is optional
    bool is_present;      // after parsing, whether this argument is present
    char *value;          // after parsing, the value given to this argument
} Argument;

struct ArgumentArray {
    Argument *arguments;
    size_t size;
    bool missing_mandatory;
};

struct ArgumentArray *arg_list_create() {
    struct ArgumentArray *al =
        (struct ArgumentArray *)malloc(sizeof(struct ArgumentArray));
    al->size = 0;
    al->arguments = (Argument *)malloc(sizeof(Argument));
    al->missing_mandatory = false;
    return al;
}

static void arg_add_internal(struct ArgumentArray *arglist, Argument arg) {
    // if we already have such an argument, replace that
    for (size_t i = 0; i < arglist->size; i++) {
        if (arglist->arguments[i].shorthand_char == arg.shorthand_char) {
            arglist->arguments[i] = arg;
            return;
        }
    }
    // reallocate the array
    arglist->arguments = (Argument *)realloc(
        arglist->arguments, sizeof(Argument) * (arglist->size + 1));
    arglist->arguments[arglist->size] = arg;
    arglist->size++;
}

void arg_add(struct ArgumentArray *list, const char shorthand, const char *full,
             bool value_required, bool is_optional) {
    Argument a;
    a.full_string = strdup(full);
    a.shorthand_char = shorthand;
    a.value_required = value_required;
    a.value = NULL;
    a.is_present = false;
    a.is_optional = is_optional;
    arg_add_internal(list, a);
}

static Argument *get_shorthand_argument(struct ArgumentArray l, const char *arg,
                                        size_t len) {
    // strictly only allow "-option"
    if (len < 2) return NULL;
    if (len > 2) return NULL;
    for (size_t i = 0; i < l.size; i++) {
        if (l.arguments[i].shorthand_char == arg[1]) return &(l.arguments[i]);
    }
    return NULL;
}

static Argument *get_full_argument(struct ArgumentArray l, const char *str,
                                   size_t len) {
    if (len < 3) return NULL;
    for (size_t i = 0; i < l.size; i++) {
        if (strcmp(l.arguments[i].full_string, &str[2]) == 0)
            return &(l.arguments[i]);
    }
    return NULL;
}

static void arg_highlight(int argc, char **argv, int pointer, int from, int to,
                          bool iserror) {
    printf("\n");
    for (int i = 0; i < argc; i++) {
        printf("%s ", argv[i]);
    }
    printf("\n");
    for (int i = 0; i < pointer; i++) {
        for (int j = 0; argv[i][j] != '\0'; j++) printf(" ");
        printf(" ");
    }
    for (int i = 0; i < from; i++) {
        printf(" ");
    }
    for (int i = from; i < to; i++) {
        if (iserror)
            pred(ANSI_FONT_BOLD "^" ANSI_COLOR_RESET);
        else
            pylw(ANSI_FONT_BOLD "^" ANSI_COLOR_RESET);
    }
}

static void parse_argument(int argc, char **argv, Argument *arg, int *i,
                           size_t len, size_t minlen) {
    if (len < minlen) {
        perr("Too short argument");
        arg_highlight(argc, argv, *i, 0, len, true);
        return;
    }

    if (arg == NULL) {
        pwarn("Ignoring unknown argument");
        arg_highlight(argc, argv, *i, 0, len, false);
        return;
    }

    if (arg->is_present) {
        perr("Redefinition of argument");
        arg_highlight(argc, argv, *i, 0, len, true);
        return;
    }

    if (arg->value_required == false) {
        arg->is_present = true;
        return;
    }
    if (*i == argc - 1) {
        perr("Expected value for argument '%s'", argv[*i]);
        arg_highlight(argc, argv, *i, 0, len, true);
        return;
    }
    arg->value = argv[*i + 1];
    arg->is_present = true;
    (*i)++;
}

void arg_parse(int argc, char **argv, struct ArgumentArray *list) {
    for (int i = 1; i < argc; i++) {
        char f = argv[i][0];
        size_t len = strlen(argv[i]);
        if (f == '-') {
            if (len < 2) {
                perr("Too short argument");
                arg_highlight(argc, argv, i, 0, len, true);
                continue;
            }
            if (argv[i][1] == '-')
                parse_argument(argc, argv,
                               get_full_argument(*list, argv[i], len), &i, len,
                               3);
            else
                parse_argument(argc, argv,
                               get_shorthand_argument(*list, argv[i], len), &i,
                               len, 2);
        } else {
            pwarn("Ignoring unknown argument");
            arg_highlight(argc, argv, i, 0, len, false);
        }
    }
    // check if all mandatory arguments are present
    for (size_t i = 0; i < list->size; i++) {
        if (list->arguments[i].is_optional == false &&
            list->arguments[i].is_present == false) {
            perr("Missing mandatory argument '-%c'/'--%s'!",
                 list->arguments[i].shorthand_char,
                 list->arguments[i].full_string);
            list->missing_mandatory = true;
        }
    }
}

bool arg_is_present(struct ArgumentArray *list, const char shorthand) {
    char arg[2];
    arg[0] = '-';
    arg[1] = shorthand;
    Argument *a = get_shorthand_argument(*list, arg, 2);
    return a != NULL && a->is_present;
}

char *arg_value(struct ArgumentArray *list, const char shorthand) {
    char arg[2];
    arg[0] = '-';
    arg[1] = shorthand;
    Argument *a = get_shorthand_argument(*list, arg, 2);
    return a == NULL ? NULL : a->value;
}

void arg_free(struct ArgumentArray *l) {
    for (size_t i = 0; i < l->size; i++) {
        free((void *)l->arguments[i].full_string);
    }
    free(l->arguments);
    free(l);
}
