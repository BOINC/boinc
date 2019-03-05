<?php

/**
 * @file
 * Hooks provided by the Facet API module.
 */

/**
 * @addtogroup hooks
 * @{
 */

/**
 * Define all searchers provided by the module.
 *
 * Searchers are synonymous with search pages, or environments. Each searcher is
 * associated with an adapter instance. Multiple searchers can share the same
 * adapter class, but each searcher will spawn a separate instance of the
 * adapter. Each searcher must be unique, so it is common practice to prefix the
 * name with the module implementing the hook, such as "apachesolr@searcher-1",
 * "apachesolr@searcher-2", "search_api@searcher-1", etc.
 *
 * @return array
 *   An associative array keyed by unique name of the searcher. Each searcher is
 *   an associative array containing:
 *   - label: The human readable name of the searcher displayed in the admin UI.
 *   - adapter: The adapter plugin ID associated with the searcher.
 *   - url processor: (optional) The URL processor plugin ID associated with the
 *     searcher. Defaults to "standard".
 *   - types: (optional) An array containing the types of content indexed by the
 *     searcher. A type is usually an entity such as 'node', but it can contain
 *     non-entities as well. Defaults to array('node').
 *   - path: (optional) The MENU_DEFAULT_LOCAL_TASK item which the admin UI page
 *     is added to as a MENU_LOCAL_TASK. An empty string if the backend manages
 *     the admin UI menu items internally. Defaults to an empty string meaning
 *     the backend is responsible for adding the admin UI menu items.
 *   - supports facet missing: (optional) TRUE if the searcher supports
 *     "missing" facets. Defaults to FALSE.
 *   - supports facet mincount: (optional) TRUE if the searcher supports the
 *     minimum facet count setting. Defaults to FALSE.
 *   - include default facets: (optional) TRUE if the searcher should include
 *     the facets defined in facetapi_facetapi_facet_info() when indexing node
 *     content, FALSE if they should be skipped.
 */
function hook_facetapi_searcher_info() {
  return array(
    'search' => array(
      'label' => t('Search'),
      'adapter' => 'search',
      'url processor' => 'standard',
      'types' => array('node'),
      'path' => 'admin/settings/search',
      'supports facet missing' => TRUE,
      'supports facet mincount' => TRUE,
      'include default facets' => TRUE,
    ),
  );
}

/**
 * Allows for alterations to the searcher definitions.
 *
 * @param array &$searcher_info
 *   The return values of hook_facetapi_searcher_info() implementations.
 *
 * @see hook_facetapi_searcher_info()
 */
function hook_facetapi_searcher_info_alter(array &$searcher_info) {
  $searcher_info['search']['label'] = t('Core search module');
}

/**
 * Define all realms provided by the module.
 *
 * A realm is a group of facets that are rendered in a similar fashion. For
 * example, the "block" realm displays each facet in a separate block, whereas
 * the "fieldset" realm displays facets as form elements in a fieldset under the
 * search form.
 *
 * @return array
 *   An associative array keyed by unique name of the realm. Each realm is an
 *   associative array containing:
 *   - label: The human readable name of the realm displayed in the admin UI.
 *   - description: The description of the realm displayed in the admin UI.
 *   - element type: The type of element facets are rendered as, such as "links"
 *     or "form elements".
 *   - default widget: The default widget plugin id for facets.
 *   - settings callback: (optional) A callback that alters the realm settings
 *     form, defaults to FALSE meaning no callback is defined.
 *   - sortable: (optional) Whether the facets can be sorted via the admin UI,
 *     defaults to TRUE.
 *   - weight: (optional) The weight of the realm's menu item in comparison to
 *     the others, defaults to 0.
 */
function hook_facetapi_realm_info() {
  return array(
    'block' => array(
      'label' => t('Blocks'),
      'sortable' => FALSE,
      'weight' => -10,
      'default widget' => 'facetapi_links',
      'element type' => 'links',
      'settings callback' => 'facetapi_block_realm_settings',
      'description' => t(
        'The <em>Blocks</em> realm displays each facet in a separate <a href="@block-page">block</a>. Users are able to refine their searches in a drill-down fashion.',
        array('@block-page' => url('admin/build/block', array('query' => array('destination' => $_GET['q']))))
      ),
    ),
  );
}

/**
 * Allows for alterations to the realm definitions.
 *
 * @param array &$realm_info
 *   The return values of hook_facetapi_realm_info() implementations.
 *
 * @see hook_facetapi_realm_info()
 */
