<?php

$plugin = array(
  'schema' => 'panels_mini',
  'access' => 'administer mini panels',
  'create access' => 'create mini panels',

  'menu' => array(
    'menu item' => 'mini-panels',
    'menu title' => 'Mini panels',
    'menu description' => 'Add, edit or delete mini panels, which can be used as blocks or content panes in other panels.',
  ),

  'title singular' => t('mini panel'),
  'title singular proper' => t('Mini panel'),
  'title plural' => t('mini panels'),
  'title plural proper' => t('Mini panels'),

  'handler' => array(
    'class' => 'panels_mini_ui',
    'parent' => 'ctools_export_ui',
  ),

  'use wizard' => TRUE,
  'form info' => array(
    'order' => array(
      'basic' => t('Settings'),
      'context' => t('Context'),
      'layout' => t('Layout'),
      'content' => t('Content'),
    ),
    // We have to add this form specially because it's invisible.
    'forms' => array(
      'move' => array(
        'form id' => 'ctools_export_ui_edit_item_wizard_form',
      ),
    ),
  ),

);

