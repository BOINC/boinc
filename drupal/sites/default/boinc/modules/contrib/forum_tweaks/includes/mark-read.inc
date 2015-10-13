<?php


/**
 * @file
 * Holds functions relating to the Mark Forum/All Read functionality.
 *
 * NOTE: This is pulled from the Advanced forum module. Only the names have
 * been changed to protect the innocent.
 */

/**
 * Either fill a $links array or return a string version of the link to mark read.
 */
function forum_tweaks_get_mark_read_link($tid = 0, &$links = array()) {
  if (forum_tweaks_markasread_access() && !in_array($tid, variable_get('forum_containers', array()))) {
    if ($tid) {
      $links['mark-read']['title'] = t('Mark all topics read');
      $links['mark-read']['href'] = "community/forum/{$tid}/read";

      return l(t('Mark all topics read') . '<span class="image-replace"></span>', "forum/markasread/$tid", array('html' => TRUE));
    }
    else {
      $links['mark-read']['title'] = t('Mark all forums read');
      $links['mark-read']['href'] = "community/forum/0/read";

      return l(t('Mark all forums read') . '<span class="image-replace"></span>', "forum/markasread", array('html' => TRUE));
    }
  }
}

/**
 * Marks all posts in forums or in a given forum as read by the current user.
 */
function forum_tweaks_markasread($current_forum_id = 0) {
  global $user;

  // See if we're on a forum or on the forum overview
  // Path will be /forum/markasread or /forum/markasread/tid
  if ($current_forum_id) {
    // Delete the current history entries so already visited nodes get updated.
    $sql = "DELETE h
            FROM {history} AS h
              INNER JOIN {term_node} AS tn ON (h.nid = tn.nid)
            WHERE h.uid = %d AND tn.tid = %d";
    db_query($sql, $user->uid, $current_forum_id);

    // Update the history table with all forum nodes newer than the cutoff.
    $sql = "INSERT INTO {history} (uid, nid, timestamp)
            SELECT DISTINCT %d, n.nid, %d
            FROM {node} AS n
              INNER JOIN {term_node} AS tn ON n.nid = tn.nid
              INNER JOIN {node_comment_statistics} AS ncs ON ncs.nid = n.nid
            WHERE (n.changed > %d OR ncs.last_comment_timestamp > %d) AND tn.tid = %d";

    $args = array($user->uid, time(), NODE_NEW_LIMIT, NODE_NEW_LIMIT, $current_forum_id);
    db_query($sql, $args);

    // Readpath integration
    if (module_exists('readpath')) {
      readpath_clear_readpath();
    }

    drupal_set_message(t('All content in this forum has been marked as read'));
    drupal_goto('community/forum/' . $current_forum_id);
  }

  // We are on the forum overview, requesting all forums be marked read
  $forum_vocabulary_id = variable_get('forum_nav_vocabulary', '');

  // Delete the current history entries so already visited nodes get updated.
  $sql = "DELETE h
          FROM {history} AS h
            INNER JOIN {term_node} AS tn ON (h.nid = tn.nid)
            INNER JOIN {term_data} AS td ON (td.tid = tn.tid)
          WHERE h.uid = %d AND td.vid = %d";
  db_query($sql, $user->uid, $forum_vocabulary_id);

  // Update the history table with all forum nodes newer than the cutoff.
  $sql = "INSERT INTO {history} (uid, nid, timestamp)
          SELECT DISTINCT %d, n.nid, %d
          FROM {node} AS n
            INNER JOIN {term_node} AS tn ON n.nid=tn.nid
            INNER JOIN {node_comment_statistics} AS ncs ON ncs.nid = n.nid
            INNER JOIN {term_data} AS td ON tn.tid = td.tid
          WHERE (n.changed > %d OR ncs.last_comment_timestamp > %d) AND td.vid = %d";

  $args = array($user->uid, time(), NODE_NEW_LIMIT, NODE_NEW_LIMIT, $forum_vocabulary_id);

  db_query($sql, $args);

  drupal_set_message(t('All forum content has been marked as read'));
  drupal_goto('community/forum');
}

/**
 * Access callback for menus and link display.
 *
 * This separate function is needed because the Drupal 6 menu system doesn't
 * run hook_menu() every time and the logged-in status of the user can get
 * cached and re-used for other users.
 */
function forum_tweaks_markasread_access() {
  global $user;
  return user_access('access content') && $user->uid;
}