function hook_facetapi_realm_info_alter(array &$realm_info) {
  $realm_info['block']['weight'] = 5;
}

/**
 * Define all facets provided by the module.
 *
 * @param array $searcher_info
 *   The definition of the searcher that facets are being collected for.
 *
 * @return array
 *   An associative array keyed by unique name of the facet. Each facet is an
 *   associative array containing:
 *   - name: Machine readable name of the facet.
 *   - label: Human readable name of the facet displayed in settings forms.
 *   - description: Description of the facet displayed in settings forms.
 *   - field: The field name used by the backend to store and retrieve data from
 *     the search index it is associated with. Defaults to the machine name of
 *     the facet.
 *   - field alias: The query string variable inside of the filter key used to
 *     pass the filter information through the query string. Defaults to the
 *     machine name of the facet.
 *   - field api name: (optional) The machine readable name of the Field API
 *     field data the facet is associated with, FALSE if it is not associated
 *     with a field.
 *   - field api bundles: (optional) An array of entity names that this field
 *     contains bundle information for. Defaults to an empty array.
 *   - query types: The query type plugins that that this facet supports. For
 *     example, numeric fields support "term" and "range_filter" queries.
 *   - alter callbacks: (optional) Callbacks that alter the initialized render
 *     array returned by the query type plugin. Defaults to an empty array.
 *   - dependency plugins: (optional) An array of dependency plugin IDs that are
 *     supported by this facet.
 *   - default widget: (optional) The widget plugin ID used if no plugin has
 *     been selected or the one selected is not valid. Defaults to FALSE which
 *     sets the default widget as the one defined by the realm.
 *   - allowed operators: (optional) An array keyed by operator constant to
 *     boolean values specifying whether the operator is supported. Defaults to
 *     an array containing:
 *     - FACETAPI_OPERATOR_AND: TRUE
 *   - facet missing allowed: (optional) Whether or not missing facets are
 *     allowed for this facet. Defaults to FALSE.
 *   - facet mincount allowed: (optional)  Whether or not the facet supports the
 *     "minimum facet count" setting. Defaults to FALSE.
 *   - weight: (optional) The weight of the facet. Defaults to 0.
 *   - map callback: (optional) The callback used to map the raw values returned
 *     by the index to something human readable. Defaults to FALSE
 *   - map options: (optional) An array of options passed to the map callback,
 *     defaults to en empty array.
 *   - hierarchy callback: (optional) A callback that maps the parent / child
 *     relationships of the facet data, defaults to FALSE meaning the list is
 *     flat.
 *   - values callback: (optional) In instances where facet data is not returned
 *     by the backend, provide a list of values that can be used. Defaults to
 *     FALSE.
 *   - min callback: (optional) For facets containing ranges, a callback
 *     returning the minimum value in the index. Defaults to FALSE.
 *   - max callback: (optional) For facets containing ranges, a callback
 *     returning the maximum value in the index. Defaults to FALSE.
 *   - default sorts: (optional) An array of available sorts. Each item is an
 *     array containing two values, the first being the item being filtered on,
 *     the second being the SORT_* constant. Defaults to an array containing:
 *     - active: SORT_DESC
 *     - count: SORT_DESC
 *     - display: SORT_ASC
 */
function hook_facetapi_facet_info(array $searcher_info) {
  $facets = array();

  // Facets are usually associated with the type of content stored in the index.
  if (isset($searcher_info['types']['my_type'])) {

    $facets['my_field'] = array(
      'name' => 'my_field',
      'label' => t('My field'),
      'description' => t('My field index some content we can facet by.'),
      'field' => 'my_field_index_field_name',
      'field alias' => 'my_alias',
      'field api name' => FALSE,
      'field api bundles' => array(),
      'query types' => array('term', 'date'),
      'dependency plugins' => array('role'),
      'default widget' => 'links',
      'allowed operators' => array(FACETAPI_OPERATOR_AND => TRUE, FACETAPI_OPERATOR_OR => TRUE),
      'facet missing allowed' => FALSE,
      'facet mincount allowed' => FALSE,
      'weight' => 0,
      'map callback' => 'mymodule_map_my_field',
      'map options' => array('field_option_1', 'field_option_2'),
      'hierarchy callback' => FALSE,
      'values callback' => FALSE,
      'min callback' => FALSE,
      'max callback' => FALSE,
      'default sorts' => array(
        array('active', SORT_DESC),
        array('count', SORT_DESC),
        array('display', SORT_ASC),
      ),
    );
  }

  return $facets;
}

