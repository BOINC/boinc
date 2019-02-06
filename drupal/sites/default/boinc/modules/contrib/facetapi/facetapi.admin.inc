<?php

/**
 * @file
 * Admin page callbacks for the Facet API module.
 */

/**
 * Tests whether a configuration setting is overridden or not.
 *
 * @param stdClass $settings
 *   The configuration settings being tested.
 *
 * @return boolean
 *   TRUE if the settings are overridden, FALSE otherwise.
 */
function facetapi_is_overridden(stdClass $settings) {
  return ($settings->export_type & EXPORT_IN_CODE && $settings->export_type & EXPORT_IN_DATABASE);
}

/**
 * Form constructor for the realm settings form.
 *
 * @param $searcher
 *   The machine readable name of the searcher.
 * @param $realm_name
 *   The machine readable name of the realm.
 *
 * @see facetapi_realm_settings_form_submit()
 * @ingroup forms
 */
function facetapi_realm_settings_form(&$form_state, $searcher, $realm_name) {
  $form = array();
  // Instantiates adapter, loads realm.
  $adapter = facetapi_adapter_load($searcher);
  $realm = facetapi_realm_load($realm_name);

  $form['#facetapi'] = array(
    '#type' => 'value',
    'adapter' => $adapter,
    'realm' => $realm,
    'facet_info' => facetapi_get_facet_info($searcher),
  );

  $form['description'] = array(
    '#prefix' => '<div class="facetapi-realm-description">',
    '#value' => filter_xss_admin($realm['description']),
    '#suffix' => "</div>\n",
  );

  $form['performance'] = array(
    '#type' => 'markup',
    '#prefix' => '<div class="facetapi-performance-note">',
    '#value' => t('For performance reasons, you should only enable facets that you intend to have available to users on the search page.'),
    '#suffix' => "</div>\n",
  );

  $form['table'] = array(
    '#theme' => 'facetapi_realm_settings_table',
    '#facetapi' => &$form['#facetapi'],
    'operations' => array('#tree' => TRUE),
    'weight' => array('#tree' => TRUE),
  );

  // Builds "enabled_facets" options.
  $options = $default_value = array();
  foreach ($form['#facetapi']['facet_info'] as $facet_name => $facet) {
    $settings = $adapter->getFacetSettings($facet, $realm);
    $global_settings = $adapter->getFacetSettingsGlobal($facet);

    // Builds array of operations to use in the dropbutton.
    $operations = array();
    $operations[] = array(
      'title' => t('Configure display'),
      'href' => facetapi_get_settings_path($searcher, $realm['name'], $facet_name, 'edit')
    );
    if ($facet['dependency plugins']) {
      $operations[] = array(
        'title' => t('Configure dependencies'),
        'href' => facetapi_get_settings_path($searcher, $realm['name'], $facet_name, 'dependencies')
      );
    }
    if (facetapi_filters_load($facet_name, $searcher, $realm['name'])) {
      $operations[] = array(
        'title' => t('Configure filters'),
        'href' => facetapi_get_settings_path($searcher, $realm['name'], $facet_name, 'filters')
      );
    }
    $operations[] = array(
      'title' => t('Export configuration'),
      'href' => facetapi_get_settings_path($searcher, $realm['name'], $facet_name, 'export')
    );
    if (facetapi_is_overridden($settings) || facetapi_is_overridden($global_settings)) {
      $operations[] = array(
        'title' => t('Revert configuration'),
        'href' => facetapi_get_settings_path($searcher, $realm['name'], $facet_name, 'revert')
      );
    }
    $links = theme('links', $operations);

    $form['table']['operations'][$facet_name] = array(
      '#type' => 'markup',
      '#value' => $links,
    );

    // Adds weight if sortable.
    if ($realm['sortable']) {

      $form['#facetapi']['facet_info'][$facet_name]['weight'] = $settings->settings['weight'];
      $form['table']['weight'][$facet_name] = array(
        '#type' => 'select',
        '#title' => t('Weight for @title', array('@title' => $facet['label'])),
        '#title_display' => 'invisible',
        '#options' => drupal_map_assoc(range(-50, 50)),
        '#default_value' => $settings->settings['weight'],
        '#attributes' => array('class' => array('facetapi-facet-weight')),
      );
    }

    $options[$facet_name] = '';
    $default_value[$facet_name] = (!$settings->enabled) ? 0 : $facet_name;
  }

  // Sorts by the weight appended above.
  uasort($form['#facetapi']['facet_info'], 'facetapi_sort_weight');

  $form['table']['enabled_facets'] = array(
    '#type' => 'checkboxes',
    '#options' => $options,
    '#default_value' => $default_value,
  );

  // Checks whether block caching is enabled, sets description accordingly.
  if (!$disabled = (module_implements('node_grants') || !variable_get('block_cache', FALSE))) {
    $description = t('Configure the appropriate cache setting for facet blocks.');
  }
  else {
    $description = t(
      'To enable block caching, visit the <a href="@performance-page">performance page</a>.',
      array('@performance-page' => url('admin/settings/performance', array('query' => array('destination' => $_GET['q']))))
    );
  }
  $form['block_cache'] = array(
    '#type' => 'select',
    '#access' => ('block' == $realm_name),
    '#title' => t('Block cache settings'),
    '#disabled' => $disabled,
    '#options' => array(
      BLOCK_NO_CACHE => t('Do not cache'),
      BLOCK_CACHE_PER_ROLE | BLOCK_CACHE_PER_PAGE => t('Per role'),
      BLOCK_CACHE_PER_USER | BLOCK_CACHE_PER_PAGE => t('Per user'),
    ),
    '#default_value' => variable_get('facetapi:block_cache:' . $searcher, 0),
    '#description' => $description,
  );

  $form['actions']['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Save configuration'),
  );

  $form['#submit'][] = 'facetapi_realm_settings_form_submit';

  return $form;
}

