<?php

/*
 * views_handler_field_boincwork_host_app_turnaround_avg: This handler converts
 * a number of seconds to days.
 */

class views_handler_field_boincwork_host_app_turnaround_avg extends views_handler_field {
  function construct() {
    parent::construct();
  }

  function render($values) {
    $time_in_days = $values->host_app_version_turnaround_avg / (24*60*60);
    return round($time_in_days, 2) . ' ' . t('days');
  }
}