/**
 * Allows for alterations to the facet definitions.
 *
 * @param array &$facet_info
 *   The return values of hook_facetapi_facet_info() implementations.
 * @param array $searcher_info
 *   The definition of the searcher that facets are being collected for.
 *
 * @see hook_facetapi_facet_info()
 */
function hook_facetapi_facet_info_alter(array &$facet_info, array $searcher_info) {
  // Change the author index field for Apache Solr searchers indexing node data.
  if ('apachesolr' == $searcher_info['adapter'] && isset($searcher_info['types']['node'])) {
    $facet_info['author']['field'] = 'is_uid';
  }
}

/**
 * Define all facets sorting algorithms provided by the module.
 *
 * @return array
 *   An associative array keyed by unique name of the sort. Each sort is an
 *   associative array containing:
 *   - title: The human readable name of the sort displayed in the admin UI.
 *   - callback: The uasort() callback the render array is passed to.
 *   - description: The description of the sort displayed in the admin UI.
 *   - weight: (optional) The default weight of the sort specifying its
 *     default processing order. Defaults to 0.
 */
function hook_facetapi_sort_info() {
  $sorts = array();

  $sorts['active'] = array(
    'label' => t('Facet active'),
    'callback' => 'facetapi_sort_active',
    'description' => t('Sort by whether the facet is active or not.'),
    'weight' => -50,
  );

  return $sorts;
}

/**
 * Allows for alterations to the sort definitions.
 *
 * @param array &$sort_info
 *   The return values of hook_facetapi_sort_info() implementations.
 *
 * @see hook_facetapi_sort_info()
 */
function hook_facetapi_sort_info_alter(array &$sort_info) {
  $sort_info['active']['weight'] = 10;
}

/**
 * Define all adapter plugins provided by the module.
 *
 * Adapters are the abstraction layer through which searchers integrate with
 * Facet API and various backend functionality.
 *
 * @return array
 *   An associative array keyed by unique name of the adapter. Each adapter is
 *   an associative array keyed by "handler" containing:
 *   - class: The name of the plugin class.
 *
 * @see FacetapiApachesolr
 */
function hook_facetapi_adapters() {
  return array(
    'apachesolr' => array(
      'handler' => array(
        'class' => 'FacetapiApachesolrFacetapiAdapter',
      ),
    ),
  );
}

/**
 * Define all dependency plugins provided by the module.
 *
 * Dependency plugins control what criteria must be met for the backend to
 * process the facet. It is designed to implement the "progressive disclosure"
 * pattern where more facets are displayed to users the deeper they get into
 * the refinement of their search.
 *
 * @return array
 *   An associative array keyed by unique name of the dependency. Each
 *   dependency is an associative array keyed by "handler" containing:
 *   - label: The human readable name of the plugin displayed in the admin UI.
 *   - class: The name of the plugin class.
 *
 * @see FacetapiDependency
 */
function hook_facetapi_dependencies() {
  return array(
    'bundle' => array(
      'handler' => array(
        'label' => t('Content types'),
        'class' => 'FacetapiDependencyBundle',
      ),
    ),
  );
}

/**
 * Define all empty behavior plugins provided by the module.
 *
 * Empty behavior plugins determine what to display when a facet has no
 * available items.
 *
 * @return array
 *   An associative array keyed by unique name of the empty behavior. Each empty
 *   behavior is an associative array keyed by "handler" containing:
 *   - label: The human readable name of the plugin displayed in the admin UI.
 *   - class: The name of the plugin class.
 *
 * @see FacetapiEmptyBehavior
 */
function hook_facetapi_empty_behaviors() {
  return array(
    'none' => array(
      'handler' => array(
        'label' => t('Do not display facet'),
        'class' => 'FacetapiEmptyBehaviorNone',
      ),
    ),
  );
}

/**
 * Define all filter plugins provided by the module.
 *
 * Filter plugins provide last minute modifications to the facet's render array
 * prior to being acted on by the widget plugin. Filters are also an alter
 * mechanism for the facet that developers can use to make any customizations
 * prior to the widget being rendered.
 *
 * @return array
 *   An associative array keyed by unique name of the filter. Each filter is an
 *   associative array keyed by "handler" containing:
 *   - label: The human readable name of the plugin displayed in the admin UI.
 *   - class: The name of the plugin class.
 *
 * @see FacetapiFilter
 */
