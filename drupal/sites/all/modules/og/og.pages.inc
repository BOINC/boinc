<?php
// $Id: og.pages.inc,v 1.1.2.5 2010/03/14 07:57:59 amitaibu Exp $

/**
 * @file
 * Page callbacks for Organic groups.
 */

/**
 * Admins may broadcast messages to all their members.
 *
 * @ingroup forms
 * @param $node
 *   The group node.
 */
 function og_broadcast_form($form_state, $node) {
   drupal_set_title(t('Send message to %group', array('%group' => $node->title)));

   if (!empty($form_state['post'])) {
     drupal_set_message(t('Your message will be sent to all members of this group.'));
   }
   
   $form['subject'] = array(
    '#type' => 'textfield', 
    '#title' => t('Subject'), 
    '#size' => 70,
    '#maxlength' => 250,
    '#description' => t('Enter a subject for your message.'), 
    '#required' => TRUE,
  );
  $form['body'] = array(
    '#type' => 'textarea', 
    '#title' => t('Body'), 
    '#rows' => 5,
    '#cols' => 90,
    '#description' => t('Enter a body for your message.'), 
    '#required' => TRUE
  );
  $form['send'] = array('#type' => 'submit', '#value' => t('Send message'));
  $form['gid'] = array('#type' => 'value', '#value' => $node->nid);
  return $form;
}

function og_broadcast_form_submit($form, &$form_state) {
  global $user;

  $sql = og_list_users_sql(1);
  $result = db_query($sql, $form_state['values']['gid']);
  $recipients = array();
  while ($row = db_fetch_object($result)) {
    $recipients[] = $row->uid;
  }

  $node = node_load($form_state['values']['gid']);
  $variables = array(
    '@group' => $node->title,
    '@subject' => $form_state['values']['subject'],
    '@body' => $form_state['values']['body'],
    '@site' => variable_get('site_name', 'drupal'),
    '!url_group' => url("node/$node->nid", array('absolute' => TRUE)),
    '!url_unsubscribe' => url("og/unsubscribe/$node->nid/$user->uid", array('absolute' => TRUE))
  );

  $message = array(
    'from' => $user,
    'subject' => $form_state['values']['subject'],
    'body' => _og_mail_text('og_admin_email_body', $variables)
  );

  // Send notifications to each member; Sending an array of recipients implies
  // that this is a bulk message.
  module_invoke_all('og', 'user broadcast', $node->nid, $recipients, $message);
  drupal_set_message(format_plural(count($recipients), '1 message queued for delivery.', '@count messages queued for delivery.'));
  
}

function og_manage($group_node) {
  global $user;

  $bc = og_get_breadcrumb($group_node);
  drupal_set_breadcrumb($bc);
  
  return drupal_get_form('og_manage_form', $group_node);
}

function og_manage_form($form_state, $group) {
  global $user;
  // avoid double messages on form submit
  if (!$form_state['post']) {
    // group manager can't leave
    if ($group->og_selective == OG_CLOSED) {
      drupal_set_message(t('You may not leave this group because it is a <em>closed</em> group. You should request removal from a group administrator.'));
    }
    elseif ($group->uid == $user->uid) {
      drupal_set_message(t('You may not leave this group because you are its owner. A site administrator can assign ownership to another user and then you may leave.'));
    }
    else {
      $links[] = l(t('Leave this group'), "og/unsubscribe/$group->nid/$user->uid");
      $form['unsubscribe'] = array('#value' => theme('item_list', $links));
    }
  }

  $form['gid'] = array('#type' => 'value', '#value' => $group->nid);
  return $form;
}

function og_manage_form_submit($form, &$form_state) {
  global $user;
  $passed_values = $form_state['values'];
  unset($passed_values['gid'], $passed_values['op'], $passed_values['form_id'], $passed_values['form_build_id'], $passed_values['form_token']);
  og_save_subscription($form_state['values']['gid'], $user->uid, $passed_values);
  drupal_set_message(t('Membership saved.'));
}

/**
 * OG create admin form
 */
function og_create_admin_confirm($form_state, $node, $account) {
  $form['node'] = array('#type' => 'value', '#value' => $node);
  $form['account'] = array('#type' => 'value', '#value' => $account);
  return confirm_form($form, 
           t('Are you sure you want to make !name a group administrator for the group %title?', array('!name' => theme('username', $account), '%title' => $node->title)),
           "og/users/$node->nid", 
           ' ',
           t('Confirm'), 
           t('Cancel'));
}

