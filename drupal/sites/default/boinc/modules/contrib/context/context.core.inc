<?php

/**
 * Implementation of hook_help().
 */
function context_help($path, $arg) {
  switch ($path) {
    case 'admin/help#context':
      $output = file_get_contents(drupal_get_path('module', 'context') .'/README.txt');
      return module_exists('markdown') ? filter_xss_admin(module_invoke('markdown', 'filter', 'process', 0, -1, $output)) : '<pre>'. check_plain($output) .'</pre>';
  }
}

/**
 * Implementation of hook_menu_alter().
 * If core block handling is disabled, remove menu items.
 */
function context_menu_alter(&$items) {
  if (variable_get('context_reaction_block_disable_core', FALSE)) {
    foreach ($items as $path => $item) {
      if (strpos($path, 'admin/build/block') === 0) {
        unset($items[$path]);
      }
    }
  }
}

/**
 * Implementation of hook_theme().
 */
function context_theme() {
  $items = array();
  $items['context_block_form'] = array(
    'arguments' => array('form' => array()),
    'path' => drupal_get_path('module', 'context') . '/theme',
    'file' => 'context_reaction_block.theme.inc',
  );
  $items['context_block_regions_form'] = array(
    'arguments' => array('form' => array()),
    'path' => drupal_get_path('module', 'context') . '/theme',
    'file' => 'context_reaction_block.theme.inc',
  );
  $items['context_block_editor'] = array(
    'arguments' => array('form' => array()),
    'path' => drupal_get_path('module', 'context') . '/theme',
    'file' => 'context_reaction_block.theme.inc',
  );
  $items['context_block_browser'] = array(
    'arguments' => array('blocks' => array(), 'context' => array()),
    'path' => drupal_get_path('module', 'context') . '/theme',
    'template' => 'context-block-browser',
    'file' => 'context_reaction_block.theme.inc',
  );
  $items['context_block_browser_item'] = array(
    'arguments' => array('block' => array()),
    'path' => drupal_get_path('module', 'context') . '/theme',
    'template' => 'context-block-browser-item',
    'file' => 'context_reaction_block.theme.inc',
  );
  $items['context_block_editable_block'] = array(
    'arguments' => array('block' => array()),
    'path' => drupal_get_path('module', 'context') . '/theme',
    'template' => 'context-block-editable-block',
    'file' => 'context_reaction_block.theme.inc',
  );
  $items['context_block_editable_region'] = array(
    'arguments' => array('region' => '', 'blocks' => array(), 'editable' => FALSE),
    'path' => drupal_get_path('module', 'context') . '/theme',
    'template' => 'context-block-editable-region',
    'file' => 'context_reaction_block.theme.inc',
  );
  $items['context_block_script_placeholder'] = array(
    'arguments' => array('text' => NULL),
    'path' => drupal_get_path('module', 'context') . '/theme',
    'file' => 'context_reaction_block.theme.inc',
  );
  return $items;
}

/**
 * Implementation of hook_theme_registry_alter().
 */
function context_theme_registry_alter(&$theme_registry) {
  // Push theme_page() through a context_preprocess to provide
  // context-sensitive menus and variables. Ensure that
  // context_preprocess_page() comes immediately after
  // template_preprocess_page().
  $position = array_search('context_preprocess_page', $theme_registry['page']['preprocess functions']);
  if ($position !== FALSE) {
    unset($theme_registry['page']['preprocess functions'][$position]);
  }
  $position = array_search('template_preprocess_page', $theme_registry['page']['preprocess functions']);
  $position = $position ? $position + 1 : 2;
  array_splice($theme_registry['page']['preprocess functions'], $position, 0, 'context_preprocess_page');

  // Add a page preprocess function to the very top of the theme_page()
  // stack so that we can actually set contexts *before* the page theming
  // is executed.
  if (!in_array('context_page_alter', $theme_registry['page']['preprocess functions'])) {
    array_unshift($theme_registry['page']['preprocess functions'], 'context_page_alter');
  }
  // Reroute theme_blocks() through context_blocks to determine block
  // visibility by context. Only override theme_blocks() if a theme
  // has not overridden it. It is the responsibility of any themes
  // implementing theme_blocks() to take advantage of context block
  // visibility on their own.
  if (!isset($theme_registry['blocks']['type']) || !in_array($theme_registry['blocks']['type'], array('base_theme_engine', 'theme_engine')) && !isset($theme_registry['blocks']['file'])) {
    unset($theme_registry['blocks']['preprocess functions']);
    $theme_registry['blocks']['function'] = 'context_blocks';
  }
}

/**
 * Implementation of hook_ctools_render_alter().
 * Used to detect the presence of a page manager node view or node form.
 */
function context_ctools_render_alter($info, $page, $args, $contexts, $task, $subtask) {
  if ($page && in_array($task['name'], array('node_view', 'node_edit'), TRUE)) {
    foreach ($contexts as $ctools_context) {
      if ($ctools_context->type === 'node' && !empty($ctools_context->data)) {
        context_node_condition($ctools_context->data, $task['name'] === 'node_view' ? 'view' : 'form');
        break;
      }
    }
  }
}

/**
 * Implementation of hook_nodeapi().
 */
function context_nodeapi(&$node, $op, $teaser, $page) {
  if ($op == 'view' && $page) {
    $object = menu_get_object();
    if (isset($object->nid) && $object->nid === $node->nid) {
      context_node_condition($node, 'view');
    }
  }
}

/**
 * Implementation of hook_form_alter().
 */
