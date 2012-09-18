<?php

/**
 * @file
 * Themable for the wizard tool.
 */

function ctools_wizard_theme(&$theme) {
  $theme['ctools_wizard_trail'] = array(
    'arguments' => array('trail'),
    'file' => 'includes/wizard.theme.inc',
  );
}

/**
 * Themable display of the 'breadcrumb' trail to show the order of the
 * forms.
 */
function theme_ctools_wizard_trail($trail) {
  if (!empty($trail)) {
    return '<div class="wizard-trail">' . implode(' » ', $trail) . '</div>';
  }
}

