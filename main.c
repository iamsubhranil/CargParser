#include <stdio.h>

#include "cargparser.h"

int main(int argc, char *argv[]) {
    ArgumentList list = arg_list_create();

    arg_add(list, 'a', "apply", "Optional argument without value", false, true);
    arg_add(list, 'c', "check", "Optional argument with value", true, true);
    arg_add(list, 't', "test", "Mandatory argument without value", false,
            false);
    arg_add(list, 'b', "bad", "Mandatory argument with value", true, false);
    arg_add_positional(list, 'p', "positional1", "Optional positional argument",
                       true);
    arg_add_positional(list, 'q', "positional2",
                       "Mandatory positional argument", false);

    arg_parse(argc, &argv[0], list);

    char args[] = {'a', 'c', 't', 'b', 'p', 'q'};

    for (int i = 0; i < 6; i++) {
        if (arg_is_present(list, args[i])) {
            printf("\n-%c : %s", args[i], arg_value(list, args[i]));
        }
    }

    arg_free(list);
    printf("\n");
    return 1;
}
