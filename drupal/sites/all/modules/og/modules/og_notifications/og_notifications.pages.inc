<?php
// $Id: og_notifications.pages.inc,v 1.1.2.2 2010/02/07 13:36:25 amitaibu Exp $

/**
 * @file
 * Group subscriptions management methods.
 */

/**
 * Menu callback: Display the user subscription management forms.
 *
 * @param Object $account
 *   User object of the user whose page is to be displayed.
 */
function og_notifications_user_page($account = NULL) {
  global $user;
  $account = $account ? $account : $user;

  $groups = array();
  foreach($account->og_groups as $gid => $group) {
    // Handle unpublished groups.
    if ($group['status']) {
      $groups[$gid] = $group['title'];
    }
  }

  if (!empty($groups)) {
    $output = drupal_get_form('og_notifications_add_form', $account, $groups);
  }
  else {
    // If there are no active groups available do not show either form. This
    // also applies to unpublished groups even if related subscriptions are
    // still active.
    $output = t('There are no active group subscriptions available.');
  }

  return $output;
}

/**
 * Grouptype subscription creation form.
 *
 * @param Object $account
 *   User object of the user whose page is to be displayed.
 * @return Array $form
 *   Form array.
 */
function og_notifications_add_form($form_state, $account, $groups) {
  $content_types = array_filter(variable_get('og_notifications_content_types', array()));
  $content_names = node_get_types('names');
  foreach ($content_types as $type) {
    $content_types[$type] = $content_names[$type];
  }
  $defaults = _notifications_subscription_defaults($account);
  $send_methods = _notifications_send_methods();
  $send_intervals = _notifications_send_intervals();

  $header = array(t('Group'), t('Type'), t('Send method'), t('Send Interval'));
  // Reuse notifications theme function for the embedded table. This also
  // necessitates the use of a keyed array.
  $form['subscription'] = array(
    '#type' => 'fieldset',
    '#title' => t('Add subscription'),
    '#tree' => TRUE,
    '#header' => &$header
  );
  $form['subscription']['group'][0] = array(
    '#type' => 'select',
    '#options' => $groups
  );
  $form['subscription']['node_type'][0] = array(
    '#type' => 'select',
    '#options' => array('all' => t('All content types')) + $content_types
  );
  // Hide send methods if only one available.
  if (count($send_methods) > 1) {
    $form['subscription']['send_method'][0] = array(
      '#type' => 'select',
      '#options' => $send_methods,
      '#default_value' => $defaults['send_method']
    );
  }
  else {
    // Unset send method column if only one is available.
    unset($header[2]);
    // Pass default outside the subscriptions fieldset to avoid theming issues.
    $form['send_method'] = array(
      '#type' => 'value',
      '#value' => $defaults['send_method']
    );
  }
  $form['subscription']['send_interval'][0] = array(
    '#type' => 'select',
    '#options' => $send_intervals,
    '#default_value' => $defaults['send_interval']
  );
  $form['subscription']['submit'] = array('#type' => 'submit', '#value' => t('Add'));
  $form['account'] = array('#type' => 'value', '#value' => $account);

  return $form;
}

/**
 * Process og_notifications_add_form submission.
 */
function og_notifications_add_form_submit($form, &$form_state) {
  $form_values = $form_state['values'];

  $subscription = new stdClass;
  $subscription->type = 'grouptype';
  $subscription->uid = $form_values['account']->uid;
  $subscription->send_interval = $form_values['subscription']['send_interval'][0];
  $subscription->send_method = isset($form_values['send_method']) ? $form_values['send_method'] : $form_values['subscription']['send_method'][0];
  // String cast due to notifications requiring it as the value field is
  // a varchar.
  $subscription->fields = array('group' => (string) $form_values['subscription']['group'][0], 'type' => $form_values['subscription']['node_type'][0]);
  if ($form_values['subscription']['node_type'][0] == 'all') {
    $types = array_filter(variable_get('og_notifications_content_types', array()));
    foreach ($types as $type) {
      $subscription->fields['type'] = $type;
      notifications_save_subscription($subscription);
      unset($subscription->sid);
    }
  }
  else {
    notifications_save_subscription($subscription);
  }

  drupal_set_message(t('Subscription saved.'));
}

/**
 * Grouptype subscription management form.
 *
 * @param Object $account
 *   User object of the user whose page is to be displayed.
 * @return Array $form
 *   Form array.
 */
