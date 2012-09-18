<?php
class og_views_handler_filter_og_uid_nid extends views_handler_filter_many_to_one {
  function get_value_options() {
    $this->value_options = og_all_groups_options();
  }
}
