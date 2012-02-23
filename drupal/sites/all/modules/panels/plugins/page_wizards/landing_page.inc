<?php
/**
 * @file
 * A page creation wizard to quickly create simple landing pages.
 *
 * This wizard strips out a lot of features that are not normally needed for
 * simple landing pages, such as access control, tabs, contexts, and the
 * selection of a variant. It will just assume you want a panel and let you
 * select the layout right away. This is 2 fewer forms than the standard
 * add page process and it will point you at the finished page rather than
 * sending you directly to the edit UI when finished.
 *
 * It also will auto-enable IPE if it can.
 */
$plugin = array(
  'title' => t('Landing page'),
  'page title' => t('Landing page wizard'),
  'description' => t('Landing pages are simple pages that have a path, possibly a visible menu entry, and a panel layout with simple content.'),

  'type' => 'panels',

  'form info' => array(
    'order' => array(
      'basic' => t('Basic settings'),
      'content' => t('Content'),
    ),

    'forms' => array(
      'basic' => array(
        'form id' => 'panels_landing_page_basic',
      ),
      'content' => array(
        'form id' => 'panels_landing_page_content',
      ),
    ),
  ),

  'default cache' => 'panels_landing_page_new_page',

  'finish' => 'panels_landing_page_finish',
);

/**
 * Provide defaults for a new cache.
 *
 * The cache will store all our temporary data; it isn't really a page
 * in itself, but it does contain everything we need to make one at the end.
 */
function panels_landing_page_new_page(&$cache) {
  $cache->name = '';
  $cache->admin_title = '';
  $cache->admin_description = '';
  $cache->path = '';
  $cache->menu_entry = FALSE;
  $cache->menu = array(
    'type' => 'none',
    'title' => '',
    'weight' => 0,
    'name' => 'navigation',
    'parent' => array(
      'type' => 'none',
      'title' => '',
      'weight' => 0,
      'name' => 'navigation',
    ),
  );
  $cache->display = panels_new_display();
  $cache->display->layout = 'flexible';
}

/**
 * First page of our page creator wizard.
 */
function panels_landing_page_basic(&$form, &$form_state) {
  $cache = &$form_state['cache'];
  ctools_include('dependent');

  $form['admin_title'] = array(
    '#type' => 'textfield',
    '#title' => t('Administrative title'),
    '#description' => t('The name of this page. This will appear in the administrative interface to easily identify it.'),
    '#default_value' => $cache->admin_title,
    '#required' => TRUE,
  );

  $form['name'] = array(
    '#type' => 'textfield',
    '#title' => t('Machine name'),
    '#description' => t('The machine readable name of this page. It must be unique, and it must contain only alphanumeric characters and underscores. Once created, you will not be able to change this value!'),
    '#default_value' => $cache->name,
    '#required' => TRUE,
  );

  $form['admin_description'] = array(
    '#type' => 'textarea',
    '#title' => t('Administrative description'),
    '#description' => t('A description of what this page is, does or is for, for administrative use.'),
    '#default_value' => $cache->admin_description,
  );

  // path
  $form['path'] = array(
    '#type' => 'textfield',
    '#title' => t('Path'),
    '#default_value' => $cache->path,
    '#field_prefix' => url(NULL, array('absolute' => TRUE)) . (variable_get('clean_url', 0) ? '' : '?q='),
    '#required' => TRUE,
  );

  $form['menu_entry'] = array(
    '#type' => 'checkbox',
    '#default_value' => $cache->menu_entry,
    '#title' => t('Add a visible menu entry for this page'),
  );

  $form['menu']['#tree'] = TRUE;

  $form['menu']['title'] = array(
    '#title' => t('Menu title'),
    '#type' => 'textfield',
    '#default_value' => $cache->menu['title'],
    '#process' => array('ctools_dependent_process'),
    '#dependency' => array('edit-menu-entry' => array(1)),
  );

  // Only display the menu selector if menu module is enabled.
  if (module_exists('menu')) {
    $form['menu']['name'] = array(
      '#title' => t('Menu'),
      '#type' => 'select',
      '#options' => menu_get_menus(),
      '#default_value' => $cache->menu['name'],
      '#process' => array('ctools_dependent_process'),
    '#dependency' => array('edit-menu-entry' => array(1)),
    );
  }
  else {
    $form['menu']['name'] = array(
      '#type' => 'value',
      '#value' => $cache->menu['name'],
    );
    $form['menu']['markup'] = array(
      '#value' => t('Menu selection requires the activation of menu module.'),
    );
  }
  $form['menu']['weight'] = array(
    '#title' => t('Weight'),
    '#type' => 'textfield',
    '#default_value' => isset($cache->menu['weight']) ? $cache->menu['weight'] : 0,
    '#description' => t('The lower the weight the higher/further left it will appear.'),
    '#process' => array('ctools_dependent_process'),
    '#dependency' => array('edit-menu-entry' => array(1)),
  );

  ctools_include('page-wizard', 'panels');
  panels_page_wizard_add_layout($form, $form_state);
}

