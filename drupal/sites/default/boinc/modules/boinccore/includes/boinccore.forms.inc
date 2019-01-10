<?php
// $Id$

/**
 * Form submission handlers
 */

/**
 * Implement form boinccore_delete_confirm for deleting nodes,
 * separate text for user profiles.
 */
function boinccore_delete_confirm(&$form_state, $node) {
  $form = array();
  $form['nid'] = array(
    '#type' => 'value',
    '#value' => $node->nid,
  );

  // Build confirm form from elements, depending on node type.
  $description =  bts('This action cannot be undone.', array(), NULL, 'boinc:delete-node');
  $no = bts('Cancel', array(), NULL, 'boinc:form-cancel');

  switch($node->type) {
  case 'profile':
    $question = bts('Are you sure you want to clear your profile?', array(), NULL, 'boinc:delete-node');
    $path = isset($_GET['destination']) ? $_GET['destination'] : 'account';
    $yes = bts('Confirm', array(), NULL, 'boinc:form-confirm');
    break;

  default:
    $question = bts('Are you sure you want to Delete %title?', array('%title' => $node->title), NULL, 'boinc:delete-node');
    $path = isset($_GET['destination']) ? $_GET['destination'] : 'node/'. $node->nid;
    $yes = bts('Delete', array(), NULL, 'boinc:form-delete');
  }
  return confirm_form($form, $question, $path, $description, $yes, $no);
}

/**
 * Execute node deletion.
 */
function boinccore_delete_confirm_submit($form, &$form_state) {
  if ($form_state['values']['confirm']) {
    node_delete($form_state['values']['nid']);
  }

  $form_state['redirect'] = 'account';
}
