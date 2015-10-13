<?php

// Plugin definition
$plugin = array(
  'title' => t('Three column 25/50/25 stacked'),
  'category' => t('Columns: 3'),
  'icon' => 'threecol_25_50_25_stacked.png',
  'theme' => 'panels_threecol_25_50_25_stacked',
  'css' => 'threecol_25_50_25_stacked.css',
  'panels' => array(
    'top' => t('Top'),
    'left' => t('Left side'),
    'middle' => t('Middle column'),
    'right' => t('Right side'),
    'bottom' => t('Bottom'),
  ),
);
