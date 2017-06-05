<?php
/**
 * Filter by node_access records.
 */
class views_handler_filter_node_access extends views_handler_filter {
  function admin_summary() { }
  function operator_form(&$form, &$form_state) { }
  function can_expose() {
    return FALSE;
  }

  /**
   * See _node_access_where_sql() for a non-views query based implementation.
   */
  function query() {
    if (!user_access('administer nodes')) {
      $table = $this->ensure_my_table();
      $grants = array();
      foreach (node_access_grants('view') as $realm => $gids) {
        foreach ($gids as $gid) {
          $grants[] = "($table.gid = $gid AND $table.realm = '$realm')";
        }
      }
      $grants_sql = '';
      if (count($grants)) {
        $grants_sql = implode(' OR ', $grants);
      }
      $this->query->add_where('AND', $grants_sql);
      $this->query->add_where('AND', "$table.grant_view >= 1");
    }
  }
}
