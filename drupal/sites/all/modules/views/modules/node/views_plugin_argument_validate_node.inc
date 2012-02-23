<?php
// $Id: views_plugin_argument_validate_node.inc,v 1.2 2009/06/10 22:04:57 merlinofchaos Exp $
/**
 * @file
 * Contains the 'node' argument validator plugin.
 */

/**
 * Validate whether an argument is an acceptable node.
 */
class views_plugin_argument_validate_node extends views_plugin_argument_validate {
  var $option_name = 'validate_argument_node_type';

  function validate_form(&$form, &$form_state) {
    $types = node_get_types();
    foreach ($types as $type => $info) {
      $options[$type] = check_plain(t($info->name));
    }

    $arg = $this->get_argument();
    if (empty($arg)) {
      $arg = array();
    }

    $form[$this->option_name] = array(
      '#type' => 'checkboxes',
      '#prefix' => '<div id="edit-options-validate-argument-node-type-wrapper">',
      '#suffix' => '</div>',
      '#title' => t('Types'),
      '#options' => $options,
      '#default_value' => $arg,
      '#description' => t('If you wish to validate for specific node types, check them; if none are checked, all nodes will pass.'),
      '#process' => array('expand_checkboxes', 'views_process_dependency'),
      '#dependency' => array('edit-options-validate-type' => array($this->id)),
    );

    $form['validate_argument_node_access'] = array(
      '#type' => 'checkbox',
      '#title' => t('Validate user has access to the node'),
      '#default_value' => !empty($this->argument->options['validate_argument_node_access']),
      '#process' => array('views_process_dependency'),
      '#dependency' => array('edit-options-validate-type' => array($this->id)),
    );

    $form['validate_argument_nid_type'] = array(
      '#type' => 'select',
      '#title' => t('Argument type'),
      '#options' => array(
        'nid' => t('Node ID'),
        'nids' => t('Node IDs separated by , or +'),
      ),
      '#default_value' => isset($this->argument->options['validate_argument_nid_type']) ? $this->argument->options['validate_argument_nid_type'] : 'nid',
      '#process' => array('views_process_dependency'),
      '#dependency' => array('edit-options-validate-type' => array($this->id)),
    );
  }

  function validate_argument($argument) {
    $types = array_filter($this->argument->options[$this->option_name]);

    $type = isset($this->argument->options['validate_argument_nid_type']) ? $this->argument->options['validate_argument_nid_type'] : 'nid';

    switch ($type) {
      case 'nid':
        if (!is_numeric($argument)) {
          return FALSE;
        }
        $node = node_load($argument);
        if (!$node) {
          return FALSE;
        }

        if (!empty($this->argument->options['validate_argument_node_access'])) {
          if (!node_access('view', $node)) {
            return FALSE;
          }
        }

        // Save the title() handlers some work.
        $this->argument->validated_title = check_plain($node->title);

        if (empty($types)) {
          return TRUE;
        }

        return isset($types[$node->type]);
      break;
      case 'nids':
        $nids = new stdClass();
        $nids->value = array($argument);
        $nids = views_break_phrase($argument, $nids);
        if ($nids->value == -1) {
          return FALSE;
        }

        $placeholders = implode(', ', array_fill(0, sizeof($nids->value), '%d'));

        $test = drupal_map_assoc($nids->value);
        $titles = array();

        $result = db_query("SELECT * FROM {node} WHERE nid IN ($placeholders)", $nids->value);
        while ($node = db_fetch_object($result)) {
          if ($types && empty($types[$node->type])) {
            return FALSE;
          }

          if (!empty($this->argument->options['validate_argument_node_access'])) {
            if (!node_access('view', $node)) {
              return FALSE;
            }
          }

          $titles[] = check_plain($node->title);
          unset($test[$node->nid]);
        }

        $this->argument->validated_title = implode($nids->operator == 'or' ? ' + ' : ', ', $titles);
        // If this is not empty, we did not find a nid.
        return empty($test);
    }
  }
}

