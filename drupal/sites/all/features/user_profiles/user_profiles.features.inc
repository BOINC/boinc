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
            '#weight' => 0,
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
                '0' => 1271379760,
              ),
              'operation' => 'OR',
              '#argument map' => array(
                'user' => 'user',
              ),
            ),
            '#type' => 'condition',
          ),
        ),
        '#actions' => array(
          '1' => array(
            '#type' => 'action',
            '#settings' => array(
              '#argument map' => array(
                'node' => 'node',
              ),
            ),
            '#name' => 'boinccore_rules_action_mark_content_for_moderation',
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
            '#weight' => 0,
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
            '#type' => 'condition',
            '#weight' => 0,
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
            '#negate' => 1,
          ),
          '2' => array(
            '#type' => 'condition',
            '#settings' => array(
              '#argument map' => array(
                'user1' => 'user',
                'user2' => 'author',
              ),
            ),
            '#name' => 'rules_condition_user_comparison',
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
            '#weight' => 0,
          ),
        ),
        '#actions' => array(
          '1' => array(
            '#type' => 'action',
            '#settings' => array(
              '#argument map' => array(
                'node' => 'node',
              ),
            ),
            '#name' => 'boinccore_rules_action_mark_content_for_moderation',
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
            '#weight' => 0,
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
