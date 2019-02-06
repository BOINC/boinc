<?php

/**
 * @file
 * The standard URL processor.
 */

/**
 * Extension of FacetapiUrlProcessor.
 */
class FacetapiUrlProcessorStandard extends FacetapiUrlProcessor {

  /**
   * Implements FacetapiUrlProcessor::fetchParams().
   *
   * Pulls facet params from the $_GET variable.
   */
  public function fetchParams() {
    return $_GET;
  }

  public function fetchQueryParams(array $query = NULL, array $exclude = array('q'), $parent = '') {
    // Set defaults, if none given.
    if (!isset($query)) {
      $query = $_GET;
    }
    // If $exclude is empty, there is nothing to filter.
    if (empty($exclude)) {
      return $query;
    }
    elseif (!$parent) {
      $exclude = array_flip($exclude);
    }

    $params = array();
    foreach ($query as $key => $value) {
      $string_key = ($parent ? $parent . '[' . $key . ']' : $key);
      if (isset($exclude[$string_key])) {
        continue;
      }

      if (is_array($value)) {
        $params[$key] = $this->fetchQueryParams($value, $exclude, $string_key);
      }
      else {
        $params[$key] = $value;
      }
    }

    return $params;
  }

  /**
   * Implements FacetapiUrlProcessor::normalizeParams().
   *
   * Strips the "q" and "page" variables from the params array.
   */
  public function normalizeParams(array $params, $filter_key = 'f') {
    return $this->fetchQueryParams($params, array('q', 'page'));
  }

  /**
   * Implements FacetapiUrlProcessor::getQueryString().
   */
  public function getQueryString(array $facet, array $values, $active) {
    $qstring = $this->params;
    $active_items = $this->adapter->getActiveItems($facet);

    // Appends to qstring if inactive, removes if active.
    foreach ($values as $value) {
      if ($active && isset($active_items[$value])) {
        unset($qstring[$this->filterKey][$active_items[$value]['pos']]);
      }
      elseif (!$active) {
        $field_alias = rawurlencode($facet['field alias']);
        $qstring[$this->filterKey][] = $field_alias . ':' . $value;
      }
    }

    // Removes duplicates, resets array keys and returns query string.
    // @see http://drupal.org/node/1340528
    $qstring[$this->filterKey] = array_values(array_unique($qstring[$this->filterKey]));
    return array_filter($qstring);
  }

  /**
   * Implements FacetapiUrlProcessor::setBreadcrumb().
   */
  public function setBreadcrumb() {
    $breadcrumb = drupal_get_breadcrumb();

    // Gets search keys and active items form the adapter.
    $keys = $this->adapter->getSearchKeys();
    $active_items = $this->adapter->getAllActiveItems();

    $item = menu_get_item();
    $last_load_func = is_array($item['load_functions']) ? end($item['load_functions']) : NULL;
    if (!empty($item['title']) && ((!$keys && $active_items) || ($keys && $last_load_func != 'menu_tail_load'))) {
      $last = end($breadcrumb);
      $this_page = l($item['title'], $item['href'], $item['localized_options']);
      if ($last != $this_page) {
        $breadcrumb[] = $this_page;
      }
    }

    // Initializes base breadcrumb query.
    $query = $this->params;
    unset($query[$this->filterKey]);

    // Adds the current search to the query.
    if ($keys) {
      // The last item should be text, not a link.
      $breadcrumb[] = $active_items ? l($keys, $_GET['q'], array('query' => $query)) : check_plain($keys);
    }

    // Adds filters to the breadcrumb trail.
    $last = end($active_items);
    foreach ($active_items as $item) {
      $query[$this->filterKey][] = rawurlencode($item['field alias']) . ':' . $item['value'];

      // Replaces with the mapped value.
      $value = $this->adapter->getMappedValue($item['facets'][0], $item['value']);

      // The last item should be text, not a link.
      if ($last == $item) {
        $breadcrumb[] = !empty($value['#html']) ? $value['#value'] : check_plain($value['#value']);
      }
      else {
        // Appends the filter to the breadcrumb trail.
        $breadcrumb[] = l($value['#value'], $_GET['q'], array('query' => $query, 'html' => !empty($value['#html'])));
      }
    }

    // Sets the breadcrumb trail with h keys and filters.
    drupal_set_breadcrumb($breadcrumb);
  }
}
