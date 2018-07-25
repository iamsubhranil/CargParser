#include <stdio.h>
#include "cargparser.h"

int main(int argc, char *argv[]){
    ArgumentList list = arg_list_create(2);

    arg_add(list, 't', "test", true);
    arg_add(list, 'b', "bad", true);
    
    arg_parse(argc, &argv[0], list);
    
    if(arg_is_present(list, 't'))
        printf("\n-t : %s", arg_value(list, 't'));
    if(arg_is_present(list, 'b'))
        printf("\n-b : %s", arg_value(list, 'b'));
    
    arg_free(list);
    printf("\n");
    return 1;
}
