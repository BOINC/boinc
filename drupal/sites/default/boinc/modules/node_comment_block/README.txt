
CONTENTS OF THIS FILE
---------------------

 * About this module
 * Usage

ABOUT THIS MODULE
-----------------

Current maintainer: Oliver Davies (http://drupal.org/user/381388).

This module moves any node comments from within the node variables and into a
block - allowit it to be placed within any region in your page, or place blocks
between the end of the main content and the comments.

USAGE
-----

 * Download and enable the module.
 * Go to admin/structure/blocks and place the 'Node comments' block into a
   region.
 * Optionally, configure it to only display on certain content types,
   for certain roles etc., depending on your requirements.

With the block placed in a region, it loads any node on which it is placed. If
comments are enabled on the node, and comments are found, they are removed from
the usual node variables and placed into the block.
