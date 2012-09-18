<?php

$plugin = array(
  'schema' => 'ctools_custom_content',
  'access' => 'administer custom content',

  'menu' => array(
    'menu item' => 'ctools-content',
    'menu title' => 'Custom content panes',
    'menu description' => 'Add, edit or delete custom content panes.',
  ),

  'title singular' => t('content pane'),
  'title singular proper' => t('Content pane'),
  'title plural' => t('content panes'),
  'title plural proper' => t('Content panes'),

  'handler' => array(
    'class' => 'ctools_custom_content_ui',
    'parent' => 'ctools_export_ui',
  ),
);

