<?php
// $Id: context_reaction_debug.inc,v 1.1.2.3 2010/08/04 23:43:04 yhahn Exp $

/**
 * Output context debug information.
 */
class context_reaction_debug extends context_reaction {
  function options_form($context) {
    return array('debug' => array('#type' => 'value', '#value' => TRUE));
  }

  function options_form_submit($values) {
    return array('debug' => 1);
  }

  /**
   * Output a list of active contexts.
   */
  function execute() {
    $contexts = context_active_contexts();
    foreach ($contexts as $context) {
      if (!empty($context->reactions['debug'])) {
        if (user_access('administer site configuration') && module_exists('context_ui')) {
          $name = l($context->name, "admin/build/context/list/{$context->name}", array('query' => 'destination=' . $_GET['q']));
        }
        else {
          $name = check_plain($context->name);
        }
        drupal_set_message(t("Active context: !name", array('!name' => $name)));
      }
    }
  }
}
