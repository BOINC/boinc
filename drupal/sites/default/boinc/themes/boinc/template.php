<?php
// $Id: template.php,v 1.21 2009/08/12 04:25:15 johnalbin Exp $

/**
 * @file
 * Contains theme override functions and preprocess functions for the theme.
 *
 * ABOUT THE TEMPLATE.PHP FILE
 *
 *   The template.php file is one of the most useful files when creating or
 *   modifying Drupal themes. You can add new regions for block content, modify
 *   or override Drupal's theme functions, intercept or make additional
 *   variables available to your theme, and create custom PHP logic. For more
 *   information, please visit the Theme Developer's Guide on Drupal.org:
 *   http://drupal.org/theme-guide
 *
 * OVERRIDING THEME FUNCTIONS
 *
 *   The Drupal theme system uses special theme functions to generate HTML
 *   output automatically. Often we wish to customize this HTML output. To do
 *   this, we have to override the theme function. You have to first find the
 *   theme function that generates the output, and then "catch" it and modify it
 *   here. The easiest way to do it is to copy the original function in its
 *   entirety and paste it here, changing the prefix from theme_ to boinc_.
 *   For example:
 *
 *     original: theme_breadcrumb()
 *     theme override: boinc_breadcrumb()
 *
 *   where boinc is the name of your sub-theme. For example, the
 *   zen_classic theme would define a zen_classic_breadcrumb() function.
 *
 *   If you would like to override any of the theme functions used in Zen core,
 *   you should first look at how Zen core implements those functions:
 *     theme_breadcrumbs()      in zen/template.php
 *     theme_menu_item_link()   in zen/template.php
 *     theme_menu_local_tasks() in zen/template.php
 *
 *   For more information, please visit the Theme Developer's Guide on
 *   Drupal.org: http://drupal.org/node/173880
 *
 * CREATE OR MODIFY VARIABLES FOR YOUR THEME
 *
 *   Each tpl.php template file has several variables which hold various pieces
 *   of content. You can modify those variables (or add new ones) before they
 *   are used in the template files by using preprocess functions.
 *
 *   This makes THEME_preprocess_HOOK() functions the most powerful functions
 *   available to themers.
 *
 *   It works by having one preprocess function for each template file or its
 *   derivatives (called template suggestions). For example:
 *     THEME_preprocess_page    alters the variables for page.tpl.php
 *     THEME_preprocess_node    alters the variables for node.tpl.php or
 *                              for node-forum.tpl.php
 *     THEME_preprocess_comment alters the variables for comment.tpl.php
 *     THEME_preprocess_block   alters the variables for block.tpl.php
 *
 *   For more information on preprocess functions and template suggestions,
 *   please visit the Theme Developer's Guide on Drupal.org:
 *   http://drupal.org/node/223440
 *   and http://drupal.org/node/190815#template-suggestions
 */


/**
 * Implementation of HOOK_theme().
 */
function boinc_theme(&$existing, $type, $theme, $path) {
  $hooks = zen_theme($existing, $type, $theme, $path);
  // Add your theme hooks like this:
  /*
  $hooks['hook_name_here'] = array( // Details go here );
  */
  // @TODO: Needs detailed comments. Patches welcome!
  return $hooks;
}


/**
 * Adjust the rendering of the menu
 */
function boinc_links__system_main_menu($links, $menu, $element) {
  $html .= '<ul id="' . $menu['id'] . '" class="' . $menu['class'] . '">' . "\n";
  $item_count = count($links);
  $i = 1;
  foreach ($links as $key => $link) {
    $classes = array($key);
    if (strpos($key, 'active-trail')) $classes[] = 'active';
    if ($i == 1) $classes[] = 'first';
    if ($i == $item_count) $classes[] = 'last';
    $html .= '<li class="' . implode(' ', $classes) .'">';
    if (module_exists('privatemsg')) {
      // Put a new mail notification next to the Account menu item
      if ($link['href'] == 'dashboard') {
        $item_count = privatemsg_unread_count();
        if ($item_count) {
          $link['title'] .= '</a> <a href="/messages" class="compound secondary"><div class="item-count-wrapper"><span class="item-count">' . $item_count . '</span></div>';
          $link['html'] = TRUE;
          $link['attributes']['class'] = 'compound';
        }
      }
    }
    // Put a count of items on the Moderation menu item
    if ($link['href'] == 'moderate') {
      $item_count = boincuser_moderation_queue_count();
      if ($item_count) {
        $link['title'] .= ' <div class="item-count-wrapper"><span class="item-count">' . $item_count . '</span></div>';
        $link['html'] = TRUE;
      }
    }
    $html .= l($link['title'], $link['href'], $link);
    $html .= '</li>';
    $i++;
  }
  $html .= '</ul>' . "\n";
  return $html;
}


