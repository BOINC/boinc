<?php
// $Id: user_from_userref.inc,v 1.1.2.3 2009/07/21 20:06:12 markuspetrux Exp $

/**
 * @file
 * Implements the user reference relationship for Panels.
 */

/**
 * Implementation of hook_ctools_relationships().
 */
function userreference_user_from_userref_ctools_relationships() {
  return array(
    'title' => t('User from reference'),
    'keyword' => 'userreference',
    'description' => t('Adds a user from a user reference in a node context; if multiple users are referenced, this will get the first referenced user only.'),
    'required context' => new ctools_context_required(t('Node'), 'node'),
    'context' => 'userreference_user_from_userref_context',
    'settings form' => 'userreference_user_from_userref_settings_form',
    'settings form validate' => 'userreference_user_from_userref_settings_form_validate',
  );
}

/**
 * Return a new ctools context based on an existing context.
 */
function userreference_user_from_userref_context($context, $conf) {
  // If unset it wants a generic, unfilled context, which is just NULL.
  if (empty($context->data)) {
    return ctools_context_create_empty('user', NULL);
  }

  // Prevent whitescreens on stale data.
  if (empty($conf)) {
    return ctools_context_create_empty('user', NULL);
  }

  if (isset($context->data->{$conf['field_name']}[0]['uid']) && ($uid = $context->data->{$conf['field_name']}[0]['uid'])) {
    if ($account = user_load(array('uid' => $uid))) {
      return ctools_context_create('user', $account);
    }
  }
}

/**
 * Settings form for the ctools relationship.
 */
function userreference_user_from_userref_settings_form($conf) {
  $options = array();
  foreach (content_fields() as $field) {
    if ($field['type'] == 'userreference') {
      $options[$field['field_name']] = t($field['widget']['label']);
    }
  }
  $form['field_name'] = array(
    '#title' => t('User reference field'),
    '#type' => 'select',
    '#options' => $options,
    '#default_value' => isset($conf['field_name']) ? $conf['field_name'] : '',
    '#prefix' => '<div class="clear-block">',
    '#suffix' => '</div>',
  );

  return $form;
}
