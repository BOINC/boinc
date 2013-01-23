<?php

/**
 * @file
 * Rules integration for the Flag module.
 */

/**
 * Implementation of hook_rules_data_type_info().
 * @ingroup rules
 */
function flag_rules_data_type_info() {
  return array(
    'flag' => array(
      'label' => t('Flag'),
      'class' => 'flag_rules_data_type',
      'savable' => TRUE,
      'identifiable' => TRUE,
      'use_input_form' => TRUE,
      'module' => 'Flag',
    ),
  );
}

/**
 * Defines the flag rule data type.
 * @ingroup rules
 */
class flag_rules_data_type extends rules_data_type {

  function save() {
    $flag = &$this->get();
    $flag->save;
    return TRUE;
  }

  function load($name) {
    return flag_get_flag($name);
  }

  function get_identifier() {
    $flag = &$this->get();
    return $flag->name;
  }

  /**
   * Returns radios for selecting a flag of the type given in
   * $info['flag_type'].
   */
  function get_default_input_form($info, $value, &$form_state) {
    $options = _flag_rules_flags_options(isset($info['flag_type']) ? $info['flag_type'] : NULL);
    $form = array(
      '#type' => 'radios',
      '#title' => $info['label'],
      '#options' => $options,
      '#required' => TRUE,
      '#disabled' => !$options,
      '#default_value' =>  isset($value) ? $value : NULL,
      '#theme' => 'flag_rules_radios',
      '#printed' => TRUE, //don't use the elements type theme function
    );
    if (!$options) {
      $form['#description'] = t('Error: There is no suiting flag available.');
    }
    return $form;
  }

  function check_value($info, $value) {
    if ($flag = flag_get_flag($value)) {
      return $flag;
    }
    rules_log(t('The flag %name does not exist any more.', array('%name' => $value)));
  }
}

/**
 * Implementation of hook_rules_event_info().
 */
function flag_rules_event_info() {
  $items = array();

  $flags = flag_get_flags();
  foreach ($flags as $flag) {

    $arguments = array(
      // First, define ubiquitous arguments.
      'flag' => array(
        'type' => 'flag',
        'label' => t('flag'),
      ),
      'flag_content_id' => array(
        'type' => 'number',
        'hidden' => TRUE,
      ),
      'flagging_user' => array(
        'type' => 'user',
        'label' => t('flagging user'),
      ),
    );
    // Then, define flag-specific arguments.
    $arguments += $flag->rules_get_event_arguments_definition();

    // For each flag we define two events.
    $items['flag_flagged_' . $flag->name] = array(
      'module' => 'Flag',
      'label' => t('A @flag-type has been flagged, under "@flag-title"', array('@flag-title' => $flag->get_title(), '@flag-type' => t($flag->content_type))),
      'arguments' => $arguments,
    );
    $items['flag_unflagged_' . $flag->name] = array(
      'module' => 'Flag',
      'label' => t('A @flag-type has been unflagged, under "@flag-title"', array('@flag-title' => $flag->get_title(), '@flag-type' => t($flag->content_type))),
      'arguments' => $arguments,
    );
  }
  return $items;
}

/**
 * Argument handler: Fetches the flags content for use with rules.
 */
function flag_rules_get_event_argument($flag, $content_id, $account) {
  return $flag->fetch_content($content_id);
}

function flag_rules_get_node_author($flag, $content_id, $account) {
  $node = $flag->fetch_content($content_id);
  return rules_events_argument_node_author($node);
}

function flag_rules_get_comment_author($flag, $content_id, $account) {
  $comment = $flag->fetch_content($content_id);
  return rules_events_argument_comment_author($comment);
}

function flag_rules_get_comment_content($flag, $content_id, $account) {
  $comment = $flag->fetch_content($content_id);
  return node_load($comment->nid);
}

function flag_rules_get_comment_node_author($flag, $content_id, $account) {
  $comment = $flag->fetch_content($content_id);
  return rules_events_argument_comment_node_author($comment);
}

/**
 * Implementation of hook_rules_action_info().
 */
