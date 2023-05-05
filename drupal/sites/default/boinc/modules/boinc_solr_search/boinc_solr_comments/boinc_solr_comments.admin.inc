<?php
// $Id$

/**
 * @file
 * Administration page callbacks for boinc_solr_comments module.
 */

function boinc_solr_comments_form(&$form_state) {
  $form = array();

  $form['config'] = array(
    '#type' => 'fieldset',
    '#title' => t('Configuration'),
    '#collapsible' => TRUE,
  );

  $options = array();
  foreach (content_types() as $key => $info) {
    // Exclude node types profile, team, and panel
    switch ($key) {
      case 'profile':
      case 'team':
      case 'panel':
        continue 2;
    }// switch key
    $options[$key] = $info['name'];
  }
  asort($options);

  // Form defaults
  $default = array(
    'boinc_solr_comments_nodetypes' => variable_get('boinc_solr_comments_nodetypes', ''),
  );
  if (empty($default['boinc_solr_comments_nodetypes'])) {
    $default['boinc_solr_comments_nodetypes'] = array('forum','team_forum','news','page','story');
  }

  // Define the form
  $form['config']['nodes'] = array(
    '#type'  => 'markup',
    '#value' => bts('Select the content types where the comments are indexed by Apace Solr. Your content must be re-indexed after changing this configuration.', array(), NULL, 'boinc:admin-solr-search-comments-option'),
  );

  $form['config']['boinc_solr_comments_nodetypes'] = array(
    '#type'          => 'checkboxes',
    '#title'         => 'node',
    '#options'       => $options,
    '#default_value' => $default['boinc_solr_comments_nodetypes'],
  );

  return system_settings_form($form);
}

