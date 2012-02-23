<?php
class og_views_handler_filter_og_is_admin extends views_handler_filter_in_operator {
  function get_value_options() {
    if (!isset($this->value_options)) {
      $this->value_title = t('Member types');
      $options = array(t('Members'), t('Group administrators'));
      $this->value_options = $options;
    }
  }
}