/**
 * Confirm og create admin form
 */
function og_create_admin_confirm_submit($form, &$form_state) {
  $account = $form_state['values']['account'];
  $node = $form_state['values']['node'];

  og_save_subscription($node->nid, $account->uid, array('is_admin' => 1));
  drupal_set_message(t('%name was promoted to <em>group administrator</em>.', array('%name' => $account->name)));

  $variables = array(
    '@group' => $node->title,
    '!group_url' => url("node/$node->nid", array('absolute' => TRUE)),
    '@username' => $account->name
  );

  $message = array(
    'subject' => _og_mail_text('og_new_admin_subject', $variables),
    'body' => _og_mail_text('og_new_admin_body', $variables)
  );

  module_invoke_all('og', 'admin new', $node->nid, $account->uid, $message);

  $form_state['#redirect'] = "og/users/$node->nid";
}

/**
 * OG remove admin form
 */
function og_remove_admin_confirm($form_state, $node, $account) {
  $form['gid'] = array('#type' => 'value', '#value' => $node->nid);
  $form['account'] = array('#type' => 'value', '#value' => $account);
  return confirm_form($form, 
    t('Are you sure you want to remove !name as a group administrator for the group %title?', array('!name' => theme('username', $account), '%title' => $node->title)),
    "og/users/$node->nid",
    ' ', 
    t('Remove'), 
    t('Cancel')
  );
}

/**
 * Confirm og remove admin form
 */
function og_remove_admin_confirm_submit($form, &$form_state) {
  $account = $form_state['values']['account'];
  $gid = $form_state['values']['gid'];
  og_save_subscription($gid, $account->uid, array('is_admin' => 0));
  drupal_set_message(t('%name is no longer a <em>group administrator</em>.', array('%name' => $account->name)));
  $form_state['redirect'] = "og/users/$gid";
}

function og_invite_form($form_state, $node) {
  $bc = og_get_breadcrumb($node);
  drupal_set_breadcrumb($bc);
  
  $max = variable_get('og_email_max', 10);

  $form['mails'] = array(
    '#type' => 'textarea', 
    '#title' => t('Email addresses or usernames'),  
    '#description' => t('Enter up to %max email addresses or usernames. Separate multiple addresses by commas or new lines. Each person will receive an invitation message from you.', array('%max' => $max))
  );
  $form['pmessage'] = array(
    '#type' => 'textarea', 
    '#title' => t('Personal message'), 
    '#description' => t('Optional. Enter a message which will become part of the invitation email.')
  );
  $form['op'] = array('#type' => 'submit', '#value' => t('Send invitation'));
  $form['gid'] = array ('#type' => 'value', '#value' => $node->nid);
  return $form;
}


// TODOL Use #element_validate as per http://drupal.org/node/144132#element-validate
function og_invite_form_validate($form, &$form_state) {
  global $user;
  $max = variable_get('og_email_max', 10);

  $mails = $form_state['values']['mails'];
  $mails = str_replace("\n", ',', $mails);
  $emails = explode(',', $mails);
  if (count($emails) > $max) {
    form_set_error('mails', t('You may not specify more than %max email addresses or usernames.', array('%max' => $max)));
  }
  elseif (in_array($user->mail, $emails)) {
    form_set_error('mails', t('You may not invite yourself - @self.', array('@self' => $user->mail)));
  }
  else {
    $valid_emails = array();
    $bad = array();
    foreach ($emails as $email) {
      $email = trim($email);
      if (empty($email)) {
        continue;
      }
      if (valid_email_address($email)) {
        $valid_emails[] = $email;
      }
      else {
        $account = user_load(array('name' => check_plain($email)));
        if ($account->mail) {
          $valid_emails[] = $account->mail;
        }
        else {
          $bad[] = $email;
        }
      }
    }
    if (count($bad)) {
      form_set_error('mails', t('Invalid email address or username: @value.', array('@value' => implode(', ', $bad))));
    }
    else {
      // Store valid e-mails so we don't have to go through that looping again on submit
      $form_state['valid_emails'] = $valid_emails;
    }
  }
}