function og_notifications_manage_form($form_state, $account) {
  $content_types = array_filter(variable_get('og_notifications_content_types', array()));
  $content_names = node_get_types('names');
  $send_methods = _notifications_send_methods();
  $send_intervals = _notifications_send_intervals();

  $header = array(
    theme('table_select_header_cell'),
    array('data' => t('Group'), 'field' => 'n.title', 'sort' => 'asc'),
    array('data' => t('Type'), 'field' => 'node_type'),
    array('data' => t('Send method'), 'field' => 'no.send_method'),
    array('data' => t('Send Interval'), 'field' => 'no.send_interval')
  );

  $sql = "SELECT n.nid AS group_nid, n.title, nof2.value AS node_type, no.* FROM {notifications} no
    INNER JOIN {notifications_fields} nof1 ON no.sid = nof1.sid
    INNER JOIN {notifications_fields} nof2 ON no.sid = nof2.sid
    INNER JOIN {node} n ON nof1.value = n.nid
    WHERE no.uid = %d AND (no.type = 'grouptype') AND no.conditions = 2 AND nof1.field = 'group' AND nof2.field = 'type' AND n.status = 1";
  $sql .= tablesort_sql($header);
  $count_sql = "SELECT COUNT(nof2.value) FROM {notifications} no
    INNER JOIN {notifications_fields} nof1 ON no.sid = nof1.sid
    INNER JOIN {notifications_fields} nof2 ON no.sid = nof2.sid
    INNER JOIN {node} n ON nof1.value = n.nid
    WHERE no.uid = %d AND (no.type = 'grouptype') AND no.conditions = 2 AND nof1.field = 'group' AND nof2.field = 'type' AND n.status = 1";
  $result = pager_query($sql, 50, 0, $count_sql, $account->uid);

  // Reuse notifications theme function for the embedded table.
  $form['subscriptions'] = array(
    '#type' => 'fieldset',
    '#title' => t('Current subscriptions'),
    '#tree' => TRUE,
    '#collapsible' => TRUE,
    '#theme' => 'notifications_form_table',
    '#header' => &$header
  );
  $subscriptions_current = array();
  while ($subscription = db_fetch_object($result)) {
    $key = $subscription->sid;
    $subscriptions_current[$key] = $subscription;
    $form['subscriptions']['checkbox'][$key] = array(
      '#type' => 'checkbox',
      '#default_value' => $subscription->sid
    );
    $form['subscriptions']['group'][$key] = array('#value' => l($subscription->title, 'node/'. $subscription->group_nid));
    $form['subscriptions']['node_type'][$key] = array('#value' => $content_names[$subscription->node_type]);
    // Hide send methods if only one available.
    if (count($send_methods) > 1) {
      $form['subscriptions']['send_method'][$key] = array(
        '#type' => 'select',
        '#options' => $send_methods,
        '#default_value' => $subscription->send_method
      );
    }
    else {
      // Unset send method column if only one is available.
      unset($header[3]);
      // Pass default outside the subscriptions fieldset to avoid theming issues.
      $form['send_method'] = array(
        '#type' => 'value',
        '#value' => $subscription->send_method
      );
    }
    $form['subscriptions']['send_interval'][$key] = array(
      '#type' => 'select',
      '#options' => $send_intervals,
      '#default_value' => $subscription->send_interval
    );
  }

  if (empty($subscriptions_current)) {
    $form = array();
  }
  else {
    $form['subscriptions']['current'] = array('#type' => 'value', '#value' => $subscriptions_current);
    $form['subscriptions']['submit'] = array('#type' => 'submit', '#value' => t('Update'));
  }

  return $form;
}

/**
 * Process og_notifications_manage_form form submission.
 */
function og_notifications_manage_form_submit($form, &$form_state) {
  $form_values = $form_state['values'];
  $current = $form_values['subscriptions']['current'];
  foreach ($form_values['subscriptions']['checkbox'] as $sid => $check) {
    $subscription = $current[$sid];
    // Insert fields array as per notifications' requirements.
    $subscription->fields = array('group' => (string) $subscription->group_nid, 'type' => $subscription->node_type);
    if ($check == 1) {
      if ($subscription->send_interval != $form_values['subscriptions']['send_interval'][$sid] || (!isset($form_values['send_method']) && $subscription->send_method != $form_values['subscriptions']['send_method'][$sid])) {
        $subscription->send_interval = $form_values['subscriptions']['send_interval'][$sid];
        $subscription->send_method = isset($form_values['send_method']) ? $form_values['send_method'] : $form_values['subscriptions']['send_method'][$sid];
        // String cast due to notifications requiring it as the value field is
        // a varchar.
        notifications_save_subscription($subscription);
      }
    }
    else {
      // Unsubscribe if not checked ...
      notifications_delete_subscription($sid);
    }
  }

  drupal_set_message(t('Subscriptions updated.'));
}
