<?php
/**
 * Field handler to allow show 'join' link or empty if already member.
 *
 * @ingroup views_field_handlers
 */
class og_views_handler_field_og_subscribe extends views_handler_field {
  function construct() {
    parent::construct();
    $this->additional_fields['og_selective'] = 'og_selective';
  }

  function query() {
    $this->ensure_my_table();
    $this->add_additional_fields();
  }

  function render($values) {
    global $user;
    if (!in_array($values->nid, array_keys($user->og_groups))) {
      switch ((int)$values->{$this->aliases['og_selective']}) {
        case OG_CLOSED:
          return '<em>'. t('Closed'). '</em>';
        case OG_INVITE_ONLY:
          return '<em>'. t('Invite only'). '</em>';
        default:
          return og_subscribe_link(node_load((int)$values->nid));
      }
    }
  }
}