/**
 * Submit function to store the form data in our cache.
 */
function panels_landing_page_basic_validate(&$form, &$form_state) {
  // Ensure all 'page' features are loaded.
  $page_task = page_manager_get_task('page');

  // Validate that the name is ok.
  $test = page_manager_page_load($form_state['values']['name']);
  if ($test) {
    form_error($form['name'], t('That name is used by another page: @page', array('@page' => $test->admin_title)));
  }

  // Ensure name fits the rules:
  if (preg_match('/[^a-zA-Z0-9_]/', $form_state['values']['name'])) {
    form_error($form['name'], t('Page name must be alphanumeric or underscores only.'));
  }

  // Validate that the path is ok.
  if (preg_match('/[%!\?#&]/', $form_state['values']['path'])) {
    form_error($form['path'], t('%, !, ?, #, or & cannot appear in the path.'));
  }

  // Check to see if something is already using the path
  $result = db_query("SELECT * FROM {menu_router} WHERE path = '%s'", $form_state['values']['path']);
  while ($router = db_fetch_object($result)) {
    form_error($form['path'], t('That path is already in use. This system cannot override existing paths.'));
    return;
  }

  // Ensure the path is not already an alias to something else.
  $result = db_query("SELECT src, dst FROM {url_alias} WHERE dst = '%s'", $form_state['values']['path']);
  if ($alias = db_fetch_object($result)) {
    form_error($form['path'], t('That path is currently assigned to be an alias for @alias. This system cannot override existing aliases.', array('@alias' => $alias->src)));
  }
}

/**
 * Submit function to store the form data in our cache.
 */
function panels_landing_page_basic_submit(&$form, &$form_state) {
  $cache = &$form_state['cache'];
  $cache->name = $form_state['values']['name'];
  $cache->admin_title = $form_state['values']['admin_title'];
  $cache->admin_description = $form_state['values']['admin_description'];
  $cache->path = $form_state['values']['path'];
  $cache->menu_entry = $form_state['values']['menu_entry'];
  $cache->menu['title'] = $form_state['values']['menu']['title'];
  $cache->menu['weight'] = $form_state['values']['menu']['weight'];
  $cache->menu['name'] = $form_state['values']['menu']['name'];
  $cache->menu['type'] = $cache->menu_entry ? 'normal' : 'none';
  $cache->display->layout = $form_state['values']['layout'];
  $cache->display->title = $form_state['values']['admin_title'];
}

/**
 * Second page of our wizard. This one provides a layout and lets the
 * user add content.
 */
function panels_landing_page_content(&$form, &$form_state) {
  ctools_include('page-wizard', 'panels');
  panels_page_wizard_add_content($form, $form_state);
}

/**
 * Submit function to store the form data in our cache.
 */
function panels_landing_page_submit(&$form, &$form_state) {
  panels_page_wizard_add_content_submit($form, $form_state);
}

/**
 * Finish callback for the wizard.
 *
 * When the wizard is finished, this callback will create the actual
 * page, save it, and redirect the user to view the new work.
 */
function panels_landing_page_finish(&$form_state) {
  $cache = &$form_state['cache'];

  // Ensure all 'page' features are loaded.
  $page_task = page_manager_get_task('page');

  // Assemble a new page subtask.
  $subtask = page_manager_page_new();
  $subtask->name = $cache->name;
  $subtask->path = $cache->path;
  $subtask->admin_title = $cache->admin_title;
  $subtask->admin_description = $cache->admin_description;
  $subtask->path = $cache->path;
  $subtask->menu = $cache->menu;

  // Create the the panel context variant configured with our display
  $plugin = page_manager_get_task_handler('panel_context');

  // Create a new handler.
  $handler = page_manager_new_task_handler($plugin);
  $handler->conf['title'] = t('Landing page');
  $handler->conf['display'] = $cache->display;
  $handler->conf['pipeline'] = 'ipe';

  // Assemble a new $page cache and assign it our page subtask and task
  // handler.
  $page = new stdClass();
  page_manager_page_new_page_cache($subtask, $page);
  page_manager_handler_add_to_page($page, $handler);

  // Save it
  page_manager_save_page_cache($page);

  // Send us to the new page immediately.
  $form_state['redirect'] = url($cache->path);
}
