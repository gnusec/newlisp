#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

// newLISP runtime types
typedef long long nl_int;
typedef double nl_float;
typedef char* nl_string;
typedef int nl_bool;

// newLISP runtime support functions
void nl_println(nl_string str) {
    if (str) printf("%s\n", str);
    else printf("nil\n");
}

void nl_print_int(nl_int value) {
    printf("%lld", value);
}

void nl_print_float(nl_float value) {
    printf("%.6f", value);
}

int main(int argc, char* argv[]) {
    nl_println("Hello, World!");
    return 0;
}
