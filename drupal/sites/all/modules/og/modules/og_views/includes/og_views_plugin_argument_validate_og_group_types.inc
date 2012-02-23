<?php
#define('OG_VIEWS_DO_NOT_VALIDATE_MEMBERSHIP', 0);
#define('OG_VIEWS_VALIDATE_GROUP_MEMBER', 1);
#define('OG_VIEWS_VALIDATE_GROUP_ADMIN', 2);

/**
 * Validate whether an argument is a group node. Borrows heavily form the Node argument validator.
 */
class og_views_plugin_argument_validate_og_group_types extends views_plugin_argument_validate {
  // What does this do?
  var $option_name = 'validate_argument_og_group_types';

  function validate_form(&$form, &$form_state) {
    $form['validate_argument_nid_type'] = array(
      '#type' => 'select',
      '#title' => t('Argument type'),
      '#options' => array(
        'nid' => t('Node ID'),
        'nids' => t("Node ID's separated by , or +"),
      ),
      '#default_value' => isset($this->argument->options['validate_argument_nid_type']) ? $this->argument->options['validate_argument_nid_type'] : 'nid',
      '#process' => array('views_process_dependency'),
      '#dependency' => array('edit-options-validate-type' => array($this->id)),
    );

    $options = array(
      OG_VIEWS_DO_NOT_VALIDATE_MEMBERSHIP => t('Do not validate user\'s group membership'),
      OG_VIEWS_VALIDATE_GROUP_MEMBER => t('Validate current user is a member of a specified group'),
      OG_VIEWS_VALIDATE_GROUP_ADMIN => t('Validate current user is an admin of a specified group'),
    );

    $form['validate_argument_is_member'] = array(
      '#type' => 'select',
      '#title' => t('Group membership validation'),
      '#options' => $options,
      '#default_value' => isset($this->argument->options['validate_argument_is_member']) ? $this->argument->options['validate_argument_is_member'] : 0,
      '#process' => array('views_process_dependency'),
      '#dependency' => array('edit-options-validate-type' => array($this->id)),
    );

    $options = array();
    $types = node_get_types();
    $group_types = og_get_types('group');
    foreach ($group_types as $type) {
      $info = $types[$type];
      $options[$type] = check_plain(t($info->name));
    }

    $default_types = $this->argument->options['validate_argument_group_node_type'];
    if (empty($default_types)) {
      $default_types = array();
    }

    $form['validate_argument_group_node_type'] = array(
      '#type' => 'checkboxes',
      '#prefix' => '<div id="edit-options-validate-argument-group-node-type-wrapper">',
      '#suffix' => '</div>',
      '#title' => t('Group node types'),
      '#options' => $options,
      '#default_value' => $default_types,
      '#description' => t('If you wish to validate for specific group node types, check them; if none are checked, all group nodes will pass.'),
      '#process' => array('expand_checkboxes', 'views_process_dependency'),
      '#dependency' => array('edit-options-validate-type' => array($this->id)),
    );
  }

  function validate_argument($argument) {
    $types = og_get_types('group');

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

        if (!$this->validate_group_node_type($node->type)) {
          return FALSE;
        }

        if (!$this->validate_membership($node)) {
          return FALSE;
        }

        // Save the title() handlers some work.
        $this->argument->validated_title = check_plain($node->title);

        // Admin has not setup any content types to behave as a group. Thats unsupported.
        if (empty($types)) {
          return TRUE;
        }

        return TRUE;
      case 'nids':
        $nids = new stdClass();
        $nids->value = array($argument);
        $nids = views_break_phrase($argument, $nids);
        if ($nids->value == -1) {
          return FALSE;
        }

        $placeholders = implode(', ', array_fill(0, sizeof($nids->value), '%d'));

        $has_membership = FALSE;
        $titles = array();
        $test_nids = drupal_map_assoc($nids->value);

        $result = db_query("SELECT nid, type, title FROM {node} WHERE nid IN ($placeholders)", $nids->value);
        while ($node = db_fetch_object($result)) {
          if (!$this->validate_group_node_type($node->type)) {
            return FALSE;
          }

          if (!$has_membership) {
            $has_membership = $this->validate_membership($node);
          }

          $titles[] = check_plain($node->title);
          unset($test_nids[$node->nid]);
        }

        $this->argument->validated_title = implode($nids->operator == 'or' ? ' + ' : ', ', $titles);
        // If $test is not empty, we did not find a nid.
        return empty($test_nids) && $has_membership;
    }
  }

  function validate_membership($node) {
    if (!empty($this->argument->options['validate_argument_is_member'])) {
      switch ($this->argument->options['validate_argument_is_member']) {
        case OG_VIEWS_VALIDATE_GROUP_MEMBER :
          if (!og_is_group_member($node->nid)) {
            return FALSE;
          }
          break;
        case OG_VIEWS_VALIDATE_GROUP_ADMIN :
          if (!og_is_group_admin($node)) {
            return FALSE;
          }
          break;
      }
    }

    return TRUE;
  }

  function validate_group_node_type($type) {
    $types = array();
    if (!empty($this->argument->options['validate_argument_group_node_type'])) {
      $types = array_filter($this->argument->options['validate_argument_group_node_type']);
    }
    if (empty($types)) {
      return og_is_group_type($type);
    }
    else {
      return isset($types[$type]);
    }
  }
}
