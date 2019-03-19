<?php

/**
 * @file
 * Adapter plugin and adapter related calsses.
 */

/**
 * Abstract class extended by search backends that retrieves facet information
 * from the database.
 */
abstract class FacetapiAdapter {

  /**
   * Searcher information retrieved by the hook.
   *
   * @var array
   */
  protected $info = array();

  /**
   * The search keys passed by the user.
   *
   * @var string
   */
  protected $keys;

  /**
   * An array of FacetapiFacet objects.
   *
   * @var array
   */
  protected $facets = array();

  /**
   * An array of FacetapiFacetProcessor objects.
   *
   * @var array
   */
  protected $processors = array();

  /**
   * An array of executed query type plugins keyed by field name.
   *
   * @var array
   */
  protected $queryTypes = array();

  /**
   * The URL processor associated with this adapter.
   *
   * @var FacetapiUrlProcessor
   */
  protected $urlProcessor;

  /**
   * An array of active filters.
   *
   * @var array
   */
  protected $activeItems;

  /**
   * A boolean flagging whether the facets have been processed.
   *
   * @var boolean
   */
  protected $processed = FALSE;

  /**
   * Stores the search path so we only need to calculate it once.
   *
   * @var string
   */
  protected $searchPath;

  /**
   * Stores whether each facet passed dependencies.
   *
   * @var array
   */
  protected $dependenciesPassed = array();

  /**
   * Stores settings with defaults.
   *
   * @var array
   */
  protected $settings = array();

  /**
   * Constructor, sets searcher and type of content being indexed.
   *
   * @param array $searcher_info
   *   The searcher definition.
   */
  public function __construct(array $searcher_info) {
    $this->info = $searcher_info;

    // Registers the query type plugins classes associated with this adapter.
    $registered_types = array();
    foreach (ctools_get_plugins('facetapi', 'query_types') as $plugin) {
      if ((isset($searcher_info['adapter']) && isset($plugin['handler']['adapter'])) &&
          ($searcher_info['adapter'] == $plugin['handler']['adapter'])) {
        $class = ctools_plugin_get_class($plugin, 'handler');
        $type = call_user_func(array($class, 'getType'));
        $registered_types[$type] = $class;
      }
    }

    // Iterates over facets and registers query type plugins.
    foreach ($this->getEnabledFacets() as $facet) {

      // Gets widget type from setting if there are more than one available.
      if (1 == count($facet['query types'])) {
        $query_type = $facet['query types'][0];
      }
      else {
        $settings = $this->getFacetSettingsGlobal($facet)->settings;
        $query_type = !empty($settings['query_type']) ? $settings['query_type'] : FALSE;
      }

      // If we found a query type, register the query type plugin.
      if ($query_type && isset($registered_types[$query_type])) {
        $plugin = new $registered_types[$query_type]($this, $facet);
        $this->queryTypes[$facet['name']] = $plugin;
      }
      else {
        $this->queryTypes[$facet['name']] = FALSE;
      }
    }

    // Instantiates URL processor plugin.
    $id = $searcher_info['url processor'];
    $class = ctools_plugin_load_class('facetapi', 'url_processors', $id, 'handler');
    if (!$class) {
      $class = ctools_plugin_load_class('facetapi', 'url_processors', 'standard', 'handler');
    }
    $this->urlProcessor = new $class($this);

    // Fetches, normalizes, and sets filter params.
    $filter_key = $this->urlProcessor->getFilterKey();
    $params = $this->urlProcessor->fetchParams();
    $this->setParams($params, $filter_key);
  }

  /**
   * Returns a boolean flagging whether $this->info['searcher'] executed a search.
   *
   * @return
   *   A boolean flagging whether $this->info['searcher'] executed a search.
   *
   * @todo Generic search API should provide consistent functionality.
   */
  abstract public function searchExecuted();

  /**
   * Returns a boolean flagging whether facets in a realm shoud be displayed.
   *
   * Useful, for example, for suppressing sidebar blocks in some cases.
   *
   * @return
   *   A boolean flagging whether to display a given realm.
   *
   * @todo Generic search API should provide consistent functionality.
   */
  abstract public function suppressOutput($realm_name);

  /**
   * Processes a raw array of active filters.
   *
   * @param array $params
   *   An array of keyed params, such as $_GET.
   * @param string $filter_key
   *   The array key in $params corresponding to filters.
   *
   * @return FacetapiAdapter
   *   An instance of this class.
   */
  public function setParams(array $params = array(), $filter_key = 'f') {
    $this->facets = array();
    $normalized = $this->urlProcessor->normalizeParams($params, $filter_key);
    $this->urlProcessor->setParams($normalized, $filter_key);
    $this->processActiveItems();
    return $this;
  }