/**
 * Remove undesired local task tabs
 */
function boinc_menu_local_task($link, $active = FALSE) {
  if (strpos($link, 'admin/build/pages') !== FALSE
  AND strpos($link, 'Edit Panel')) {
    // Remove Edit Panel tab
    return '';
  }
  else {
    return '<li '. ($active ? 'class="active" ' : '') .'>'. $link ."</li>\n";
  }
}


/**
 * Override or insert variables into all templates.
 *
 * @param $vars
 *   An array of variables to pass to the theme template.
 * @param $hook
 *   The name of the template being rendered (name of the .tpl.php file.)
 */
/* -- Delete this line if you want to use this function
function boinc_preprocess(&$vars, $hook) {
  //$vars['sample_variable'] = t('Lorem ipsum.');
  drupal_add_feed(
    url(
      'rss.xml',
      array('absolute' => TRUE)
    ),
    'BOINC'
  );
  $vars['head'] = drupal_get_html_head();
  $vars['feed_icons'] = drupal_get_feeds();
}
// */

/**
 * Override or insert variables into the page templates.
 *
 * @param $vars
 *   An array of variables to pass to the theme template.
 * @param $hook
 *   The name of the template being rendered ("page" in this case.)
 */
///* -- Delete this line if you want to use this function
function boinc_preprocess_page(&$vars, $hook) {
    
    $server_status_url = variable_get('boinc_server_status_url', '');
    if (!$server_status_url) {
      $server_status_url = 'server_status.php';
    }
    $vars['server_status_url'] = $server_status_url;
    
    // Remove title from search page
    if (arg(0) == 'search') {
      unset($vars['title']);
    }
    
    // Apply classes to tabs to allow for better styling options
    $tabs = explode("\n", $vars['tabs']);
    array_pop($tabs);
    end($tabs);
    $last_key = key($tabs);

    foreach ($tabs as $key => &$tab) {
        if (strpos($tab, 'li class=')) {
            if ($key == 0) {
                $tab = str_replace('li class="', 'li class="first ', $tab);
            }
            if ($key == $last_key) {
                $tab = str_replace('li class="', 'li class="last ', $tab) . '</ul>';
            }
        }
        elseif (strpos($tab, 'li ')) {
            if ($key == 0) {
                $tab = str_replace('li ', 'li class="first" ', $tab);
            }
            if ($key == $last_key) {
                $tab = str_replace('li ', 'li class="last" ', $tab) . '</ul>';
            }
        }
    }
    $vars['tabs'] = implode("\n", $tabs);
}
// */

/**
 * Override or insert variables into the node templates.
 *
 * @param $vars
 *   An array of variables to pass to the theme template.
 * @param $hook
 *   The name of the template being rendered ("node" in this case.)
 */
///* -- Delete this line if you want to use this function
function boinc_preprocess_node(&$vars, $hook) {
  
  //$vars['sample_variable'] = t('Lorem ipsum.');

  // Optionally, run node-type-specific preprocess functions, like
  // boinc_preprocess_node_page() or boinc_preprocess_node_story().
  $function = __FUNCTION__ . '_' . $vars['node']->type;
  if (function_exists($function)) {
    $function($vars, $hook);
  }
}
// */

/**
 * Preprocessing for forum lists
 */
function boinc_preprocess_forums(&$vars, $hook) {
  // Add a link to mark all forums as read
  module_load_include('inc', 'forum_tweaks', 'includes/mark-read');
  forum_tweaks_get_mark_read_link($vars['tid'], $vars['links']);
  if (!$vars['parents']) {
    // Remove the "Post new forum topic" link from the top level forum list
    unset($vars['links']['forum']);
    // Add a link to manage subscriptions for the user
    $vars['links']['subscriptions'] = array(
      'title' => bts('Manage subscriptions'),
      'href' => 'account/prefs/subscriptions',
    );
  }
}

