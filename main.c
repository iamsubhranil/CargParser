#include <stdio.h>

#include "cargparser.h"

int main(int argc, char *argv[]) {
    ArgumentList list = arg_list_create();

    arg_add(list, 't', "test", "Optional argument with value", true, true);
    arg_add(list, 'b', "bad", "Mandatory argument with value", true, false);

    arg_parse(argc, &argv[0], list);

    if (arg_is_present(list, 't')) printf("\n-t : %s", arg_value(list, 't'));
    if (arg_is_present(list, 'b')) printf("\n-b : %s", arg_value(list, 'b'));

    arg_free(list);
    printf("\n");
    return 1;
}
