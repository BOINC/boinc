<?php
// $Id: views_handler_filter_node_status.inc,v 1.2 2009/06/03 00:26:39 merlinofchaos Exp $
/**
 * Filter by published status
 */
class views_handler_filter_node_status extends views_handler_filter {
  function admin_summary() { }
  function operator_form() { }

  function query() {
    $table = $this->ensure_my_table();
    $this->query->add_where($this->options['group'], "$table.status <> 0 OR ($table.uid = ***CURRENT_USER*** AND ***CURRENT_USER*** <> 0) OR ***ADMINISTER_NODES*** = 1");
  }
}