/**
 * Preprocessing for forum type nodes
 */
function boinc_preprocess_node_forum(&$vars, $hook) {
  global $user;
  //drupal_set_message('<pre>' . print_r(get_defined_vars(),TRUE) . '</pre>'); // print what variables are available
  //drupal_set_message('<pre>' . print_r($vars['node']->links, TRUE . '</pre>')); // print what links are available
  
  // Get the author of the node
  $account = user_load($vars['uid']);
  
  // Detach subscribe link from the Links list
  $vars['subscribe_link'] = $vars['node']->links['flag-subscriptions']['title'];
  unset($vars['node']->links['flag-subscriptions']);
  
  // Add topic moderator controls
  if (user_access('edit any forum topic')) {
    $vars['moderator_links'] = array();
    $node_control = "node_control/{$vars['node']->nid}";
    if ($vars['node']->status) {
      $vars['moderator_links']['hide'] = array(
        'title' => bts('Hide'),
        'href' => "{$node_control}/hide",
        'attributes' => array(
          'title' => bts('Hide this topic')
        )
      );
    }
    else {
      $vars['moderator_links']['unhide'] = array(
        'title' => bts('Unhide'),
        'href' => "{$node_control}/unhide",
        'attributes' => array(
          'title' => bts('Unhide this topic')
        )
      );
    }
    if ($vars['node']->comment == 2) {
      $vars['moderator_links']['lock'] = array(
        'title' => bts('Lock'),
        'href' => "{$node_control}/lock",
        'attributes' => array(
          'title' => bts('Lock this thread for comments')
        )
      );
    }
    else {
      $vars['moderator_links']['unlock'] = array(
        'title' => bts('Unlock'),
        'href' => "{$node_control}/unlock",
        'attributes' => array(
          'title' => bts('Unlock this thread for comments')
        )
      );
    }
    if ($vars['node']->sticky) {
      $vars['moderator_links']['make_unsticky'] = array(
        'title' => bts('Make unsticky'),
        'href' => "{$node_control}/unsticky",
        'attributes' => array(
          'title' => bts('Remove sticky status from this topic')
        )
      );
    }
    else {
      $vars['moderator_links']['make_sticky'] = array(
        'title' => bts('Make sticky'),
        'href' => "{$node_control}/sticky",
        'attributes' => array(
          'title' => bts('Make this topic sticky')
        )
      );
    }
  }
  else {
    // Hide these links for any other than moderators
    //$vars['node']->links = array();
  }
  // Move the new comment link to the end
  if (user_access('post comments')) {
    $vars['node']->links['reply'] = $vars['node']->links['comment_add'];
    unset($vars['node']->links['comment_add']);
  }
  else {
    unset($vars['node']->links['comment_forbidden']);
  }
  $vars['links'] = theme_links($vars['node']->links, array('class' => 'links inline'));
  $vars['moderator_links'] = theme_links($vars['moderator_links']);
  
  // Add signature
  $vars['signature'] = $account->signature;
  
  // Show signatures based on user preference
  $vars['show_signatures'] = ($user->hide_signatures) ? FALSE : TRUE;
}

/**
 * Override or insert variables into the comment templates.
 *
 * @param $vars
 *   An array of variables to pass to the theme template.
 * @param $hook
 *   The name of the template being rendered ("comment" in this case.)
 */
