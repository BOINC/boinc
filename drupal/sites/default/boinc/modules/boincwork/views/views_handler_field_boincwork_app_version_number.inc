<?php

/*
 * views_handler_field_boincwork_app_version_number: This handler converts a
 * application version number to decimal form.
 */

class views_handler_field_boincwork_app_version_number extends views_handler_field {
  function construct() {
    parent::construct();
  }

  function render($values) {
    return sprintf("%01.2f", $values->app_version_host_app_version_version_num / 100);
  }
}
