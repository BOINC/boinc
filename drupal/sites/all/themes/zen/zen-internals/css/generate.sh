#!/bin/sh

# This script is used by the MAINTAINER to generate composite stylesheets for
# the base Zen theme from the stylesheets in the STARTERKIT.

FIXED=( \
  html-reset \
  wireframes \
  layout-fixed \
  page-backgrounds \
  tabs \
  messages \
  pages \
  block-editing \
  blocks \
  navigation \
  views-styles \
  nodes \
  comments \
  forms \
  fields \
  );

STYLESHEET='zen-fixed.css';
RTL_STYLESHEET='zen-fixed-rtl.css';
echo > $STYLESHEET;
echo > $RTL_STYLESHEET;
for FILENAME in ${FIXED[*]}; do
  echo >> $STYLESHEET;
  echo "/* $FILENAME.css */" >> $STYLESHEET;
  echo >> $STYLESHEET;
  cat ../../STARTERKIT/css/$FILENAME.css >> $STYLESHEET;
  if [[ -e ../../STARTERKIT/css/$FILENAME-rtl.css ]]; then
    echo >> $RTL_STYLESHEET;
    echo "/* $FILENAME-rtl.css */" >> $RTL_STYLESHEET;
    echo >> $RTL_STYLESHEET;
    cat ../../STARTERKIT/css/$FILENAME-rtl.css >> $RTL_STYLESHEET;
  fi
done

LIQUID=( \
  html-reset \
  wireframes \
  layout-liquid \
  page-backgrounds \
  tabs \
  messages \
  pages \
  block-editing \
  blocks \
  navigation \
  views-styles \
  nodes \
  comments \
  forms \
  fields \
  );

STYLESHEET='zen-liquid.css';
RTL_STYLESHEET='zen-liquid-rtl.css';
echo > $STYLESHEET;
echo > $RTL_STYLESHEET;
for FILENAME in ${LIQUID[*]}; do
  echo >> $STYLESHEET;
  echo "/* $FILENAME.css */" >> $STYLESHEET;
  echo >> $STYLESHEET;
  cat ../../STARTERKIT/css/$FILENAME.css >> $STYLESHEET;
  if [[ -e ../../STARTERKIT/css/$FILENAME-rtl.css ]]; then
    echo >> $RTL_STYLESHEET;
    echo "/* $FILENAME-rtl.css */" >> $RTL_STYLESHEET;
    echo >> $RTL_STYLESHEET;
    cat ../../STARTERKIT/css/$FILENAME-rtl.css >> $RTL_STYLESHEET;
  fi
done

cp ../../STARTERKIT/css/print.css .;

cp ../../STARTERKIT/css/ie.css .;
cp ../../STARTERKIT/css/ie6.css .;
cp ../../STARTERKIT/css/ie6-rtl.css .;
