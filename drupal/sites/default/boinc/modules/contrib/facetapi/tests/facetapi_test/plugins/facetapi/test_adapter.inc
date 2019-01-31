<?php

/**
 * Facet API adapter for the test module.
 */
class FacetapiTestAdapter extends FacetapiAdapter {

  /**
   * The total number of results in the query.
   */
  protected $resultCount;

  /**
   * Returns a boolean flagging whether $this->searcher executed a search.
   */
  public function searchExecuted() {
    return !empty($this->keys);
  }

  /**
   * Returns a boolean flagging whether facets in a realm shoud be displayed.
   */
  public function suppressOutput($realm_name) {

  }

  /**
   * Sets the result count.
   */
  public function setResultCount($count) {
    $this->resultCount = $count;
    return $this;
  }

  /**
   * Returns the nmber of total results found for the current search.
   */
  public function getResultCount() {
    return $this->resultCount;
  }
}