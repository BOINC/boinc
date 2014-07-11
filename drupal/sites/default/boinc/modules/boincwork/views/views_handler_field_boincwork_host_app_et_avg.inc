<?php

/*
 * views_handler_field_boincwork_host_app_et_avg: This handler converts
 * et_avg to gigaflops.
 */

class views_handler_field_boincwork_host_app_et_avg extends views_handler_field {
  function construct() {
    parent::construct();
  }

  function render($values) {
    $gflops = round(1e-9 / $values->host_app_version_et_avg, 2);
    return "{$gflops} GFLOPS";
  }
}