///* -- Delete this line if you want to use this function
function boinc_preprocess_comment(&$vars, $hook) {
  global $user;
  //$vars['sample_variable'] = t('Lorem ipsum.');
  //drupal_set_message('debug: <pre>' . print_r($vars,true) . '</pre>');
  $links = array();
  $moderator_links = array();
  $cid = $vars['comment']->cid;
  $nid = $vars['comment']->nid;
  if ($hook == 'comment') {
    if (user_access('administer comments')) {
      // Reorganize links for moderators
      $vars['links'] = array();
      $links['reply'] = array(
        'title' => bts('Reply'),
        'href' => "comment/reply/{$nid}/{$cid}",
        'attributes' => array(
          'title' => bts('Reply to this comment')
        )
      );
      $links['quote'] = array(
        'title' => bts('Quote'),
        'href' => "comment/reply/{$nid}/{$cid}",
        'attributes' => array(
          'title' => bts('Reply to this comment with quote')
        ),
        'fragment' => 'comment-form',
        'query' => 'quote=1',
      );
      // Move edit and delete controls into moderator links
      $moderator_links['edit'] = array(
        'title' => bts('Edit'),
        'href' => "comment/edit/{$cid}",
        'attributes' => array(
          'title' => bts('Edit this comment')
        )
      );
      $moderator_links['delete'] = array(
        'title' => bts('Delete'),
        'href' => "comment/delete/{$cid}",
        'attributes' => array(
          'title' => bts('Delete this comment')
        )
      );
      
      // Add hide link
      $comment_control = "comment_control/{$cid}";
      if ($vars['comment']->status == 0) {
        $moderator_links['hide'] = array(
          'title' => bts('Hide'),
          'href' => "{$comment_control}/hide",
          'attributes' => array(
            'title' => bts('Hide this comment')
          )
        );
      }
      else {
        $moderator_links['unhide'] = array(
          'title' => bts('Unhide'),
          'href' => "{$comment_control}/unhide",
          'attributes' => array(
            'title' => bts('Unhide this comment')
          )
        );
      }
      
      // Add link to convert comment into a new topic
      $reply_count = db_result(db_query('
        SELECT COUNT(*) FROM comments WHERE pid = %d', $cid
      ));
      if ($reply_count == 0) {
        $moderator_links['convert'] = array(
          'title' => bts('Convert'),
          'href' => "{$comment_control}/convert",
          'attributes' => array(
            'title' => bts('Convert this comment to a new topic')
          ) 
        );
      }
      $vars['moderator_links'] = theme_links($moderator_links);
    }
    else {
      $links = comment_links($vars['comment'], FALSE);
      if (user_access('post comments')) {
        $links['comment_quote'] = array(
          'title' => bts('Quote'),
          'href' => "comment/reply/{$nid}/{$cid}",
          'attributes' => array(
            'title' => bts('Reply to this comment with quote'),
          ),
          'fragment' => 'comment-form',
          'query' => 'quote=1',
        );
      }
    }
    ksort($links);
    $vars['links'] = theme_links($links);
    
    if ($user->uid) {
      $abuse_link = flag_create_link('abuse_comment', $cid);
      if ($abuse_link) {
        $report_comment_link = '' .
          '<ul class="links">' .
            '<li class="first">' . $abuse_link . '</li>' .
          '</ul>';
        $vars['links'] = $report_comment_link . $vars['links'];
      }
    }
    
    // Show signatures based on user preference
    $vars['show_signatures'] = ($user->hide_signatures) ? FALSE : TRUE;
  }
}
// */

/**
 *
 */
function boinc_preprocess_forum_topic_list(&$variables) {
  if (!empty($variables['topics'])) {
    global $user;
    foreach ($variables['topics'] as $id => $topic) {
      if ($topic->new_replies) {
        $cid = db_result(db_query("
          SELECT c.cid
          FROM {node} n
          INNER JOIN {comments} c ON c.nid = n.nid
          LEFT JOIN {history} h ON n.nid = h.nid AND h.uid = %d
          WHERE n.nid = %d
          AND n.status = 1
          AND c.timestamp > h.timestamp
          ORDER BY c.timestamp ASC
          LIMIT 1",
          $user->uid, $topic->nid
        ));
        if ($cid) {
          $variables['topics'][$id]->new_url = url("goto/comment/{$cid}");
        }
      }
    }
  }
}

/**
 * Override or insert variables into the default view template.
 *
 * @param $vars
 *   An array of variables to pass to the theme template.
 * @param $hook
 *   The name of the template being rendered
 */