  /**
   * Processes active items.
   *
   * @see FacetapiAdapter::setParams()
   */
  public function processActiveItems() {
    $this->activeItems = array('facet' => array(), 'filter' => array());

    // Groups enabled facets by facet alias.
    $enabled_aliases = array();
    foreach ($this->getEnabledFacets() as $facet) {
      $enabled_aliases[$facet['field alias']][] = $facet['name'];
      $this->activeItems['facet'][$facet['name']] = array();
    }

    // Extracts valid filters from query string.
    $filter_key = $this->urlProcessor->getFilterKey();
    $params = $this->urlProcessor->getParams();
    foreach ($params[$filter_key] as $pos => $filter) {
      // Bails if an object or array.
      if (!is_scalar($filter)) {
        continue;
      }

      // Performs basic parsing of the filter.
      $parts = explode(':', $filter, 2);
      $field_alias = rawurldecode($parts[0]);
      if (isset($parts[1]) && isset($enabled_aliases[$field_alias])) {

        // Stores the base item.
        $item = array(
          'field alias' => $field_alias,
          'value' => $parts[1],
          'pos' => $pos,
        );

        // Stores active items in the global active item array.
        $this->activeItems['filter'][$filter] = $item;
        $this->activeItems['filter'][$filter]['facets'] = array();

        // Stores active items per facet.
        foreach ($enabled_aliases[$field_alias] as $facet_name) {
          $item += $this->queryTypes[$facet_name]->extract($item);
          $this->activeItems['filter'][$filter]['facets'][] = $facet_name;
          $this->activeItems['facet'][$facet_name][$parts[1]] = $item;
        }
      }
    }
  }

  /**
   * Returns the URL Processor.
   *
   * @return FacetapiUrlProcessor
   *   The URL Processor plugin.
   */
  public function getUrlProcessor() {
    return $this->urlProcessor;
  }

  /**
   * Returns all active filters.
   *
   * @return array
   *   An array of active filters.
   */
  public function getAllActiveItems() {
    return $this->activeItems['filter'];
  }

  /**
   * Returns a facet's active items.
   *
   * @param array|string $facet
   *   The facet definition or facet name,
   *
   * @return array
   *   The facet's active items.
   */
  public function getActiveItems(array $facet) {
    return $this->activeItems['facet'][$facet['name']];
  }

  /**
   * Tests whether a facet item is active by passing it's value.
   *
   * @param string $facet_name
   *   The facet name.
   * @param string $value
   *   The facet item's value.
   *
   * @return
   *   Returns 1 if the item is active, 0 if it is inactive.
   */
  public function itemActive($facet_name, $value) {
    return (int) isset($this->activeItems['facet'][$facet_name][$value]);
  }

  /**
   * Returns the id of the adapter plugin.
   *
   * @return string
   *   The machine readable if of the adapter plugin.
   */
  public function getId() {
    return $this->info['adapter'];
  }

  /**
   * Returns the machine readable name of the searcher.
   *
   * @return string
   *   The machine readable name of the searcher.
   */
  public function getSearcher() {
    return $this->info['name'];
  }

  /**
   * Returns the type of content indexed by $this->info['searcher'].
   *
   * @return
   *   The type of content indexed by $this->info['searcher'].
   */
  public function getTypes() {
    return $this->info['types'];
  }

  /**
   * Returns the path to the admin settings for a given realm.
   *
   * @param $realm_name
   *   The name of the realm.
   *
   * @return
   *   The path to the admin settings.
   */
  public function getPath($realm_name) {
    return $this->info['path'] . '/facets/' . $realm_name;
  }

  /**
   * Returns the search path.
   *
   * @return string
   *   A string containing the search path.
   *
   * @todo D8 should provide an API function for this.
   */
  public function getSearchPath() {
    if (NULL === $this->searchPath) {
      // Backwards compatibility with apachesolr <= beta8.
      // @see http://drupal.org/node/1305748#comment-5102352
      foreach (array($this->info['module'], $this->info['module'] . '_search') as $module) {
        if ($path = module_invoke($module, 'search_info')) {
          $this->searchPath = 'search/' . $path['path'];
          if (!isset($_GET['keys']) && ($keys = $this->getSearchKeys())) {
            $this->searchPath .= '/' . $keys;
          }
          break;
        }
      }
    }
    return $this->searchPath;
  }

  /**
   * Sets the search keys.
   *
   * @param string $keys
   *   The search keys entered by the user.
   *
   * @return FacetapiAdapter
   *   An instance of this class.
   */
  public function setSearchKeys($keys) {
    $this->keys = $keys;
    return $this;
  }

  /**
   * Gets the search keys.
   *
   * @return string
   *   The search keys entered by the user.
   */
  public function getSearchKeys() {
    return $this->keys;
  }

  /**
   * Returns the number of results returned by the search query.
   *
   * @return int
   *   An integer containing the number of results.
   */
  public function getResultCount() {
    global $pager_total;
    return isset($pager_total[0]) ? $pager_total[0] : 0;
  }

