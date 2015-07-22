<?php

/**
 * Implementation of hook_ctools_plugin_api().
 */
function user_profiles_ctools_plugin_api() {
  list($module, $api) = func_get_args();
  if ($module == "context" && $api == "context") {
    return array("version" => 3);
  }
  elseif ($module == "panels_mini" && $api == "panels_default") {
    return array("version" => 1);
  }
  elseif ($module == "strongarm" && $api == "strongarm") {
    return array("version" => 1);
  }
}

/**
 * Implementation of hook_node_info().
 */
function user_profiles_node_info() {
  $items = array(
    'profile' => array(
      'name' => t('Profile'),
      'module' => 'features',
      'description' => t('A user profile built as content.'),
      'has_title' => '1',
      'title_label' => t('Title'),
      'has_body' => '1',
      'body_label' => t('Body'),
      'min_word_count' => '0',
      'help' => '',
    ),
  );
  return $items;
}

/**
 * Implementation of hook_rules_defaults().
 */
function user_profiles_rules_defaults() {
  return array(
    'rules' => array(
      'rules_new_user_profile_has_been_created' => array(
        '#type' => 'rule',
        '#set' => 'event_node_insert',
        '#label' => 'New user profile has been created',
        '#active' => 1,
        '#weight' => '0',
        '#categories' => array(
          '0' => 'moderation queue',
          'user_profiles' => 'user_profiles',
        ),
        '#status' => 'default',
        '#conditions' => array(
          '0' => array(
            '#weight' => 0,
            '#info' => array(
              'label' => 'Created content is Profile',
              'arguments' => array(
                'node' => array(
                  'type' => 'node',
                  'label' => 'Content',
                ),
              ),
              'module' => 'Node',
            ),
            '#name' => 'rules_condition_content_is_type',
            '#settings' => array(
              'type' => array(
                'profile' => 'profile',
              ),
              '#argument map' => array(
                'node' => 'node',
              ),
            ),
            '#type' => 'condition',
          ),
          '1' => array(
            '#settings' => array(
              'roles' => array(
                '0' => 1271379760,
              ),
              'operation' => 'OR',
              '#argument map' => array(
                'user' => 'user',
              ),
            ),
            '#type' => 'condition',
            '#name' => 'rules_condition_user_hasrole',
            '#info' => array(
              'label' => 'User has role(s)',
              'arguments' => array(
                'user' => array(
                  'type' => 'user',
                  'label' => 'User',
                ),
              ),
              'module' => 'User',
            ),
            '#weight' => 0,
            '#negate' => 1,
          ),
          '2' => array(
            '#type' => 'OR',
            '0' => array(
              '#info' => array(
                'label' => 'Created content\'s field \'field_background\' has value',
                'arguments' => array(
                  'node' => array(
                    'type' => 'node',
                    'label' => 'Content',
                  ),
                ),
                'eval input' => array(
                  '0' => 'code',
                  '1' => 'value|0|value',
                  '2' => 'value|0|format',
                ),
                'module' => 'CCK',
              ),
              '#negate' => 1,
              '#name' => 'content_rules_field_has_value',
              '#settings' => array(
                'field_name' => 'field_background',
                '#argument map' => array(
                  'node' => 'node',
                ),
                'value' => array(
                  '0' => array(
                    'value' => '',
                    'format' => '4',
                  ),
                ),
                'code' => 'return array(
  0 => array(\'value\' => NULL, \'format\' => NULL),
);',
                'vars' => array(),
                '#eval input' => array(
                  'token_rules_input_evaluator' => array(
                    'code' => array(
                      '0' => ':global',
                    ),
                    'value|0|value' => array(
                      '0' => ':global',
                    ),
                    'value|0|format' => array(
                      '0' => ':global',
                    ),
                  ),
                ),
              ),
              '#weight' => 0,
              '#type' => 'condition',
            ),
            '#weight' => 0,
            '1' => array(
              '#negate' => 1,
              '#weight' => 0,
              '#info' => array(
                'label' => 'Created content\'s field \'field_opinions\' has value',
                'arguments' => array(
                  'node' => array(
                    'type' => 'node',
                    'label' => 'Content',
                  ),
                ),
                'eval input' => array(
                  '0' => 'code',
                  '1' => 'value|0|value',
                  '2' => 'value|0|format',
                ),
                'module' => 'CCK',
              ),
              '#name' => 'content_rules_field_has_value',
              '#settings' => array(
                'field_name' => 'field_opinions',
                '#argument map' => array(
                  'node' => 'node',
                ),
                'value' => array(
                  '0' => array(
                    'value' => '',
                    'format' => '4',
                  ),
                ),
                'code' => 'return array(
  0 => array(\'value\' => NULL, \'format\' => NULL),
);',
                'vars' => array(),
                '#eval input' => array(
                  'token_rules_input_evaluator' => array(
                    'code' => array(
                      '0' => ':global',
                    ),
                    'value|0|value' => array(
                      '0' => ':global',
                    ),
                    'value|0|format' => array(
                      '0' => ':global',
                    ),
                  ),
                ),
              ),
              '#type' => 'condition',
            ),
            '2' => array(
              '#type' => 'condition',
              '#settings' => array(
                'field_name' => 'field_url',
                '#argument map' => array(
                  'node' => 'node',
                ),
                'value' => array(
                  '0' => array(
                    'value' => '',
                  ),
                ),
                'code' => 'return array(
  0 => array(\'value\' => NULL),
);',
                'vars' => array(),
                '#eval input' => array(
                  'token_rules_input_evaluator' => array(
                    'code' => array(
                      '0' => ':global',
                    ),
                    'value|0|value' => array(
                      '0' => ':global',
                    ),
                  ),
                ),
              ),
              '#name' => 'content_rules_field_has_value',
              '#negate' => 1,
              '#info' => array(
                'label' => 'Created content\'s field \'field_url\' has value',
                'arguments' => array(
                  'node' => array(
                    'type' => 'node',
                    'label' => 'Content',
                  ),
                ),
                'eval input' => array(
                  '0' => 'code',
                  '1' => 'value|0|value',
                ),
                'module' => 'CCK',
              ),
              '#weight' => 0,
            ),
            '5' => array(
              '#type' => 'condition',
              '#settings' => array(
                'boolean' => '<?php echo ($node->field_image[0][\'fid\'] ? 1 : 0); ?>',
                '#eval input' => array(
                  'token_rules_input_evaluator' => array(
                    'boolean' => array(
                      '0' => ':global',
                    ),
                  ),
                  'rules_input_evaluator_php' => array(
                    'boolean' => array(
                      '0' => 'node',
                    ),
                  ),
                ),
              ),
              '#name' => 'rules_condition_check_boolean',
              '#info' => array(
                'label' => 'New profile has avatar',
                'label callback' => FALSE,
                'arguments' => array(
                  'boolean' => array(
                    'type' => 'boolean',
                    'label' => 'Truth value',
                  ),
                ),
                'module' => 'Rules',
              ),
              '#weight' => 0,
            ),
            '6' => array(
              '#weight' => 0,
              '#info' => array(
                'label' => 'New profile has profile image',
                'label callback' => FALSE,
                'arguments' => array(
                  'boolean' => array(
                    'type' => 'boolean',
                    'label' => 'Truth value',
                  ),
                ),
                'module' => 'Rules',
              ),
              '#name' => 'rules_condition_check_boolean',
              '#settings' => array(
                'boolean' => '<?php echo ($node->field_profile_image[0][\'fid\'] ? 1 : 0); ?>',
                '#eval input' => array(
                  'token_rules_input_evaluator' => array(
                    'boolean' => array(
                      '0' => ':global',
                    ),
                  ),
                  'rules_input_evaluator_php' => array(
                    'boolean' => array(
                      '0' => 'node',
                    ),
                  ),
                ),
              ),
              '#type' => 'condition',
            ),
          ),
        ),
        '#actions' => array(
          '1' => array(
            '#weight' => 0,
            '#info' => array(
              'label' => 'Mark content for moderation',
              'arguments' => array(
                'node' => array(
                  'type' => 'node',
                  'label' => 'Content',
                ),
              ),
              'module' => 'BOINC core',
            ),
            '#name' => 'boinccore_rules_action_mark_content_for_moderation',
            '#settings' => array(
              '#argument map' => array(
                'node' => 'node',
              ),
            ),
            '#type' => 'action',
          ),
        ),
        '#version' => 6003,
      ),
      'rules_user_profile_has_been_modified' => array(
        '#type' => 'rule',
        '#set' => 'event_node_update',
        '#label' => 'User profile has been modified',
        '#active' => 1,
        '#weight' => '0',
        '#categories' => array(
          '0' => 'moderation queue',
          'user_profiles' => 'user_profiles',
        ),
        '#status' => 'default',
        '#conditions' => array(
          '0' => array(
            '#type' => 'condition',
            '#settings' => array(
              'type' => array(
                'profile' => 'profile',
              ),
              '#argument map' => array(
                'node' => 'node',
              ),
            ),
            '#name' => 'rules_condition_content_is_type',
            '#info' => array(
              'label' => 'Created content is Profile',
              'arguments' => array(
                'node' => array(
                  'type' => 'node',
                  'label' => 'Content',
                ),
              ),
              'module' => 'Node',
            ),
            '#weight' => 0,
          ),
          '1' => array(
            '#negate' => 1,
            '#info' => array(
              'label' => 'User has role(s)',
              'arguments' => array(
                'user' => array(
                  'type' => 'user',
                  'label' => 'User',
                ),
              ),
              'module' => 'User',
            ),
            '#name' => 'rules_condition_user_hasrole',
            '#settings' => array(
              'roles' => array(
                '0' => 3519698132,
                '1' => 1271379760,
              ),
              'operation' => 'OR',
              '#argument map' => array(
                'user' => 'user',
              ),
            ),
            '#type' => 'condition',
            '#weight' => 0,
          ),
          '2' => array(
            '#weight' => 0,
            '#info' => array(
              'label' => 'Compare two users',
              'arguments' => array(
                'user1' => array(
                  'type' => 'user',
                  'label' => 'User account 1',
                ),
                'user2' => array(
                  'type' => 'user',
                  'label' => 'User account 2',
                ),
              ),
              'module' => 'User',
            ),
            '#name' => 'rules_condition_user_comparison',
            '#settings' => array(
              '#argument map' => array(
                'user1' => 'user',
                'user2' => 'author',
              ),
            ),
            '#type' => 'condition',
          ),
          '3' => array(
            '#type' => 'OR',
            '0' => array(
              '#info' => array(
                'label' => 'Updated content\'s field \'field_background\' has been changed',
                'arguments' => array(
                  'node' => array(
                    'type' => 'node',
                    'label' => 'Content containing changes',
                  ),
                  'node_unchanged' => array(
                    'type' => 'node',
                    'label' => 'Content not containing changes',
                  ),
                ),
                'module' => 'CCK',
              ),
              '#name' => 'content_rules_field_changed',
              '#settings' => array(
                'field_name' => 'field_background',
                '#argument map' => array(
                  'node' => 'node',
                  'node_unchanged' => 'node_unchanged',
                ),
              ),
              '#type' => 'condition',
              '#weight' => 0,
            ),
            '#weight' => 0,
            '1' => array(
              '#weight' => 0,
              '#info' => array(
                'label' => 'Updated content\'s field \'field_opinions\' has been changed',
                'arguments' => array(
                  'node' => array(
                    'type' => 'node',
                    'label' => 'Content containing changes',
                  ),
                  'node_unchanged' => array(
                    'type' => 'node',
                    'label' => 'Content not containing changes',
                  ),
                ),
                'module' => 'CCK',
              ),
              '#name' => 'content_rules_field_changed',
              '#settings' => array(
                'field_name' => 'field_opinions',
                '#argument map' => array(
                  'node' => 'node',
                  'node_unchanged' => 'node_unchanged',
                ),
              ),
              '#type' => 'condition',
            ),
            '2' => array(
              '#type' => 'condition',
              '#settings' => array(
                'field_name' => 'field_url',
                '#argument map' => array(
                  'node' => 'node',
                  'node_unchanged' => 'node_unchanged',
                ),
              ),
              '#name' => 'content_rules_field_changed',
              '#info' => array(
                'label' => 'Updated content\'s field \'field_url\' has been changed',
                'arguments' => array(
                  'node' => array(
                    'type' => 'node',
                    'label' => 'Content containing changes',
                  ),
                  'node_unchanged' => array(
                    'type' => 'node',
                    'label' => 'Content not containing changes',
                  ),
                ),
                'module' => 'CCK',
              ),
              '#weight' => 0,
            ),
            '4' => array(
              '#type' => 'AND',
              '#weight' => 0,
              '1' => array(
                '#weight' => 0,
                '#info' => array(
                  'label' => 'Modified profile has profile image',
                  'label callback' => FALSE,
                  'arguments' => array(
                    'boolean' => array(
                      'type' => 'boolean',
                      'label' => 'Truth value',
                    ),
                  ),
                  'module' => 'Rules',
                ),
                '#name' => 'rules_condition_check_boolean',
                '#settings' => array(
                  'boolean' => '<?php echo ($node->field_profile_image[0][\'fid\'] ? 1 : 0); ?>',
                  '#eval input' => array(
                    'token_rules_input_evaluator' => array(
                      'boolean' => array(
                        '0' => ':global',
                      ),
                    ),
                    'rules_input_evaluator_php' => array(
                      'boolean' => array(
                        '0' => 'node',
                      ),
                    ),
                  ),
                ),
                '#type' => 'condition',
              ),
              '0' => array(
                '#info' => array(
                  'label' => 'Modified profile image is different from previous profile image',
                  'label callback' => FALSE,
                  'arguments' => array(
                    'boolean' => array(
                      'type' => 'boolean',
                      'label' => 'Truth value',
                    ),
                  ),
                  'module' => 'Rules',
                ),
                '#name' => 'rules_condition_check_boolean',
                '#settings' => array(
                  'boolean' => '<?php echo ($node->field_profile_image[0][\'fid\'] != $node_unchanged->field_profile_image[0][\'fid\']); ?>',
                  '#eval input' => array(
                    'token_rules_input_evaluator' => array(
                      'boolean' => array(
                        '0' => ':global',
                      ),
                    ),
                    'rules_input_evaluator_php' => array(
                      'boolean' => array(
                        '0' => 'node',
                        '1' => 'node_unchanged',
                      ),
                    ),
                  ),
                ),
                '#type' => 'condition',
                '#weight' => 1,
              ),
            ),
            '3' => array(
              '#weight' => 1,
              '#type' => 'AND',
              '1' => array(
                '#type' => 'condition',
                '#settings' => array(
                  'boolean' => '<?php echo ($node->field_image[0][\'fid\'] ? 1 : 0); ?>',
                  '#eval input' => array(
                    'token_rules_input_evaluator' => array(
                      'boolean' => array(
                        '0' => ':global',
                      ),
                    ),
                    'rules_input_evaluator_php' => array(
                      'boolean' => array(
                        '0' => 'node',
                      ),
                    ),
                  ),
                ),
                '#name' => 'rules_condition_check_boolean',
                '#info' => array(
                  'label' => 'Modified profile has avatar',
                  'label callback' => FALSE,
                  'arguments' => array(
                    'boolean' => array(
                      'type' => 'boolean',
                      'label' => 'Truth value',
                    ),
                  ),
                  'module' => 'Rules',
                ),
                '#weight' => 0,
              ),
              '0' => array(
                '#weight' => 1,
                '#info' => array(
                  'label' => 'Modified profile avatar is different from previous avatar',
                  'label callback' => FALSE,
                  'arguments' => array(
                    'boolean' => array(
                      'type' => 'boolean',
                      'label' => 'Truth value',
                    ),
                  ),
                  'module' => 'Rules',
                ),
                '#name' => 'rules_condition_check_boolean',
                '#settings' => array(
                  'boolean' => '<?php echo ($node->field_image[0][\'fid\'] != $node_unchanged->field_image[0][\'fid\']); ?>',
                  '#eval input' => array(
                    'token_rules_input_evaluator' => array(
                      'boolean' => array(
                        '0' => ':global',
                      ),
                    ),
                    'rules_input_evaluator_php' => array(
                      'boolean' => array(
                        '0' => 'node',
                        '1' => 'node_unchanged',
                      ),
                    ),
                  ),
                ),
                '#type' => 'condition',
              ),
            ),
          ),
        ),
        '#actions' => array(
          '1' => array(
            '#weight' => 0,
            '#info' => array(
              'label' => 'Mark content for moderation',
              'arguments' => array(
                'node' => array(
                  'type' => 'node',
                  'label' => 'Content',
                ),
              ),
              'module' => 'BOINC core',
            ),
            '#name' => 'boinccore_rules_action_mark_content_for_moderation',
            '#settings' => array(
              '#argument map' => array(
                'node' => 'node',
              ),
            ),
            '#type' => 'action',
          ),
        ),
        '#version' => 6003,
      ),
    ),
  );
}

/**
 * Implementation of hook_views_api().
 */
function user_profiles_views_api() {
  return array(
    'api' => '2',
  );
}