/**
 * Form submission handler for facetapi_realm_settings_form().
 */
function facetapi_realm_settings_form_submit($form, &$form_state) {
  $success = TRUE;

  // Pulls variables for code readability.
  $adapter = $form['#facetapi']['adapter'];
  $realm = $form['#facetapi']['realm'];
  $facet_info = $form['#facetapi']['facet_info'];

  // Builds settings object, saves to database.
  foreach ($facet_info as $facet_name => $facet) {
    $status = empty($form_state['values']['enabled_facets'][$facet_name]) ? 0 : 1;
    $weight = $realm['sortable'] ? $form_state['values']['weight'][$facet_name] : 0;
    if (!facetapi_save_facet_status($adapter, $realm, $facet, $status, $weight, TRUE)) {
      $success = FALSE;
      $message = t(
        'Error saving configuration options for the %label facet.',
        array('%label' => $facet['label'])
      );
      drupal_set_message($message, 'error');
    }
  }

  // Sets message if all configurations were saved.
  if ($success) {
    drupal_set_message(t('The configuration options have been saved.'));
    if ('block' == $form['#facetapi']['realm']['name']) {
      drupal_set_message(t(
        'To enable or arrange the facet blocks, visit the <a href="@block-page">blocks administration page</a>.',
        array('@block-page' => url('admin/build/block', array('query' => array('destination' => $_GET['q']))))
      ));
    }
  }

  // Saves block cache settings, clears block cache if setting was changed.
  if (isset($form_state['values']['block_cache'])) {
    $name = 'facetapi:block_cache:' . $adapter->getSearcher();
    $original = variable_get($name, BLOCK_NO_CACHE);
    variable_set($name, $form_state['values']['block_cache']);
    if ($original != $form_state['values']['block_cache']) {
      cache_clear_all(NULL, 'cache_block');
    }
  }

  // Clears delta map cache.
  cache_clear_all('facetapi:delta_map', 'cache');
}

/**
 * Returns the realm settings table.
 *
 * @param $variables
 *   An associative array containing:
 *   - element: A render element representing the form.
 *
 * @ingroup themable
 */
function theme_facetapi_realm_settings_table($variables) {
  $output = '';

  // Adds CSS to ensure that the dropbutton looks nice.
  $path = drupal_get_path('module', 'facetapi');
  drupal_add_css($path . '/facetapi.admin.css');

  // Gets variables for code readability.
  $searcher = $variables['#facetapi']['adapter']->getSearcher();
  $realm = $variables['#facetapi']['realm'];

  $header = array(
    'enabled' => array('data' => t('Enabled')),
    'label' => array('data' => t('Facet'), 'sort' => 'asc'),
    'operations' => array('data' => t('Operations')),
  );
  if ($realm['sortable']) {
    $header['weight'] = array('data' => t('Weight'));
  }

  // Builds field options.
  $rows = array();
  $facet_info = $variables['#facetapi']['facet_info'];
  foreach ($facet_info as $facet_name => $facet) {

    // Builds rows.
    $rows[$facet_name] = array(
      'class' => $realm['sortable'] ? 'draggable' : '',
      'data' => array(
        drupal_render($variables['enabled_facets'][$facet_name]),
        check_plain($facet['label']) . "<div class='description'>" . filter_xss($facet['description']) . '</div>',
        array(
          'class' => 'facetapi-operations',
          'data' => drupal_render($variables['operations'][$facet_name]),
        ),
      ),
    );
    if ($realm['sortable']) {
      $rows[$facet_name]['data'][] = drupal_render($variables['weight'][$facet_name]);
    }
  }

  if ($realm['sortable']) {
    drupal_add_tabledrag('facetapi-realm-settings', 'order', 'sibling', 'facetapi-facet-weight');
  }
  //$output .= drupal_render_children($variables['element']);
  $output .= theme('table', $header, $rows, array('id' => 'facetapi-realm-settings'));

  return $output;
}

/**
 * Returns the path to a facet's settings page.
 *
 * @param $searcher
 *   The machine readable name of the searcher.
 * @param $realm_name
 *   The machine readable name of the realm.
 * @param $facet_name
 *   The machine readable name of the realm.
 * @param $op
 *   The operation being requested, one of the following values:
 *   - edit: The display settings form.
 *   - dependencies: The dependency settings form.
 *   - filters: The filters settings form.
 *   - export: The settings export form.
 *
 * @return
 *   The path to the settings page.
 */
function facetapi_get_settings_path($searcher, $realm_name, $facet_name, $op) {
  $path_parts = array('admin', 'settings', 'facetapi');
  $path_parts[] = $searcher;
  $path_parts[] = $realm_name;
  $path_parts[] = $facet_name;
  $path_parts[] = $op;
  return join('/', $path_parts);
}

/**
 * Returns the widget plugin definitions available to the facet.
 *
 * All widget plugins are checked against their requirmenets. Only widgets that
 * pall all requirements are returned by this function.
 *
 * @param array $realm
 *   The realm definition.
 * @param array $facet
 *   The facet definition.
 *
 * @return array
 *   An array of available widget plugins.
 */
