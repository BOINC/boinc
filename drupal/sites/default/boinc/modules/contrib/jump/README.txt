*******************************************************
    README.txt for jump.module for Drupal
*******************************************************
The jump module allows administrators to create a quick dropdown "Jump Menu"
from any configured menu or vocabulary in the system.  The module provides a
block for every top-level menu and every vocabulary.  Jump menu blocks are
named "Jump menu: vocabulary name" and "Jump menu: menu name".

In addition, administrators may continue to use the jump_quickly() API that
was used in the Drupal 5.x version, with the exception being that menus are
now referenced by name as opposed to id.

To programmatically make a jump menu appear in a block whose input format is
set to "PHP code" use one of:

<?php
  print jump_quickly(5, 'taxo');
?>

Where 5 is the vocabulary with vid 5.

* OR *

<?php
  print jump_quickly('primary-links');
?>

Where 'primary-links' could be the name of any menu in the system.

* OR *

<?php
  global $user;
  $options = array(
    'user/'. $user->uid => t('My User Account'),
    'tracker/'. $user->uid => t('My Posts'),
    'logout' => t('Log Out')
  );
  print jump_quickly($options);
?>

Where the keys to the $options array are menu paths and the values in the
array are labels that appear in the drop-down.

Starting with version 6.x-1.1 you can pass a 3rd parameter to jump_quickly()
that specifies whether to base the default selection in the jump menu on the
currently viewed page.  Examples for how to do this are:

<?php
  print jump_quickly('primary-links', 'menu', TRUE);
  print jump_quickly('vocabulary name', 'taxo', TRUE);
  print jump_quickly($options, 'custom', TRUE);
?>

Where $options in the last example is a $options array that's formatted
the same way as the one given earlier in this document.  For jump module
supplied blocks, there is a checkbox on the block configuration form that
controls this setting on a per-block basis.  There's also a global setting
at admin/settings/jump that is the fallback setting for blocks that don't
have a preference set.

Note: At this time there is no automatic upgrade code to convert 5.x
API calls using id-based menus to the 6.x name-based menus.
