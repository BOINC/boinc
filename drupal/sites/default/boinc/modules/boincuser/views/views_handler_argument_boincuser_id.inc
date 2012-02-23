<?php

/*
 * views_handler_argument_boincuser_id: This handler is used to convert
 * the Drupal user ID passed as a URL argument into a BOINC user ID
 * for accessing data in the BOINC database.
 */

class views_handler_argument_boincuser_id extends views_handler_argument_numeric {
  function construct() {
    parent::construct();
  }

  function set_argument($arg) {
    // When setting the ID argument, convert to BOINC ID
    $id = is_numeric($arg) ? $arg : 0;
    $boinc_id = db_result(db_query("SELECT boinc_id FROM {boincuser} WHERE uid = %d", $id));
    $this->argument = $boinc_id;
    return $this->validate_arg($boinc_id);
  }
}
