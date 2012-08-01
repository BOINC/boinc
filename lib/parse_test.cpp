// test program for XML parser

#include "parse.h"

#include <stdio.h>

void parse(FILE* f) {
    bool flag;
    MIOFILE mf;
    XML_PARSER xp(&mf);
    char name[64];
    char foo[64];
    int val;
    double x;

    mf.init_file(f);
    if (!xp.parse_start("blah")) {
        printf("missing start tag\n");
        return;
    }
    strcpy(foo, "xxx");
    while (!xp.get_tag()) {
        if (!xp.is_tag) {
            printf("unexpected text: %s\n", xp.parsed_tag);
            continue;
        }
        if (xp.match_tag("/blah")) {
            printf("success\n");
            return;
        } else if (xp.parse_str("str", name, sizeof(name))) {
            printf("got str: %s\n", name);
        } else if (xp.parse_int("int", val)) {
            printf("got int: %d\n", val);
        } else if (xp.parse_double("double", x)) {
            printf("got double: %f\n", x);
        } else if (xp.parse_bool("bool", flag)) {
            printf("got bool: %d\n", flag);
        } else {
            printf("unparsed tag: %s\n", xp.parsed_tag);
            xp.skip_unexpected(true, "xml test");
        }
    }
    printf("unexpected EOF\n");
}

int main() {
    FILE* f = fopen("foo.xml", "r");
    parse(f);
}

/* try it with something like:

<?xml version="1.0" encoding="ISO-8859-1" ?>
<blah>
    <x>
    asdlfkj
      <x> fj</x>
    </x>
    <str>blah</str>
    <int>  6
    </int>
    <double>6.555</double>
    <bool>0</bool>
</blah>

*/