  /**
   * Returns the number of results per page.
   *
   * @return int
   *   The number of results per page, or the limit.
   */
  public function getPageLimit() {
    global $pager_limits;
    return isset($pager_limits[0]) ? $pager_limits[0] : 10;
  }

  /**
   * Returns the page number of the search result set.
   *
   * @return int
   *   The current page of the result set.
   */
  public function getPageNumber() {
    return pager_find_page() + 1;
  }

  /**
   * Returns the total number of pages in the result set.
   *
   * @return int
   *   The total number of pages.
   */
  public function getPageTotal() {
    global $pager_total;
    return isset($pager_total[0]) ? $pager_total[0] : 0;
  }

  /**
   * Allows for backend specific overrides to the settings form.
   */
  public function settingsForm(&$form_state) {
    // Nothing to do...
  }

  /**
   * Provides default values for the backend specific settings.
   *
   * @return array
   *   The defaults keyed by setting name to value.
   */
  public function getDefaultSettings() {
    return array();
  }

  /**
   * Returns TRUE if the back-end supports "missing" facets.
   *
   * @return bool
   *   TRUE or FALSE.
   */
  public function supportsFacetMissing() {
    return $this->info['supports facet missing'];
  }

  /**
   * Returns TRUE if the back-end supports "minimum facet counts".
   *
   * @return bool
   *   TRUE or FALSE.
   */
  public function supportsFacetMincount() {
    return $this->info['supports facet mincount'];
  }

  /**
   * Adds facet query type plugins to the queue and invokes the execute() hook
   * to allow for the backend to add filters to its native query object.
   *
   * @param mixed $query
   *   The backend's native object.
   */
  function addActiveFilters($query) {
    module_load_include('inc', 'facetapi', 'facetapi.callbacks');
    facetapi_add_active_searcher($this->info['name']);

    // Runs initActiveFilters hook, finds active facets.
    $this->initActiveFilters($query);
    foreach ($this->getEnabledFacets() as $facet) {
      $settings = $this->getFacet($facet)->getSettings();

      // Invoke the dependency plugins.
      $display = TRUE;
      foreach ($facet['dependency plugins'] as $id) {
        $class = ctools_plugin_load_class('facetapi', 'dependencies', $id, 'handler');
        $plugin = new $class($id, $this, $facet, $settings, $this->activeItems['facet']);
        if (NULL !== ($return = $plugin->execute())) {
          $display = $return;
        }
      }

      // Stores whether this facet passed its dependencies.
      $this->dependenciesPassed[$facet['name']] = $display;

      // Add query type plugin if dependencies were met, otherwise remove the
      // facet's active items so they don't display in the current search block
      // or appear as active in the breadcrumb trail.
      if ($display && $this->queryTypes[$facet['name']]) {
        $this->queryTypes[$facet['name']]->execute($query);
      }
      else {
        foreach ($this->activeItems['facet'][$facet['name']] as $item) {
          $this->urlProcessor->removeParam($item['pos']);
          $filter = $item['field alias'] . ':' . $item['value'];
          unset($this->activeItems['filter'][$filter]);
        }
        $this->activeItems['facet'][$facet['name']] = array();
      }
    }
  }

  /**
   * Allows the backend to initialize its query object before adding the facet
   * filters.
   *
   * @param mixed $query
   *   The backend's native object.
   */
  public function initActiveFilters($query) {
    // Nothing to do ...
  }

  /**
   * Initializes a new settings object.
   *
   * @param $name
   *   A string containing the unique name of the configuration.
   * @param array $facet_name
   *   A string containing the machine readable name of the facet.
   * @param $realm_name
   *   A string containing the machine readable name of the realm, NULL if we
   *   are initializing global settings.
   *
   * @return stdClass
   *   An object containing the initialized settings.
   */
  public function initSettingsObject($name, $facet_name, $realm_name = NULL) {
    $cached_settings = facetapi_get_searcher_settings($this->info['name']);
    if (!isset($cached_settings[$name])) {
      $settings = ctools_export_crud_new('facetapi');
      $settings->name = $name;
      $settings->searcher = $this->info['name'];
      $settings->realm = (string) $realm_name;
      $settings->facet = $facet_name;
      $settings->enabled = 0;
      $settings->settings = array();
    }
    else {
      $settings = $cached_settings[$name];
    }
    return $settings;
  }

