<?php
// $Id: token_node.inc,v 1.5.4.30 2010/07/24 20:25:04 davereid Exp $

/**
 * @file
 * Implementations of token module hooks for the core node and book modules.
 *
 * The token module requires specific hooks to be added to modules
 * so that those modules can return data about their objects to the
 * token API.  Until and unless token becomes a part of core, the
 * implementations of the token hooks for core modules are provided
 * in the token module itself.
 *
 * @ingroup token
 */

/**
 * Implementation of hook_token_values().
 */
function node_token_values($type, $object = NULL, $options = array()) {
  $values = array();
  switch ($type) {
    case 'node':
      $node = $object;
      $account = db_fetch_object(db_query("SELECT name, mail FROM {users} WHERE uid = %d", $node->uid));

      // Adjust for the anonymous user name.
      if (!$node->uid && !$account->name) {
        $account->name = variable_get('anonymous', t('Anonymous'));
      }

      $values['nid']                = $node->nid;
      $values['type']               = $node->type;
      $values['type-name']          = node_get_types('name', $node->type);
      $values['language']           = filter_xss_admin($node->language);
      $values['title']              = check_plain($node->title);
      $values['title-raw']          = $node->title;
      $values['node-path-raw']      = drupal_get_path_alias('node/'. $node->nid);
      $values['node-path']          = check_plain($values['node-path-raw']);
      $values['node-url']           = url('node/' . $node->nid, array('absolute' => TRUE));
      $values['author-uid']         = $node->uid;
      $values['author-name']        = check_plain($account->name);
      $values['author-name-raw']    = $account->name;
      $values['author-mail']        = check_plain($account->mail);
      $values['author-mail-raw']    = $account->mail;

      $values['log-raw']            = isset($node->log) ? $node->log : '';
      $values['log']                = filter_xss($values['log-raw']);

      if (module_exists('comment')) {
        $values['node_comment_count'] = isset($node->comment_count) ? $node->comment_count : 0;
        $values['unread_comment_count'] = comment_num_new($node->nid);
      }
      else {
        $values['node_comment_count'] = 0;
        $values['unread_comment_count'] = 0;
      }

      if (isset($node->created)) {
        $values += token_get_date_token_values($node->created);
      }

      if (isset($node->changed)) {
        $values += token_get_date_token_values($node->changed, 'mod-');
      }

      // Try to get the menu data.
      $mlid = db_result(db_query("SELECT mlid FROM {menu_links} WHERE link_path = '%s'", 'node/'. $node->nid));

      // Now get the menu related information.
      if (!empty($mlid) || !empty($node->menu['mlid']) || !empty($node->menu['plid'])) {
        $menu_link = menu_link_load($mlid);
        $menus = menu_get_menus();
        $menu = isset($menus[$menu_link['menu_name']]) ? $menus[$menu_link['menu_name']] : '';
        $trail_raw = _menu_titles($menu_link, $node->nid);

        $trail = array();
        foreach ($trail_raw as $title) {
          $trail[] = check_plain($title);
        }

        $values['menupath']            = implode('/', $trail);
        $values['menupath-raw']        = implode('/', $trail_raw);
        $values['menu']                = check_plain($menu);
        $values['menu-raw']            = $menu;
        $values['menu-link-title']     = check_plain($menu_link['title']);
        $values['menu-link-title-raw'] = $menu_link['link_title'];
        $values['menu-link-mlid']      = $menu_link['mlid'];
        $values['menu-link-plid']      = $menu_link['plid'];
      }
      else {
        $values['menu']                = '';
        $values['menu-raw']            = '';
        $values['menupath']            = '';
        $values['menupath-raw']        = '';
        $values['menu-link-title']     = '';
        $values['menu-link-title-raw'] = '';
        $values['menu-link-mlid']      = '';
        $values['menu-link-plid']      = '';
      }

      // And now taxonomy, which is a bit more work. This code is adapted from
      // pathauto's handling code; it's intended for compatibility with it.
      if (module_exists('taxonomy') && !empty($node->taxonomy) && is_array($node->taxonomy)) {
        foreach ($node->taxonomy as $term) {
          $original_term = $term;
          if ((object)$term) {
            // With free-tagging it's somewhat hard to get the tid, vid, name values
            // Rather than duplicating taxonomy.module code here you should make sure your calling module
            // has a weight of at least 1 which will run after taxonomy has saved the data which allows us to
            // pull it out of the db here.
            if (!isset($term->name) || !isset($term->tid)) {
              $vid = db_result(db_query_range("SELECT t.vid FROM {term_node} r INNER JOIN {term_data} t ON r.tid = t.tid INNER JOIN {vocabulary} v ON t.vid = v.vid WHERE r.nid = %d ORDER BY v.weight, t.weight, t.name", $object->nid, 0, 1));
              if (!$vid) {
                continue;
              }
              $term = db_fetch_object(db_query_range("SELECT t.tid, t.name FROM {term_data} t INNER JOIN {term_node} r ON r.tid = t.tid WHERE t.vid = %d AND r.vid = %d ORDER BY t.weight", $vid, $object->vid, 0, 1));
              $term->vid = $vid;
            }

            // Ok, if we still don't have a term name maybe this is a pre-taxonomy submit node
            // So if it's a number we can get data from it
            if (!isset($term->name) && is_array($original_term)) {
              $tid = array_shift($original_term);
              if (is_numeric($tid)) {
                $term = taxonomy_get_term($tid);
              }
            }
            $values['term'] = check_plain($term->name);
            $values['term-raw'] = $term->name;
            $values['term-id'] = $term->tid;
            $vid = $term->vid;

            if (!empty($vid)) {
              $vocabulary = taxonomy_vocabulary_load($vid);
              $values['vocab'] = check_plain($vocabulary->name);
              $values['vocab-raw'] = $vocabulary->name;
              $values['vocab-id'] = $vocabulary->vid;
            }

            // The 'catpath' (and 'cat') tokens have been removed, as they caused quite a bit of confusion,
            // and the catpath was a relatively expensive query when the taxonomy tree was deep.
            //
            // It existed only to provide forward-compatability with pathauto module, and
            // for most uses of token.module, it was a relatively useless token -- it exposed
            // a list of term names formatted as a URL/path string. Once pathauto supports
            // tokens, *it* should handle this catpath alias as it's the primary consumer.
            break;
          }
        }
      }
      // It's possible to leave that block and still not have good data.
      // So, we test for these and if not set, set them.
      if (!isset($values['term'])) {
        $values['term'] = '';
        $values['term-raw'] = '';
        $values['term-id'] = '';
        $values['vocab'] = '';
        $values['vocab-raw'] = '';
        $values['vocab-id'] = '';
      }

      break;
  }

  return $values;
}