function og_invite_form_submit($form, &$form_state) {
  $emails = $form_state['valid_emails'];
  $node = node_load($form_state['values']['gid']);

  $variables = array(
    '@group' => $node->title,
    '@description' => $node->og_description,
    '@site' => variable_get('site_name', 'drupal'),
    '!group_url' => url("og/subscribe/$node->nid", array('absolute' => TRUE)),
    '@body' => $form_state['values']['pmessage'],
  );
  
  global $user;
  $from = $user->mail;
  foreach ($emails as $mail) {
    drupal_mail('og', 'invite_user', $mail, $GLOBALS['language'], $variables, $from);
  }
  drupal_set_message(format_plural(count($emails), '1 invitation sent.', '@count invitations sent.'));
}

function og_subscribe($node, $uid = NULL) {
  global $user;
  if (is_null($uid)) {
    if ($user->uid) {
      $account = $user;
    }
    else {
      $dest = drupal_get_destination();
      if (variable_get('user_register', 1) == 0) {
        drupal_set_message(t('In order to join this group, you must <a href="!login">login</a>. After you have successfully done so, you will need to request membership again.', array('!login' => url("user/login", array('query' => $dest)))));
      }
      else {
        drupal_set_message(t('In order to join this group, you must <a href="!login">login</a> or <a href="!register">register</a> a new account. After you have successfully done so, you will need to request membership again.', array('!register' => url("user/register", array('query' => $dest)), '!login' => url("user/login", array('query' => $dest)))));
      }
      drupal_goto('user');
    }
  }
  else {
    $account = user_load(array('uid' => $uid));
  }

  if ($node->og_selective >= OG_INVITE_ONLY || $node->status == 0 || !og_is_group_type($node->type)) {
    drupal_access_denied();
    exit();
  }
  
  // Only admins can add another member.
  if ($account->uid != $user->uid && !og_is_group_admin($node)) {
    drupal_access_denied();
    exit();
  }
  // User is already a member of this group, redirect to group homepage.
  else if (isset($account->og_groups[$node->nid])) {
    drupal_set_message(t('@user is already a member the group @group.', array('@user' => $account->name, '@group' => $node->title)));
    drupal_goto('node/'. $node->nid);
  }
  else {
    return drupal_get_form('og_confirm_subscribe', $node->nid, $node, $account);
  }
}

/**
 * Confirm og membership form
 */
function og_confirm_subscribe($form_state, $gid, $node, $account) {
  $form['gid'] = array('#type' => 'value', '#value' => $gid);
  $form['account'] = array('#type' => 'value', '#value' => $account);
  if ($node->og_selective == OG_MODERATED) {
    $form['request'] = array(
      '#type' => 'textarea', 
      '#title' => t('Additional details'), 
      '#description' => t('Add any detail which will help an administrator decide whether to approve or deny your membership request.')
    );
  }
  else {
    $form['request'] = array(
      '#type' => 'value', 
      '#value' => '',
   );
 }
 return confirm_form($form, 
                t('Are you sure you want to join the group %title?', array('%title' => $node->title)),
                'node/'. $node->nid, ' ',
                t('Join'), t('Cancel'));
}

/**
 * Confirm og membership submit handler
 */
function og_confirm_subscribe_submit($form, &$form_state) {
  $return = og_subscribe_user($form_state['values']['gid'], $form_state['values']['account'], $form_state['values']['request']);
  if (!empty($return['message'])) {
      drupal_set_message($return['message']);
  }
  $form_state['redirect'] = 'node/'. $form_state['values']['gid'];
} 


/**
 * Confirm og unsubscription form
 */
function og_confirm_unsubscribe($form_state, $group_node, $account) {
  global $user;
  $form['group_node'] = array('#type' => 'value', '#value' => $group_node);
  $form['account'] = array('#type' => 'value', '#value' => $account);
  if ($account->uid == $user->uid) {
    $message = t('Are you sure you want to leave the group %title?', array('%title' => $group_node->title));
    $confirm_text = t("Leave"); 
  }
  else {
    $message = t('Are you sure you want to remove !name from the group %title?', array('!name' => theme('username', $account), '%title' => $group_node->title));
    $confirm_text = t("Remove");
  }
  return confirm_form($form, $message, 'og/users/'. $group_node->nid, ' ', $confirm_text, t('Cancel'));
}

/**
 * Confirm og unsubscription submit handler
 */
