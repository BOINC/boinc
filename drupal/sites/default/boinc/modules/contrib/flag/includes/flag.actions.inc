<?php

/**
 * @file
 * Hooks for flag actions.
 */

/**
 * Implementation of hook_hook_info().
 */
function flag_hook_info() {
  $hooks = array(
    'flag' => array(
      'flag' => array(
        'flag' => array(
          'runs when' => t('Content has been flagged with any flag'),
        ),
        'unflag' => array(
          'runs when' => t('Content has been unflagged with any flag')
        ),
      ),
    ),
  );

  foreach (flag_get_flags() as $flag) {
    $hooks['flag']['flag']['flag_'. $flag->name]['runs when'] = t('A %type has been flagged with %name', array('%type' => $flag->content_type, '%name' => $flag->name));
    $hooks['flag']['flag']['unflag_'. $flag->name]['runs when'] = t('A %type has been unflagged with %name', array('%type' => $flag->content_type, '%name' => $flag->name));
  }

  return $hooks;
}

/**
 * Implementation of hook_action_info().
 */
function flag_action_info() {
  return array(
    'flag_node_action' => array(
      'type' => 'node',
      'description' => t('Flag (or unflag) a node'),
      'configurable' => TRUE,
      'hooks' => array(
        'node' => array('view', 'presave', 'insert', 'update', 'delete', 'view'),
        'comment' => array('insert', 'update', 'delete', 'view'),
      ),
    ),
    'flag_comment_action' => array(
      'type' => 'comment',
      'description' => t('Flag (or unflag) a comment'),
      'configurable' => TRUE,
      'hooks' => array(
        'comment' => array('insert', 'update', 'delete', 'view'),
      ),
    ),
    'flag_user_action' => array(
      'type' => 'user',
      'description' => t('Flag (or unflag) a user'),
      'configurable' => TRUE,
      'hooks' => array(
        'user' => array('insert', 'update', 'delete', 'login', 'logout', 'view'),
      ),
    ),
  );
}

/**
 * Implements hook_action_info_alter().
 *
 * Enable Flag actions on Node, Comment, and User hooks without
 * the trigger_unlock.module.
 */
function flag_action_info_alter(&$actions) {
  $node_flags = flag_get_flags('node');
  $comment_flags = flag_get_flags('comment');
  $user_flags = flag_get_flags('user');

  foreach ($actions as $name => $action) {
    if (strpos($name, 'node') === 0) {
      $actions[$name]['hooks']['flag'] = array('flag', 'unflag');
      foreach ($node_flags as $flag) {
        $actions[$name]['hooks']['flag'][] = 'flag_' . $flag->name;
        $actions[$name]['hooks']['flag'][] = 'unflag_' . $flag->name;
      }
    }
    if (strpos($name, 'comment') === 0) {
      $actions[$name]['hooks']['flag'] = array('flag', 'unflag');
      foreach ($comment_flags as $flag) {
        $actions[$name]['hooks']['flag'][] = 'flag_' . $flag->name;
        $actions[$name]['hooks']['flag'][] = 'unflag_' . $flag->name;
      }
    }
    if (strpos($name, 'user') === 0) {
      $actions[$name]['hooks']['flag'] = array('flag', 'unflag');
      foreach ($user_flags as $flag) {
        $actions[$name]['hooks']['flag'][] = 'flag_' . $flag->name;
        $actions[$name]['hooks']['flag'][] = 'unflag_' . $flag->name;
      }
    }
  }
}

/**
 * Implements Drupal action. Flags a node.
 *
 * Note the first parameter is "object" because it may be a comment or a node.
 */
function flag_node_action(&$object, $context = array()) {
  if ($flag = flag_get_flag($context['flag_action']['flag'])) {
    $account = isset($context['account']) ? $context['account'] : $GLOBALS['user'];
    $flag->flag($context['flag_action']['op'], $object->nid, $account, TRUE);
  }
}

/**
 * Form for configuring the Flag node action.
 */
function flag_node_action_form($context = array()) {
  return flag_action_form($context, 'node');
}

