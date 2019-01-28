<?php

/**
 * @file
 * Interfaces and base classes for query type plugins.
 */

/**
 * Interface that must be implemented by all query type plugins.
 */
interface FacetapiQueryTypeInterface {

  /**
   * Returns the query type associated with the plugin.
   *
   * @return string
   *   The query type.
   */
  static public function getType();

  /**
   * Adds the filter to the query object.
   *
   * @param stdClass $query
   *   An object containing the query in the backend's native API.
   */
  public function execute($query);

  /**
   * Initializes the facet's render array.
   *
   * @return
   *   The initialized render array.
   */
  public function build();
}

/**
 * Base class for query type plugins.
 */
class FacetapiQueryType {

  /**
   * The adapter object.
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
   *
   * @param FacetapiAdapter $adapter
   *   The adapter object.
   */
  public function __construct(FacetapiAdapter $adapter, array $facet) {
    $this->adapter = $adapter;
    $this->facet = $facet;
  }

  /**
   * Allows additional information to be added to the active items.
   *
   * A common use case is extracting the start and end values in a range facet.
   *
   * @param array $value
   *   The active item.
   *
   * @return array
   *   An array containing the addition information to add to the item.
   */
  public function extract(array $item) {
    return array();
  }

  /**
   * Returns settings.
   *
   * @param string|array $realm
   *   The machine readable name of the realm or realm definition. Pass null to
   *   get global settings.
   *
   * @return
   *   An object containing the settings.
   */
  public function getSettings($realm = NULL) {
    return $this->adapter->getFacet($this->facet)->getSettings($realm);
  }

  /**
   * Returns the facet's active items.
   *
   * @return array
   *   The facet's active items.
   */
  public function getActiveItems() {
    return $this->adapter->getActiveItems($this->facet);
  }
}

/**
 * Base class for date query type plugins.
 */
class FacetapiQueryTypeDate extends FacetapiQueryType {

  /**
   * Loads the include file containing the date API functions.
   */
  public function __construct(FacetapiAdapter $adapter, array $facet) {
    module_load_include('inc', 'facetapi', 'facetapi.date');
    parent::__construct($adapter, $facet);
  }

  /**
   * Parses the start and end values for the date.
   */
  public function extract(array $item) {
    $return = array();
    if (preg_match(FACETAPI_REGEX_DATE_RANGE, $item['value'], $matches)) {
      $return['start'] = $matches[1];
      $return['end'] = $matches[8];
    }
    return $return;
  }
}

/**
 * Base class for range query type plugins.
 */
class FacetapiQueryTypeRange extends FacetapiQueryType {

  /**
   * Parses the start and end values for the date.
   */
  public function extract(array $item) {
    $return = array();
    if (preg_match(FACETAPI_REGEX_RANGE, $item['value'], $matches)) {
      $return['start'] = $matches[1];
      $return['end'] = $matches[2];
    }
    return $return;
  }
}