function facetapi_get_widgets(array $realm, array $facet) {
  $plugins = array();

  // Iterates over all defined plugins, initializes requirements.
  foreach (ctools_get_plugins('facetapi', 'widgets') as $id => $plugin) {
    //Exclude abstract classes
    if(!isset($plugin['handler']['abstract']) || !$plugin['handler']['abstract']) {
      $plugin['handler'] += array(
        'requirements' => array(
          'facetapi_requirement_realm_property' => array('element type' => 'links')
        ),
      );

      // Checks requirements, only saves widgets that pass all requirements.
      if (facetapi_check_requirements($plugin['handler']['requirements'], $realm, $facet)) {
        $plugins[$id] = $plugin;
      }
    }
  }

  return $plugins;
}

/**
 * Returns the sorts available to the facet.
 *
 * All sorts are checked against their requirmenets. Only sorts that pass all
 * requirements are returned by this function.
 *
 * @param array $realm
 *   The realm definition.
 * @param array $facet
 *   The facet definition.
 *
 * @return array
 *   An array of available sorts plugins.
 */
function facetapi_get_available_sorts(array $realm, array $facet) {
  $sort_info = array();
  foreach (facetapi_get_sort_info() as $name => $info) {
    if (!$info['requirements'] || facetapi_check_requirements($info['requirements'], $realm, $facet)) {
      $sort_info[$name] = $info;
    }
  }
  return $sort_info;
}

/**
 * Form constructor for the facet display settings form.
 *
 * @param array $realm
 *   The realm definition.
 * @param array $facet
 *   The facet definition.
 *
 * @see facetapi_facet_display_form_submit()
 * @ingroup forms
 */
