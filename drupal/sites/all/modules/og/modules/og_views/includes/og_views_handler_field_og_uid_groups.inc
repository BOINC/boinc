<?php
/**
 * Field handler for displaying a list of groups for a user.
 */
class og_views_handler_field_og_uid_groups extends views_handler_field_prerender_list {
  /**
   * Fake the field alias -- we don't want to actually join.
   */
  function init(&$view, $options) {
    parent::init($view, $options);
    switch ($view->base_table) {
      case 'node':
        $this->additional_fields['users_uid'] = array('table' => 'users', 'field' => 'uid');
        $this->field_alias = 'users_uid';
        break;
      case 'users':
        $this->field_alias = 'uid';
        break;
    }
  }

  /**
   * Add this term to the query
   */
  function query() {
    $this->add_additional_fields();
  }

  /**
   * Query in pre_render to grab what we need.
   */
  function pre_render($values) {
    $uids = array();
    foreach ($values as $row) {
      if (!empty($row->{$this->field_alias})) {
        $uids[] = $row->{$this->field_alias};
      }
    }
    if (!empty($uids)) {
      $placeholders = db_placeholders($uids, 'int');
      $result = db_query(db_rewrite_sql("SELECT n.nid, n.title, ogu.uid FROM {node} n JOIN {og_uid} ogu ON ogu.nid = n.nid WHERE ogu.uid IN ($placeholders) AND n.status = 1"), $uids);
      while ($row = db_fetch_object($result)) {
        $this->items[$row->uid][$row->nid] = l($row->title, "node/{$row->nid}");
      }
    }
  }
}
