<?php
/**
 * Field handler to approve/deny a subscription request, or remove a member.
 *
 * @ingroup views_field_handlers
 */
class og_views_handler_field_og_managelink extends og_views_handler_field_og_uid {
  function construct() {
    parent::construct();
    $this->additional_fields['nid'] = array('table' => 'og_uid', 'field' => 'nid');
    $this->additional_fields['is_active'] = array('table' => 'og_uid', 'field' => 'is_active');
  }

  function render($values) {
    if (isset($values->{$this->field_alias})) {
      $uid = $values->{$this->field_alias};
      $nid = $values->{$this->aliases['nid']};
      $is_active = $values->{$this->aliases['is_active']};
      $node = node_load($nid);
      // Only show links to group admins. And don't show next to group manager.
      if ($uid != $node->uid) {
        if ($is_active) {
          $item = menu_get_item("og/unsubscribe/$nid/$uid");
          if ($item['access']) {
            return l(t('Remove membership'), "og/unsubscribe/$nid/$uid", array('query' => drupal_get_destination()));
          }
        }
        else {
          $token = og_get_token($nid);
          $item = menu_get_item("og/approve/$nid/$uid/$token");
          if ($item['access']) {
            return t('Request: <a href="@approve">approve</a> or <a href="@deny">deny</a>.', array('@approve' => url("og/approve/$nid/$uid/$token", array('query' => drupal_get_destination())), '@deny' => url("og/deny/$nid/$uid/$token", array('query' => drupal_get_destination()))));
          }
        }
      }
    }
  }
}
