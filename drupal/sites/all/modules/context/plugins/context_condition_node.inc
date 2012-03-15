<?php
// $Id: context_condition_node.inc,v 1.1.2.4 2010/08/03 03:25:23 yhahn Exp $

/**
 * Trigger context on node view only.
 */
define('CONTEXT_NODE_VIEW', 0);

/**
 * Trigger context on node view and node form.
 */
define('CONTEXT_NODE_FORM', 1);

/**
 * Trigger context on node form only.
 */
define('CONTEXT_NODE_FORM_ONLY', 2);

/**
 * Expose node views/node forms of specific node types as a context condition.
 */
class context_condition_node extends context_condition {
  function condition_values() {
    $values = array();
    foreach (node_get_types() as $type) {
      $values[$type->type] = check_plain($type->name);
    }
    return $values;
  }

  function options_form($context) {
    $defaults = $this->fetch_from_context($context, 'options');
    return array(
      'node_form' => array(
        '#title' => t('Set on node form'),
        '#type' => 'select',
        '#options' => array(
           CONTEXT_NODE_VIEW => t('No'),
           CONTEXT_NODE_FORM => t('Yes'),
           CONTEXT_NODE_FORM_ONLY => t('Only on node form')
        ),
        '#description' => t('Set this context on node forms'),
        '#default_value' => isset($defaults['node_form']) ? $defaults['node_form'] : TRUE,
      ),
    );
  }

  function execute($node, $op) {
    foreach ($this->get_contexts($node->type) as $context) {
      // Check the node form option.
      $options = $this->fetch_from_context($context, 'options');
      if ($op === 'form') {
        $options = $this->fetch_from_context($context, 'options');
        if (!empty($options['node_form']) && in_array($options['node_form'], array(CONTEXT_NODE_FORM, CONTEXT_NODE_FORM_ONLY))) {
          $this->condition_met($context, $node->type);
        }
      }
      elseif (empty($options['node_form']) || $options['node_form'] != CONTEXT_NODE_FORM_ONLY) {
        $this->condition_met($context, $node->type);
      }
    }
  }
}