///* -- Delete this line if you want to use this function
function boinc_preprocess_views_view(&$vars, $hook) {
  switch ($vars['name']) {
  case 'boinc_account_computers': 
    switch ($vars['display_id']) {
    case 'page_1':
    case 'panel_pane_1':
      $vars['empty'] = boincwork_views_host_list_empty_text();
      break;
    case 'page_2':
      $vars['empty'] = boincwork_views_host_list_empty_text('active');
      break;
    case 'block_1':
      $vars['empty'] = boincwork_views_host_list_empty_text('preferences');
      break;
    default:
    }
    break;
  case 'boinc_account_tasks_all':
    $vars['empty'] = boincwork_views_task_list_empty_text();
    break;
  case 'boinc_friends':
    if ($vars['display_id'] == 'block_1') {
      $vars['header'] = boincuser_views_friends_block_header(); 
    }
    break;
  case 'boinc_host_list':
    if ($vars['display_id'] == 'page_2') {
     $vars['empty'] = boincwork_views_host_list_empty_text();
    }
    elseif ($vars['display_id'] == 'page_1') {
      $vars['empty'] = boincwork_views_host_list_empty_text('active');
    }
    break;
  case 'boinc_task':
    // Load view object (view data is not available in header / footer); execute view query
    $view = views_get_current_view();
    $view->execute();
    $result = reset($view->result);
    // Display the stderr output in the footer
    $vars['footer'] = '<h3>' . bts('Stderr output') .'</h3>';
    $vars['footer'] .= '<pre>' . htmlspecialchars($result->result_stderr_out) . '</pre>';
    break;
  case 'boinc_teams':
    if ($vars['display_id'] == 'panel_pane_3') {
      $team_id = arg(2);
      $vars['header'] = boincteam_manage_admins_panel_header($team_id);
    }
    break;
  case 'boinc_workunit':
    ob_start();
    // Get the workunit ID from the URL
    $result_id = arg(1);
    require_boinc(array('util','boinc_db'));
    $wu = BoincWorkunit::lookup_id($result_id);
    project_workunit($wu);
    // Output of project_workunit() gets caught in the buffer
    $vars['footer'] = ob_get_clean();
  default:
  }
}
// */

/**
 * Override or insert variables into the privatemsg view templates.
 *
 * @param $vars
 *   An array of variables to pass to the theme template.
 */
///* -- Delete this line if you want to use this function
function boinc_preprocess_privatemsg_view(&$vars, $hook) {
  $user_image = boincuser_get_user_profile_image($vars['message']['author']->uid);
  if ($user_image['image']['filepath']) {
    $author_picture = '<div class="picture">';
    $author_picture .= theme('imagefield_image', $user_image['image'], $user_image['alt'], $user_image['alt'], array(), false);
    $author_picture .= '</div>';
  }
  $vars['author_picture'] = $author_picture;
  $vars['message_timestamp'] = date('j M Y H:i:s T', $vars['message']['timestamp']);
}
// */

/**
 * Override or insert variables into the block templates.
 *
 * @param $vars
 *   An array of variables to pass to the theme template.
 * @param $hook
 *   The name of the template being rendered ("block" in this case.)
 */
/* -- Delete this line if you want to use this function
function boinc_preprocess_block(&$vars, $hook) {
  $vars['sample_variable'] = t('Lorem ipsum.');
}
// */ 

function boinc_preprocess_search_result(&$variables) {
  $type = strtolower($variables['result']['type']);
  switch ($type) {
  case 'team':
    global $base_url;
    $node = $variables['result']['node'];
    $variables['url'] = $base_url .'/community/teams/' . $node->entity_id;
    break;
  default:
  }
}

// Remove the mess of text under the search form and don't display "no results"
// if a search hasn't even been submitted
function boinc_apachesolr_search_noresults() {
  $message = bts('No results found...');
  if (!arg(2)) {
    $message = '';
  }
  return '<p>' . $message . '</p>';
}

/**
 * Override the username theme function so that it returns a display name
 * rather than the unique Drupal auth name
 */
function phptemplate_username($object) {
  
  if ($object->uid && $object->name) {
    
    // Show the profile name in general, not the username
    $name = user_load($object->uid)->boincuser_name;
    
    // Shorten the name when it is too long or it will break many tables.
    if (drupal_strlen($name) > 20) {
      $name = drupal_substr($name, 0, 15) . '...';
    }

    if (user_access('access user profiles')) {
      $output = l($name, 'account/' . $object->uid, array('attributes' => array('title' => bts('View user profile.'))));
    }
    else {
      $output = check_plain($name);
    }
  }
  else if ($object->name) {
    // Sometimes modules display content composed by people who are
    // not registered members of the site (e.g. mailing list or news
    // aggregator modules). This clause enables modules to display
    // the true author of the content.
    if (!empty($object->homepage)) {
      $output = l($object->name, $object->homepage, array('attributes' => array('rel' => 'nofollow')));
    }
    else {
      $output = check_plain($object->name);
    }

    $output .= ' (' . bts('not verified') . ')';
  }
  else {
    $output = check_plain(variable_get('anonymous', bts('Anonymous')));
  }

  return $output;
}

