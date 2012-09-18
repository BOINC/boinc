<?php
/**
 * Field handler to show 'invite' link.
 *
 * @ingroup views_field_handlers
 */
class og_views_handler_field_og_invite extends views_handler_field {
  function construct() {
    parent::construct();
    $this->additional_fields['og_selective'] = 'og_selective';
    $this->additional_fields['nid'] = array('table' => 'node', 'field' => 'nid');
  }

  function query() {
    $this->ensure_my_table();
    $this->add_additional_fields();
  }

  function option_definition() {
    $options = parent::option_definition();
    $options['text'] = array('default' => '', 'translatable' => TRUE);
    return $options;
  }

  function options_form(&$form, &$form_state) {
    parent::options_form($form, $form_state);
    $form['text'] = array(
      '#type' => 'textfield',
      '#title' => t('Text to display'),
      '#default_value' => $this->options['text'],
    );
  }

  function render($values) {
    global $user;
    if (in_array($values->{$this->aliases['nid']}, array_keys($user->og_groups))) {
      $text = !empty($this->options['text']) ? $this->options['text'] : t('Invite');
      switch ((int)$values->{$this->aliases['og_selective']}) {
        case OG_CLOSED:
          return;
        case OG_INVITE_ONLY:
          $node = node_load((int) $values->{$this->aliases['nid']});
          if (og_is_group_admin($node)) {
            return l($text, 'og/invite/' . $node->nid, array('attributes' => array('rel' => 'nofollow'), 'query' => drupal_get_destination()));
          }
          return;
        default:
          return l($text, 'og/invite/' . $values->{$this->aliases['nid']}, array('attributes' => array('rel' => 'nofollow'), 'query' => drupal_get_destination()));
      }
    }
  }
}

