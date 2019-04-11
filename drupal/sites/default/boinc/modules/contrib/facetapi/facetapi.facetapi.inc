<?php

/**
 * @file
 * Facet API hook implementations.
 */

/**
 * Implements hook_facetapi_realm_info().
 */
function facetapi_facetapi_realm_info() {
  $realms = array();

  $realms['block'] = array(
    'label' => t('Blocks'),
    'weight' => -10,
    'sortable' => FALSE,
    'default widget' => 'facetapi_links',
    'element type' => 'links',
    'description' => t(
      'The <em>Blocks</em> realm displays each facet in a separate <a href="@block-page">block</a>. Users are able to refine their searches in a drill-down fashion.',
      array('@block-page' => url('admin/build/block', array('query' => array('destination' => $_GET['q']))))
    ),
  );

  return $realms;
}

/**
 * Implements hook_facetapi_sort_info().
 */
function facetapi_facetapi_sort_info() {
  $sorts = array();

  $sorts['active'] = array(
    'label' => t('Facet active'),
    'callback' => 'facetapi_sort_active',
    'description' => t('Sort by whether the facet is active or not.'),
    'weight' => -50,
  );

  $sorts['count'] = array(
    'label' => t('Count'),
    'callback' => 'facetapi_sort_count',
    'description' => t('Sort by the facet count.'),
    'weight' => -49,
  );

  $sorts['display'] = array(
    'label' => t('Display value'),
    'callback' => 'facetapi_sort_display',
    'description' => t('Sort by the value displayed to the user.'),
    'weight' => -48,
  );

  $sorts['indexed'] = array(
    'label' => t('Indexed value'),
    'callback' => 'facetapi_sort_indexed',
    'description' => t('Sort by the raw value stored in the index.'),
    'weight' => -47,
  );

  return $sorts;
}

/**
 * Implements hook_facetapi_widgets().
 */
function facetapi_facetapi_adapters() {
  $path = drupal_get_path('module', 'facetapi') . '/plugins/facetapi/';
  return array(
    'adapter' => array(
      'handler' => array(
        'label' => t('Abstract class for adapters'),
        'class' => 'FacetapiAdapter',
        'abstract' => TRUE,
        'path' => $path,
        'file' => 'adapter.inc',
      ),
    ),
  );
}

/**
 * Implements hook_facetapi_widgets().
 */
//TODO: why other naming convention?
function facetapi_facetapi_widgets() {
  $path = drupal_get_path('module', 'facetapi') . '/plugins/facetapi';
  return array(
    'facetapi_widget' => array(
      'handler' => array(
        'label' => t('Abstract class for widgets'),
        'class' => 'FacetapiWidget',
        'abstract' => TRUE,
        'path' => $path,
        'file' => 'widget.inc',
        'query types' => array('term', 'date'),
      ),
    ),
    'facetapi_links' => array(
      'handler' => array(
        'label' => t('Links'),
        'class' => 'FacetapiWidgetLinks',
        'parent' => 'facetapi_widget',
        'path' => $path,
        'file' => 'widget_links.inc',
        'query types' => array('term', 'date'),
      ),
    ),
    'facetapi_checkbox_links' => array(
      'handler' => array(
        'label' => t('Links with checkboxes'),
        'class' => 'FacetapiWidgetCheckboxLinks',
        'parent' => 'facetapi_widget',
        'path' => $path,
        'file' => 'widget_links.inc',
        'query types' => array('term', 'date'),
      ),
    ),
  );
}

/**
 * Implements hook_facetapi_filters().
 */
function facetapi_facetapi_filters() {
  $path = drupal_get_path('module', 'facetapi') . '/plugins/facetapi';
  return array(
    'filter' => array(
      'handler' => array(
        'label' => t('Abstract classs for filters'),
        'class' => 'FacetapiFilter',
        'abstract' => TRUE,
        'path' => $path,
        'file' => 'filter.inc',
      ),
    ),
    'active_items' => array(
      'handler' => array(
        'label' => t('Do not display active items'),
        'class' => 'FacetapiFilterActiveItems',
        'parent' => 'filter',
        'path' => $path,
        'file' => 'filter.inc', //not really needed since parent file will be included
      ),
    ),
    'current_depth' => array(
      'handler' => array(
        'label' => t('Only show items in the current level of the hierarchy'),
        'class' => 'FacetapiFilterCurrentDepth',
        'parent' => 'filter',
        'path' => $path,
        'file' => 'filter.inc', //not really needed since parent file will be included
        'requirements' => array('facetapi_requirement_facet_hierarchical' => TRUE),
      ),
    ),
  );
}

