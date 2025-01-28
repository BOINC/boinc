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
  $html = '';
  $html .= '<ul id="' . $menu['id'] . '" class="' . $menu['class'] . '">' . "\n";
  $item_count = count($links);
  $i = 1;
  foreach ($links as $key => $link) {
    $classes = array($key);
    if (strpos($key, 'active-trail')) $classes[] = 'active';
    if ($i == 1) $classes[] = 'first';
    if ($i == $item_count) $classes[] = 'last';
    $html .= '<li class="' . implode(' ', $classes) .'">';
    if ($link['title'] == 'Home') {
      $link['title'] = bts('Home', array(), NULL, 'boinc:menu-link');
    }
    if (module_exists('privatemsg')) {
      // Put a new mail notification next to the Account menu item
      if ($link['href'] == 'dashboard') {
        $item_count = privatemsg_unread_count();
        if ($item_count) {
          $link['title'] .= '</a> <a href="' . url('messages') . '" class="compound secondary"><div class="item-count-wrapper"><span class="item-count">' . $item_count . '</span></div>';
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
function boinc_preprocess_page(&$vars, $hook) {
  // Responsive Design: Add viewport meta tag to HTML head
  drupal_set_html_head('<meta name="viewport" content="width=device-width, initial-scale=1.0" />');
  $vars['head'] = drupal_get_html_head();
  //dpm($vars['head'], "preprocess (all) vars[head]");

  // Expose comments to template files; this is needed so that comments can be
  // rendered in locations other than directly below the node content
  $vars['comments'] = $vars['comment_form'] = '';
  if (module_exists('comment') && isset($vars['node'])) {
    $vars['comments'] = comment_render($vars['node']);
    $vars['comment_form'] = drupal_get_form('comment_form', array('nid' => $vars['node']->nid));
  }

  // Determine locale region code so the correct flag is displayed in footer
  global $language;
  global $theme_path;
  $locality = $language->language;
  if (strpos($language->language, '-')) {
    $flag_icon = "{$theme_path}/images/flags/{$language->language}.png";
    if (!file_exists($flag_icon)) {
      $lang_code = explode('-', $language->language);
      $locality = $lang_code[0];
    }
  }
  // If there is no language set for some reason, default to English (en).
  if (empty($locality)) {
    $locality = "en";
  }
  $vars['flag_path'] = base_path() . path_to_theme() . "/images/flags/{$locality}.png";

  $server_status_url = variable_get('boinc_server_status_url', '');
  if (!$server_status_url) {
    $server_status_url = 'server_status.php';
  }
  $vars['server_status_url'] = $server_status_url;

  $app_list_url = variable_get('boinc_app_list_url', '');
  if (!$app_list_url) {
    $app_list_url = 'apps.php';
  }
  $vars['app_list_url'] = $app_list_url;

  // Remove title from certain pages using URL.
  // This is a kludge to remove the title of the page but not the
  // "head_title" which is placed in the HTML <head> section. Most of
  // these pages are defined in the Page Manager module.
  // See: https://dev.gridrepublic.org/browse/DBOINC-65
  if (arg(0) == 'search') {
    unset($vars['title']);
  }
  else if ( (arg(0)=='account') AND (is_numeric(arg(1))) AND (empty(arg(2))) ) {
    unset($vars['title']);
  }
  else if ( (arg(0)=='account') AND (arg(1)=='profile') ) {
    unset($vars['title']);
  }
  else if ( (arg(0)=='dashboard') ) {
    unset($vars['title']);
  }
  else if ( (arg(0)=='community') AND ( (arg(1)=='teams') OR (arg(1)=='stats') ) ) {
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

  // Get the main menu but only for the branch the page is on.
  $vars['menu_tree_onlyactive'] = menu_tree('primary-links');

  // Create tertiary menu
  $vars['tertiary_links'] = menu_navigation_links(variable_get('menu_primary_links_source', 'primary-links'), 2);

  // Create action links
  $vars['action_links'] = _boinc_action_links();
}

/**
 * Override or insert variables into the node templates.
 *
 * @param $vars
 *   An array of variables to pass to the theme template.
 * @param $hook
 *   The name of the template being rendered ("node" in this case.)
 */
function boinc_preprocess_node(&$vars, $hook) {

  //$vars['sample_variable'] = t('Lorem ipsum.');

  // Detach subscribe link from the links list. Subscribe link will be placed
  // on page separately from links.
  if (!empty($vars['node']->links['flag-subscriptions']['title'])) {
    $vars['subscribe_link'] = $vars['node']->links['flag-subscriptions']['title'];
    unset($vars['node']->links['flag-subscriptions']);
  }

  // Optionally, run node-type-specific preprocess functions, like
  // boinc_preprocess_node_page() or boinc_preprocess_node_story().
  $function = __FUNCTION__ . '_' . $vars['node']->type;
  if (function_exists($function)) {
    $function($vars, $hook);
  }
}

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
      'title' => bts('Manage subscriptions', array(), NULL, 'boinc:forum-footer'),
      'href' => 'account/prefs/subscriptions',
    );
  }
}

/**
 * Preprocessing for forum type nodes
 */
function boinc_preprocess_node_forum(&$vars, $hook) {
  global $language;
  global $user;

  // Locality
  $vars['locality'] = $language->language;

  // Get the author of the node
  $account = user_load($vars['uid']);
  $comments_per_page = ($user->comments_per_page) ? $user->comments_per_page : variable_get("comment_default_per_page_{$vars['node']->type}", 50);

  // Add signature
  $vars['signature'] = check_markup($account->signature, $vars['node']->format);

  // Show signatures based on user preference
  $vars['show_signatures'] = ($user->hide_signatures) ? FALSE : TRUE;

  // Expose comment sort order so that the template can put the topic node
  // content (i.e. initial post) at the very end if "Newest post first" is the
  // preference used by this user
  $vars['oldest_post_first'] = ($user->sort != 1) ? TRUE : FALSE;
  $vars['node']->comment = 0;

  $vars['first_page'] = (!isset($_GET['page']) OR ($_GET['page'] < 1));
  $page_count = max(ceil($vars['comment_count'] / $comments_per_page), 1);
  $vars['last_page'] = ($page_count == 1 OR ($page_count > 1 AND $_GET['page'] == $page_count - 1));

  $links = $vars['links'];
  $moderator_links = array();
  _boinc_create_moderator_links($links, $moderator_links);
  $vars['links'] = $links;
  $vars['moderator_links'] = $moderator_links;

  // Ignore user link
  $vars['ignore_link'] = _boinc_ignore_user_link('node', $vars['node']);
}


/**
 * Preprocessing for team_forum type nodes
 */
function boinc_preprocess_node_team_forum(&$vars, $hook) {
    // Process this node in the same way as node_forum
    boinc_preprocess_node_forum($vars, $hook);
}

/**
 * Override or insert variables into the comment templates.
 *
 * @param $vars
 *   An array of variables to pass to the theme template.
 * @param $hook
 *   The name of the template being rendered ("comment" in this case.)
 */
function boinc_preprocess_comment(&$vars, $hook) {
  global $language;
  global $user;

  // Locality
  $vars['locality'] = $language->language;

  // Show signatures based on user preference
  $vars['show_signatures'] = ($user->hide_signatures) ? FALSE : TRUE;

  $links = $vars['links'];
  $moderator_links = array();
  _boinc_create_moderator_links($links, $moderator_links);
  $vars['links'] = $links;
  $vars['moderator_links'] = $moderator_links;

  // Ignore user link
  $vars['ignore_link'] = _boinc_ignore_user_link('comment', $vars['comment']);
}

/**
 *
 */
function boinc_preprocess_forum_topic_list(&$variables) {
  if (!empty($variables['topics'])) {
    foreach ($variables['topics'] as $id => $topic) {
      if ($topic->new_replies) {
        $cid = boincuser_get_first_unread_comment_id($topic->nid);
        if ($cid) {
          $variables['topics'][$id]->new_url = url("goto/comment/{$cid}");
        }
        else {
          // User hasn't visited this topic before, so all replies are new...
          $topic->new_replies = NULL;
        }
      }
      // Use same logic in forum.module to change message if topic has
      // moved. Changed link to match boinc path-added "community".
      if ($topic->forum_tid != $variables['tid']) {
        $variables['topics'][$id]->message = l(t('This topic has been moved'), "community/forum/$topic->forum_tid");
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
  case 'boinc_host':
      $view = views_get_current_view();
      if (!($view->result)) {
        $vars['footer'] = '<h3>' . bts ('Host not found in database.', array(), NULL, 'boinc:host-details') . '</h3>';
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

    if ($result) {
      // Display the stderr output in the footer
      $vars['footer'] = '<h3>' . bts('Stderr output', array(), NULL, 'boinc:task-details-errorlog') .'</h3>';
      $vars['footer'] .= '<pre>' . htmlspecialchars($result->result_stderr_out) . '</pre>';
    } else {
      $vars['footer'] = '<h3>' . bts ('Task not found in database.', array(), NULL, 'boinc:task-details') . '</h3>';
    }
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
    if ($wu) {
      // Output from admin defined BOINC project-specific function
      project_workunit($wu);
      // Output of project_workunit() gets caught in the buffer
      $vars['footer'] = ob_get_clean();
    } else {
      $vars['footer'] = '<h3>' . bts ('Workunit not found in database.', array(), NULL, 'boinc:workunit-details') . '</h3>';
    }
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
  $author_picture = '<div class="picture">';
  $user_image = boincuser_get_user_profile_image($vars['message']['author']->uid);
  if ($user_image) {
    if (is_array($user_image) AND $user_image['image']['filepath']) {
      $author_picture .= theme('imagefield_image', $user_image['image'], $user_image['alt'], $user_image['alt'], array(), false);
    }
    elseif (is_string($user_image)) {
      $author_picture .= '<img src="' . $user_image . '"/>';
    }
  }
  $author_picture .= '</div>';
  $vars['author_picture'] = $author_picture;
  $vars['message_timestamp'] = date('j M Y G:i:s T', $vars['message']['timestamp']);
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
  global $language;
  // Locality
  $variables['locality'] = $language->language;

  // Change the format of the search result date/time in the info string.
  if ($variables['result']['date']) {
    $variables['info_split']['date'] = date('j M Y G:i:s T', $variables['result']['date']);
  }
  $variables['info'] = implode(' - ', $variables['info_split']);

  $type = strtolower($variables['result']['bundle']);
  switch ($type) {
  case 'profile':
  case 'user':
    $node = $variables['result']['node'];
    $variables['url'] = url('account/' . $node->is_uid);
    $variables['title'] = $node->tos_name;
    $variables['user_image'] = boincuser_get_user_profile_image($node->is_uid);
    $variables['account'] = user_load($node->is_uid);
    break;
  case 'team':
    $node = $variables['result']['node'];
    $variables['url'] = url('/community/teams/' . $node->entity_id);;
    break;
  case 'forum':
    $node = $variables['result']['node'];
    $drupalnode = node_load($node->entity_id);
    // Get the taxonomy for the node, creates a link to the parent forum
    $taxonomy = reset($drupalnode->taxonomy);
    if ($vocab = taxonomy_vocabulary_load($taxonomy->vid)) {
      $variables['parent_forum'] = l($taxonomy->name, "community/forum/{$taxonomy->tid}");
    }
    break;
  case 'comment':
    // Get the node id for this comment
    $nid = $variables['result']['fields']['tos_content_extra'];
    $drupalnode = node_load($nid);
    // Parent forum topic title
    $variables['parent_title'] = $drupalnode->title;
    // Link to the parent forum topic
    $variables['parent_topic'] = l($drupalnode->title, drupal_get_path_alias('node/' . $nid) );
    // Get the taxonomy for the node, creates a link to the parent forum
    $taxonomy = reset($drupalnode->taxonomy);
    if ($vocab = taxonomy_vocabulary_load($taxonomy->vid)) {
      $variables['parent_forum'] = l($taxonomy->name, "community/forum/{$taxonomy->tid}");
    }
  break;
  default:
  }
}

// Remove the mess of text under the search form and don't display "no results"
// if a search hasn't even been submitted
function boinc_apachesolr_search_noresults() {
  $message = bts('No results found...', array(), NULL, 'boinc:search-with-no-results');
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
      $output = l($name, 'account/' . $object->uid, array('attributes' => array('title' => bts('View user profile.', array(), NULL, 'boinc:users-table'))));
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

    $output .= ' (' . bts('not verified', array(), NULL, 'boinc:user-not-found') . ')';
  }
  else {
    $output = check_plain(variable_get('anonymous', bts('Anonymous', array(), NULL, 'boinc:anonymous-user')));
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
        '!site' => variable_get('site_name', 'Drupal-BOINC'),
        ), NULL, 'boinc:friend-request-email');
      $email['body'] = bts('!name confirmed you as a friend on !site.

Follow this link to view his or her profile:
!link

!message

Thanks,
The !site team', array(
        '!name' => isset($sender->boincuser_name) ? $sender->boincuser_name : $sender->name,
        '!site' => variable_get('site_name', 'Drupal-BOINC'),
        '!message' => $flag->friend_message ? bts('Message', array(), NULL, 'boinc:friend-request-email:-1:a-private-message') . ': ' . $flag->friend_message : '',
        '!link' => url('account/'. $sender->uid, array('absolute' => TRUE)),
        ), array(), NULL, 'boinc:friend-request-email');
      break;

    case FLAG_FRIEND_PENDING:
      // Sender is requesting to be recipient's friend
      $email['subject'] = bts('Friend request from !name [!site]', array('!name' => $sender->boincuser_name, '!site' => variable_get('site_name', 'Drupal-BOINC')), NULL, 'boinc:friend-request-email');
      $email['body'] = bts('!name added you as a friend on !site. You can approve or deny this request. Denying a request will not send a notification, but will remove the request from both of your accounts.

Follow the link below to view this request:
!link

!message

Thanks,
The !site team', array(
        '!name' => isset($sender->boincuser_name) ? $sender->boincuser_name : $sender->name,
        '!site' => variable_get('site_name', 'Drupal-BOINC'),
        '!message' => $flag->friend_message ? bts('Message', array(), NULL, 'boinc:friend-request-email:-1:a-private-message') . ': ' . $flag->friend_message : '',
        '!link' => url('goto/friend-requests', array('absolute' => TRUE)),
        ),
      array(), NULL, 'boinc:friend-request-email');
      break;
  }
  return $email;
}

/**
 * Edit action links
 */
function phptemplate_links($links, $attributes = array('class' => 'links')) {
  if ($links){
    // Remove flag-subscriptions link. It will be placed elsewhere.
    if (isset($links['flag-subscriptions'])) {
      unset($links['flag-subscriptions']);
    }
    // Reorder the links however you need them.
    $links = reorder_links($links, array('comment_edit','quote','comment_add','comment_reply','flag-abuse_comment','flag-abuse_node'), array('comment_delete'));
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

/*
 * Private function to process the $links string, separate it into two
 * strings for $links and $moderator_links.
 *
 * Parameters:
 *   @params $links
 *     links is a string of links to manipulate. The function will
 *     return a altered string of links.
 *   @params $moderator_links
 *     moderator_links will be filled from elements from $links.
 *
 */
function _boinc_create_moderator_links(&$links, &$moderator_links) {
  // If there are no links, then do nothing
  if (empty($links)) {
    return;
  }

  $alllinks = array();
  $modlinks = array();

  // Create an array of HTML elements from the $links string, keys
  // are the class attribute for the <li> tags.
  $dom = new DOMDocument;
  $dom->loadHTML(mb_convert_encoding($links, 'HTML-ENTITIES', 'UTF-8'));
  foreach($dom->getElementsByTagName('li') as $node) {
    $key = $node->getAttribute("class");
    $alllinks[$key] = $dom->saveHTML($node);
  }

  // Select classes to be placed into moderator links array
  $selected_classes = array(
    "make_sticky", "make_unsticky",
    "lock", "unlock",
    "convert",
    "hide", "unhide",
    "comment_delete",
  );
  foreach(array_keys($alllinks) as $key1) {
    // Select string up to first space, if present.
    $class1 = strtok($key1, ' ');
    if (in_array($class1, $selected_classes)) {
      if (empty($modlinks)) {
        _boinc_firstlink($alllinks[$key1]);
      }
      $modlinks[$key1] = $alllinks[$key1];
      unset($alllinks[$key1]);
    }
  }
  // Convert the HTML arrays back into strings, wrap them in <ul>
  // tags
  $links = "<ul class=\"links\">".implode($alllinks)."</ul>";
  $moderator_links = "<ul class=\"links\">".implode($modlinks)."</ul>";

  return;
}

/*
 * Private function that modifies a single link, adding the 'first'
 * attribute to class.
 */
function _boinc_firstlink(&$alink) {
  if (!empty($alink)) {
    $dom = new DomDocument;
    $dom->loadHTML(mb_convert_encoding($alink, 'HTML-ENTITIES', 'UTF-8'));

    $myli = $dom->getElementsByTagName('li');
    if ($myli->length>0) {
      $newclasses = trim(($myli[0]->getAttribute("class"))." first");
      $myli[0]->setAttribute("class", $newclasses);
      $alink = $dom->saveHTML($myli[0]);
    }
  }
}

/*
 * Private function to generate the action links
 */
function _boinc_action_links() {
  global $user;
  global $base_path;

  $output = '<ul class="menu"><li class="first">';
  if ($user->uid) {
    $output .= '<a href="' . url('logout') . '">' . bts('Logout', array(), NULL, 'boinc:menu-link') . '</a>';
  } else {
    $output .= '<a href="' . url('user/login', array('query' => drupal_get_destination()) ) . '">' . bts('Login', array(), NULL, 'boinc:menu-link') . '</a>';
  }
  $output .= '</li>';
  if (module_exists('global_search') OR module_exists('global_search_solr')) {
    $output .= '<li class="last"> <a class="search" href="' . url('search/site') . '">' . bts('search', array(), NULL, 'boinc:menu-link') .'</a> </l1>';
  }
  $output .= '</ul>';
  return $output;
}

/**
 * Private function, based on ignore_user ignore_user_link()
 * function. Modified for boinc functionality.
 */
function _boinc_ignore_user_link($type, $object = NULL, $teaser = FALSE) {
  global $user;

  if (!$user || !$user->uid) {
    return;
  }

  static $ignored;
  $links = array();

  if ($type == 'node' && $user->uid != $object->uid && $object->uid != 0 && user_access('ignore user')) {
    if (!isset($ignored[$object->uid])) {
      $ignored[$object->uid] = db_result(db_query('SELECT COUNT(*) FROM {ignore_user} WHERE uid = %d AND iuid = %d', $user->uid, $object->uid));
    }
    if ($ignored[$object->uid] == 0) {
      $links['ignore_user'] = array(
        'title' => bts('Ignore user', array(), NULL, 'boinc:ignore-user-add'),
        'href' => 'account/prefs/privacy/ignore_user/add/'. $object->uid,
        'query' => 'destination='. $_GET['q'],
        'attributes' => array(
          'class' => 'ignore-user',
          'title' => bts('Add user to your ignore list', array(), NULL, 'boinc:ignore-user-add'),
        ),
      );
    }
  }
  else if ($type == 'comment' && $user->uid != $object->uid && $object->uid != 0 && user_access('ignore user')) {
    if (!isset($ignored[$object->uid])) {
      $ignored[$object->uid] = db_result(db_query('SELECT COUNT(*) FROM {ignore_user} WHERE uid = %d AND iuid = %d', $user->uid, $object->uid));
    }
    if ($ignored[$object->uid] == 0) {
      $links['ignore_user'] = array(
        'title' => bts('Ignore user', array(), NULL, 'boinc:ignore-user-add'),
        'href' => 'account/prefs/privacy/ignore_user/add/'. $object->uid,
        'query' => 'destination='. $_GET['q'],
        'attributes' => array(
          'class' => 'ignore-user',
          'title' => bts('Add user to your ignore list', array(), NULL, 'boinc:ignore-user-add'),
        ),
      );
    }
  }

  return $links;
}
