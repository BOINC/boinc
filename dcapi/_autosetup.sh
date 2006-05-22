#!/bin/sh

AUTORECONF=autoreconf

gtkdocize --copy
$AUTORECONF -I m4 -i -s
