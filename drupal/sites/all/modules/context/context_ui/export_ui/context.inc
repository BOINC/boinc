<?php
// $Id: context.inc,v 1.1.2.1 2010/08/05 16:46:26 yhahn Exp $

$plugin = array(
  'schema' => 'context',
  'menu' => array(
    'menu prefix' => 'admin/build',
    'menu item' => 'context',
    'menu title' => 'Context',
    'menu description' => 'Associate menus, views, blocks, etc. with different contexts to structure your site.',
  ),
  'title singular' => t('context'),
  'title singular proper' => t('Context'),
  'title plural' => t('contexts'),
  'title plural proper' => t('Contexts'),
  'form' => array(
    'settings' => 'context_ui_form',
    'submit' => 'context_ui_form_submit',
  ),
  'handler' => array(
    'class' => 'context_export_ui',
    'parent' => 'ctools_export_ui',
  ),
);
