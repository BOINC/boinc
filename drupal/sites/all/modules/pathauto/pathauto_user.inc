<?php

/**
 * @file
 * Hook implementations for user module integration.
 *
 * @ingroup pathauto
 */

/**
 * Implements hook_pathauto().
 */
function user_pathauto($op) {
  switch ($op) {
    case 'settings':
      $settings = array();
      $settings['module'] = 'user';
      $settings['token_type'] = 'user';
      $settings['groupheader'] = t('User paths');
      $settings['patterndescr'] = t('Pattern for user account page paths');
      $settings['patterndefault'] = 'users/[user-raw]';
      $patterns = token_get_list('user');
      foreach ($patterns as $type => $pattern_set) {
        if ($type != 'global') {
          foreach ($pattern_set as $pattern => $description) {
            $settings['placeholders']['['. $pattern .']'] = $description;
          }
        }
      }
      $settings['bulkname'] = t('Bulk generate aliases for users that are not aliased');
      $settings['bulkdescr'] = t('Generate aliases for all existing user account pages which do not already have aliases.');
      return (object) $settings;
    default:
      break;
  }
}

/**
 * Implements hook_pathauto().
 */
function blog_pathauto($op) {
  switch ($op) {
    case 'settings':
      $settings = array();
      $settings['module'] = 'blog';
      $settings['token_type'] = 'user';
      $settings['groupheader'] = t('Blog paths');
      $settings['patterndescr'] = t('Pattern for blog page paths');
      $settings['patterndefault'] = 'blogs/[user-raw]';
      $patterns = token_get_list('user');
      foreach ($patterns['user'] as $pattern => $description) {
        $settings['placeholders']['['. $pattern .']'] = $description;
      }
      $settings['supportsfeeds'] = 'feed';
      $settings['bulkname'] = t('Bulk generate aliases for blogs that are not aliased');
      $settings['bulkdescr'] = t('Generate aliases for all existing blog pages which do not already have aliases.');
      return (object) $settings;
    default:
      break;
  }
}

/**
 * Implementation of hook_pathauto() for user-tracker aliases.
 */
function tracker_pathauto($op) {
  switch ($op) {
    case 'settings':
      $settings = array();
      $settings['module'] = 'tracker';
      $settings['token_type'] = 'user';
      $settings['groupheader'] = t('User tracker paths');
      $settings['patterndescr'] = t('Pattern for user-tracker page paths');
      $settings['patterndefault'] = 'users/[user-raw]/track';
      $patterns = token_get_list('user');
      foreach ($patterns['user'] as $pattern => $description) {
        $settings['placeholders']['['. $pattern .']'] = $description;
      }
      $settings['supportsfeeds'] = 'feed';
      $settings['bulkname'] = t('Bulk generate aliases for user-tracker paths that are not aliased');
      $settings['bulkdescr'] = t('Generate aliases for all existing user-tracker pages which do not already have aliases.');
      return (object) $settings;
    default:
      break;
  }
}

/**
 * Implements hook_pathauto().
 */
function contact_pathauto($op) {
  switch ($op) {
    case 'settings':
      $settings = array();
      $settings['module'] = 'contact';
      $settings['token_type'] = 'user';
      $settings['groupheader'] = t('User contact forms paths');
      $settings['patterndescr'] = t('Pattern for the user contact form paths');
      $settings['patterndefault'] = 'users/[user-raw]/contact';
      $patterns = token_get_list('user');
      foreach ($patterns['user'] as $pattern => $description) {
        $settings['placeholders']['['. $pattern .']'] = $description;
      }
      $settings['bulkname'] = t('Bulk generate aliases for user contact form paths that are not aliased');
      $settings['bulkdescr'] = t('Generate aliases for all existing user contact form pages which do not already have aliases.');
      return (object) $settings;
    default:
      break;
  }
}

/**
 * Bulk generate aliases for all users without aliases.
 */
