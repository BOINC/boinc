<?php
/**
 * We do not try to run db_rewrite_sql() on this subquery so the count is potentially more than the user can see.
 */
class og_views_handler_field_og_post_count_new extends views_handler_field_numeric {
  function query() {
    global $user;
    $table = $this->query->ensure_table('og');
    $sql = "SELECT COUNT(*) FROM {node} n INNER JOIN {og_ancestry} oga ON n.nid = oga.nid LEFT JOIN {history} h ON n.nid=h.nid AND h.uid = %uid WHERE n.status = 1 AND oga.group_nid = $table.nid AND h.timestamp IS NULL AND n.created > %created";
    $sql = strtr($sql, array('%uid' => $user->uid, '%created' => NODE_NEW_LIMIT));
    $this->query->add_field('', "($sql)", 'post_count_new');
    $this->field_alias = 'post_count_new';
  }
  
  function render($values) {
    if ($cnt = $values->{$this->field_alias}) {
      return $cnt. theme('mark', MARK_NEW);
    }
  }
}