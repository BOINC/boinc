<?php

/**
 * @file
 * Hooks provided by Panels In-Place Editor.
 */

/**
 * Allow modules to control access to the Panels IPE.
 *
 * @param panels_display $display
 *   The panels display about to be rendered.
 *
 * @return TRUE|FALSE|NULL
 *   Returns TRUE to allow access, FALSE to deny, or NULL if the module
 *   implementing this hook doesn't care about access for the given display.
 */
function hook_panels_ipe_access($panels_display) {
  // We only care about displays with the 'panelizer' context.
  if (!isset($display->context['panelizer'])) {
    return NULL;
  }

  if ($display->context['panelizer']->type[0] == 'entity:node') {
    // Allow or deny IPE access based on node type.
    return $display->context['panelizer']->data->type == 'awesome_page';
  }

  // Otherwise, deny access to everything!
  return FALSE;
}
