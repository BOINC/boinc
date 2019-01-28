<?php

/**
 * @file
 * Export UI display customizations.
 */

/**
 * CTools export UI extending class. Slightly customized for Context.
 */
class current_search_export_ui extends ctools_export_ui {

  /**
   * Implements ctools_export_ui::list_form().
   *
   * Simplifies the form similar to how the Context module does it.
   */
  function list_form(&$form, &$form_state) {
    parent::list_form($form, $form_state);
    $form['top row']['submit'] = $form['bottom row']['submit'];
    $form['top row']['reset'] = $form['bottom row']['reset'];
    $form['bottom row']['#access'] = FALSE;
    return;
  }

  /**
   * Implements ctools_export_ui::list_build_row().
   */
  function list_build_row($item, &$form_state, $operations) {
    parent::list_build_row($item, $form_state, $operations);
  }

  /**
   * Implements ctools_export_ui::edit_execute_form().
   *
   * This is hacky, but since CTools Export UI uses drupal_goto() we have to
   * effectively change the plugin to modify the redirect path dynamically.
   */
  function edit_execute_form(&$form_state) {
    $output = parent::edit_execute_form($form_state);
    if (!empty($form_state['executed'])) {
      $clicked = $form_state['clicked_button']['#value'];
      if (t('Add item') == $clicked || t('Save and edit') == $clicked) {
        // We always want to redirect back to this page when adding an item,
        // but we want to preserve the destination so we can be redirected back
        // to where we came from after clicking "Save".
        $options = array();
        if (!empty($_GET['destination'])) {
          $options['query']['destination'] = $_GET['destination'];
          unset($_GET['destination']);
        }

        // Sets redirect path and options.
        $op = $form_state['op'];
        $name = $form_state['values']['name'];
        $path = ('add' != $op) ? $_GET['q'] : 'admin/settings/current_search/list/' . $name . '/edit';
        $this->plugin['redirect'][$op] = array($path, $options);
      }
    }
    return $output;
  }

  /**
   * Implements ctools_export_ui::edit_page().
   *
   * Allows passing of options to drupal_goto() as opposed to just a path.
   *
   * @see http://drupal.org/node/1373048
   */
  function edit_page($js, $input, $item, $step = NULL) {
    $args = func_get_args();
    drupal_set_title($this->get_page_title('edit', $item));

    // Check to see if there is a cached item to get if we're using the wizard.
    if (!empty($this->plugin['use wizard'])) {
      $cached = $this->edit_cache_get($item, 'edit');
      if (!empty($cached)) {
        $item = $cached;
      }
    }

    $form_state = array(
      'plugin' => $this->plugin,
      'object' => &$this,
      'ajax' => $js,
      'item' => $item,
      'op' => 'edit',
      'form type' => 'edit',
      'rerender' => TRUE,
      'no_redirect' => TRUE,
      'step' => $step,
      // Store these in case additional args are needed.
      'function args' => $args,
    );

    $output = $this->edit_execute_form($form_state);
    if (!empty($form_state['executed'])) {
      // @see @see http://drupal.org/node/1373048
      $export_key = $this->plugin['export']['key'];
      $args = (array) $this->plugin['redirect']['edit'];
      $args[0] = str_replace('%ctools_export_ui', $form_state['item']->{$export_key}, $args[0]);
      call_user_func_array('drupal_goto', $args);
    }

    return $output;
  }

