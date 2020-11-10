<?php

/**
 * Plugin for "term" query types.
 */
class FacetapiTestTerm extends FacetapiQueryType implements FacetapiQueryTypeInterface {

  static public function getType() {
    return 'term';
  }

  public function execute($query) {
    // Nothing to do...
  }

  public function build() {
    $build = array();

    $build['testone'] = array('#count' => 1);
    $build['testtwo'] = array('#count' => 2);
    $build['testthree'] = array('#count' => 3);

    // Returns initialized build.
    return $build;
  }
}