function facetapi_facet_display_form(&$form_state, FacetapiAdapter $adapter, array $realm, array $facet) {
  $form = array();
  $path = drupal_get_path('module', 'facetapi');
  drupal_add_css($path . '/facetapi.admin.css');
  drupal_add_js($path . '/facetapi.admin.js');
  ctools_include('plugins');

  // We have to set the title due to contextual link magic.
  // @see http://drupal.org/node/1147588#comment-4428940
  //drupal_set_title(t('Configure facet display'));

  // Captures variables and settings for code readability.
  $searcher = $adapter->getSearcher();
  $facet_settings = $adapter->getFacet($facet)->getSettings($realm);
  $global_settings = $adapter->getFacet($facet)->getSettings();

  $form['#facetapi'] = array(
    'adapter' => $adapter,
    'realm' => $realm,
    'facet' => $facet,
    'sort_info' => array(),
    'excluded_values' => array(
      'form_build_id', 'form_token', 'form_id', 'op', 'submit', 'submit_list',
      'settings__active_tab',
    ),
  );

  ////
  ////
  //// Widget settings
  ////
  ////

  $form['widget'] = array(
    '#type' => 'fieldset',
    '#title' => t('Display settings'),
  );

  // Builds select options for widgets, allows widgets to add settings.
  $widget_options = array();
  foreach (facetapi_get_widgets($realm, $facet) as $id => $plugin) {
    $widget_options[$id] = $plugin['handler']['label'];
    $class = $plugin['handler']['class'];
    $plugin = new $class($id, $realm, $adapter->getFacet($facet), $facet_settings);
    $plugin->settingsForm($form, $form_state);
  }

  $form['widget']['widget'] = array(
    '#type' => 'select',
    '#title' => t('Display widget'),
    '#default_value' => $facet_settings->settings['widget'],
    '#options' => $widget_options,
    '#weight' => -10,
    '#description' => t('Select the display widget used to render this facet.'),
  );

  ////
  ////
  //// Sort settings
  ////
  ////

  $form['widget']['sort'] = array(
    '#prefix' => '<div class="facetapi-sort-table">',
    '#suffix' => '</div>',
    '#weight' => -10,
  );

  $form['widget']['sort']['table'] = array(
    '#theme' => 'facetapi_sort_settings_table',
    '#facetapi' => &$form['#facetapi'],
    'sort_weight' => array('#tree' => TRUE),
    'sort_order' => array('#tree' => TRUE),
  );

  // Initializes sorts with default settings, orders by default weight. Ordering
  // the weights allows us to iterate over them in order when building the
  // form elements in the foreach() loop below.
  $sort_weight = $facet_settings->settings['sort_weight'];
  $sort_order = $facet_settings->settings['sort_order'];
  foreach (facetapi_get_available_sorts($realm, $facet) as $sort_name => $sort_info) {
    $weight = (isset($sort_weight[$sort_name])) ? $sort_weight[$sort_name] : 0;
    $form['#facetapi']['sort_info'][$sort_name] = $sort_info;
    $form['#facetapi']['sort_info'][$sort_name]['weight'] = $weight;
  }

  // Orders the sorts by the default weights set above.
  uasort($form['#facetapi']['sort_info'], 'facetapi_sort_weight');

  // Builds checkbox options and weight dropboxes.
  $sort_options = array();
  foreach ($form['#facetapi']['sort_info'] as $sort_name => $sort) {
    $sort_options[$sort_name] = '';

    $order_default = (isset($sort_order[$sort_name])) ? $sort_order[$sort_name] : SORT_ASC;
    $form['widget']['sort']['table']['sort_order'][$sort_name] = array(
      '#type' => 'select',
      '#title' => t('Order for sort @title', array('@title' => $sort['label'])),
      '#title_display' => 'invisible',
      '#options' => array(
        SORT_DESC => t('Descending'),
        SORT_ASC => t('Ascending'),
      ),
      '#default_value' => $order_default,
    );

    $weight_default = (isset($sort_weight[$sort_name])) ? $sort_weight[$sort_name] : 0;
    $form['widget']['sort']['table']['sort_weight'][$sort_name] = array(
      '#type' => 'weight',
      '#title' => t('Weight for sort @title', array('@title' => $sort['label'])),
      '#title_display' => 'invisible',
      '#delta' => 50,
      '#default_value' => $weight_default,
      '#attributes' => array('class' => array('facetapi-sort-weight')),
    );
  }

  $form['widget']['sort']['table']['active_sorts'] = array(
    '#type' => 'checkboxes',
    '#options' => $sort_options,
    '#default_value' => $facet_settings->settings['active_sorts'],
  );

  $form['widget']['empty'] = array(
    '#prefix' => '<div class="facetapi-empty-setting">',
    '#suffix' => '</div>',
    '#weight' => 10,
  );

  $empty_options = array();
  foreach (ctools_get_plugins('facetapi', 'empty_behaviors') as $id => $plugin) {
    //Exclude abstract classes
    if(!isset($plugin['handler']['abstract']) || !$plugin['handler']['abstract']) {
      $empty_options[$id] = $plugin['handler']['label'];
      $class = ctools_plugin_get_class($plugin, 'handler');
      $plugin_instance = new $class($facet_settings);
      $plugin_instance->settingsForm($form, $form_state);
    }
  }

  $form['widget']['empty']['empty_behavior'] = array(
    '#type' => 'select',
    '#title' => t('Empty facet behavior'),
    '#default_value' => $facet_settings->settings['empty_behavior'],
    '#options' => $empty_options,
    '#weight' => -10,
    '#description' => t('The action to take when a facet has no items.'),
  );

  ////
  ////
  //// Global settings
  ////
  ////

  $form['global'] = array(
    '#type' => 'fieldset',
    '#tree' => TRUE,
    '#title' => t('Global settings'),
    '#description' => t('The configuration options below apply to this facet across <em>all</em> realms.'),
  );

  // Loop through the query types so the key is the same as the value
  $query_types = drupal_map_assoc($facet['query types'], 'check_plain');
  $form['global']['query_type'] = array(
    '#type' => 'select',
    '#access' => count($facet['query types']) > 1,
    '#title' => t('Query type'),
    '#prefix' => '<div class="facetapi-global-setting">',
    '#suffix' => '</div>',
    '#default_value' => $global_settings->settings['query_type'],
    '#options' => $query_types,
    '#description' => t('Select the query type this facet will use.'),
  );

  $all_options = array(
    FACETAPI_OPERATOR_AND => t('AND'),
    FACETAPI_OPERATOR_OR => t('OR'),
  );
  $options = array_intersect_key($all_options, array_filter($facet['allowed operators']));
  $form['global']['operator'] = array(
    '#type' => 'radios',
    '#access' => count($options) > 1,
    '#title' => t('Operator'),
    '#prefix' => '<div class="facetapi-global-setting">',
    '#suffix' => '</div>',
    '#default_value' => $global_settings->settings['operator'],
    '#options' => $options,
    '#description' => t('AND filters are exclusive and narrow the result set. OR filters are inclusive and widen the result set.')
  );

  $hard_limit_options = drupal_map_assoc(array(3, 5, 10, 15, 20, 30, 40, 50, 75, 100));
  $hard_limit_options[-1] = t('No limit');

  $form['global']['hard_limit'] = array(
    '#type' => 'select',
    '#title' => t('Hard limit'),
    '#prefix' => '<div class="facetapi-global-setting">',
    '#suffix' => '</div>',
    '#default_value' => $global_settings->settings['hard_limit'],
    '#options' => $hard_limit_options,
    '#description' => t('Display no more than this number of facet items.')
  );

  $form['global']['flatten'] = array(
    '#type' => 'radios',
    '#access' => $facet['hierarchy callback'],
    '#title' => t('Flatten hierarchy'),
    '#prefix' => '<div class="facetapi-global-setting">',
    '#suffix' => '</div>',
    '#default_value' => $global_settings->settings['flatten'],
    '#options' => array(0 => t('No'), 1 => t('Yes')),
    '#description' => t('Do not process hierarchical relationships and display facet items as a flat list.'),
  );

  $form['global']['facet_mincount'] = array(
    '#type' => 'textfield',
    '#access' => $facet['facet mincount allowed'] && $adapter->supportsFacetMincount(),
    '#title' => t('Minimum facet count'),
    '#size' => 5,
    '#prefix' => '<div class="facetapi-global-setting">',
    '#suffix' => '</div>',
    '#default_value' => $global_settings->settings['facet_mincount'],
    '#description' => t('Only display facets that are matched in at least this many documents.'),
    '#element_validate' => array('facetapi_element_validate_integer'),
  );

  $form['global']['facet_missing'] = array(
    '#type' => 'radios',
    '#access' => $facet['facet missing allowed'] && $adapter->supportsFacetMissing(),
    '#title' => t('Add facet for missing values'),
    '#prefix' => '<div class="facetapi-global-setting">',
    '#suffix' => '</div>',
    '#default_value' => $global_settings->settings['facet_missing'],
    '#options' => array(0 => t('No'), 1 => t('Yes')),
    '#description' => t('Adds an extra facet matching documents with no value at all for this field.'),
  );

  ////
  ////
  //// Finalizes form
  ////
  ////

  $form['actions'] = array(
    '#weight' => 20,
  );

  // Gets destination from query string which is set when the page is navigated
  // to via a contextual link. Builds messages based on where user came from.
  if (isset($_GET['destination']) && !url_is_external($_GET['destination'])) {
    $submit_text = t('Save and go back to search page');
    $cancel_title = t('Return to the search page without saving configuration changes.');
    $url = facetapi_parse_url($_GET['destination']);
  }
  else {
    $submit_text = t('Save configuration');
    $cancel_title = t('Return to the realm settings page without saving configuration changes.');
    $url = array();
  }

  $form['actions']['submit'] = array(
    '#type' => 'submit',
    '#value' => $submit_text,
  );

  // Do not show the button if the page was navigated to via a contextual link
  // because it would redirect the user back to the search page.
  $form['actions']['submit_realm'] = array(
    '#type' => 'submit',
    '#access' => (!$url),
    '#value' => t('Save and go back to realm settings'),
  );

  $options = array('attributes' => array('title' => $cancel_title));
  if ($url) {
    $options['query'] = $url['query'];
  }
  $form['actions']['cancel'] = array(
    '#value' => l(t('Cancel'), (!$url) ? $adapter->getPath($realm['name']) : $url['path'], $options),
  );

  // Adds form submissions and validation handlers.
  $form['#submit'][] = 'facetapi_facet_display_form_submit';
  $form['#validate'][] = 'facetapi_facet_display_form_validate';

  // Allow the realm and adapter to alter the form.
  if ($realm['settings callback']) {
    $realm['settings callback']($form, $form_state);
  }
  $adapter->settingsForm($form, $form_state);

  return $form;
}