   /**
   * Implements ctools_export_ui::add_page().
   *
   * Allows passing of options to drupal_goto() as opposed to just a path.
   *
   * @see http://drupal.org/node/1373048
   */
  function add_page($js, $input, $step = NULL) {
    $args = func_get_args();
    drupal_set_title($this->get_page_title('add'));

    // If a step not set, they are trying to create a new item. If a step
    // is set, they're in the process of creating an item.
    if (!empty($this->plugin['use wizard']) && !empty($step)) {
      $item = $this->edit_cache_get(NULL, 'add');
    }
    if (empty($item)) {
      $item = ctools_export_crud_new($this->plugin['schema']);
    }

    $form_state = array(
      'plugin' => $this->plugin,
      'object' => &$this,
      'ajax' => $js,
      'item' => $item,
      'op' => 'add',
      'form type' => 'add',
      'rerender' => TRUE,
      'no_redirect' => TRUE,
      'step' => $step,
      // Store these in case additional args are needed.
      'function args' => $args,
    );

    $output = $this->edit_execute_form($form_state);
    if (!empty($form_state['executed'])) {
      // @see @see http://drupal.org/node/1373048
      $export_key = $this->plugin['export']['key'];
      $args = (array) $this->plugin['redirect']['add'];
      $args[0] = str_replace('%ctools_export_ui', $form_state['item']->{$export_key}, $args[0]);
      call_user_func_array('drupal_goto', $args);
    }

    return $output;
  }
}

/**
 * Define the preset add/edit form.
 *
 * @see current_search_add_item_submit()
 * @see current_search_settings_form_submit()
 * @ingroup forms
 */
