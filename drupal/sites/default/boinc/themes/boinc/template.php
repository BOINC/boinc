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
 * Preprocessing for forum type nodes
 */
function boinc_preprocess_node_forum(&$vars, $hook) {
  //drupal_set_message('<pre>' . print_r(get_defined_vars(),TRUE) . '</pre>'); // print what variables are available
  //drupal_set_message('<pre>' . print_r($vars['node']->links, TRUE . '</pre>')); // print what links are available
  if (user_access('edit any forum topic')) {
    $node_control = "node_control/{$vars['node']->nid}";
    if ($vars['node']->status) {
      $vars['node']->links['hide'] = array(
        'title' => t('Hide'),
        'href' => "{$node_control}/hide",
        'attributes' => array(
          'title' => t('Hide this topic')
        )
      );
    }
    else {
      $vars['node']->links['unhide'] = array(
        'title' => t('Unhide'),
        'href' => "{$node_control}/unhide",
        'attributes' => array(
          'title' => t('Unhide this topic')
        )
      );
    }
    if ($vars['node']->comment == 2) {
      $vars['node']->links['lock'] = array(
        'title' => t('Lock'),
        'href' => "{$node_control}/lock",
        'attributes' => array(
          'title' => t('Lock this thread for comments')
        )
      );
    }
    else {
      $vars['node']->links['unlock'] = array(
        'title' => t('Unlock'),
        'href' => "{$node_control}/unlock",
        'attributes' => array(
          'title' => t('Unlock this thread for comments')
        )
      );
    }
    if ($vars['node']->sticky) {
      $vars['node']->links['make_unsticky'] = array(
        'title' => t('Make unsticky'),
        'href' => "{$node_control}/unsticky",
        'attributes' => array(
          'title' => t('Make this topic sticky')
        )
      );
    }
    else {
      $vars['node']->links['make_sticky'] = array(
        'title' => t('Make sticky'),
        'href' => "{$node_control}/sticky",
        'attributes' => array(
          'title' => t('Remove sticky status from this topic')
        )
      );
    }
  }
  $vars['links'] = theme_links($vars['node']->links, array('class' => 'links inline'));
}

/**
 * Override or insert variables into the comment templates.
 *
 * @param $vars
 *   An array of variables to pass to the theme template.
 * @param $hook
 *   The name of the template being rendered ("comment" in this case.)
 */
/* -- Delete this line if you want to use this function
function boinc_preprocess_comment(&$vars, $hook) {
  $vars['sample_variable'] = t('Lorem ipsum.');
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

// Remove the mess of text under the search form and don't display "no results"
// if a search hasn't even been submitted
function boinc_apachesolr_search_noresults() {
  $message = t('No results found...');
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
    
    // Set the object name to use the profile name
    $object->name = user_load($object->uid)->boincuser_name;
    
    // Shorten the name when it is too long or it will break many tables.
    if (drupal_strlen($object->name) > 20) {
      $name = drupal_substr($object->name, 0, 15) . '...';
    }
    else {
      $name = $object->name;
    }

    if (user_access('access user profiles')) {
      $output = l($name, 'account/' . $object->uid, array('attributes' => array('title' => t('View user profile.'))));
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

    $output .= ' (' . t('not verified') . ')';
  }
  else {
    $output = check_plain(variable_get('anonymous', t('Anonymous')));
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