  /**
   * Returns realm specific settings for a facet.
   *
   * @param array $facet
   *   An array containing the facet definition.
   * @param array $realm
   *   An array containing the realm definition.
   *
   * @return stdClass
   *   An object containing the settings.
   *
   * @see ctools_export_crud_load()
   */
  public function getFacetSettings(array $facet, array $realm) {
    // Builds the unique name of the configuration settings and loads.
    $name = $this->info['name'] . ':' . $realm['name'] . ':' . $facet['name'];
    if (!isset($this->settings[$name])) {
      $this->settings[$name] = $this->initSettingsObject($name, $facet['name'], $realm['name']);
      $is_new = empty($this->settings[$name]->settings);

      // Use realm's default widget if facet doesn't define one.
      if (!empty($facet['default widget'])) {
        $widget = $facet['default widget'];
      }
      else {
        $widget = $realm['default widget'];
      }

      // Apply default settings.
      $this->settings[$name]->settings += array(
        'weight' => 0,
        'widget' => $widget,
        'filters' => array(),
        'active_sorts' => array(),
        'sort_weight' => array(),
        'sort_order' => array(),
        'empty_behavior' => 'none',
      );

      // Apply default sort info if necessary.
      if ($is_new) {
        $weight = -50;
        foreach ($facet['default sorts'] as $sort => $default) {
          $this->settings[$name]->settings['active_sorts'][$default[0]] = $default[0];
          $this->settings[$name]->settings['sort_weight'][$default[0]] = $weight++;
          $this->settings[$name]->settings['sort_order'][$default[0]] = $default[1];
        }
      }

      // Apply the widget plugin's default settings.
      $id = $this->settings[$name]->settings['widget'];
      $class = ctools_plugin_load_class('facetapi', 'widgets', $id, 'handler');
      // If we have an invalid widget, fall back to the realm's default.
      if (!$class) {
        $id = $this->settings[$name]->settings['widget'] = $realm['default widget'];
        $class = ctools_plugin_load_class('facetapi', 'widgets', $id, 'handler');
      }
      $plugin = new $class($id, $realm, $this->getFacet($facet), $this->settings[$name]);
      $this->settings[$name]->settings += $plugin->getDefaultSettings();

      // @todo Save for performance?
    }

    return $this->settings[$name];
  }

  /**
   * Returns realm specific settings for a facet.
   *
   * @param array $facet
   *   An array containing the facet definition.
   *
   * @return
   *   An object containing the settings.
   *
   * @see ctools_export_crud_load()
   */
  public function getFacetSettingsGlobal(array $facet) {
    $name = $this->info['name'] . '::' . $facet['name'];
    if (!isset($this->settings[$name])) {
      $this->settings[$name] = $this->initSettingsObject($name, $facet['name']);
      $is_new = empty($this->settings[$name]->settings);

      // Ensure the default operator and query type are valid.
      // @see http://drupal.org/node/1443340
      $default_query_type = reset($facet['query types']);
      $allowed_operators = array_filter($facet['allowed operators']);
      $default_operator = key($allowed_operators);

      // Apply defaults common across all configs.
      $this->settings[$name]->settings += array(
        'operator' => $default_operator,
        'hard_limit' => 50,
        'dependencies' => array(),
        'facet_mincount' => 1,
        'facet_missing' => 0,
        'flatten' => 0,
        'query_type' => $default_query_type,
      );

      // Apply the adapter's default settings.
      $this->settings[$name]->settings += $this->getDefaultSettings();

      // Applies each dependency plugin's default settings.
      foreach ($facet['dependency plugins'] as $id) {
        if ($is_new) {
          $this->settings[$name]->settings['dependencies'] = array();
        }
        $class = ctools_plugin_load_class('facetapi', 'dependencies', $id, 'handler');
        $plugin = new $class($id, $this, $facet, $this->settings[$name], array());
        $this->settings[$name]->settings['dependencies'] += $plugin->getDefaultSettings();
      }

      // @todo Save for performance?
    }

    return $this->settings[$name];
  }

  /**
   * Returns the enabled facets associated with the instance of the adapter.
   *
   * @param string $realm_name
   *   The machine readable name of the realm, pass NULL to get the enabled
   *   facets in all realms.
   *
   * @return array
   *   An array of enabled facets.
   */
  public function getEnabledFacets($realm_name = NULL) {
    return facetapi_get_enabled_facets($this->info['name'], $realm_name);
  }

  /**
   * Returns a FacetapiFacet instance for the facet being rendered.
   *
   * @param array $facet
   *   The facet definition.
   *
   * @return FacetapiFacet
   *   The facet rendering object object.
   */
  public function getFacet(array $facet) {
    if (!isset($this->facets[$facet['name']])) {
      $this->facets[$facet['name']] = new FacetapiFacet($this, $facet);
    }
    return $this->facets[$facet['name']];
  }

  /**
   * Returns a registered facet query
   *
   * @param array|string $facet
   *   The facet definition or facet name.
   *
   * @return FacetapiQueryTypeInterface
   *   The instantiated query type plugin.
   */
  public function getFacetQuery($facet) {
    $facet_name = (is_array($facet)) ? $facet['name'] : $facet;
    if (isset($this->queryTypes[$facet_name])) {
      return $this->queryTypes[$facet_name];
    }
  }