/**
 * Submit function for the Flag node action form.
 */
function flag_node_action_submit($form, $form_state) {
  return flag_action_submit($form, $form_state);
}

/**
 * Implements Drupal action. Flags a comment.
 */
function flag_comment_action(&$comment, $context = array()) {
  if ($flag = flag_get_flag($context['flag_action']['flag'])) {
    $account = isset($context['account']) ? $context['account'] : $GLOBALS['user'];
    $flag->flag($context['flag_action']['op'], $comment->cid, $account, TRUE);
  }
}

/**
 * Form for configuring the Flag comment action.
 */
function flag_comment_action_form($context) {
  return flag_action_form($context, 'comment');
}

/**
 * Submit function for the Flag comment action form.
 */
function flag_comment_action_submit($form, $form_state) {
  return flag_action_submit($form, $form_state);
}

/**
 * Implements Drupal action. Flags a user.
 */
function flag_user_action(&$user, $context = array()) {
  if ($flag = flag_get_flag($context['flag_action']['flag'])) {
    $account = isset($context['account']) ? $context['account'] : $GLOBALS['user'];
    $flag->flag($context['flag_action']['op'], $user->uid, $account, TRUE);
  }
}

/**
 * Form for configuring the Flag user action.
 */
function flag_user_action_form($context) {
  return flag_action_form($context, 'user');
}

/**
 * Submit function for the Flag user action form.
 */
function flag_user_action_submit($form, $form_state) {
  return flag_action_submit($form, $form_state);
}

/**
 * Generic form for configuring Flag actions.
 *
 * @param $context
 *   The current action context.
 * @param $content_type
 *   The content type applicable to this action, such as "node" or "comment".
 */
function flag_action_form($context, $content_type) {
  $form = array();

  $flags = flag_get_flags($content_type);
  // If this is a flag_action action, do not allow the triggering flag.
  if (isset($context['actions_flag'])) {
    unset($flags[$context['actions_flag']]);
  }
  $options = array();
  foreach ($flags as $key => $flag) {
    $options[$key] = $flag->title;
  }

  $form['flag_action']['#tree'] = TRUE;
  $form['flag_action']['warning'] = array(
    '#value' => '<div class="messages status">' . t("Note when setting a flag through actions, the selected flag will be flagged regardless of the user's permissions.") . '</div>',
  );
  $form['flag_action']['flag'] = array(
    '#title' => t('Flag to affect'),
    '#type' => 'radios',
    '#options' => $options,
    '#required' => TRUE,
    '#description' => t('When this action is fired, which flag should be flagged (or unflagged)?'),
    '#default_value' => isset($context['flag_action']['flag']) ? $context['flag_action']['flag'] : reset($options),
  );

  $form['flag_action']['op'] = array(
    '#title' => t('Flag operation'),
    '#type' => 'radios',
    '#options' => array('flag' => t('Flag'), 'unflag' => t('Unflag')),
    '#description' => t('When this action is fired, which operation should be performed on the flag?'),
    '#default_value' => isset($context['flag_action']['op']) ? $context['flag_action']['op'] : 'flag',
  );

  if (empty($options)) {
    $error = t('There are no available %type flags. Before you can create an action of this type, you need to <a href="!url">create a %type flag</a>.', array('%type' => $content_type, '!url' => url('admin/build/flags/add')));
    $form['flag_action']['flag']['#type'] = 'item';
    $form['flag_action']['flag']['#children'] = $error;
    $form['flag_action']['flag']['#element_validate'][] = 'flag_action_validate_flag';
    $form['flag_action']['flag']['#flag_error'] = $error;
  }

  return $form;
}

/**
 * Generic validation handler for validating Flag action configuration.
 */
function flag_action_validate_flag($element) {
  if (isset($element['#flag_error'])) {
    form_error($element, $element['#flag_error']);
  }
}

/**
 * Generic submission handler for saving Flag action configuration.
 */
function flag_action_submit($form, $form_state) {
  return array(
    'flag_action' => $form_state['values']['flag_action'],
  );
}
