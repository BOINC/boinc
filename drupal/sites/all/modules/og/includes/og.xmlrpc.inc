<?php

//TODO: this function does not perform complete access control. It is safe to use when users are subscribing selves to groups. It should not be used for one user to subscribe another to a group
function og_xmlrpc_subscribe_user($username, $password, $gid, $uid) {
  $admin = user_authenticate($username, $password);
  if ($admin->uid) {
    $account = user_load(array('uid' => $uid));
    if ($account->uid) {
      $return = og_subscribe_user($gid, $account);
      return $return;
    }
    else {
      return og_xmlrpc_error(t('Unrecognized user ID: %uid.', array('%uid' => $uid)));
    }
  }
  else {
    return og_xmlrpc_error(t('Wrong username or password.'));
  }
}

function og_xmlrpc_get_all_subscribers($username, $password, $gid, $min_is_active, $min_is_admin) {
  $account = user_authenticate($username, $password);
  if ($account->uid) {
    if (in_array($gid, array_keys($account->og_groups)) || user_access('administer organic groups', $account)) {
      $sql = og_list_users_sql($min_is_active, $min_is_admin);
      $result = db_query($sql, $gid);
      while ($row = db_fetch_array($result)) {
        $users[$row['uid']] = $row;
      }
      return $users;
    }
    else {
      return og_xmlrpc_error(t('User is not a member of the specified group.'));
    }
  }
  else {
    return og_xmlrpc_error(t('Wrong username or password.'));
  }
}

function og_xmlrpc_get_user_groups($username, $password, $uid) {
  $caller = user_authenticate($username, $password);
  if ($caller->uid) {
    if ($caller->uid == $uid || user_access('administer organic groups', $caller)) {
      return og_get_subscriptions($uid);  
    }
    else {
      return og_xmlrpc_error(t('User is not allowed to retrieve membership information for another user.'));
    }
  }
  else {
    return og_xmlrpc_error(t('Wrong username or password.'));
  }
}

/**
 * Prepare an error message for returning to the XMLRPC caller.
 */
function og_xmlrpc_error($message) {
  static $xmlrpcusererr;
  if (!is_array($message)) {
    $message = array($message);
  }
  $message = implode(' ', $message);

  return xmlrpc_error($xmlrpcusererr + 1, strip_tags($message));
}
