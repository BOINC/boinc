<?php
// $Id: export-ui.menu.inc,v 1.1.2.2 2010/07/21 23:04:59 merlinofchaos Exp $

/**
 * Delegated implementation of hook_menu().
 */
function ctools_export_ui_menu(&$items) {
  ctools_include('export-ui');

  foreach (ctools_get_export_uis() as $plugin) {
    if ($plugin['has menu']) {
      $handler = ctools_export_ui_get_handler($plugin);
      if ($handler) {
        $handler->hook_menu($items);
      }
    }
  }
}
