#!/bin/bash

echo '32-128,' > oled_font.map
cat oled_str.txt | iconv -f utf-8 -t c99 | sed 's/\\u\([0-9a-f]\{4\}\)/\$\1,\'$'\n/g' | sort | uniq | sed '/^$/d' | tr '/a-f/' '/A-F/' >> oled_font.map
$BDFCONV $BDFFONT -b 0 -f 1 -M oled_font.map -n oled_font -o _tmp.c
echo '#include "oled_font.h"' > ../oled_font.c
cat _tmp.c >> ../oled_font.c
rm _tmp.c
