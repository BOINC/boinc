<?php

/**
 * Expose active contexts as a context condition.
 */
class context_condition_context extends context_condition_path {
  function execute() {
    if ($this->condition_used()) {
      $active_contexts = array_keys(context_active_contexts());
      foreach ($this->get_contexts() as $context) {
        if (!in_array($context->name, $active_contexts, TRUE) && $values = $this->fetch_from_context($context, 'values')) {
          if ($this->match($active_contexts, $values)) {
            $this->condition_met($context);
          }
        }
      }
      // If the list of active contexts has changed, we need to recurse.
      if ($active_contexts != array_keys(context_active_contexts())) {
        $this->execute();
      }
    }
  }
}