/**
 * Helper form element validator: integer.
 *
 * @see _element_validate_integer
 */
function facetapi_element_validate_integer($element, &$form_state) {
  $value = $element['#value'];
  if ($value !== '' && (!is_numeric($value) || intval($value) != $value)) {
    form_error($element, t('%name must be an integer.', array('%name' => $element['#title'])));
  }
}

/**
 * Returns facet sort order table.
 *
 * @param $variables
 *   An associative array containing:
 *   - element: A render element representing the form.
 *
 * @ingroup themeable
 */
function theme_facetapi_sort_settings_table($variables) {
  $output = '';

  // Gets variables for code readability.
  $searcher = $variables['#facetapi']['adapter']->getSearcher();

  $header = array(
    'active' => array(),
    'label' => array('data' => t('Sort')),
    'order' => array('data' => t('Order')),
    'weight' => array('data' => t('Weight'), 'sort' => 'asc'),
  );

  // Builds field options.
  $rows = array();
  $sort_info = $variables['#facetapi']['sort_info'];
  foreach ($sort_info as $sort_name => $sort) {
    $rows[$sort_name] = array(
      'class' => array('draggable'),
      'data' => array(
        drupal_render($variables['active_sorts'][$sort_name]),
        check_plain($sort['label']) . "<div class='description'>" . filter_xss($sort['description']) . '</div>',
        drupal_render($variables['sort_order'][$sort_name]),
        drupal_render($variables['sort_weight'][$sort_name]),
      ),
    );
  }

  drupal_add_tabledrag('facetapi-sort-settings', 'order', 'sibling', 'facetapi-sort-weight');
  //$output .= drupal_render_children($variables);
  $output .= theme('table', $header, $rows, array('id' => 'facetapi-sort-settings'));

  return $output;
}

/**
 * Form validation handler for facetapi_facet_display_form().
 */
function facetapi_facet_display_form_validate($form, &$form_state) {
  // Gets the selected query type and widget.
  $query_type = $form_state['values']['global']['query_type'];
  $widget_name = $form_state['values']['widget'];

  // Loads the widget plugin, makes sure it supports the selected type.
  if ($widget = ctools_get_plugins('facetapi', 'widgets', $widget_name)) {
    $widget_types = drupal_map_assoc($widget['handler']['query types']);
    if (!isset($widget_types[$query_type])) {
      $error = t('The widget does not support the %type query type', array('%type' => $query_type));
      form_set_error('widget', $error);
    }
  }
  else {
    $error = t('Widget %name not valid.', array('%name' => $widget_name));
    form_set_error('widget', $error);
  }
}

/**
 * Form submission handler for facetapi_facet_display_form().
 */
function facetapi_facet_display_form_submit($form, &$form_state) {

  // Pulls variables for code readability.
  $adapter = $form['#facetapi']['adapter'];
  $realm = $form['#facetapi']['realm'];
  $facet = $form['#facetapi']['facet'];
  $global_values = $form_state['values']['global'];
  unset($form_state['values']['global']);

  // Loads settings, saves all form values as settings other than excluded.
  $facet_settings = $adapter->getFacet($facet)->getSettings($realm);
  $facet_settings->settings = array_merge($facet_settings->settings, array_diff_key(
    $form_state['values'],
    array_flip($form['#facetapi']['excluded_values'])
  ));

  $global_settings = $adapter->getFacet($facet)->getSettings();
  foreach ($global_values as $key => $value) {
    $global_settings->settings[$key] = $value;
  }

  $success = TRUE;
  if (FALSE === ctools_export_crud_save('facetapi', $facet_settings)) {
    drupal_set_message(t('Error saving configuration options.'), 'error');
    $success = FALSE;
  }
  if (FALSE === ctools_export_crud_save('facetapi', $global_settings)) {
    drupal_set_message(t('Error saving configuration options.'), 'error');
    $success = FALSE;
  }

  // Sets message if both sets of configurations were saved.
  if ($success) {
    drupal_set_message(t('The configuration options have been saved.'));
  }

  // Redirects back to the realm settings page if necessary.
  $clicked = $form_state['clicked_button']['#value'];
  if (t('Save and go back to realm settings') == $clicked) {
    $form_state['redirect'] = $adapter->getPath($realm['name']);
  }
}

/**
 * Form constructor for the facet filter settings form.
 *
 * @param FacetapiAdapter $adapter
 *   The adapter object.
 * @param array $realm
 *   The realm definition.
 * @param array $filter_options
 *   The available filter options.
 *
 * @see facetapi_facet_filters_form_submit()
 * @ingroup forms
 */
