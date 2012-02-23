<?php
/**
 * Field handler to add/remove an admin.
 *
 * @ingroup views_field_handlers
 */
class og_views_handler_field_og_managelinkadmin extends og_views_handler_field_og_uid {
  function construct() {
    parent::construct();
    $this->additional_fields['nid'] = array('table' => 'og_uid', 'field' => 'nid');
    $this->additional_fields['is_admin'] = array('table' => 'og_uid', 'field' => 'is_admin');
  }

  function render($values) {
    if (isset($values->{$this->field_alias})) {
      $uid = $values->{$this->field_alias};
      $nid = $values->{$this->aliases['nid']};
      $is_admin = $values->{$this->aliases['is_admin']};
      $node = node_load($nid);
      // Only show links to group admins. And don't show next to group manager.
      if ($uid != $node->uid) {
        if ($is_admin) {
          $item = menu_get_item("og/delete_admin/$nid/$uid");
          if ($item['access']) {
            return l(t('Admin: Remove'), "og/delete_admin/$nid/$uid", array('query' => drupal_get_destination()));
          }
        }
        else {
          $item = menu_get_item("og/create_admin/$nid/$uid");
          if ($item['access']) {
            return l(t('Admin: Create'), "og/create_admin/$nid/$uid", array('query' => drupal_get_destination()));
          }
        }
      }
    }
  }
}
