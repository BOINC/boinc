#!/bin/sh

# $Id: generate.sh,v 1.4.2.2 2010/06/26 16:01:52 johnalbin Exp $
#
# This script is used by the MAINTAINER to generate composite stylesheets for
# the base Zen theme from the stylesheets in the STARTERKIT.

/bin/echo -n '/* $' > header.txt;
/bin/echo 'Id$ */' >> header.txt;
/bin/echo >> header.txt;

cat header.txt ../../STARTERKIT/css/html-reset.css ../../STARTERKIT/css/wireframes.css ../../STARTERKIT/css/layout-fixed.css ../../STARTERKIT/css/page-backgrounds.css ../../STARTERKIT/css/tabs.css ../../STARTERKIT/css/messages.css ../../STARTERKIT/css/pages.css ../../STARTERKIT/css/block-editing.css ../../STARTERKIT/css/blocks.css ../../STARTERKIT/css/navigation.css ../../STARTERKIT/css/panels-styles.css ../../STARTERKIT/css/views-styles.css ../../STARTERKIT/css/nodes.css ../../STARTERKIT/css/comments.css ../../STARTERKIT/css/forms.css ../../STARTERKIT/css/fields.css | perl -e 'while(<>) { $_ =~ s|^\/\* \$(Id: [^\$]+) \$|\/* \1|; print $_; }' - > zen-fixed.css;

cat header.txt ../../STARTERKIT/css/html-reset.css ../../STARTERKIT/css/wireframes.css ../../STARTERKIT/css/layout-liquid.css ../../STARTERKIT/css/page-backgrounds.css ../../STARTERKIT/css/tabs.css ../../STARTERKIT/css/messages.css ../../STARTERKIT/css/pages.css ../../STARTERKIT/css/block-editing.css ../../STARTERKIT/css/blocks.css ../../STARTERKIT/css/navigation.css ../../STARTERKIT/css/panels-styles.css ../../STARTERKIT/css/views-styles.css ../../STARTERKIT/css/nodes.css ../../STARTERKIT/css/comments.css ../../STARTERKIT/css/forms.css ../../STARTERKIT/css/fields.css | perl -e 'while(<>) { $_ =~ s|^\/\* \$(Id: [^\$]+) \$|\/* \1|; print $_; }' - > zen-liquid.css

cat header.txt ../../STARTERKIT/css/print.css | perl -e 'while(<>) { $_ =~ s|^\/\* \$(Id: [^\$]+) \$|\/* \1|; print $_; }' - > print.css;

cat header.txt ../../STARTERKIT/css/ie.css | perl -e 'while(<>) { $_ =~ s|^\/\* \$(Id: [^\$]+) \$|\/* \1|; print $_; }' - > ie.css;
cat header.txt ../../STARTERKIT/css/ie6.css | perl -e 'while(<>) { $_ =~ s|^\/\* \$(Id: [^\$]+) \$|\/* \1|; print $_; }' - > ie6.css;

rm header.txt;