function facetapi_facet_filters_form(&$form_state, FacetapiAdapter $adapter, array $realm, array $filters) {
  $form = array();
  // We have to set the title due to contextual link magic.
  // @see http://drupal.org/node/1147588#comment-4428940
  drupal_set_title(t('Configure facet filters'));

  // Loads variables for code readability.
  $facet = $filters['facet'];
  $settings = $adapter->getFacet($facet)->getSettings($realm);

  // Adds Facet API settings, excluded values aren't saved.
  $form['#facetapi'] = array(
    'adapter' => $adapter,
    'realm' => $realm,
    'facet' => $facet,
    'settings' => $settings,
  );

  // Filter order (tabledrag).
  $form['settings']['filters']['table'] = array(
    '#type' => 'item',
    '#title' => t('Active filters'),
    '#theme' => 'facetapi_filter_settings_table',
  );

  $form['settings']['filters']['filter_settings_title'] = array(
    '#type' => 'item',
    '#title' => t('Filter settings'),
    '#prefix' => '<div id="filters-status-wrapper">',
    '#suffix' => '</div>',
    '#weight' => 10,
  );

  $form['settings']['filters']['filter_settings'] = array(
    '#type' => 'vertical_tabs',
    '#weight' => 20,
  );

  // Builds plugins.
  $weight = -50;
  $plugins = array();
  foreach ($filters['plugins'] as $id => $plugin) {
    if ($class = ctools_plugin_load_class('facetapi', 'filters', $id, 'handler')) {
      // Gets filter defaults.
      $filter_plugin = new $class($id, $adapter, $settings);
      $default = $filter_plugin->getDefaultSettings() + array(
        'status' => 0,
        'weight' => $weight++,
      );
      // Merges in default settings.
      if (empty($settings->settings['filters'][$id])) {
        $settings->settings['filters'][$id] = array();
      }
      $settings->settings['filters'][$id] += $default;
      // Adds weight to plugin so we can sort it.
      $plugins[$id] = $plugin + array(
        'weight' => $settings->settings['filters'][$id]['weight'],
      );
    }
  }
  uasort($plugins, 'facetapi_sort_weight');

  // Builds table, adds settings to vertical tabs.
  $has_settings = FALSE;
  foreach ($plugins as $id => $plugin) {

    // Instantiates filter plugins.
    $filter_plugin = new $class($id, $adapter, $settings);
    $defaults = $filter_plugin->getDefaultSettings();

    // Table elements.
    $form['settings']['filters']['table'][$id]['status'] = array(
      '#type' => 'checkbox',
      '#title' => t('Enable @title', array('@title' => $plugin['handler']['label'])),
      '#title_display' => 'invisible',
      '#default_value' => $settings->settings['filters'][$id]['status'],
      '#parents' => array('settings', 'filters', $id, 'status'),
    );
    $form['settings']['filters']['table'][$id]['filter'] = array(
      '#value' => filter_xss_admin($plugin['handler']['label']),
    );
    $form['settings']['filters']['table'][$id]['weight'] = array(
      '#type' => 'weight',
      '#title' => t('Weight for @title', array('@title' => $plugin['handler']['label'])),
      '#title_display' => 'invisible',
      '#delta' => 50,
      '#default_value' => $settings->settings['filters'][$id]['weight'],
      '#parents' => array('settings', 'filters', $id, 'weight'),
      '#weight' => 0,
    );

    // Initializes vertical tab.
    $form['settings']['filters']['filter_settings'][$id] = array(
      '#type' => 'fieldset',
      '#title' => check_plain($plugin['handler']['label']),
      '#group' => 'settings',
      '#tree' => TRUE,
    );

    // Allows plugin to add settings to the form.
    if ($class = ctools_plugin_load_class('facetapi', 'filters', $id, 'handler')) {
      $filter = new $class($id, $adapter, $settings);
      $filter->settingsForm($form['settings']['filters']['filter_settings'][$id], $form_state);
    }

    // Removes vertical tab if no settings were added.
    if (element_children($form['settings']['filters']['filter_settings'][$id])) {
      $has_settings = TRUE;
    }
    else {
      unset($form['settings']['filters']['filter_settings'][$id]);
    }
  }

  // Removes title and fieldset if there aren't any settings.
  if (!$has_settings) {
    unset($form['settings']['filters']['filter_settings']);
    unset($form['settings']['filters']['filter_settings_title']);
  }

  $form['actions'] = array(
    '#weight' => 20,
  );

  // Gets destination from query string which is set when the page is navigated
  // to via a contextual link. Builds messages based on where user came from.
  if (isset($_GET['destination']) && !url_is_external($_GET['destination'])) {
    $submit_text = t('Save and go back to search page');
    $cancel_title = t('Return to the search page without saving configuration changes.');
    $url = facetapi_parse_url($_GET['destination']);
  }
  else {
    $submit_text = t('Save configuration');
    $cancel_title = t('Return to the realm settings page without saving configuration changes.');
    $url = array();
  }

  $form['actions']['submit'] = array(
    '#type' => 'submit',
    '#value' => $submit_text,
  );

  // Do not show the button if the page was navigated to via a contextual link
  // because it would redirect the user back to the search page.
  $form['actions']['submit_realm'] = array(
    '#type' => 'submit',
    '#access' => (!$url),
    '#value' => t('Save and go back to realm settings'),
  );

  $options['attributes'] = array('title' => $cancel_title);
  if ($url) {
    $options['query'] = $url['query'];
  }
  $form['actions']['cancel'] = array(
    '#value' => l(t('Cancel'), (!$url) ? $adapter->getPath($realm['name']) : $url['path'], $options),
  );

  $form['#submit'][] = 'facetapi_facet_filters_form_submit';

  return $form;
}

