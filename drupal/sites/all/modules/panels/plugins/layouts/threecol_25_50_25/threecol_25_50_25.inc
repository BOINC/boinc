<?php

/**
 * implementation of hook_panels_layouts
 */
// Plugin definition
$plugin = array(
  'title' => t('Three column 25/50/25'),
  'category' => t('Columns: 3'),
  'icon' => 'threecol_25_50_25.png',
  'theme' => 'panels_threecol_25_50_25',
  'theme arguments' => array('id', 'content'),
  'css' => 'threecol_25_50_25.css',
  'panels' => array(
    'left' => t('Left side'),
    'middle' => t('Middle column'),
    'right' => t('Right side')
  ),
);

