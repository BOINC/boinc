<?php
/**
 * Field handler to show if user is admin or not.
 *
 * @ingroup views_field_handlers
 */
class og_views_handler_field_og_is_admin extends og_views_handler_field_og_uid {
  function render($values) {
    // If the base table of the view is "users", use the user being returned
    // by the view. If its a node view, use the currently logged in user.
    switch($this->view->base_table) {
      case 'users':
        if (isset($values->{$this->field_alias})) {
          $is_admin = $values->{$this->field_alias};
        }
        break;
      case 'node':
        global $user;
        if (!empty($user->og_groups[$values->{$this->aliases['nid']}]['is_admin'])) {
          $is_admin = $user->og_groups[$values->{$this->aliases['nid']}]['is_admin'];
        }
        break;
    }

    // don't use og_is_group_admin() here since we don't want mark on 'admin nodes' people.
    if ($is_admin) {
      return t('admin');
    }
  }
}