/**
 * Returns facet filter order table.
 *
 * @param $variables
 *   An associative array containing:
 *   - element: A render element representing the form.
 *
 * @ingroup themeable
 */
function theme_facetapi_filter_settings_table($variables) {
  $element = $variables;

// Initializes table header.
  $header = array(
    'status' => array('data' => t('Status')),
    'filter' => array('data' => t('Filter')),
    'weight' => array('data' => t('Weight'), 'sort' => 'asc'),
  );
  // Filter order (tabledrag).
  $rows = array();
  foreach (element_children($element, TRUE) as $name) {
    $element[$name]['weight']['#attributes']['class'] = 'facetapi-filter-weight';
    $rows[] = array(
      'data' => array(
        drupal_render($element[$name]['status']),
        drupal_render($element[$name]['filter']),
        drupal_render($element[$name]['weight']),
      ),
      'class' => array('draggable'),
    );
  }
  //$output = drupal_render_children($element);
  $output .= theme('table', $header, $rows, array('id' => 'facetapi-filter-settings'));
  drupal_add_tabledrag('facetapi-filter-settings', 'order', 'sibling', 'facetapi-filter-weight', NULL, NULL, TRUE);

  return $output;
}

/**
 * Form submission handler for facetapi_facet_filters_form().
 */
function facetapi_facet_filters_form_submit($form, &$form_state) {
  // Pulls variables for code readability.
  $adapter = $form['#facetapi']['adapter'];
  $realm = $form['#facetapi']['realm'];
  $facet = $form['#facetapi']['facet'];
  $settings = $form['#facetapi']['settings'];

  // Merges passed settings in with the settings object.
  $form_state_values_settings = $form_state['values']['settings'];
  // Go though all filters and if filter is enabled save its form settings.
  foreach ($form_state_values_settings['filters'] as $filter_name => $filter_settings) {
    if ($filter_settings['status'] && !empty($form_state['values'][$filter_name])) {
      $form_state_values_settings += $form_state['values'][$filter_name];
    }
  }
  $settings->settings = array_merge($settings->settings, $form_state_values_settings);

  // Saves the configuration options.
  if (FALSE !== ctools_export_crud_save('facetapi', $settings)) {
    drupal_set_message(t('The configuration options have been saved.'));
  }
  else {
    drupal_set_message(t('Error saving configuration options.'), 'error');
  }

  // Redirects back to the realm settings page if necessary.
  $clicked = $form_state['clicked_button']['#value'];
  if (t('Save and go back to realm settings') == $clicked) {
    $form_state['redirect'] = $adapter->getPath($realm['name']);
  }
}

/**
 * Form constructor for the facet dependency settings form.
 *
 * @param FacetapiAdapter $adapter
 *   The adapter object the settings apply to.
 * @param array $realm
 *   The realm definition.
 * @param array $dependencies
 *   An array of dependencies.
 *
 * @see facetapi_facet_dependencies_form_submit()
 * @ingroup forms
 */
function facetapi_facet_dependencies_form(&$form_state, FacetapiAdapter $adapter, array $realm, array $dependencies) {
  $form = array();
  // We have to set the title due to contextual link magic.
  // @see http://drupal.org/node/1147588#comment-4428940
  drupal_set_title(t('Configure facet dependencies'));

  // Adds Facet API settings, excluded values aren't saved.
  $form['#facetapi'] = array(
    'adapter' => $adapter,
    'realm' => $realm,
    'settings' => FALSE,
    'defaults' => array(),
  );

  $form['description'] = array(
    '#prefix' => '<div class="facetapi-realm-description">',
    '#value' => t('Dependencies are conditions that must be met in order for the facet to be processed by the server and displayed to the user. Hiding facets via the core block system or through the Drupal forms API will not prevent the server from processing the facets.'),
    '#suffix' => "</div>\n",
  );

  $form['plugins'] = array(
    '#type' => 'vertical_tabs',
    '#weight' => 10,
  );

  // Iterates over plugins, adds settings as vertical tabs.
  $plugins = ctools_get_plugins('facetapi', 'dependencies');
  foreach ($dependencies as $plugin) {
    // Only gets settings once.
    if (!$form['#facetapi']['settings']) {
      $settings = $adapter->getFacet($plugin->getFacet())->getSettings();
      $form['#facetapi']['settings'] = $settings;
    }

    // Initializes vertical tab.
    $id = $plugin->getId();
    $form[$id] = array(
      '#type' => 'fieldset',
      '#title' => check_plain($plugins[$id]['handler']['label']),
      '#group' => 'plugins',
    );

    // Allows plugin to add settings to the form, adds defaults.
    $plugin->settingsForm($form, $form_state);
    $form['#facetapi']['defaults'] += $plugin->getDefaultSettings();

    // Removes vertical tab if nothing was added.
    if (!element_children($form[$id])) {
      unset($form[$id]);
    }
  }

  $form['actions'] = array(
    '#weight' => 20,
  );

  // Gets destination from query string which is set when the page is navigated
  // to via a contextual link. Builds messages based on where user came from.
  if (isset($_GET['destination']) && !url_is_external($_GET['destination'])) {
    $submit_text = t('Save and go back to search page');
    $cancel_title = t('Return to the search page without saving configuration changes.');
    $url = facetapi_parse_url($_GET['destination']);
  }
  else {
    $submit_text = t('Save configuration');
    $cancel_title = t('Return to the realm settings page without saving configuration changes.');
    $url = array();
  }

  $form['actions']['submit'] = array(
    '#type' => 'submit',
    '#value' => $submit_text,
  );

  // Do not show the button if the page was navigated to via a contextual link
  // because it would redirect the user back to the search page.
  $form['actions']['submit_realm'] = array(
    '#type' => 'submit',
    '#access' => (!$url),
    '#value' => t('Save and go back to realm settings'),
  );

  $options = array('attributes' => array('title' => $cancel_title));
  if ($url) {
    $options['query'] = $url['query'];
  }
  $form['actions']['cancel'] = array(
    '#value' => l(t('Cancel'), (!$url) ? $adapter->getPath($realm['name']) : $url['path'], $options),
  );

  $form['#submit'][] = 'facetapi_facet_dependencies_form_submit';

  return $form;
}

