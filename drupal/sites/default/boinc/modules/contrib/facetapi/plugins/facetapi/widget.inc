<?php

/**
 * @file
 * Abstract widget plugin class.
 */

/**
 * Base class for widget plugins.
 */
abstract class FacetapiWidget {

  /**
   * The machine readable name of the widget.
   *
   * @var string
   */
  protected $id;

  /**
   * The realm definition.
   *
   * @var array
   */
  protected $realm;

  /**
   * The facet object.
   *
   * @var FacetapiFacet
   */
  protected $facet;

  /**
   * The facet settings.
   *
   * @var stdClass
   */
  protected $settings;

  /**
   * The normalized render array.
   *
   * @var array
   */
  protected $build = array();

  /**
   * The key of the facet's render array added to the realm's render array.
   *
   * @var string
   */
  protected $key;

  /**
   * JavaScript settings.
   *
   * @var array
   */
  protected $jsSettings = array();

  /**
   * @param string $id
   *   The machine readable name of the widget.
   * @param array $realm
   *   The realm being rendered.
   * @param array $settings
   *   The realm settings.
   * @param FacetapiFacet $facet
   *   The facet object.
   */
  public function __construct($id, array $realm, FacetapiFacet $facet, stdClass $settings) {
    // Stores variables.
    $this->id = $id;
    $this->realm = $realm;
    $this->settings = $settings;
    $this->settings->settings += $this->getDefaultSettings();
    $this->facet = $facet;

    // Sets the key of the element when appended to the realm's render array.
    $this->key = $facet['field alias'];
  }

  /**
   * Initializes the build, must be invoked prior to executing this widget.
   *
   * @return FacetapiWidget
   *   An instance of this class.
   */
  public function init() {

    // Captures searcher for code readability.
    $searcher = $this->facet->getAdapter()->getSearcher();

    $classes = implode(' ', array("facetapi-" . $this->id, "facetapi-facet-{$this->facet['name']}"));

    // Initializes render array.
    $this->build = array(
      '#title' => $this->facet['label'],
      '#description' => $this->facet['description'],
      '#weight' => $this->facet['weight'],
      '#adapter' => $this->facet->getAdapter(),
      '#realm_name' => $this->realm['name'],
      '#facet' => $this->facet->getFacet(),
      '#settings' => $this->settings,
      $this->facet['field alias'] => $this->facet->getBuild(),
      '#attributes' => array(
        'class' => $classes,
        'id' => "facetapi-facet-$searcher-{$this->realm['name']}-{$this->facet['name']}",
      ),
    );

    // Applies sorting algorithms to the render array.
    $this->sortFacet($this->build);

    // Initializes JavaScript settings.
    $this->jsSettings += array(
      'id' => $this->build['#attributes']['id'],
      'searcher' => $searcher,
      'realmName' => $this->realm['name'],
      'facetName' => $this->facet['name'],
      'queryType' => $this->facet['query type'],
      'widget' => $this->settings->settings['widget'],
    );

    // Add our accessibility CSS
    drupal_add_css(drupal_get_path('module', 'facetapi') . '/facetapi.css');

    return $this;
  }

  /**
   * Abstract public function.
   */
  abstract public function execute();

  /**
   * Allows the widget to provide additional settings to the form.
   */
  public function settingsForm(&$form, &$form_state) {
    // Nothing to do ...
  }

  /**
   * Returns an array of default settings
   */
  public function getDefaultSettings() {
    return array();
  }

  /**
   * Returns the machine readable name of the widget.
   *
   * @return string
   *   The machine readable name of the widget.
   */
  public function getId() {
    return $this->id;
  }

  /**
   * Returns the render array.
   *
   * @return array
   *   The render array.
   */
  public function getBuild() {
    return $this->build;
  }

  /**
   * Returns the element's key value.
   *
   * @return string
   *   The element's key.
   */
  public function getKey() {
    return $this->key;
  }

  /**
   * Returns the JavaScript settings.
   *
   * @return array
   *   The JavaScript settings.
   */
  public function getJavaScriptSettings() {
    return $this->jsSettings;
  }

  /**
   * Applies selected sorting algorithms to the render array.
   *
   * @param array &$build
   *   The facet's render array.
   */
  function sortFacet(array &$build) {
    $settings = $build['#settings']->settings;

    // Gets active sort definitions.
    $this->sorts = array_intersect_key(
      facetapi_get_sort_info(),
      array_filter($settings['active_sorts'])
    );

    // Finalizes sort definitions with settings or defaults.
    foreach ($this->sorts as $name => &$info) {
      $info['weight'] = $settings['sort_weight'][$name];
      $info['order'] = $settings['sort_order'][$name];
    }

    if ($this->sorts) {
      // Orders the sorts, applies sorting algorithms in that order.
      uasort($this->sorts, 'facetapi_sort_weight');
      $this->applySorts($build[$this->facet['field alias']]);
    }
  }

  /**
   * Sorts the facet's build array.
   *
   * @param array &$build
   *   Reference to the render array, allows us to sort one hierarchical level
   *   at a time.
   */
  protected function applySorts(&$build) {
    foreach (element_children($build) as $value) {
      if (!empty($build[$value]['#item_children'])) {
        $this->applySorts($build[$value]['#item_children']);
      }
    }
    uasort($build, array($this, 'sortCallback'));
  }

  /**
   * Generic sort callback, useful as a callback to uasort().
   *
   * Applies sorts in the order they are specified in the settings.
   */
  protected function sortCallback(array $a, array $b) {
    $return = 0;
    foreach ($this->sorts as $sort) {
      if ($return = $sort['callback']($a, $b)) {
        if (SORT_DESC == $sort['order']) {
          $return *= -1;
        }
        break;
      }
    }
    return $return;
  }
}

/**
 * Sorts by whether or not a facet is active.
 */
function facetapi_sort_active(array $a, array $b) {
  $a_active = (isset($a['#active'])) ? $a['#active'] : 0;
  $b_active = (isset($b['#active'])) ? $b['#active'] : 0;
  if ($a_active == $b_active) {
    return 0;
  }
  return ($a_active < $b_active) ? -1 : 1;
}

/**
 * Sorts by facet count.
 */
function facetapi_sort_count(array $a, array $b) {
  $a_count = (isset($a['#count'])) ? $a['#count'] : 0;
  $b_count = (isset($b['#count'])) ? $b['#count'] : 0;
  if ($a_count == $b_count) {
    return 0;
  }
  return ($a_count < $b_count) ? -1 : 1;
}

/**
 * Sorts by raw indexed value.
 */
function facetapi_sort_indexed(array $a, array $b) {
  $a_value = (isset($a['#indexed_value'])) ? $a['#indexed_value'] : '';
  $b_value = (isset($b['#indexed_value'])) ? $b['#indexed_value'] : '';
  if ($a_value == $b_value) {
    return 0;
  }
  return ($a_value < $b_value) ? -1 : 1;
}

/**
 * Sorts by display value.
 */
function facetapi_sort_display(array $a, array $b) {
  $a_count = (isset($a['#value'])) ? $a['#value'] : '';
  $b_count = (isset($b['#value'])) ? $b['#value'] : '';
  return strcasecmp($a['#value'], $b['#value']);
}