function facetapi_facetapi_query_types() {
  $path = drupal_get_path('module', 'facetapi') . '/plugins/facetapi';
  return array(
    'query_type' => array(
      'handler' => array(
        'label' => t('Abstract classs for query types'),
        'class' => 'FacetapiQueryType',
        'abstract' => TRUE,
        'path' => $path,
        'file' => 'query_type.inc',
      ),
    ),
    'range' => array(
      'handler' => array(
        'label' => t('Range'),
        'class' => 'FacetapiQueryTypeRange',
        'path' => $path,
        'file' => 'query_type.inc', //not really needed since parent file will be included
      ),
    ),
    'date' => array(
      'handler' => array(
        'label' => t('Date'),
        'class' => 'FacetapiQueryTypeDate',
        'path' => $path,
        'file' => 'query_type.inc', //not really needed since parent file will be included
      ),
    ),
  );
}

/**
 * Implements hook_facetapi_dependencies().
 */
function facetapi_facetapi_dependencies() {
  $path = drupal_get_path('module', 'facetapi') . '/plugins/facetapi';
  return array(
    'dependency' => array(
      'handler' => array(
        'label' => t('Abstract class for dependencies'),
        'class' => 'FacetapiDependency',
        'abstract' => TRUE,
        'path' => $path,
        'file' => 'dependency.inc',
      ),
    ),
    'bundle' => array(
      'handler' => array(
        'label' => t('Bundles'),
        'class' => 'FacetapiDependencyBundle',
        'parent' => 'dependency',
        'path' => $path,
        'file' => 'dependency_bundle.inc',
      ),
    ),
    'role' => array(
      'handler' => array(
        'label' => t('Roles'),
        'class' => 'FacetapiDependencyRole',
        'parent' => 'dependency',
        'path' => $path,
        'file' => 'dependency_role.inc',
      ),
    ),
  );
}

/**
 * Implements hook_facetapi_empty_behaviors().
 */
function facetapi_facetapi_empty_behaviors() {
  $path = drupal_get_path('module', 'facetapi') . '/plugins/facetapi';
  return array(
    'empty_behavior' => array(
      'handler' => array(
        'label' => t('Abstract class for empty behaviors'),
        'class' => 'FacetapiEmptyBehavior',
        'abstract' => TRUE,
        'path' => $path,
        'file' => 'empty_behavior.inc',
      ),
    ),
    'none' => array(
      'handler' => array(
        'label' => t('Do not display facet'),
        'class' => 'FacetapiEmptyBehaviorNone',
        'parent' => 'empty_behavior',
        'path' => $path,
        'file' => 'empty_behavior.inc', //not really needed since parent file will be included
      ),
    ),
    'text' => array(
      'handler' => array(
        'label' => t('Display text'),
        'class' => 'FacetapiEmptyBehaviorText',
        'parent' => 'empty_behavior',
        'path' => $path,
        'file' => 'empty_behavior_text.inc',
      ),
    ),
  );
}

/**
 * Implements hook_facetapi_url_processors().
 */
function facetapi_facetapi_url_processors() {
  $path = drupal_get_path('module', 'facetapi') . '/plugins/facetapi';
  return array(
    'url_processor' => array(
      'handler' => array(
        'label' => t('Abstract URL processor'),
        'class' => 'FacetapiUrlProcessor',
        'abstract' => TRUE,
        'path' => $path,
        'file' => 'url_processor.inc',
      ),
    ),
    'standard' => array(
      'handler' => array(
        'label' => t('Standard URL processor'),
        'class' => 'FacetapiUrlProcessorStandard',
        'parent' => 'url_processor',
        'path' => $path,
        'file' => 'url_processor_standard.inc',
      ),
    ),
  );
}