/**
 * Form submission handler for facetapi_facet_dependencies_form().
 */
function facetapi_facet_dependencies_form_submit($form, &$form_state) {
  $success = TRUE;

  // Pulls variables for code readability.
  $adapter = $form['#facetapi']['adapter'];
  $realm = $form['#facetapi']['realm'];
  $settings = $form['#facetapi']['settings'];
  $defaults = $form['#facetapi']['defaults'];

  // Gets dependency settings from form.
  $values = array_intersect_key($form_state['values'], $defaults);
  $settings->settings['dependencies'] = $values + $defaults;

  // Writes the settings to the database.
  if (FALSE === ctools_export_crud_save('facetapi', $settings)) {
    drupal_set_message(t('Error saving configuration options.'), 'error');
    $success = FALSE;
  }

  // Sets message if both sets of configurations were saved.
  if ($success) {
    drupal_set_message(t('The configuration options have been saved.'));
  }

  // Redirects back to the realm settings page if necessary.
  $clicked = $form_state['clicked_button']['#value'];
  if (t('Save and go back to realm settings') == $clicked) {
    $form_state['redirect'] = $adapter->getPath($realm['name']);
  }
}

/**
 * Form constructor for the export form.
 *
 * @param FacetapiAdapter $adapter
 *   The adapter object the settings apply to.
 * @param array $realm
 *   The realm definition.
 * @param array $facet
 *   The facet definition.
 *
 * @see ctools_export_form()
 * @ingroup forms
 */
function facetapi_export_page(FacetapiAdapter $adapter, array $realm, array $facet) {

  // Gets export fieldset for a realm display settings.
  $title = t('Display settings');
  $settings = $adapter->getFacetSettings($facet, $realm);
  $export = ctools_export_crud_export('facetapi', $settings);
  $output = drupal_get_form('ctools_export_form', $export, $title);

  // Gets export fieldset for global display settings.
  $title = t('Global settings');
  $settings = $adapter->getFacetSettingsGlobal($facet);
  $export = ctools_export_crud_export('facetapi', $settings);
  $output .= drupal_get_form('ctools_export_form', $export, $title);
  return $output;
}

/**
 * Form constructor for the revert form.
 *
 * @param FacetapiAdapter $adapter
 *   The adapter object the settings apply to.
 * @param array $realm
 *   The realm definition.
 * @param array $facet
 *   The facet definition.
 *
 * @ingroup forms
 */
function facetapi_revert_form(&$form_state, FacetapiAdapter $adapter, array $realm, array $facet) {
  $form = array();
  $form['#facetapi'] = array(
    'adapter' => $adapter,
    'realm' => $realm,
    'facet' => $facet,
  );

  $form['text'] = array(
   '#value' => '<p>' . t('Any changes you have made will be lost and cannot be recovered.') . '</p>',
  );

  $form['revert_facet'] = array(
    '#type' => 'checkbox',
    '#access' => facetapi_is_overridden($adapter->getFacetSettings($facet, $realm)),
    '#title' => t('Revert per-facet display configuration settings.'),
    '#default_value' => TRUE,
  );

  $form['revert_global'] = array(
    '#type' => 'checkbox',
    '#access' => facetapi_is_overridden($adapter->getFacetSettingsGlobal($facet)),
    '#title' => t('Revert global configuration settings.'),
    '#default_value' => TRUE,
  );

  $form['actions'] = array(
    '#weight' => 20,
  );

  $form['actions']['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Revert configuration'),
  );

  $form['actions']['cancel'] = array(
    '#value' => l(t('Cancel'), $adapter->getPath($realm['name']), array('attributes' => array('title' => t('Do not revert configurations')))),
  );

  $form['#submit'][] = 'facetapi_revert_form_submit';

  return $form;
}

/**
 * Form submission handler for facetapi_revert_form_submit().
 */
function facetapi_revert_form_submit($form, &$form_state) {

  // Pulls variables for code readability.
  $adapter = $form['#facetapi']['adapter'];
  $realm = $form['#facetapi']['realm'];
  $facet = $form['#facetapi']['facet'];

  // Reverts per-facet display settings.
  if ($form_state['values']['revert_facet']) {
    $facet_settings = $adapter->getFacetSettings($facet, $realm);
    ctools_export_crud_delete('facetapi', $facet_settings);
  }

  // Reverts global settings if selected.
  if ($form_state['values']['revert_global']) {
    $global_settings = $adapter->getFacetSettingsGlobal($facet);
    ctools_export_crud_delete('facetapi', $global_settings);
  }

  // Sets message if both sets of configurations were saved.
  drupal_set_message(t('The configuration options have been reverted.'));
  $form_state['redirect'] = $adapter->getPath($realm['name']);
}
