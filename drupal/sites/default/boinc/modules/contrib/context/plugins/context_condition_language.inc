<?php

/**
 * Expose current language as a context condition.
 */
class context_condition_language extends context_condition {
  function condition_values() {
    return module_exists('locale') ? locale_language_list() : array();
  }

  function execute($value) {
    foreach ($this->get_contexts($value) as $context) {
      $this->condition_met($context, $value);
    }
  }
}