function user_pathauto_bulkupdate() {
  $concat = _pathauto_sql_concat("'user/'", 'u.uid');
  $sql = "SELECT u.uid FROM {users} u LEFT JOIN {url_alias} ua ON $concat = ua.src WHERE u.uid > 0 AND ua.src IS NULL";
  $query = db_query_range($sql, 0, variable_get('pathauto_max_bulk_update', 50));

  $count = 0;
  $placeholders = array();
  while ($uid = db_result($query)) {
    $account = user_load($uid);
    $placeholders = pathauto_get_placeholders('user', $account);
    $source = 'user/'. $account->uid;
    if (pathauto_create_alias('user', 'bulkupdate', $placeholders, $source, $account->uid)) {
      $count++;
    }
  }

  drupal_set_message(format_plural($count,
    'Bulk generation of users completed, one alias generated.',
    'Bulk generation of users completed, @count aliases generated.'));
}

/**
 * Bulk generate aliases for all blogs without aliases.
 */
function blog_pathauto_bulkupdate() {
  $concat = _pathauto_sql_concat("'blog/'", 'u.uid');
  $sql = "SELECT u.uid FROM {users} u LEFT JOIN {url_alias} ua ON $concat = ua.src WHERE u.uid > 0 AND ua.src IS NULL";
  $query = db_query_range($sql, 0, variable_get('pathauto_max_bulk_update', 50));

  $count = 0;
  $placeholders = array();
  while ($uid = db_result($query)) {
    $account = user_load($uid);
    $placeholders = pathauto_get_placeholders('user', $account);
    $source = 'blog/'. $account->uid;
    if (pathauto_create_alias('blog', 'bulkupdate', $placeholders, $source, $account->uid)) {
      $count++;
    }
  }

  drupal_set_message(format_plural($count,
    'Bulk generation of user blogs completed, one alias generated.',
    'Bulk generation of user blogs completed, @count aliases generated.'));
}

/**
 * Bulk generate aliases for user trackers without aliases.
 */
function tracker_pathauto_bulkupdate() {
  $concat = _pathauto_sql_concat("'user/'", 'u.uid', "'/track'");
  $sql = "SELECT u.uid FROM {users} u LEFT JOIN {url_alias} ua ON $concat = ua.src WHERE u.uid > 0 AND ua.src IS NULL";
  $query = db_query_range($sql, 0, variable_get('pathauto_max_bulk_update', 50));

  $count = 0;
  $placeholders = array();
  while ($uid = db_result($query)) {
    $account = user_load($uid);
    $placeholders = pathauto_get_placeholders('user', $account);
    $src = 'user/'. $account->uid .'/track';
    if (pathauto_create_alias('tracker', 'bulkupdate', $placeholders, $src, $account->uid)) {
      $count++;
    }
  }

  drupal_set_message(format_plural($count,
    'Bulk generation of user tracker pages completed, one alias generated.',
    'Bulk generation of user tracker pages completed, @count aliases generated.'));
}

/**
 * Bulk generate aliases for all users without aliases
 */
function contact_pathauto_bulkupdate() {
  $sql = "SELECT u.uid FROM {users} u LEFT JOIN {url_alias} ua ON CONCAT(CONCAT('user/', CAST(u.uid AS CHAR)), '/contact') = ua.src WHERE u.uid > 0 AND ua.src IS NULL";
  $query = db_query_range($sql, 0, variable_get('pathauto_max_bulk_update', 50));

  $count = 0;
  $placeholders = array();
  while ($uid = db_result($query)) {
    $account = user_load($uid);
    $placeholders = pathauto_get_placeholders('user', $account);
    $source = 'user/'. $account->uid .'/contact';
    if (pathauto_create_alias('contact', 'bulkupdate', $placeholders, $source, $account->uid)) {
      $count++;
    }
  }

  drupal_set_message(format_plural($count,
    'Bulk generation of contact pages completed, one alias generated.',
    'Bulk generation of contact pages completed, @count aliases generated.'));
}