function hook_facetapi_filters() {
  return array(
    'active_items' => array(
      'handler' => array(
        'label' => t('Do not display active items'),
        'class' => 'FacetapiFilterActiveItems',
      ),
    ),
  );
}

/**
 * Define all query type plugins provided by the module.
 *
 * Query type plugins are responsible for converting the active facet items into
 * facet queries that are processed by the backend. They are also responsible
 * for extracting extra information about the active item, such as the start and
 * end values of a range query.
 *
 * @return array
 *   An associative array keyed by unique name of the query type. Each query
 *   type is an associative array keyed by "handler" containing:
 *   - class: The name of the plugin class.
 *   - adapter: The adapter that the query type plugin is associated with.
 *
 * @see FacetapiQueryTypeInterface
 * @see FacetapiQueryType
 */
function hook_facetapi_query_types() {
  return array(
    'apachesolr_term' => array(
      'handler' => array(
        'class' => 'FacetapiApachesolrTerm',
        'adapter' => 'apachesolr',
      ),
    ),
  );
}

/**
 * Define all URL processor plugins provided by the module.
 *
 * URL processors are responsible for building and formatting facet URLs. The
 * standard processor passes all facet filters through the "f" query string
 * variable.
 *
 * @return array
 *   An associative array keyed by unique name of the URL processor. Each URL
 *   processor is an associative array keyed by "handler" containing:
 *   - label: The human readable name of the plugin displayed in the admin UI.
 *   - class: The name of the plugin class.
 *
 * @see FacetapiFilter
 */
function hook_facetapi_url_processors() {
  return array(
    'standard' => array(
      'handler' => array(
        'label' => t('Standard URL processors'),
        'class' => 'FacetapiUrlProcessorStandard',
      ),
    ),
  );
}

/**
 * Define all widget plugins provided by the module.
 *
 * Widget plugins process the facet render arrays to the structure that wille be
 * passed to drupal_render(), which in turn converts the facet to HTML.
 *
 * @return array
 *   An associative array keyed by unique name of the widget. Each widget is an
 *   associative array keyed by "handler" containing:
 *   - label: The human readable name of the plugin displayed in the admin UI.
 *   - class: The name of the plugin class.
 *   - query types: An array of query-types that this widget is compatible with
 *
 * @see FacetapiFilter
 */
function hook_facetapi_widgets() {
  return array(
    'facetapi_links' => array(
      'handler' => array(
        'label' => t('Links'),
        'class' => 'FacetapiWidgetLinks',
        'query types' => array('term', 'date'),
      ),
    ),
  );
}

/**
 * Forces delta mapping of a facet block.
 *
 * This obscure hook is useful for cases where facets are disabled, but their
 * block positioning needs to be set anyways. If a facet is enabled via the
 * facetapi_set_facet_enabled() API function, its block needs to be enabled
 * and assigned to a region despite the facet not being enabled in the Facet API
 * interface, which would normally prevent the block from being listed.
 *
 * @return array
 *   An associative array keyed by searcher. Each sub array is an associative
 *   array keyed by realm name to facet names whose delta mappings are forced.
 */
function hook_facetapi_force_delta_mapping() {
  return array(
    // The machine-readable name of the searcher.
    'my_searcher' => array(
      // The realm we are mapping, usually block.
      'block' => array(
        // Machine readable names of facets whose mappping are being forced.
        // Regardless of whether they are enabled via the Facet API interface,
        // their blocks will be available to enable and position via
        // admin/build/block.
        'facet_one',
        'facet_two',
      ),
    ),
  );
}

/**
 * Implemented by the translator module to translate a string.
 *
 * This hook is invoked by the facetapi_translate_string() function. The
 * "facetapi:translator_module" variable stores which translator module is
 * active since it wouldn't make sense to have multiple translator modules.
 *
 * @param $name
 *   The name of the string in "textgroup:object_type:object_key:property_name"
 *   format.
 * @param $string
 *   The string being translated.
 * @param $langcode
 *   The language code to translate to a language other than what is used to
 *   display the page. Defaults to NULL, which uses the current language.
 *
 * @return
 *   The translated string.
 *
 * @see facetapi_translate_string()
 */
function hook_facetapi_translate_string($name, $string, $langcode = NULL) {
  // In this instance, the translator module integrates with the i18n project.
  return i18n_string($name, $string, array('langcode' => $langcode));
}

/**
 * @} End of "addtogroup hooks".
 */
