<?php
// $Id$

/**
 * Form submission handlers and data processing functions are contained
 * herein to prevent littering of the main module file.
 */

/*  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *
 * Filter settings
 *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  */

/**
 * Creates the form for the boinc translate input filter settings in
 * the admin interface.
 */
function boinctranslate_filter_settings_form($format = -1) {
  // options array of node types
  $options = array();
  foreach (content_types() as $key => $info) {
    // Exclude node types profile, team, and panel
    switch ($key) {
    case 'forum':
    case 'profile':
    case 'team':
    case 'team_forum':
    case 'panel':
      continue 2;
    }// switch key
    $options[$key] = $info['name'];
  }
  asort($options);

  // Form defaults
  $default = array(
    'boinctranslate_filter_nodetypes' => variable_get('boinctranslate_filter_nodetypes', ''),
  );
  if (empty($default['boinctranslate_filter_nodetypes'])) {
    $default['boinctranslate_filter_nodetypes'] = array('page','story');
  }

  $form = array();
  $form['boinctranslate_filter'] = array(
    '#type' => 'fieldset',
    '#title' => bts('BOINC translate filter', array(), NULL, 'boinc:adminpage-boinctranslate-inputfilter'),
    '#collapsible' => TRUE,
    '#collapsed' => FALSE,
  );
  $form['boinctranslate_filter']['boinctranslate_filter_nodetypes'] = array(
    '#type' => 'checkboxes',
    '#title' => bts('Content types to filter', array(), NULL, 'boinc:adminpage-boinctranslate-inputfilter'),
    '#description' =>  bts('Choose which content (node) types wich will be split into chunks for translation.', array(), NULL, 'boinc:adminpage-boinctranslate-inputfilter'),
    '#options' => $options,
    '#default_value' => $default['boinctranslate_filter_nodetypes'],
  );
  $form['boinctranslate_filter']["boinctranslate_filter_debug_$format"] = array(
    '#type' => 'select',
    '#title' => bts('Print debugging info', array(), NULL, 'boinc:adminpage-boinctranslate-inputfilter'),
    '#default_value' => variable_get("boinctranslate_filter_debug_$format", 0),
    '#options' => array(t('Disabled'), t('Enabled')),
    '#description' => bts('Prints additional debugging information. This option should be disabled on production sites.', array(), NULL, 'boinc:adminpage-boinctranslate-inputfilter'),
  );
  return $form;
}