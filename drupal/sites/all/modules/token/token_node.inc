<?php

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
 * Implements hook_token_list() on behalf of node.module.
 */
function node_token_list($type = 'all') {
  $tokens = array();

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
  }

  return $tokens;
}

/**
 * Implements hook_token_values() on behalf of node.module.
 */
function node_token_values($type, $object = NULL, $options = array()) {
  $values = array();

  if ($type == 'node' && !empty($object)) {
    $node = $object;
    $account = db_fetch_object(db_query("SELECT name, mail FROM {users} WHERE uid = %d", $node->uid));

    // Adjust for the anonymous user name.
    if (!$node->uid && !$account->name) {
      $account->name = variable_get('anonymous', t('Anonymous'));
    }

    $values['nid']                = $node->nid;
    $values['type']               = $node->type;
    $values['type-name']          = node_get_types('name', $node->type);
    $values['language']           = check_plain($node->language);
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
      $values += token_get_date_token_values($node->created, '');
    }

    if (isset($node->changed)) {
      $values += token_get_date_token_values($node->changed, 'mod-');
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
  }

  return $values;
}

/**
 * Implements hook_token_list() on behalf of menu.module.
 */
function menu_token_list($type = 'all') {
  $tokens = array();

  if ($type == 'node' || $type == 'all') {
    $tokens['node']['menu']                = t("The name of the menu the node belongs to.");
    $tokens['node']['menu-raw']            = t("The name of the menu the node belongs to.");
    $tokens['node']['menupath']            = t("The menu path (as reflected in the breadcrumb), not including Home or [menu]. Separated by /.");
    $tokens['node']['menupath-raw']        = t("The unfiltered menu path (as reflected in the breadcrumb), not including Home or [menu]. Separated by /.");
    $tokens['node']['menu-link-title']     = t("The text used in the menu as link text for this item.");
    $tokens['node']['menu-link-title-raw'] = t("The unfiltered text used in the menu as link text for this item.");
    $tokens['node']['menu-link-mlid']      = t("The unique ID of the node's menu link.");
    $tokens['node']['menu-link-plid']      = t("The unique ID of the node's menu link parent.");
    $tokens['node']['menu-link-parent-path']     = t('The URL alias of the parent menu link of the node.');
    $tokens['node']['menu-link-parent-path-raw'] = t('The URL alias of the parent menu link of the node.');
  }

  return $tokens;
}

/**
 * Implements hook_token_values() on behalf of menu.module.
 */
function menu_token_values($type, $object = NULL, $options = array()) {
  static $menus;

  $values = array();

  // Statically cache menu_get_menus() since this causes a database query
  // every time it is called, and is not likely to change in the same page
  // request.
  if (!isset($menus)) {
    $menus = menu_get_menus();
  }

  if ($type == 'node' && !empty($object)) {
    $node = $object;

    // Initialize tokens to empty strings.
    $values['menu']                = '';
    $values['menu-raw']            = '';
    $values['menupath']            = '';
    $values['menupath-raw']        = '';
    $values['menu-link-title']     = '';
    $values['menu-link-title-raw'] = '';
    $values['menu-link-mlid']      = '';
    $values['menu-link-plid']      = '';
    $values['menu-link-parent-path'] = '';
    $values['menu-link-parent-path-raw'] = '';

    if (!isset($node->menu)) {
      // We need to clone the node as menu_nodeapi($node, 'prepare') may cause data loss.
      // @see http://drupal.org/node/1317926
      $node = drupal_clone($node);
      // Nodes do not have their menu links loaded via menu_nodeapi($node, 'load').
      menu_nodeapi($node, 'prepare');
    }

    // Now get the menu related information.
    if (!empty($node->menu['mlid'])) {
      $link = token_menu_link_load($node->menu['mlid']);

      if (isset($menus[$link['menu_name']])) {
        $values['menu-raw'] = $menus[$link['menu_name']];
        $values['menu']     = check_plain($values['menu-raw']);
      }

      $parents = token_menu_link_get_parents_all($link['mlid']);
      $trail_raw = array();
      foreach ($parents as $parent) {
        $trail_raw[] = $parent['title'];
      }
      $trail = array_map('check_plain', $trail_raw);

      $values['menupath']            = !empty($options['pathauto']) ? $trail : implode('/', $trail);
      $values['menupath-raw']        = !empty($options['pathauto']) ? $trail_raw : implode('/', $trail_raw);
      $values['menu-link-title']     = check_plain($link['title']);
      $values['menu-link-title-raw'] = $link['title'];
      $values['menu-link-mlid']      = $link['mlid'];

      if (!empty($link['plid']) && $parent = token_menu_link_load($link['plid'])) {
        $values['menu-link-plid'] = $parent['mlid'];
        $values['menu-link-parent-path-raw'] = drupal_get_path_alias($parent['href']);
        $values['menu-link-parent-path']     = check_plain($values['menu-link-parent-path-raw']);
      }
    }
  }

  return $values;
}

/**
 * Implements hook_token_list() on behalf of book.module.
 */
function book_token_list($type) {
  $tokens = array();

  if ($type == 'node' || $type == 'all') {
    $tokens['book']['book']         = t("The title of the node's book parent.");
    $tokens['book']['book-raw']     = t("The title of the node's book parent.");
    $tokens['book']['book_id']      = t("The id of the node's book parent.");
    $tokens['book']['bookpath']     = t("The titles of all parents in the node's book hierarchy.");
    $tokens['book']['bookpath-raw'] = t("The titles of all parents in the node's book hierarchy.");
  }

  return $tokens;
}

/**
 * Implements hook_token_values() on behalf of book.module.
 */
function book_token_values($type, $object = NULL, $options = array()) {
  $values = array();

  if ($type == 'node' && !empty($object)) {
    $node = $object;

    // Initialize tokens to empty strings.
    $values['book'] = '';
    $values['book-raw'] = '';
    $values['book_id'] = '';
    $values['bookpath'] = '';
    $values['bookpath-raw'] = '';

    if (!empty($node->book['mlid'])) {
      // Exclude the current node's title from the book path trail (start with
      // the book link's plid rather than mlid).
      $parents = token_menu_link_get_parents_all($node->book['plid']);
      $trail_raw = array();
      foreach ($parents as $parent) {
        $trail_raw[] = $parent['title'];
      }
      $trail = array_map('check_plain', $trail_raw);

      // Load the root book page.
      $root = token_menu_link_load($node->book['p1']);

      $values['book'] = check_plain($root['title']);
      $values['book-raw'] = $root['title'];
      $values['book_id'] = $node->book['bid'];
      $values['bookpath'] = !empty($options['pathauto']) ? $trail : implode('/', $trail);
      $values['bookpath-raw'] = !empty($options['pathauto']) ? $trail_raw : implode('/', $trail_raw);
    }
  }

  return $values;
}