/**
 * Remove the link under text areas that reads:
 * "More information about formatting options"
 */
function boinc_filter_tips_more_info () {
  return '';
}

/**
 * Theme outgoing email messages for adding friends.
 *
 * @param $status
 *   Status of the friendship.
 * @param $flag
 *   The flag object.
 * @param $recipient
 *   The user object of the person receiving the email.
 * @param $sender
 *   The user object of the person sending the email.
 * @return
 *   An array containing the email [type] (mailkey), [subject] and [body].
 */
function boinc_flag_friend_message_email($status, $flag, $recipient, $sender) {
  $email = array();
  $email['type'] = 'flag-friend';
  // Reload the sender to get a full user object
  $sender = user_load($sender->uid);
  
  switch ($status) {
    case FLAG_FRIEND_FLAGGED:
      // Sender accepted recipient's friend request
      $email['subject'] = bts('!name accepted your friend request [!site]', array(
        '!name' => $sender->boincuser_name,
        '!site' => variable_get('site_name', ''),
        ));
      $email['body'] = bts('!name confirmed you as a friend on !site.

Follow this link to view his or her profile:
!link

!message

Thanks,
The !site team', array(
        '!name' => isset($sender->boincuser_name) ? $sender->boincuser_name : $sender->name,
        '!site' => variable_get('site_name', ''),
        '!message' => $flag->friend_message ? bts('Message') . ': ' . $flag->friend_message : '',
        '!link' => url('account/'. $sender->uid, array('absolute' => TRUE)),
        ));
      break;

    case FLAG_FRIEND_PENDING:
      // Sender is requesting to be recipient's friend
      $email['subject'] = bts('Friend request from !name [!site]', array('!name' => $sender->boincuser_name, '!site' => variable_get('site_name', '')));
      $email['body'] = bts('!name added you as a friend on !site. You can approve or deny this request. Denying a request will not send a notification, but will remove the request from both of your accounts.

Follow the link below to view this request:
!link

!message

Thanks,
The !site team', array(
        '!name' => isset($sender->boincuser_name) ? $sender->boincuser_name : $sender->name,
        '!site' => variable_get('site_name', ''),
        '!message' => $flag->friend_message ? bts('Message') . ': ' . $flag->friend_message : '',
        '!link' => url('goto/friend-requests', array('absolute' => TRUE)),
        ));
      break;
  }
  return $email;
}

/**
 * Edit action links
 */
function phptemplate_links($links, $attributes = array('class' => 'links')) {
  if ($links){
    // Reorder the links however you need them.
    $links = reorder_links($links, array(), array('comment_reply', 'comment_edit'));
    // Use the built-in theme_links() function to format the $links array.
    return theme_links($links, $attributes);
  }
}

/**
 * Reorder links before passing them to default link theme function.
 * @param $links
 *   A keyed array of links to be themed.
 * @param $first_keys
 *   An array of keys which should be at the beginning of the $links array.
 * @param $last_keys
 *   An array of keys which should be at the end of the $links array.
 * @return
 *   A string containing an unordered list of links.
 *
 * Usage Note: The order in which you specify $first/last_keys is the order in
 * which they will be sorted.
 */
function reorder_links($links, $first_keys = array(), $last_keys = array()) {
    $first_links = array();
    foreach ($first_keys as $key) {
        if (isset($links[$key])) {
            $first_links[$key] = $links[$key];
            unset($links[$key]);
        }
    }
    $links = array_merge($first_links, $links);

    $last_links = array();
    foreach ($last_keys as $key) {
        if (isset($links[$key])) {
            $last_links[$key] = $links[$key];
            unset($links[$key]);
        }
    }
    $links = array_merge($links, $last_links);
   
    return $links;
}

/*
 * Override the style of table sort arrows to make it managable by CSS.
 * That is to say, get rid of it and use the views-view-table.tpl.php template.
 */
function boinc_tablesort_indicator($style) {
  return '';
  /*
  if ($style == "asc") {
    return theme('image', 'misc/arrow-asc.png', t('sort icon'), t('sort ascending'));
  }
  else {
    return theme('image', 'misc/arrow-desc.png', t('sort icon'), t('sort descending'));
  }
  */
}