  /**
   * Returns the human readable value associated with a facet's raw value.
   *
   * @param string $facet_name
   *   The machine readable name of the facet.
   * @param string $value
   *   The raw value passed through the query string.
   *
   * @return string
   *   The mapped value.
   */
  public function getMappedValue($facet_name, $value) {
    if (isset($this->processors[$facet_name])) {
      return $this->processors[$facet_name]->getMappedValue($value);
    }
    else {
      return array('#value' => $value);
    }
  }

  /**
   * Returns the processor associates with the facet.
   *
   * @param string $facet_name
   *   The machine readable name of the facet.
   *
   * @return FacetapiFacetProcessor
   */
  public function getProcessor($facet_name) {
    if (isset($this->processors[$facet_name])) {
      return $this->processors[$facet_name];
    }
    else {
      return FALSE;
    }
  }

  /**
   * Helper function that returns the query string variables for a facet item.
   *
   * @param array $facet
   *   The facet definition.
   * @param array $values
   *   An array containing the item's values being added to or removed from the
   *   query string dependent on whether or not the item is active.
   * @param int $active
   *   An integer flagging whether the item is active or not.
   *
   * @return array
   *   The query string vriables.
   */
  public function getQueryString(array $facet, array $values, $active) {
    return $this->urlProcessor->getQueryString($facet, $values, $active);
  }

  /**
   * Helper function that returns the facet path for a facet item.
   *
   * @param array $facet
   *   The facet definition.
   * @param array $values
   *   An array containing the item's values being added to or removed from the
   *   query string dependent on whether or not the item is active.
   * @param int $active
   *   An integer flagging whether the item is active or not.
   *
   * @return string
   *   The facet path.
   */
  public function getFacetPath(array $facet, array $values, $active) {
    return $this->urlProcessor->getFacetPath($facet, $values, $active);
  }

  /**
   * Initializes facet builds, adds breadcrumb trail.
   */
  public function processFacets() {
    if (!$this->processed) {
      $this->processed = TRUE;

      // Initializes each facet's render array.
      foreach ($this->getEnabledFacets() as $facet) {
        $processor = new FacetapiFacetProcessor($this->getFacet($facet));
        $this->processors[$facet['name']] = $processor;
        $this->processors[$facet['name']]->process();
      }

      // Sets the breadcrumb trail if a search was executed.
      if ($this->searchExecuted()) {
        $this->urlProcessor->setBreadcrumb();
      }
    }
  }

  /**
   * Builds the render array for facets in a realm.
   *
   * @param string $realm_name
   *   The machine readable name of the realm.
   *
   * @return array
   *   The render array.
   */
  public function buildRealm($realm_name) {
    // Bails if realm isn't valid.
    // @todo Call watchdog()?
    if (!$realm = facetapi_realm_load($realm_name)) {
      return array();
    }

    // Makes sure facet builds are initialized.
    $this->processFacets();

    // Adds JavaScript, initializes render array.
    drupal_add_js(drupal_get_path('module', 'facetapi') . '/facetapi.js');
    $build = array(
      '#adapter' => $this,
      '#realm' => $realm,
    );

    // Builds each facet in the realm, merges into realm's render array.
    foreach ($this->getEnabledFacets($realm['name']) as $facet) {
      // Continue to the next facet if this one failed its dependencies.
      if (!$this->dependenciesPassed[$facet['name']]) {
        continue;
      }

      // Gets the initialized build.
      $field_alias = $facet['field alias'];
      $processor = $this->processors[$facet['name']];
      $facet_build = $this->getFacet($facet)->build($realm, $processor);

      // Tries to be smart when merging the render arrays. Crazy things happen
      // when merging facets with the same field alias such as taxonomy terms in
      // the fieldset realm. We want to merge only the values.
      foreach (element_children($facet_build) as $child) {
        // Bails if there is nothing to render.
        if (!element_children($facet_build[$child])) {
          continue;
        }
        // Attempts to merge gracefully.
        if (!isset($build[$child])) {
          $build = array_merge_recursive($build, $facet_build);
        }
        else {
          if (isset($build[$child][$field_alias]) && isset($facet_build[$child][$field_alias])) {
            $build[$child][$field_alias] = array_merge_recursive(
              $build[$child][$field_alias],
              $facet_build[$child][$field_alias]
            );
          }
          elseif (isset($build[$child]['#options']) && isset($facet_build[$child]['#options'])) {
            $build[$child]['#options'] = array_merge_recursive(
              $build[$child]['#options'],
              $facet_build[$child]['#options']
            );
          }
          else {
            $build = array_merge_recursive($build, $facet_build);
          }
        }
      }
    }
    return $build;
  }
}

/**
 * Stores facet data, provides methods that build the facet's render array.
 */
class FacetapiFacet implements ArrayAccess {