function current_search_settings_form(&$form, &$form_state) {
  $item = &$form_state['item'];
  $form['info']['#weight'] = -30;

  if (empty($item->settings)) {
    $item->settings = current_search_get_defaults();
  }

  // NOTE: We need to add the #id in order for the machine_name to work.
  $form['info']['label'] = array(
    '#id' => 'edit-label',
    '#title' => t('Name'),
    '#type' => 'textfield',
    '#default_value' => $item->label,
    '#description' => t('The human-readable name of the current search block configuration.'),
    '#required' => TRUE,
    '#maxlength' => 255,
    '#size' => 30,
  );

  $form['info']['name'] = array(
    '#type' => 'machine_name',
    '#default_value' => $item->name,
    '#maxlength' => 32,
    '#machine_name' => array(
      'exists' => 'current_search_config_exists',
      'source' => array('info', 'label'),
    ),
    '#disabled' => !empty($item->name),
    '#description' => t('The machine readable name of the current search block configuration. This value can only contain letters, numbers, and underscores.'),
  );

  $form['info']['searcher'] = array(
    '#type' => 'select',
    '#title' => t('Search page'),
    '#options' => current_search_get_searcher_options(),
    '#description' => t('The search page this configuration will be active on.'),
    '#default_value' => current_search_get_default_searcher(),
    '#access' => empty($item->name),
  );

  // Hide the standard buttons.
  $form['buttons']['#access'] = FALSE;

  // Add our custom buttons.
  $form['actions'] = array(
    '#type' => 'actions',
    '#weight' => -100,
  );

  // Gets destination from query string which is set when the page is navigated
  // to via a contextual link. Builds messages based on where user came from.
  if (isset($_GET['destination']) && !url_is_external($_GET['destination'])) {
    $submit_text = t('Save and go back to search page');
    $cancel_title = t('Return to the search page without saving configuration changes.');
    $url = drupal_parse_url($_GET['destination']);
  }
  else {
    $submit_text = t('Save and go back to list');
    $cancel_title = t('Return to the list without saving configuration changes.');
    $url = array();
  }

  $form['actions']['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Save and edit'),
  );

  // Do not show the button if the page was navigated to via a contextual link
  // because it would redirect the user back to the search page.
  $form['actions']['submit_list'] = array(
    '#type' => 'submit',
    '#value' => $submit_text,
  );

  $form['actions']['cancel'] = array(
    '#type' => 'link',
    '#title' => t('Cancel'),
    '#href' => (!$url) ? 'admin/settings/current_search' : $url['path'],
    '#options' => (!$url) ? array() : array('query' => $url['query']),
    '#attributes' => array('title' => $cancel_title),
  );

  if (empty($item->name)) {
    $description = t('Add a new current search block configuration.');
  }
  else {
    $description = t('Add new items to the current search block or configure existing ones.');
  }
  $form['description'] = array(
    '#prefix' => '<div class="current-search-description">',
    '#suffix' => '</div>',
    '#value' => $description,
    '#weight' => -90,
  );

  drupal_add_css(drupal_get_path('module', 'current_search') . '/current_search.css');

  // If we are creating the configuration, only display the basic config items.
  // Otherwise set the breadcrumb due to possible bug in CTools Export UI.
  if (empty($form['info']['name']['#default_value'])) {
    return;
  }

  // Gets list of plugins, sanitizes label for output.
  $plugins = array_map('check_plain', current_search_get_plugins());

  ////
  ////
  //// Add plugin section
  ////
  ////

  $form['plugins_title'] = array(
    '#type' => 'item',
    '#title' => t('Add item to block'),
  );

  $form['plugins'] = array(
    '#tree' => TRUE,
    '#prefix' => '<div class="clearfix">',
    '#suffix' => '</div>',
  );

  $form['plugins']['plugin'] = array(
    '#type' => 'select',
    '#title' => t('Type'),
    '#options' => $plugins,
    '#prefix' => '<div class="current-search-setting current-search-plugin">',
    '#suffix' => '</div>',
  );

  $form['plugins']['item_label'] = array(
    '#title' => t('Name'),
    '#type' => 'textfield',
    '#default_value' => '',
    '#required' => FALSE,
    '#size' => 30,
    '#prefix' => '<div class="current-search-setting current-search-label">',
    '#suffix' => '</div>',
  );

  $form['plugins']['item_name'] = array(
    '#type' => 'machine_name',
    '#default_value' => '',
    '#maxlength' => 32,
    '#machine_name' => array(
      'exists' => 'current_search_item_exists',
      'source' => array('plugins', 'item_label'),
    ),
    '#required' => FALSE,
    '#description' => t('The machine readable name of the item being added to the current search block. This value can only contain letters, numbers, and underscores.'),
  );

  $form['plugins']['actions'] = array(
    '#type' => 'actions',
    '#prefix' => '<div class="current-search-setting current-search-button">',
    '#suffix' => '</div>',
  );

  $form['plugins']['actions']['add_item'] = array(
    '#type' => 'submit',
    '#value' => t('Add item'),
    '#submit' => array('current_search_add_item_submit'),
    '#validate' => array('current_search_add_item_validate'),
  );

  ////
  ////
  //// Sort settings
  ////
  ////

  $form['plugin_sort'] = array(
    '#type' => 'item',
    '#access' => !empty($item->settings['items']),
    '#title' => t('Item display order'),
    '#theme' => 'current_search_sort_settings_table',
    '#current_search' => $item->settings,
    '#tree' => TRUE,
  );

  // Builds checkbox options and weight dropboxes.
  foreach ($item->settings['items'] as $name => $settings) {
    $form['plugin_sort'][$name]['item'] = array(
      '#value' => check_plain($settings['label']),
    );
    $form['plugin_sort'][$name]['remove'] = array(
      '#type' => 'link',
      '#title' => t('Remove item'),
      '#href' => 'admin/settings/current_search/item/' . $item->name . '/delete/' . $name,
    );
    $form['plugin_sort'][$name]['weight'] = array(
      '#type' => 'weight',
      '#title' => t('Weight for @title', array('@title' => $settings['label'])),
      '#title_display' => 'invisible',
      '#delta' => 50,
      '#default_value' => isset($settings['weight']) ? $settings['weight'] : 0,
      '#attributes' => array('class' => array('current-search-sort-weight')),
    );
  }

  ////
  ////
  //// Filter settings
  ////
  ////

  $form['plugin_settings_title'] = array(
    '#type' => 'item',
    '#access' => !empty($item->settings['items']),
    '#title' => t('Item settings'),
  );

  $form['plugin_settings'] = array(
    '#type' => 'vertical_tabs',
    '#tree' => TRUE,
  );

  // Builds table, adds settings to vertical tabs.
  $has_settings = FALSE;
  foreach ($item->settings['items'] as $name => $settings) {
    if ($class = ctools_plugin_load_class('current_search', 'items', $settings['id'], 'handler')) {
      $plugin = new $class($name, $item);

      // Initializes vertical tab for the item's settings.
      $form['plugin_settings'][$name] = array(
        '#type' => 'fieldset',
        '#title' => check_plain($settings['label']),
        '#group' => 'settings',
        '#tree' => TRUE,
      );

      $form['plugin_settings'][$name]['id'] = array(
        '#type' => 'value',
        '#value' => $settings['id'],
      );

      $form['plugin_settings'][$name]['label'] = array(
        '#type' => 'value',
        '#value' => $settings['label'],
      );

      // Gets settings from plugin.
      $plugin->settingsForm($form['plugin_settings'][$name], $form_state);
      $has_settings = TRUE;
    }
  }

  // Removes fieldset if there aren't any settings.
  if (!$has_settings) {
    unset($form['plugin_settings']);
  }

  ////
  ////
  //// Advanced settings
  ////
  ////

  $form['advanced_settings_title'] = array(
    '#type' => 'item',
    '#title' => t('Advanced settings'),
  );

  $form['advanced_settings'] = array(
    '#tree' => TRUE,
  );

  // This setting was originally intended as a way for site builders to enable
  // the current search block on pages where no keywords were submitted by the
  // end user, which is known as an "empty search". The display settings were
  // expanded beyond empty searches at http://drupal.org/node/1779670 leaving
  // us with the unfortunate "empty_searches" key which no longer reflects what
  // this setting does.
  $form['advanced_settings']['empty_searches'] = array(
    '#type' => 'radios',
    '#title' => t('Display settings'),
    '#options' => array(
      CURRENT_SEARCH_DISPLAY_KEYS => t('Display only when keywords are entered.'),
      CURRENT_SEARCH_DISPLAY_ALWAYS => t('Display on empty searches where no keywords are entered.'),
      CURRENT_SEARCH_DISPLAY_FILTERS => t('Display only when one or more facet items are active.'),
      CURRENT_SEARCH_DISPLAY_KEYS_FILTERS => t('Display when either keywords are entered one or more facet items are active.'),
    ),
    '#default_value' => $item->settings['advanced']['empty_searches'],
    '#description' => t('This setting determines when the current search block will be displayed.'),
  );

}