/**
 * Implementation of hook_token_list().
 */
function node_token_list($type = 'all') {
  if ($type == 'node' || $type == 'all') {
    $tokens['node']['nid']             = t('The unique ID of the content item, or "node".');
    $tokens['node']['type']            = t('The type of the node.');
    $tokens['node']['type-name']       = t('The human-readable name of the node type.');
    $tokens['node']['language']        = t('The language the node is written in.');
    $tokens['node']['title']           = t('The title of the node.');
    $tokens['node']['title-raw']       = t('The title of the node.');
    $tokens['node']['node-path']       = t('The URL alias of the node.');
    $tokens['node']['node-path-raw']   = t('The URL alias of the node.');
    $tokens['node']['node-url']        = t('The URL of the node.');

    $tokens['node']['author-uid']      = t("The unique ID of the author of the node.");
    $tokens['node']['author-name']     = t("The login name of the author of the node.");
    $tokens['node']['author-name-raw'] = t("The login name of the author of the node.");
    $tokens['node']['author-mail']     = t("The email address of the author of the node.");
    $tokens['node']['author-mail-raw'] = t("The email address of the author of the node.");

    $tokens['node']['log']     = t('The explanation of the most recent changes made to the node.');
    $tokens['node']['log-raw'] = t('The explanation of the most recent changes made to the node.');

    $tokens['node'] += token_get_date_token_info(t('Node creation'));
    $tokens['node'] += token_get_date_token_info(t('Node modification'), 'mod-');

    if (module_exists('comment')) {
      $tokens['node']['node_comment_count']   = t("The number of comments posted on a node.");
      $tokens['node']['unread_comment_count'] = t("The number of comments posted on a node since the reader last viewed it.");
    }

    if (module_exists('taxonomy')) {
      $tokens['node']['term']            = t("Name of top taxonomy term");
      $tokens['node']['term-raw']        = t("Unfiltered name of top taxonomy term.");
      $tokens['node']['term-id']         = t("ID of top taxonomy term");
      $tokens['node']['vocab']           = t("Name of top term's vocabulary");
      $tokens['node']['vocab-raw']       = t("Unfiltered name of top term's vocabulary.");
      $tokens['node']['vocab-id']        = t("ID of top term's vocabulary");
      // Temporarily disabled -- see notes in node_token_values.
      // $tokens['node']['catpath']        = t("Full taxonomy tree for the topmost term");
    }

    if (module_exists('menu')) {
      $tokens['node']['menu']                = t("The name of the menu the node belongs to.");
      $tokens['node']['menu-raw']            = t("The name of the menu the node belongs to.");
      $tokens['node']['menupath']            = t("The menu path (as reflected in the breadcrumb), not including Home or [menu]. Separated by /.");
      $tokens['node']['menupath-raw']        = t("The unfiltered menu path (as reflected in the breadcrumb), not including Home or [menu]. Separated by /.");
      $tokens['node']['menu-link-title']     = t("The text used in the menu as link text for this item.");
      $tokens['node']['menu-link-title-raw'] = t("The unfiltered text used in the menu as link text for this item.");
      $tokens['node']['menu-link-mlid']      = t("The unique ID of the node's menu link.");
      $tokens['node']['menu-link-plid']      = t("The unique ID of the node's menu link parent.");
    }

    return $tokens;
  }
}

