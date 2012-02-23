<?php
// $Id: views_handler_filter_node_type.inc,v 1.2 2009/06/02 17:57:48 merlinofchaos Exp $
/**
 * Filter by node type
 */
class views_handler_filter_node_type extends views_handler_filter_in_operator {
  function get_value_options() {
    if (!isset($this->value_options)) {
      $this->value_title = t('Node type');
      $types = node_get_types();
      foreach ($types as $type => $info) {
        $options[$type] = t($info->name);
      }
      $this->value_options = $options;
    }
  }
}