/**
 * Returns the sort table.
 *
 * @param $variables
 *   An associative array containing:
 *   - element: A render element representing the form.
 *
 * @ingroup themeable
 */
function theme_current_search_sort_settings_table($variables) {
  $output = '';

  // Initializes table header.
  $header = array(
    'item' => t('Item'),
    'weight' => t('weight'),
  );
  // Builds table rows.
  $rows = array();
  foreach ($variables['#current_search']['items'] as $name => $settings) {
    $rows[$name] = array(
      'class' => array('draggable'),
      'data' => array(
        drupal_render($variables[$name]['item']),
        drupal_render($variables[$name]['weight']),
        array(
          'data' => drupal_render($variables[$name]['remove']),
          'class' => 'current-search-remove-link',
        ),
      ),
    );
  }

  // Builds table with drabble rows, returns output.
  $table_id = 'current-search-sort-settings';
  drupal_add_tabledrag($table_id, 'order', 'sibling', 'current-search-sort-weight');
  //$output .= drupal_render_children($variables['element']);
  $output .= theme('table', $header, $rows, array('id' => $table_id));
  return $output;
}

/**
 * Form validation handler for current_search_settings_form().
 * Processed when the "Add item" button is selected.
 */
function current_search_add_item_validate($form, &$form_state) {
  // NOTE: The form items are only required with the "Add item" button is
  // submitted, so we cannot use the #required property. Otherwise we could
  // not click the save or delete buttons without the form failing validation.
  if (empty($form_state['values']['plugins']['item_name'])) {
    $vars = array('!name' => 'Item name');
    form_set_error('item_name', t('!name field is required', $vars));
  }
  if (empty($form_state['values']['plugins']['item_label'])) {
    $vars = array('!name' => 'Machine-readable name');
    form_set_error('item_label', t('!name field is required', $vars));
  }
}

/**
 * Form submission handler for current_search_settings_form().
 *
 * Processed when the "Add item" button is selected.
 */