/**
 * Return an array of titles for a menu, from the top down to the specified node
 *
 * @param $menu_link
 *   Fully loaded menu link, of node to which a path is desired.
 * @param $nid
 *   Id of node.
 * @return
 *   An array of titles through the specified node
 */
function _menu_titles($menu_link, $nid) {
  $tree = menu_tree_all_data($menu_link['menu_name'], $menu_link);

  // Get mlid of all nodes in path - top-most parent to leaf node.
  $parents = array();
  for ($i = 1; $i < MENU_MAX_DEPTH; $i++) {
    if ($menu_link["p$i"]) {
      $parents[] = $menu_link["p$i"];
    }
  }

  // Build the titles in this hierarchy.
  $titles = array();
  $current = array_shift($tree);
  while ($current) {
    if (in_array($current['link']['mlid'], $parents)) {
      $titles[] = $current['link']['title'];
      if ($current['link']['href'] == "node/". $nid) {
        break;
      }
      // Go deeper in tree hierarchy.
      $tree = $current['below'];
    }
    // Go to next sibling at same level in tree hierarchy.
    $current = $tree ? array_shift($tree) : NULL;
  }
  return $titles;
}

/**
 * Implementation of hook_token_values() for book nodes
 */
function book_token_values($type, $object = NULL, $options = array()) {
  if ($type == 'node') {
    $node = $object;

    // Initialize some variables to empty.
    $tokens['book'] = '';
    $tokens['book-raw'] = '';
    $tokens['book_id'] = '';
    $tokens['bookpath'] = '';
    $tokens['bookpath-raw'] = '';

    if (!empty($node->book['menu_name'])) {
      $menus = menu_get_menus();
      $trail_raw = _menu_titles($node->book, $node->nid);
      $book_raw = $trail_raw[0];
      $book = check_plain($book_raw);
      // For book paths, we don't include the current node's title (last in
      // the array) in the trail.
      array_pop($trail_raw);
      $trail = array();
      foreach ($trail_raw as $title) {
        $trail[] = check_plain($title);
      }
      $book_title = isset($trail[0]) ? $trail[0] : '';
      $tokens = array();
      $tokens['book'] = $book;
      $tokens['book-raw'] = $book_raw;
      $tokens['book_id'] = $node->book['bid'];
      $tokens['bookpath'] = implode('/', $trail);
      $tokens['bookpath-raw'] = implode('/', $trail_raw);
    }

    return $tokens;
  }
}

function book_token_list($type) {
  if ($type == 'node' || $type == 'all') {
    $list['book']['book']           = t("The title of the node's book parent.");
    $list['book']['book_id']        = t("The id of the node's book parent.");
    $list['book']['bookpath']       = t("The titles of all parents in the node's book hierarchy.");

    $list['book']['book-raw']       = t("The unfiltered title of the node's book parent.");
    $list['book']['bookpath-raw']   = t("The unfiltered titles of all parents in the node's book hierarchy.");

    return $list;
  }
}

/**
 * Check if mid/path is present in the menu.
 *
 * @param $in
 *   Numeric input is treated as a menu-id, strings as src-paths.
 * @return
 *   An existing mid, or 0 if none found.
 */
function token_menu_get_mid($in) {
  global $_menu;

  if (!is_numeric($in)) {
    if (isset($_menu['path index'][$in])) {
      $mid = $_menu['path index'][$in];
    }
    else {
      $mid = 0;
    }
  }
  else if (!isset($_menu['visible'][$in])) {
    $mid = 0;
  }

  // Temporary paths would break much of this module.
  if ($mid < 0) {
    $mid = 0;
  }

  return $mid;
}
