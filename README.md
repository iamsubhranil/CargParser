# CargParser
### An easy to use, simple argument parser in C

The argument parser is very strict. It doesn't
allow argument redefinition, matches the exact
argument for both short and full hand arguments,
and throws error if it finds the argument to
be of shorter length.

For example, `-t` doesn't match `-tt` or `-tata` or `-test` or
anything else. It just matches `-t`, period.

Similarly, `--test` just matches `--test`.

If `-t` requires a value, if must be specified in
the format 

    -t=<value>

otherwise, it won't match the argument for anything
else, like

    -tata=<value>, -t2=<value>

and mark the whole argument `-t` as absent.

If `--test` requires a value, if must be specified in
the next argument, like

    --test <value>

It'll chew up any string as a value, but if the value
is absent, then it will mark the whole `--test` as absent.

It doesn't allow redefinitions of arguments at any
case, and doesn't change a predefined value.

Bad things will happen if an user forgets to specify
value for an argument and specifies another argument
consequently. To work it around, any value starting with
a `-` has to be ignored. I have not decided what will be
final yet.

For usage information, see `main.c`.