  /**
   * The FacetapiAdapter object.
   *
   * @var FacetapiAdapter
   */
  protected $adapter;

  /**
   * The facet definition.
   *
   * @var array
   */
  protected $facet;

  /**
   * The build array for the facet items.
   *
   * @var array
   */
  protected $build = array();

  /**
   * Constructor, sets adapter and facet definition.
   *
   * @param $adapter
   *   A FacetapiAdapter object.
   * @param $facet
   *   An array containing the facet definition.
   */
  public function __construct(FacetapiAdapter $adapter, array $facet) {
    $this->adapter = $adapter;
    $this->facet = $facet;
  }

  /**
   * Whether a offset exists
   *
   * @param mixed offset
   *   An offset to check for.
   *
   * @return boolean
   */
  public function offsetExists($offset) {
    return isset($this->facet[$offset]);
  }

  /**
   * Returns the value at specified offset.
   *
   * @param mixed offset
   *   The offset to retrieve.
   *
   * @return mixed
   */
  public function offsetGet($offset) {
    return isset($this->facet[$offset]) ? $this->facet[$offset] : NULL;
  }

  /**
   * Assigns a value to the specified offset.
   *
   * @param mixed offset
   *   The offset to assign the value to.
   * @param mixed value
   *   The value to set.
   */
  public function offsetSet($offset, $value) {
    if (NULL === $offset) {
      $this->facet[] = $value;
    }
    else {
      $this->facet[$offset] = $value;
    }
  }

  /**
   * Unsets an offset.
   *
   * @param mixed offset
   *   The offset to unset.
   */
  public function offsetUnset($offset) {
    unset($this->facet[$offset]);
  }

  /**
   * Returns the adapter object.
   *
   * @return FacetapiAdapter
   *   The adapter object.
   */
  public function getAdapter() {
    return $this->adapter;
  }

  /**
   * Returns the facet definition.
   *
   * @return array
   *   An array containing the facet definition.
   */
  public function getFacet() {
    return $this->facet;
  }

  /**
   * Returns the facet definition.
   *
   * @return array
   *   An array containing the facet definition.
   */
  public function getBuild() {
    return $this->build;
  }

  /**
   * Gets facet setting for the passed realm.
   *
   * @param string|array $realm
   *   The machine readable name of the realm or realm definition. Pass null to
   *   get global settings.
   *
   * @return
   *   An object containing the settings.
   */
  public function getSettings($realm = NULL) {
    if ($realm && !is_array($realm)) {
      $realm = facetapi_realm_load($realm);
    }
    $method = ($realm) ? 'getFacetSettings' : 'getFacetSettingsGlobal';
    return $this->adapter->$method($this->facet, $realm);
  }

  /**
   * Returns the facet's render array.
   *
   * @param array $realm
   *   An array containing the realm definition.
   * @param FacetapiFacetProcessor $processor
   *   The processor object.
   *
   * @return
   *   The facet's build array.
   */
  public function build(array $realm, FacetapiFacetProcessor $processor) {
    $settings = $this->getSettings($realm);

    // Gets the base render array from the facet processor.
    $this->build = $processor->getBuild();

    // Executes filter plugins.
    // @todo Defensive coding here for filters?
    $enabled_filters = array_filter($settings->settings['filters'], 'facetapi_filter_disabled_filters');
    uasort($enabled_filters, 'facetapi_sort_weight');
    foreach ($enabled_filters as $filter_id => $filter_settings) {
      if ($class = ctools_plugin_load_class('facetapi', 'filters', $filter_id, 'handler')) {
        $filter_plugin = new $class($filter_id, $this->adapter, $settings);
        $this->build = $filter_plugin->execute($this->build);
      }
      else {
        watchdog('facetapi', 'Filter %name not valid.', array('%name' => $filter_id), WATCHDOG_ERROR);
      }
    }

    // Instantiates the widget plugin and initializes.
    // @todo Defensive coding here for widgets?
    $widget_name = $settings->settings['widget'];
    if (!$class = ctools_plugin_load_class('facetapi', 'widgets', $widget_name, 'handler')) {
      watchdog('facetapi', 'Widget %name not valid.', array('%name' => $widget_name), WATCHDOG_ERROR);
      return array();
    }
    $widget_plugin = new $class($widget_name, $realm, $this, $settings);
    $widget_plugin->init();

    if ($this->build) {
      // Executes widget plugin.
      $widget_plugin->execute();
      $build = $widget_plugin->getBuild();
    }
    else {
      // Instantiates empty behavior plugin.
      $id = $settings->settings['empty_behavior'];
      $class = ctools_plugin_load_class('facetapi', 'empty_behaviors', $id, 'handler');
      $empty_plugin = new $class($settings);
      // Executes empty behavior plugin.
      $build = $widget_plugin->getBuild();
      $build[$this['field alias']] = $empty_plugin->execute();
    }

    // If the element is empty, unset it.
    if (!$build[$this['field alias']]) {
      unset($build[$this['field alias']]);
    }

    // Adds JavaScript settings in a way that merges with others already set.
    $merge_settings['facetapi']['facets'][] = $widget_plugin->getJavaScriptSettings();
    drupal_add_js($merge_settings, 'setting');

    // Returns array keyed by the FacetapiWidget::$key property.
    return array($widget_plugin->getKey() => $build);
  }
}