function og_confirm_unsubscribe_submit($form, &$form_state) {
  global $user;
  $group_node = $form_state['values']['group_node'];
  $account = $form_state['values']['account'];
  og_delete_subscription($group_node->nid, $account->uid);
  // If needed, reload user object to reflect unsubscribed group.
  if ($user->uid == $account->uid) {
    og_get_subscriptions($account->uid, 1, TRUE); // Clear cache.
    $user = user_load(array('uid' => $user->uid));
    $message = t('You left the group %group.', array('%group' => $group_node->title));
  } 
  else {
    $message = t('%user removed from %group.', array('%user' => $account->name, '%group' => $group_node->title));
  }
  drupal_set_message($message);
  
  // Determine where to go next. GHP if accessible, or else site front page.
  $form_state['redirect'] = node_access('view', $group_node) ? "node/". $group_node->nid : '';
}

function og_add_users($form_state, $group_node) {
  $form['og_names'] = array(
    '#type' => 'textarea',
    '#title' => t('List of users'),
    '#rows' => 5,
    '#cols' => 70,
    // No autocomplete b/c user_autocomplete can't handle commas like taxonomy. pls improve core.
    // '#autocomplete_path' => 'user/autocomplete',
    '#description' => t('Add one or more usernames in order to associate users with this group. Multiple usernames should be separated by a comma.'),
    '#element_validate' => array('og_add_users_og_names_validate'),
  );
  $form['op'] = array('#type' => 'submit', '#value' => t('Add users'));
  $form['gid'] = array('#type' => 'value', '#value' => $group_node->nid);
  return $form;
}

// An #element_validate handler
function og_add_users_og_names_validate($form, $form_state) {
  $names = explode(',', $form_state['values']['og_names']);
  foreach ($names as $name) {
    $account = user_load(array('name' => trim($name)));
    if (isset($account->uid)) {
      $accounts[] = $account;
      $uids[] = $account->uid;
    }
    else {
      $bad[] = check_plain($name);
      $err = TRUE;
    }
  }
  if (isset($err)) {
    form_set_error('og_names', format_plural(count($bad), 'Unrecognized name: %bad.', 'Unrecognized names: %bad.', array('%bad' => implode(', ', $bad))));
  }
}

function og_add_users_submit($form, &$form_state) {
  // Safest option is to do a select, filter existing members, then insert.
  $names = explode(',', $form_state['values']['og_names']);
  foreach ($names as $name) {
    $account = user_load(array('name' => trim($name)));
    if ($account->uid) {
      $accounts[] = $account;
    }
  }
  foreach ($accounts as $account) {
    og_save_subscription($form_state['values']['gid'], $account->uid, array('is_active' => 1));
  }
  drupal_set_message(format_plural(count($accounts), '1 user added to the group.', '@count users added to the group.'));
}

function og_page_activity() {
  $sql = "SELECT oga.group_nid, node_group_nid.title, node_group_nid.uid, u.name, COUNT(*) as ncount, MAX(n.created) as ncreated, SUM(ncs.comment_count) as ccount, MAX(last_comment_timestamp) AS lct FROM {node} n INNER JOIN {og_ancestry} oga ON n.nid = oga.nid LEFT JOIN {node_comment_statistics} ncs ON n.nid = ncs.nid INNER JOIN {node} node_group_nid ON oga.group_nid = node_group_nid.nid INNER JOIN {users} u ON node_group_nid.uid = u.uid WHERE n.status = 1 GROUP BY oga.group_nid, node_group_nid.title, node_group_nid.uid, u.name";
  $header = array(
    array('data' => t('Title'), 'field' => 'node_group_nid.title'),
    array('data' => t('Manager'), 'field' => 'u.name'),
    array('data' => t('Posts'), 'field' => 'ncount'),
    array('data' => t('Comments'), 'field' => 'ccount'),
    array('data' => t('Age'), 'field' => 'ncreated'),
    array('data' => t('Last comment'), 'field' => 'lct', 'sort' => 'asc'),
  );
  $result = db_query($sql. tablesort_sql($header));
  while ($row = db_fetch_object($result)) {
    $rows[] = array(
      l($row->title, "node/$row->group_nid"),
      theme('username', $row),
      $row->ncount,
      $row->ccount,
      format_interval(time()-$row->ncreated),
      format_interval(time()-$row->lct),
    );
  }
  if (!isset($rows)) {
    $rows[] = array(array('data' => t('No groups available.'), 'colspan' => 6));
  }
  return theme('table', $header, $rows);
  
}

