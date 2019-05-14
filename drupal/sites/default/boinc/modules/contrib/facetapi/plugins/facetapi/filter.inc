<?php

/**
 * @file
 * Base filter class and implementation.
 */

/**
 *
 */
abstract class FacetapiFilter {

  /**
   * The ID of this plugin.
   */
  protected $id;

  /**
   * The adapter object.
   *
   * @var FacetapiAdapter
   */
  protected $adapter;

  /**
   * The facet settings.
   *
   * @var stdClass
   */
  protected $settings;

  /**
   * Initializes the filter object.
   *
   * @param array $adapter
   *   The adapter object
   * @param array $realm
   *   The realm being rendered.
   * @param array $settings
   *   The realm settings.
   * @param FacetapiFacet $facet
   *   The facet object.
   */
  public function __construct($id, FacetapiAdapter $adapter, stdClass $settings) {
    $this->id = $id;
    $this->adapter = $adapter;
    $this->settings = $settings;
    $this->settings->settings += $this->getDefaultSettings();
  }

  /**
   * Filters the facet items.
   *
   * @param array $build
   *   The facet's render array.
   *
   * @return array
   *   The altered build array.
   */
  abstract public function execute(array $build);

  /**
   * Adds settings to the filter form.
   */
  public function settingsForm(&$form, &$form_state) {
    // Nothing to do.
  }

  /**
   * Returns default settings added to the settings form.
   */
  public function getDefaultSettings() {
    return array();
  }
}

/**
 * Plugin that filters active items.
 */
class FacetapiFilterActiveItems extends FacetapiFilter {

  /**
   * Filters active facet items.
   */
  public function execute(array $build) {
    return array_filter($build, 'facetapi_filter_active');
  }
}

/**
 * Callback for array_filter() that strips out active items.
 *
 * @param $build
 *   The facet item's render array.
 *
 * @return
 *   A boolean flagging whether the value should remain in the array.
 */
function facetapi_filter_active(array $build) {
  return empty($build['#active']);
}

/**
 * Plugin that filters all items not in lowest hierarchy.
 */
class FacetapiFilterCurrentDepth extends FacetapiFilter {

  /**
   * Filters active facet items.
   */
  public function execute(array $build) {
    foreach ($build as $item) {
      if ($item['#active'] && !empty($item['#item_children'])) {
        return $this->execute($item['#item_children']);
      }
    }
    return $build;
  }
}