/**
 * Processes facets, initializes the build.
 */
class FacetapiFacetProcessor {

  /**
   * An array of mapped values keyed by their raw value.
   *
   * @var $map
   */
  protected $map = array();

  /**
   * The facet being processed.
   *
   * @var FacetapiFacet
   */
  protected $facet;

  /**
   * The facet's initialized render array.
   *
   * @var array
   */
  protected $build = array();

  /**
   * Arrays of children keyed by their active parent's value.
   *
   * @var array
   */
  protected $activeChildren = array();

  /**
   * Constructor, initializes render array.
   *
   * @param FacetapiFacet $facet
   *   The facet being processed.
   *
   */
  public function __construct(FacetapiFacet $facet) {
    $this->facet = $facet;
  }

  /**
   * Processes the facet items.
   */
  public function process() {
    $this->build = array();

    // Only initializes facet if a query type plugin is registered for it.
    // NOTE: We don't use the chaining pattern so the methods can be tested.
    if ($this->facet->getAdapter()->getFacetQuery($this->facet->getFacet())) {
      $this->build = $this->initializeBuild($this->build);
      $this->build = $this->mapValues($this->build);
      if ($this->build) {
        $settings = $this->facet->getSettings();
        if (!$settings->settings['flatten']) {
          $this->build = $this->processHierarchy($this->build);
        }
        $this->processQueryStrings($this->build);
      }
    }
  }

  /**
   * Helper function to get the facet's active items.
   *
   * @return array
   *   The facet's active items.
   */
  public function getActiveItems() {
    return $this->facet->getAdapter()->getActiveItems($this->facet->getFacet());
  }

  /**
   * Gets an active item's children.
   *
   * @param string $value
   *   The value of the active item.
   *
   * @return array
   *   The active item's childen.
   */
  public function getActiveChildren($value) {
    return (isset($this->activeChildren[$value])) ? $this->activeChildren[$value] : array();
  }

  /**
   * Gets the initialized render array.
   */
  public function getBuild() {
    return $this->build;
  }

  /**
   * Returns the human readable value associated with a raw value.
   *
   * @param string $value
   *   The raw value passed through the query string.
   *
   * @return string
   *   The mapped value.
   */
  public function getMappedValue($value) {
    return (isset($this->map[$value])) ? $this->map[$value] : array('#value' => $value);
  }

  /**
   * Initializes the facet's render array.
   *
   * @return array
   *   The initialized render array.
   */
  protected function initializeBuild() {
    $build = array();

    // Build array defaults.
    $defaults = array(
      '#value' => '',
      '#path' => $this->facet->getAdapter()->getSearchPath(),
      '#html' => FALSE,
      '#indexed_value' => '',
      '#count' => 0,
      '#active' => 0,
      '#item_parents' => array(),
      '#item_children' => array(),
    );

    // Builds render arrays for each item.
    $adapter = $this->facet->getAdapter();
    $build = $adapter->getFacetQuery($this->facet->getFacet())->build();

    // Invoke the alter callbacks for the facet.
    foreach ($this->facet['alter callbacks'] as $callback) {
      $callback($build, $adapter, $this->facet->getFacet());
    }

    // Iterates over the render array and merges in defaults.
    foreach (element_children($build) as $value) {
      $item_defaults = array(
        '#value' => $value,
        '#indexed_value' => $value,
        '#active' => $adapter->itemActive($this->facet['name'], $value),
      );
      $build[$value] = array_merge($defaults, $item_defaults, $build[$value]);
    }

    return $build;
  }

  /**
   * Maps the IDs to human readable values via the mapping callback.
   *
   * @param array $build
   *   The initialized render array.
   *
   * @return array
   *   The initialized render array with mapped values.
   */
  protected function mapValues(array $build) {
    if ($this->facet['map callback']) {
      // Gets available items and active items, runs through map callback only
      // when there are values to map.
      // NOTE: array_merge() doesn't work here when the values are numeric.
      if ($values = array_unique(array_keys($build + $this->getActiveItems()))) {
        $this->map = call_user_func($this->facet['map callback'], $values, $this->facet['map options']);
        // Normalize all mapped values to a two element array.
        foreach ($this->map as $key => $value) {
          if (!is_array($value)) {
            $this->map[$key] = array();
            $this->map[$key]['#value'] = $value;
            $this->map[$key]['#html'] = FALSE;
          }
          if (isset($build[$key])) {
            $build[$key]['#value'] = $this->map[$key]['#value'];
            $build[$key]['#html'] = !empty($this->map[$key]['#html']);
          }
        }
      }
    }
    return $build;
  }

