<?php

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