function flag_rules_action_info() {
  $items = array(
    'flag_rules_action_trim' => array(
      'label' => t('Trim a flag'),
      'arguments' => array(
        'flag' => array(
          'type' => 'flag',
          'label' => t('Flag'),
        ),
        'flagging_user' => array(
          'type' => 'user',
          'label' => t('User whose flag to trim'),
          'description' => t('For non-global flags, this is the user whose flag to trim. (For global flags, this argument is ignored.)'),
        ),
        'cutoff_size' => array(
          'type' => 'number',
          'label' => t('Flag queue size'),
          'description' => t('The maximum number of objects to keep in the queue. Newly flagged objects will be kept; older ones will be removed. Tip: by typing "1" here you implement a singleton.'),
        ),
      ),
      'module' => 'Flag',
    ),
    'flag_fetch_entity_by_user' => array(
      'label' => t('Fetch content flagged by user'),
      'base' => 'flag_rules_action_fetch_content_by_user',
      'parameter' => array(
        'flag' => array(
          'type' => 'flag',
          'label' => t('Flag'),
        ),
        'flagging_user' => array(
          'type' => 'user',
          'label' => t('User who flagged the content'),
          'description' => t('For non-global flags, this is the user who flagged the content. (For global flags, this argument is ignored.)'),
        ),
      ),
      'provides' => array(
        'content_flagged_by_user' => array(
          'label' => t('Content flagged by user'),
          'type' => 'list<node>',
        ),
      ),
      'group' => t('Flag'),
      'access callback' => 'flag_rules_integration_access',
    ),
  );
  foreach (flag_get_types() as $type) {
    $args = array(
      'flag' => array(
        'type' => 'flag',
        'label' => t('Flag'),
        'flag_type' => $type,
      ),
    );
    $flag = flag_create_handler($type);
    if ($flag->rules_get_element_argument_definition()) {
      $args += array('object' => $flag->rules_get_element_argument_definition());

      $items += array(
        'flag_rules_action_flag_'. $type => array(
          'label' => t('Flag a @type', array('@type' => $type)),
          'base' => 'flag_rules_action_flag',
          'label callback' => 'flag_rules_action_flag_label',
          'arguments' => $args + array(
            'flagging_user' => array(
              'type' => 'user',
              'label' => t('User on whose behalf to flag'),
              'description' => t('For non-global flags, this is the user on whose behalf to flag the object. In addition, if checked below, the access permissions to the flag are checked against this user.'),
            ),
          ),
          'module' => 'Flag',
        ),
        'flag_rules_action_unflag_'. $type => array(
          'label' => t('Unflag a @type', array('@type' => $type)),
          'base' => 'flag_rules_action_unflag',
          'label callback' => 'flag_rules_action_unflag_label',
          'arguments' => $args + array(
            'flagging_user' => array(
              'type' => 'user',
              'label' => t('User on whose behalf to unflag'),
              'description' => t('For non-global flags, this is the user on whose behalf to unflag the object. In addition, if checked below, the access permissions to the flag are checked against this user.'),
            ),
          ),
          'module' => 'Flag',
        ),
      );
    }
  }
  return $items;
}

/**
 * Base action implementation: Flag.
 */
function flag_rules_action_flag($flag, $object, $flagging_user, $settings) {
  $flag->flag('flag', $flag->get_content_id($object), $flagging_user, !$settings['permission_check']);
}

/**
 * Base action implementation: Unflag.
 */
function flag_rules_action_unflag($flag, $object, $flagging_user, $settings) {
  $flag->flag('unflag', $flag->get_content_id($object), $flagging_user, !$settings['permission_check']);
}

/**
 * Base action implementation: Trim flag.
 */
function flag_rules_action_trim($flag, $flagging_user, $cutoff_size, $settings) {
  flag_trim_flag($flag, $flagging_user, $cutoff_size);
}

/**
 * Base action implementation: Fetch content flagged by a user.
 */
function flag_rules_action_fetch_content_by_user($flag, $user) {
  $result = db_select('flag_content', 'fc')
    ->fields('fc', array('content_id'))
    ->condition('content_type', $flag->content_type)
    ->condition('uid', $user->uid)
    ->condition('fid', $flag->fid)
    ->execute();

  $flagged = $result->fetchCol();
  return array('content_flagged_by_user' => $flagged);
}

/**
 * Implementation of hook_rules_condition_info().
 */
function flag_rules_condition_info() {
  $items = array();
  foreach (flag_get_types() as $type) {
    $args = array(
      'flag' => array(
        'type' => 'flag',
        'label' => t('Flag'),
        'flag_type' => $type,
      ),
    );
    $flag = flag_create_handler($type);
    if ($flag->rules_get_element_argument_definition()) {
      $args += array('object' => $flag->rules_get_element_argument_definition());

      $items += array(
        'flag_rules_condition_threshold_'. $type => array(
          'label' => drupal_ucfirst(t('@type has flagging count', array('@type' => $type))),
          'base' => 'flag_rules_condition_threshold',
          'label callback' => 'flag_rules_condition_threshold_label',
          'arguments' => $args + array(
            'number' => array(
              'type' => 'number',
              'label' => t('Number'),
              'description' => t('The number against which to test the number of times the object is flagged. For example, if you type "3" here, and choose "Greater than" for the operator, then this condition will return TRUE if the object is flagged more than three times.'),
            ),
          ),
          'module' => 'Flag',
        ),
        'flag_rules_condition_flagged_'. $type => array(
          'label' => drupal_ucfirst(t('@type is flagged', array('@type' => $type))),
          'base' => 'flag_rules_condition_flagged',
          'label callback' => 'flag_rules_condition_flagged_label',
          'arguments' => $args + array(
            'user' => array(
              'type' => 'user',
              'label' => t('User on whose behalf to check'),
              'description' => t('For non-global flags, this is the user on whose behalf the flag is checked.'),
            ),
          ),
          'module' => 'Flag',
        ),
      );
    }
  }
  return $items;
}

/**
 * Condition: Check flagging count.
 */
function flag_rules_condition_threshold($flag, $object, $number, $settings) {

  $count = $flag->get_count($flag->get_content_id($object));

  switch ($settings['operator']) {
    case '>' : return $count >  $number;
    case '>=': return $count >= $number;
    case '=' : return $count == $number;
    case '<' : return $count <  $number;
    case '<=': return $count <= $number;
  }
}

/**
 * Condition: Flag is flagged.
 */
function flag_rules_condition_flagged($flag, $object, $account, $settings) {
  return $flag->is_flagged($flag->get_content_id($object), $account->uid);
}