  /**
   * Processes hierarchical relationships between the facet items.
   *
   * @param array $build
   *   The initialized render array.
   *
   * @return array
   *   The initialized render array with processed hierarchical relationships.
   */
  protected function processHierarchy(array $build) {

    // Builds the hierarchy information if the hierarchy callback is defined.
    if ($this->facet['hierarchy callback']) {
      $parents = $this->facet['hierarchy callback'](array_keys($build));
      foreach ($parents as $value => $parents) {
        foreach ($parents as $parent) {
          if (isset($build[$parent]) && isset($build[$value])) {
            // Use a reference so we see the updated data.
            $build[$parent]['#item_children'][$value] = &$build[$value];
            $build[$value]['#item_parents'][$parent] = $parent;
          }
        }
      }
    }

    // Tests whether parents have an active child.
    // @todo: Can we make this more efficient?
    do {
      $active = 0;
      foreach ($build as $value => $item) {
        if ($item['#active'] && !empty($item['#item_parents'])) {
          // @todo Can we build facets with multiple parents? Core taxonomy
          // form cannot, so we will need a check here.
          foreach ($item['#item_parents'] as $parent) {
            if (!$build[$parent]['#active']) {
              $active = $build[$parent]['#active'] = 1;
            }
          }
        }
      }
    } while ($active);

    // Since the children are copied to their parent's "#item_parents" property
    // during processing, we have to filter the original child items from the
    // top level of the hierarchy.
    return array_filter($build, 'facetapi_filter_top_level_children');
  }

  /**
   * Initializes the render array's query string variables.
   *
   * @param array &$build
   *   The initialized render array.
   */
  protected function processQueryStrings(array &$build) {
    foreach ($build as $value => &$item) {
      $values = array($value);
      // Calculate paths for the children.
      if (!empty($item['#item_children'])) {
        $this->processQueryStrings($item['#item_children']);
        // Merges the childrens' values if the item is active so the children
        // are deactivated along with the parent.
        if ($item['#active']) {
          $values = array_merge(facetapi_get_child_values($item['#item_children']), $values);
        }
      }
      // Stores this item's active children so we can deactivate them in the
      // current search block as well.
      $this->activeChildren[$value] = $values;

      // Formats path and query string for facet item, sets theme function.
      $item['#path'] = $this->getFacetPath($values, $item['#active']);
      $item['#query'] = $this->getQueryString($values, $item['#active']);
    }
  }

  /**
   * Helper function that returns the path for a facet item.
   *
   * @param array $values
   *   An array containing the item's values being added to or removed from the
   *   query string dependent on whether or not the item is active.
   * @param int $active
   *   An integer flagging whether the item is active or not.
   *
   * @return
   *   The facet path.
   */
  public function getFacetPath(array $values, $active) {
    return $this->facet
      ->getAdapter()
      ->getFacetPath($this->facet->getFacet(), $values, $active);
  }

  /**
   * Helper function that returns the query string variables for a facet item.
   *
   * @param array $values
   *   An array containing the item's values being added to or removed from the
   *   query string dependent on whether or not the item is active.
   * @param int $active
   *   An integer flagging whether the item is active or not.
   *
   * @return
   *   An array containing the query string variables.
   */
  public function getQueryString(array $values, $active) {
    return $this->facet
      ->getAdapter()
      ->getQueryString($this->facet->getFacet(), $values, $active);
  }
}

/**
 * Recursive function that returns an array of values for all descendants of a
 * facet item.
 *
 * @param $build
 *   A render array containing the facet item's children.
 *
 * @return
 *   An array containing the values of all descendants.
 */
function facetapi_get_child_values(array $build) {
  $values = array_keys($build);
  foreach ($build as $item) {
    if (!empty($item['#item_children'])) {
      $values = array_merge(facetapi_get_child_values($item['#item_children']), $values);
    }
  }
  return $values;
}

/**
 * Callback for array_filter() that strips child items at the top level.
 *
 * When hierarchies are processed, all children are copied to their parent's
 * "#item_children" property to establish the relationship. This callback
 * filters the original child items from the top level of the hierarchy so the
 * aren't also displayed along-side their parents.
 *
 * @param $build
 *   The facet item's render array.
 *
 * @return
 *   A boolean flagging whether the value should remain in the array.
 */
function facetapi_filter_top_level_children(array $build) {
  return empty($build['#item_parents']);
}

/**
 * Callback for array_filter() that strips out disabled filters.
 *
 * @param array $settings
 *   The individual filter settings.
 *
 * @return
 *   A boolean flagging whether the value should remain in the array.
 */
function facetapi_filter_disabled_filters($settings) {
  return !empty($settings['status']);
}
