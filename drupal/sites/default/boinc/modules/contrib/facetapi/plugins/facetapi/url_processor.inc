<?php

/**
 * @file
 * Adapter and standard plugin class.
 */

/**
 *
 */
abstract class FacetapiUrlProcessor {

  /**
   * The adapter.
   *
   * @var FacetapiAdapter
   */
  protected $adapter;

  /**
   * An array of facet params.
   *
   * @var array.
   */
  protected $params = array();

  /**
   * The key containing the facet filters.
   *
   * @var string
   */
  protected $filterKey = 'f';

  /**
   *
   */
  public function __construct(FacetapiAdapter $adapter) {
    $this->adapter = $adapter;
  }

  /**
   * Automatically fetches the params when the plugin is instantiated.
   *
   * @return array
   *   An array containing the params.
   */
  abstract public function fetchParams();

  /**
   * Normalizes the array returned by FacetapiAdapter::fetchParams().
   *
   * @param array $params
   *   An array of keyed params, such as $_GET.
   * @param string $filter_key
   *   The array key in $params corresponding to filters.
   *
   * @return array
   *   An array containing the normalized params.
   */
  abstract public function normalizeParams(array $params, $filter_key = 'f');

  /**
   * Returns the query string variables for a facet item.
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
  abstract public function getQueryString(array $facet, array $values, $active);

  /**
   * Returns the path for a facet item.
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
   *   The path of the facet.
   */
  public function getFacetPath(array $facet, array $values, $active) {
    return $this->adapter->getSearchPath();
  }

  /**
   * Sets the breadcrumb trail on searches.
   */
  abstract public function setBreadcrumb();

  /**
   * Sets the parameters.
   *
   * @param array $params
   *   An array of normalized params.
   * @param string $filter_key
   *   The array key in $params corresponding to filters.
   *
   * @return FacetapiUrlParser
   *   An instance of this class.
   */
  public function setParams(array $params, $filter_key = 'f') {
    $this->params = $params;
    $this->filterKey = $filter_key;
    if (!isset($this->params[$this->filterKey]) || !is_array($this->params[$this->filterKey])) {
      $this->params[$this->filterKey] = array();
    }
    return $this;
  }

  /**
   * Returns the params.
   *
   * @return array
   *   An array containing the params.
   */
  public function getParams() {
    return $this->params;
  }

  /**
   * Removes an item from the $this->params array.
   */
  public function removeParam($pos) {
    unset($this->params[$this->filterKey][$pos]);
  }

  /**
   * Returns the filter key.
   *
   * @return string
   *   A string containing the filter key.
   */
  public function getFilterKey() {
    return $this->filterKey;
  }
}
