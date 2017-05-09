<?php

define('EC_DRUPAL_VERSION', 6);

/***************************************************************
 * D6 VERSION 
 ***************************************************************/

function _dcf_hook_boot($module) {
  return true;
}

function _dcf_hook_init($module) {
  return true;
}

function _dcf_hook_menu($items, $maycache) {
  $new_items = array();
  foreach ($items as $k => $v)
    $new_items[_dcf_internal_path($k)] = $v;
  return $new_items;
}

function _dcf_convert_render_array(&$a) {
  if (!empty($a['#markup'])) {
    $a['#value'] = $a['#markup'];
    unset($a['#markup']);
  }
  if (!empty($a['#type']) && $a['#type'] == 'actions')
    unset($a['#type']);
  foreach ($a as $k => &$v) 
    if (is_array($v))
      _dcf_convert_render_array($v);
}

function _dcr_render_array($output) {
  foreach ($output as $k => &$v)
    if ((is_numeric($k) || $k{0} != '#') && is_string($v))
      $v = array( '#type' => 'markup', '#value' => $v, '#weight' => -1 );
  _dcf_convert_render_array($output);
  return drupal_render($output);
}

function _dcr_form(&$form) {
  foreach ($form as $k => &$v)
    if ((is_numeric($k) || $k{0} != '#') && is_string($v))
      $v = array( '#type' => 'markup', '#value' => $v, '#weight' => -1 );
  _dcf_convert_render_array($form);
  return $form;
}

function _dcf_internal_path($path) {
  return str_replace(
    array( 'admin/config/system/cron', 'admin/modules' ),
    array( 'admin/build/cron', 'admin/build/modules' ),
    $path
  );
}
  
function _dcf_t($string) {
  return t($string);
}

function _dco_watchdog($type, $message, $variables = array(), $severity = WATCHDOG_NOTICE, $link = NULL) { // WARN d7 changed WATCHDOG_ costants
  return watchdog($type, $message, $variables, $severity, $link);
}

function _dco_l($text, $path, array $options = array()) {
  return l($text, $path, $options);
}

function _dcf_form_validate(&$form, &$form_state) {
  return array('form' => &$form, 'form_state' => &$form_state);
}

function _dco_theme($name, $args) {
  return call_user_func_array('theme', array_merge(array($name), array_values($args)));
}

function _dcf_theme_signature($args) {
  return array( 'variables' => $args );
}

function _dcr_hook_theme($specs) {
  foreach ($specs as $k => $v) {
    if (!empty($v['variables'])) {
      $v['arguments'] = $v['variables'];
      unset($v['variables']);
    }
    if (!empty($v['render element'])) {
      $v['arguments'] = array ( $v['render element'] => NULL );
      unset($v['render element']);
    }
  }
  return $specs;
}

function _dcf_theme_form(&$args) {
  return array( 'variables' => array( 'form' => $args ) );
}

/***************************************************************
 * D6 MISSING FUNCTIONS 
 ***************************************************************/

function drupal_render_children($form) {
  return drupal_render(_dcr_form($form));
}

/***************************************************************
 * D6 EXTRA FUNCTIONS 
 ***************************************************************/

function drupal_module_get_min_weight($except_module = false) {
  return !$except_module ? db_result(db_query("select min(weight) from {system}")) :
    db_result(db_query("select min(weight) from {system} where name != '%s'", $except_module));
}

function drupal_module_get_weight($name) {
  return db_result(db_query("select weight from {system} where name = '%s'", $name));  
}

function drupal_module_set_weight($name, $weight) {
  db_query("update {system} set weight = %d where name = '%s'", $weight, $name);
}
