<?php
/**
 * Field handler to show if user is admin or not.
 *
 * @ingroup views_field_handlers
 */
class og_views_handler_field_og_is_admin extends og_views_handler_field_og_uid {
  function render($values) {
    if (isset($values->{$this->field_alias})) {
      $is_admin = $values->{$this->field_alias};
      // don't use og_is_group_admin() here since we don't want mark on 'admin nodes' people.
      if ($is_admin) {
        return t('admin'); 
      }
    }
  }
}
