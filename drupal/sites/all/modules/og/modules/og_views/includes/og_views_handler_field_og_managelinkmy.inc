<?php
class og_views_handler_field_og_managelinkmy extends og_views_handler_field_og_uid {
  function option_definition() {
    $options = parent::option_definition();
    $options['text'] = array('default' => 'Edit membership', 'translatable' => TRUE);
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

  function construct() {
    parent::construct();
    $this->additional_fields['nid'] = array('table' => 'og_uid', 'field' => 'nid');
  }

  function render($values) {
    if (isset($values->{$this->field_alias})) {
      $nid = $values->{$this->aliases['nid']};
      global $user;
      if (in_array($nid, array_keys($user->og_groups))) {
        $text = !empty($this->options['text']) ? $this->options['text'] : t('Edit membership');
        return l($text, "og/manage/$nid", array('query' => drupal_get_destination()));
      }
    }
  }
}
