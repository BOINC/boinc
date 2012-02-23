<?php
/**
 * Field handler to show if user is manager or not of the displayed group node.
 *
 * @ingroup views_field_handlers
 */
class og_views_handler_field_og_is_manager extends og_views_handler_field_og_uid {
  function construct() {
    parent::construct();
    $this->additional_fields['nid'] = array('table' => 'og_uid', 'field' => 'nid');
  }

  function render($values) {
    if (isset($values->{$this->field_alias})) {
      $node = node_load($values->{$this->aliases['nid']});
      $manager = $node->uid;
      $member = $values->{$this->field_alias};
      if ($member == $manager) {
        return '<em>'. t('Manager'). '</em>'; 
      }
    }
  }
}