function current_search_add_item_submit($form, &$form_state) {
  $item = &$form_state['item'];
  $item->settings += current_search_get_defaults();

  // Gets variables for code readability.
  $id = $form_state['values']['plugins']['plugin'];
  $name = $form_state['values']['plugins']['item_name'];
  $label = $form_state['values']['plugins']['item_label'];

  // Adds settings to the array.
  if ($class = ctools_plugin_load_class('current_search', 'items', $id, 'handler')) {
    $plugin = new $class($name);
    $item->settings['items'][$name] = $plugin->getDefaultSettings() + array(
      'id' => $id,
      'label' => $label,
    );
  }
}

/**
 * Form submission handler for current_search_settings_form().
 */
function current_search_settings_form_submit($form, &$form_state) {
  $item = &$form_state['item'];
  $item->settings += current_search_get_defaults();

  // If there are plugin settings, we are updating an existing config.
  if (!empty($form_state['values']['plugin_settings'])) {
    $item->label = $form_state['values']['label'];
    if (!empty($form_state['values']['plugin_settings'])) {

      // Gathers settings, stores in $items->settings.
      foreach ($form_state['values']['plugin_settings'] as $name => $settings) {
        if (is_array($settings)) {
          $item->settings['items'][$name] = $settings + array(
            'weight' => $form_state['values']['plugin_sort'][$name]['weight'],
          );
        }
      }

      // Sorts settings by weight.
      uasort($item->settings['items'], 'facetapi_sort_weight');

      // Stores advanced settings.
      $item->settings['advanced'] = $form_state['values']['advanced_settings'];
    }
  }
  else {
    // Saves the block visibility settings if searcher was passed.
    if (!empty($form_state['values']['searcher'])) {
      $name = $form_state['values']['name'];
      $searcher = $form_state['values']['searcher'];
      current_search_set_block_searcher($name, $searcher);
    }
  }
}

/**
 * Form constructor for the revert form.
 *
 * @ingroup forms
 */
function current_search_delete_item_form(&$form_state, stdClass $item, $name) {
  $form['#current_search'] = array(
    'item' => $item,
    'name' => $name
  );

  $form['text'] = array(
   '#value' => '<p>' . t('Are you sure you want to remove the item %name from the current search block configuration?.', array('%name' => $name)) . '</p>',
  );

  $form['actions']['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Remove item'),
  );

  $form['actions']['cancel'] = array(
    '#type' => 'link',
    '#title' => t('Cancel'),
    '#href' => 'admin/settings/current_search/list/' . $item->name . '/edit',
    '#attributes' => array('title' => t('Go back to current search block configuration')),
  );

  $form['#submit'][] = 'current_search_delete_item_form_submit';

  return $form;
}

/**
 * Form submission handler for facetapi_revert_form_submit().
 */
function current_search_delete_item_form_submit($form, &$form_state) {
  $name = $form['#current_search']['name'];
  $item = $form['#current_search']['item'];

  // Removes item from the current search block configuration.
  if (isset($item->settings['items'][$name])) {
    $label = $item->settings['items'][$name]['label'];
    drupal_set_message(t('@label has been removed.', array('@label' => $label)));
    unset($item->settings['items'][$name]);
    ctools_export_crud_save('current_search', $item);
  }

  // Resirects back to current search block configuration page.
  $form_state['redirect'] = 'admin/settings/current_search/list/' . $item->name . '/edit';
}

/**
 * Tests if the configuration name already exists.
 *
 * @return
 *   A boolean flagging whether the item exists.
 */
function current_search_config_exists($name) {
  $configs = ctools_export_crud_load_all('current_search');
  return isset($configs[$name]);
}

/**
 * Tests if the item name already exists.
 *
 * @return
 *   A boolean flagging whether the item exists.
 */
function current_search_item_exists($name, &$element, &$form_state) {
  $item = &$form_state['item'];
  return isset($item->settings['items'][$name]);
}

/**
 * Returns default settings.
 */
function current_search_get_defaults() {
  return array(
    'items' => array(),
    'advanced' => array(
      'empty_searches' => 0,
     ),
  );
}
