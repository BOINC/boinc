<?php
// $Id$

/**
* @file
* Administration pages for the boincstats module
*/


/**
  * The BOINC stats configuration determines where statistics are sourced
  */
function boincstats_admin_stats_system(&$form_state) {
  $form = array();
  $default = array(
    'boinc_stats_server' => variable_get('boinc_stats_server', 'stats.gridrepublic.org'),
    'boinc_stats_project_list_rpc' => variable_get('boinc_stats_project_list_rpc', 'rpc/get_projects.php'),
    'boinc_stats_project_credit_history_rpc' => variable_get('boinc_stats_project_credit_history_rpc', 'rpc2/get_project_tc_history.php'),
    'boinc_stats_user_credit_history_rpc' => variable_get('boinc_stats_user_credit_history_rpc', 'rpc2/get_user_credit_history.php'),
    'boinc_stats_remote_project_id' => variable_get('boinc_stats_remote_project_id', ''),
  );
  // Define the form
  $form['boinc_stats_server'] = array(
    '#type' => 'value',
    '#value' => $default['boinc_stats_server']
  );
  $form['boinc_stats_project_list_rpc'] = array(
    '#type' => 'value',
    '#value' => $default['boinc_stats_project_list_rpc']
  );
  $form['boinc_stats_project_credit_history_rpc'] = array(
    '#type' => 'value',
    '#value' => $default['boinc_stats_project_credit_history_rpc']
  );
  $form['boinc_stats_user_credit_history_rpc'] = array(
    '#type' => 'value',
    '#value' => $default['boinc_stats_user_credit_history_rpc']
  );
  $form['boinc_stats_remote_project_id'] = array(
    '#type' => 'textfield',
    '#title' => t('Stats ID for @project', array(
      '@project' => variable_get('site_name', 'Drupal-BOINC'))),
    '#default_value' => $default['boinc_stats_remote_project_id'],
    '#description' => t('This project must be registered with a stats system in
      order for some statistics to be available. Currently only one system is
      supported. !find_project_id there.', array(
        '!find_project_id' => l(t('Find your project ID'),
        "http://{$default['boinc_stats_server']}/{$default['boinc_stats_project_list_rpc']}",
        array('attributes' => array('target' => 'external')))
      )
    )
  );
  return system_settings_form($form);
}

/**
  * Validate the BOINC stats form.
  */
function boincstats_admin_stats_system_validate($form, &$form_state) {
  $values = $form_state['values'];
  if (!$values['boinc_stats_remote_project_id']) {
    form_set_error('boinc_stats_remote_project_id',
      t('Stats may not work without a valid stats ID configured!'));
  }
}

/**
  * Handle post-validation submission of BOINC stats form.
  */
function boincstats_admin_stats_system_submit($form, &$form_state) {
  drupal_set_message(t('Stats system configuration has been updated.'));
}
