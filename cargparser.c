#include <stdarg.h>
#include <stdio.h>
#include <string.h>

#include "cargparser.h"

// Display cosmetics

#define ANSI_COLOR_RED     "\x1b[31m"
#define ANSI_COLOR_YELLOW  "\x1b[33m"
#define ANSI_COLOR_RESET   "\x1b[0m"
#define ANSI_FONT_BOLD     "\x1b[1m"

#define print(name, color)              \
    static void p##name(const char* msg, ...){ \
        printf(ANSI_COLOR_##color);     \
        va_list args;                   \
        va_start(args, msg);            \
        vprintf(msg, args);             \
        va_end(args);                   \
        printf(ANSI_COLOR_RESET);       \
    }

print(red, RED)
print(ylw, YELLOW)

#undef print

#define display(name, color, text)                  \
    static void p##name(const char *msg, ...){             \
        printf(ANSI_FONT_BOLD);                     \
        printf(ANSI_COLOR_##color "\n" text " ");   \
        printf(ANSI_COLOR_RESET);                   \
        va_list args;                               \
        va_start(args, msg);                        \
        vprintf(msg, args);                         \
        va_end(args);                               \
    }

display(err, RED, "[Error]")
display(warn, YELLOW, "[Warning]")

#undef display

// Argument format
// Either 
// shorthand:   "-" [A-Za-z] "=" <value> 
// or full:     "--" [AZaz]+ " " <value>
// Arguments with no values will exclude the "=" part
// Values must not start with "-" or "--" to distinguish
// them from argument keywords.

typedef struct{
    char shorthand_char;        // This character is used for both 
                                // parsing a '-' type argument and 
                                // as an identifier of this argument in the list,
                                // so make sure it is unique.
    const char *full_string;
    bool value_required;

    bool is_present;
    char *value;
} Argument;

struct ArgumentArray{
    Argument *arguments;
    size_t size;
    size_t pointer;
};

struct ArgumentArray* arg_list_create(size_t size){
    struct ArgumentArray* al = (struct ArgumentArray *)malloc(sizeof(struct ArgumentArray));
    al->size = size;
    al->pointer = 0;
    al->arguments = (Argument *)malloc(sizeof(Argument) * size);
    return al;
}

static void arg_add_internal(struct ArgumentArray *arglist, Argument arg){
    arglist->arguments[arglist->pointer] = arg;
    arglist->pointer++;
}

void arg_add(struct ArgumentArray *list, const char shorthand, const char *full, bool value_required){
    Argument a;
    a.full_string = full;
    a.shorthand_char = shorthand;
    a.value_required = value_required;
    a.value = NULL;
    a.is_present = false;
    arg_add_internal(list, a);
}

static Argument* get_shorthand_argument(struct ArgumentArray l, char c){
    for(size_t i = 0;i < l.pointer;i++){
        if(l.arguments[i].shorthand_char == c)
            return &(l.arguments[i]);
    }
    return NULL;
}

static Argument* get_full_argument(struct ArgumentArray l, char *str){
    for(size_t i = 0;i < l.pointer;i++){
        if(strcmp(l.arguments[i].full_string, str) == 0)
            return &(l.arguments[i]);
    }
    return NULL; 
}

static void arg_highlight(int argc, char **argv, int pointer, int from, int to, bool iserror){
    printf("\n");
    for(int i = 0;i < argc;i++){
        printf("%s ", argv[i]);
    }
    printf("\n");
    for(int i = 0;i < pointer;i++){
        for(int j = 0;argv[i][j] != '\0';j++)
            printf(" ");
        printf(" ");
    }
    for(int i = 0;i < from;i++){
        printf(" ");
    }
    for(int i = from;i < to;i++){
        if(iserror)
            pred(ANSI_FONT_BOLD "^" ANSI_COLOR_RESET);
        else
            pylw(ANSI_FONT_BOLD "^" ANSI_COLOR_RESET);
    }
}

static void parse_shorthand_argument(int argc, char **argv, int i, size_t len, struct ArgumentArray list){
    Argument *req = get_shorthand_argument(list, argv[i][1]);
    if(req == NULL){
        pwarn("Ignoring unknown argument");
        arg_highlight(argc, argv, i, 0, len, false);
        return;
    }

    if(req->value_required == false){
        if(len > 2){
            pwarn("Ignoring unknown argument");
            arg_highlight(argc, argv, i, 0, len, false);
        }
        else{ 
            if(req->is_present){
                pwarn("Redefinition of argument");
                arg_highlight(argc, argv, i, 0, len, false);
                return;
            }
            req->is_present = true;
        }
        return; 
    }
    if(len < 3){
        perr("Expected value for argument");
        arg_highlight(argc, argv, i, 0, len, true);
        return;
    }
    size_t start = 2;
    if(argv[i][start] != '='){
        pwarn("Ignoring unknown argument");
        arg_highlight(argc, argv, i, 0, len, false);
        return;
    }
    if(req->is_present){
        perr("Redefinition of argument");
        arg_highlight(argc, argv, i, 0, len, true);
        return;
    }
    if(start == len || start == len - 1){
        perr("Expected value for argument '-%c'", argv[i][1]);
        arg_highlight(argc, argv, i, 0, len, true);
        return;
    } 
    req->value = &argv[i][start + 1];
    req->is_present = true;
}

static void parse_full_argument(int argc, char **argv, int *i, size_t len, struct ArgumentArray list){
    if(len < 3){
        perr("Too short argument");
        arg_highlight(argc, argv, *i, 0, len, true);
        return;
    }
    Argument *arg = get_full_argument(list, &argv[*i][2]);
    if(arg == NULL){
        pwarn("Ignoring unknown argument");
        arg_highlight(argc, argv, *i, 0, len, false);
        return;
    }
    
    if(arg->is_present){
        perr("Redefinition of argument");
        arg_highlight(argc, argv, *i, 0, len, true);
        return;
    }

    if(arg->value_required == false){
        arg->is_present = true;
        return;
    }
    if(*i == argc - 1){
        perr("Expected value for argument '%s'", argv[*i]);
        arg_highlight(argc, argv, *i, 0, len, true);
        return;
    }
    arg->value = argv[*i + 1];
    arg->is_present = true;
    (*i)++;
}

void arg_parse(int argc, char **argv, struct ArgumentArray *list){
    for(int i = 1;i < argc;i++){
        char f = argv[i][0];
        size_t len = strlen(argv[i]);
        if(f == '-'){
            if(len < 2){
                perr("Too short argument");
                arg_highlight(argc, argv, i, 0, len, true);
                continue;
            }
            if(argv[i][1] == '-')
                parse_full_argument(argc, argv, &i, len, *list);
            else
                parse_shorthand_argument(argc, argv, i, len, *list);
        }
        else{
            pwarn("Ignoring unknown argument");
            arg_highlight(argc, argv, i, 0, len, false);
        }
    }
}

bool arg_is_present(struct ArgumentArray *list, const char shorthand){
    Argument *a = get_shorthand_argument(*list, shorthand);
    return a->is_present;
}

char* arg_value(struct ArgumentArray *list, const char shorthand){
    return get_shorthand_argument(*list, shorthand)->value;
}

void arg_free(struct ArgumentArray *l){
    free(l->arguments);
    free(l);
}