function context_form_alter(&$form, $form_state, $form_id) {
  // Prevent this from firing on admin pages... damn form driven apis...
  if ($form['#id'] === 'node-form' && arg(0) != 'admin') {
    context_node_condition($form['#node'], 'form');
    $form['#validate'][] = 'context_form_alter_node_validate';
  }
  else if ($form_id == 'system_modules') {
    context_invalidate_cache();
  }
  // Clear out block info cache when an admin area form is submitted.
  if (arg(0) === 'admin' && !empty($form_state['post']) && (!isset($form_state['method']) || $form_state['method'] !== 'get')) {
    if ($plugin = context_get_plugin('reaction', 'block')) {
      $plugin->rebuild_needed(TRUE);
    }
  }
}

/**
 * Centralized node condition call function for the ever increasing number of
 * ways to get at a node view / node form.
 */
function context_node_condition(&$node, $op) {
  if ($plugin = context_get_plugin('condition', 'node')) {
    $plugin->execute($node, $op);
  }
  if (module_exists('taxonomy')) {
    if ($plugin = context_get_plugin('condition', 'node_taxonomy')) {
      $plugin->execute($node, $op);
    }
  }
  if (module_exists('book')) {
    if ($plugin = context_get_plugin('condition', 'book')) {
      $plugin->execute($node, $op);
    }
    if ($plugin = context_get_plugin('condition', 'bookroot')) {
      $plugin->execute($node, $op);
    }
  }
  // Allow other plugins to easily be triggered on node-related events.
  drupal_alter('context_node_condition', $node, $op);
}

/**
 * Node form validation callback.
 *
 * Set context also on validate, otherwise forms that don't validate drop out
 * of context.
 */
function context_form_alter_node_validate($form, &$form_state) {
  context_node_condition($form['#node'], 'form');
}

/**
 * Implementation of hook_form_alter() for comment_form.
 */
function context_form_comment_form_alter(&$form, $form_state) {
  if ($nid = $form['nid']['#value']) {
    $node = node_load($nid);
    context_node_condition($node, 'comment');
  }
}

/**
 * Implementation of hook_views_pre_view().
 */
function context_views_pre_view($view, $args) {
  if ($plugin = context_get_plugin('condition', 'views')) {
    $plugin->execute($view);
  }
}

/**
 * Implementation of hook_user().
 */
function context_user($op, &$edit, &$account, $category = NULL) {
  if (in_array($op, array('view', 'form', 'register'))) {
    if ($plugin = context_get_plugin('condition', 'user_page')) {
      $plugin->execute($account, $op);
    }
  }
}

/**
 * BLOCK HANDLING =====================================================
 */

/**
 * This override of theme_blocks() is called because of an alter of the
 * theme registry. See context_theme_registry_alter().
 */
function context_blocks($region) {
  if ($plugin = context_get_plugin('reaction', 'block')) {
    return $plugin->execute($region);
  }
}

/**
 * THEME FUNCTIONS & RELATED ==========================================
 */

/**
 * Generates an array of links (suitable for use with theme_links)
 * to the node forms of types associated with current active contexts.
 */
function context_links($reset = FALSE) {
  static $links;
  if (!$links || $reset) {
    $contexts = context_active_contexts();
    $active_types = array();
    $conditions = array('node', 'bookroot');
    foreach ($conditions as $condition) {
      foreach ($contexts as $k => $v) {
        if (!empty($v->conditions[$condition]['values'])) {
          $active_types = array_merge($active_types, array_filter($v->conditions[$condition]['values']));
        }
      }
    }

    $links = array();
    if (!empty($active_types)) {
      // Iterate over active contexts
      foreach ($active_types as $type) {
        $add_url = 'node/add/'. str_replace('_', '-', $type);
        $item = menu_get_item($add_url);
        if ($item && $item['access'] && strpos($_GET['q'], $add_url) !== 0) {
          $links[$type] = array('title' => t('Add @type', array('@type' => node_get_types('name', $type))), 'href' => $add_url);
        }
      }
    }
    drupal_alter('context_links', $links);
    uasort($links, 'element_sort');
  }
  return $links;
}

/**
 * A preprocess_page() function that is called *before* all other
 * preprocessors (including template_preprocess_page()). This allows
 * any final context conditions to be set and any initial reactions
 * to be triggered.
 */
function context_page_alter(&$vars) {
  module_invoke_all('context_page_condition');
  module_invoke_all('context_page_reaction');
}

/**
 * Implementation of hook_context_page_condition().
 */
function context_context_page_condition() {
  if ($plugin = context_get_plugin('condition', 'menu')) {
    $plugin->execute();
  }
  if ($plugin = context_get_plugin('condition', 'sitewide')) {
    $plugin->execute(1);
  }
  if ($plugin = context_get_plugin('condition', 'context')) {
    $plugin->execute();
  }
}

/**
 * Implementation of hook_context_page_reaction().
 */
function context_context_page_reaction() {
  if ($plugin = context_get_plugin('reaction', 'breadcrumb')) {
    $plugin->execute();
  }
  if ($plugin = context_get_plugin('reaction', 'css_injector')) {
    $plugin->execute();
  }
  if ($plugin = context_get_plugin('reaction', 'debug')) {
    $plugin->execute();
  }
}

/**
 * Implementation of preprocess_page().
 */
function context_preprocess_page(&$vars) {
  if ($plugin = context_get_plugin('reaction', 'menu')) {
    $plugin->execute($vars);
  }
  if ($plugin = context_get_plugin('reaction', 'theme')) {
    $plugin->execute($vars);
  }
  if ($context_links = context_links()) {
    $vars['context_links'] = theme('links', $context_links);
  }
  else {
    $vars['context_links'] = '';
  }
}
