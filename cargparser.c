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
    char shorthand_char;      // This character is used for both
                              // parsing a '-' type argument and
                              // as an identifier of this argument in the list,
                              // so make sure it is unique.
    const char *full_string;  // string for --option
    const char *help_string;  // help string to print
    bool value_required;      // whether this argument requires a value
    bool is_optional;         // whether this argument is optional
    bool is_present;          // after parsing, whether this argument is present
    bool is_positional;       // denotes whether this is a positional argument
    char *value;              // after parsing, the value given to this argument
} Argument;

struct ArgumentArray {
    Argument *arguments;
    size_t size;
    bool missing_mandatory;
    bool has_default_help;
};

struct ArgumentArray *arg_list_create() {
    struct ArgumentArray *al =
        (struct ArgumentArray *)malloc(sizeof(struct ArgumentArray));
    al->size = 0;
    al->arguments = NULL;
    al->missing_mandatory = false;
    arg_add(al, 'h', "help", "Shows this help", false, true);
    al->has_default_help = true;
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

void arg_add2(struct ArgumentArray *list, const char shorthand,
              const char *full, const char *help, bool value_required,
              bool is_optional, bool is_positional) {
    Argument a;
    a.full_string = strdup(full);
    a.help_string = strdup(help);
    a.shorthand_char = shorthand;
    a.value_required = value_required;
    a.value = NULL;
    a.is_present = false;
    a.is_optional = is_optional;
    a.is_positional = is_positional;
    if (shorthand == 'h') {
        list->has_default_help = false;
    }
    arg_add_internal(list, a);
}

void arg_add(struct ArgumentArray *list, const char shorthand, const char *full,
             const char *help, bool value_required, bool is_optional) {
    arg_add2(list, shorthand, full, help, value_required, is_optional, false);
}
void arg_add_positional(struct ArgumentArray *list, const char shorthand,
                        const char *full, const char *help, bool is_optional) {
    arg_add2(list, shorthand, full, help, true, is_optional, true);
}

static Argument *get_argument(struct ArgumentArray *l, char c) {
    for (size_t i = 0; i < l->size; i++) {
        if (l->arguments[i].shorthand_char == c) return &(l->arguments[i]);
    }
    return NULL;
}

static Argument *get_shorthand_argument(struct ArgumentArray *l,
                                        const char *arg, size_t len) {
    // strictly only allow "-option"
    if (len < 2) return NULL;
    if (len > 2) return NULL;
    for (size_t i = 0; i < l->size; i++) {
        if (l->arguments[i].is_positional == false &&
            l->arguments[i].shorthand_char == arg[1])
            return &(l->arguments[i]);
    }
    return NULL;
}

static Argument *get_full_argument(struct ArgumentArray *l, const char *str,
                                   size_t len) {
    if (len < 3) return NULL;
    for (size_t i = 0; i < l->size; i++) {
        if (l->arguments[i].is_positional == false &&
            strcmp(l->arguments[i].full_string, &str[2]) == 0)
            return &(l->arguments[i]);
    }
    return NULL;
}

static Argument *get_next_positional(struct ArgumentArray *l) {
    // return the first non optional argument, if available
    for (size_t i = 0; i < l->size; i++) {
        if (l->arguments[i].is_positional == true &&
            l->arguments[i].is_optional == false &&
            l->arguments[i].is_present == false)
            return &(l->arguments[i]);
    }
    // return first optional argument, if available
    for (size_t i = 0; i < l->size; i++) {
        if (l->arguments[i].is_positional == true &&
            l->arguments[i].is_present == false)
            return &(l->arguments[i]);
    }
    // no more positional arguments available
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
                           size_t len, size_t minlen, bool is_positional) {
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
    if (!is_positional) {
        if (*i == argc - 1) {
            perr("Expected value for argument '%s'", argv[*i]);
            arg_highlight(argc, argv, *i, 0, len, true);
            return;
        }
        arg->value = argv[*i + 1];
        arg->is_present = true;
        (*i)++;
    } else {
        arg->value = argv[*i];
        arg->is_present = true;
    }
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
                               get_full_argument(list, argv[i], len), &i, len,
                               3, false);
            else
                parse_argument(argc, argv,
                               get_shorthand_argument(list, argv[i], len), &i,
                               len, 2, false);
        } else {
            parse_argument(argc, argv, get_next_positional(list), &i, len, 0,
                           true);
        }
    }
    bool silentError = false;
    // print default help if needed to
    if (list->has_default_help && arg_is_present(list, 'h')) {
        arg_print_default_help(list, argv);
        silentError = true;
    }
    // check if all mandatory arguments are present
    for (size_t i = 0; i < list->size; i++) {
        if (list->arguments[i].is_optional == false &&
            list->arguments[i].is_present == false) {
            if (!silentError) {
                if (list->arguments[i].is_positional) {
                    perr("Missing mandatory argument '%s'!",
                         list->arguments[i].full_string);
                } else {
                    perr("Missing mandatory argument '-%c'/'--%s'!",
                         list->arguments[i].shorthand_char,
                         list->arguments[i].full_string);
                }
            }
            list->missing_mandatory = true;
        }
    }
}

static void arg_print_arg(Argument a) {
    if (a.is_optional) {
        printf("[ ");
    }
    if (a.is_positional) {
        printf("%s", a.full_string);
    } else {
        printf("-%c/--%s", a.shorthand_char, a.full_string);
    }
    if (!a.is_positional && a.value_required) {
        printf(" <value>");
    }
    if (a.is_optional) {
        printf(" ]");
    }
    printf(" ");
}

void arg_print_default_help(struct ArgumentArray *list, char **argv) {
    printf("Usage: %s ", argv[0]);
    // print all optional positional values in the end
    for (size_t i = 0; i < list->size; i++) {
        Argument a = list->arguments[i];
        if (a.is_positional && a.is_optional) continue;
        arg_print_arg(a);
    }
    for (size_t i = 0; i < list->size; i++) {
        Argument a = list->arguments[i];
        if (a.is_positional && a.is_optional) arg_print_arg(a);
    }
    printf("\n\nDetails: \n");
    for (size_t i = 0; i < list->size; i++) {
        Argument a = list->arguments[i];
        if (a.is_positional) {
            printf("\t%s", a.full_string);
        } else {
            printf("\t-%c/--%s", a.shorthand_char, a.full_string);
        }
        if (!a.is_positional && a.value_required) {
            printf(" <value>");
        } else {
            printf("        ");
        }
        printf("\t\t%s\n", a.help_string);
    }
}

bool arg_is_present(struct ArgumentArray *list, const char shorthand) {
    Argument *a = get_argument(list, shorthand);
    return a != NULL && a->is_present;
}

char *arg_value(struct ArgumentArray *list, const char shorthand) {
    Argument *a = get_argument(list, shorthand);
    return a == NULL ? NULL : a->value;
}

void arg_free(struct ArgumentArray *l) {
    for (size_t i = 0; i < l->size; i++) {
        free((void *)l->arguments[i].full_string);
        free((void *)l->arguments[i].help_string);
    }
    free(l->arguments);
    free(l);
}
