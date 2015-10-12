<?php

// Plugin definition
$plugin = array(
  'title' => t('Three column 33/34/33 stacked'),
  'category' => t('Columns: 3'),
  'icon' => 'threecol_33_34_33_stacked.png',
  'theme' => 'panels_threecol_33_34_33_stacked',
  'css' => 'threecol_33_34_33_stacked.css',
  'panels' => array(
    'top' => t('Top'),
    'left' => t('Left side'),
    'middle' => t('Middle column'),
    'right' => t('Right side'),
    'bottom' => t('Bottom')
  ),
);
