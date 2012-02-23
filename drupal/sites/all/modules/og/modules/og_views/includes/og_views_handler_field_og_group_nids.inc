<?php
class og_views_handler_field_og_group_nids extends views_handler_field_prerender_list {
  function init(&$view, $options) {
    parent::init($view, $options);
    $this->additional_fields['og_ancestry_nid'] = array('table' => 'og_ancestry', 'field' => 'nid');
  }
  
  /**
   * Add this term to the query
   */
  function query() {
    $this->add_additional_fields();
  }
  
  // we have to query to get all the groups into a single element
  function pre_render($values) {
    // field_alias is used by our render method.
    $this->field_alias = $this->aliases['og_ancestry_nid'];
    foreach ($values as $value) {
      $result = og_get_node_groups_result($value->nid);
      while ($row = db_fetch_object($result)) {
        $this->items[$value->nid][$row->group_nid] = l($row->title, "node/$row->group_nid");
      }
    }
  }
}