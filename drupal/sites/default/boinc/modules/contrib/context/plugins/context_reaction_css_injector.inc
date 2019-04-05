<?php

class context_reaction_css_injector extends context_reaction {

  function options_form($context) {
    $list = array();
    foreach (_css_injector_load_rule() as $css_rule) {
      $list[$css_rule['crid']] = $css_rule['title'];
    }
    ksort($list);

    return array(
      '#title' => $this->title,
      '#description' => $this->description,
      '#options' => $list,
      '#type' => 'checkboxes',
      '#default_value' => $this->fetch_from_context($context),
    );
  }

  function execute() {
    $contexts = $this->get_contexts();
    foreach ($contexts as $context) {
      if (!empty($context->reactions[$this->plugin])) {
        foreach ($context->reactions[$this->plugin] as $crid) {
          if ($css_rule = _css_injector_load_rule($crid)) {
            drupal_add_css(file_create_path($css_rule['file_path']), 'module', $css_rule['media'], $css_rule['preprocess']);
          }
        }
      }
    }
  }
}