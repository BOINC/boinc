<?php

/**
 * Implementation of hook_ctools_plugin_api().
 */
function boinc_standard_ctools_plugin_api() {
  list($module, $api) = func_get_args();
  if ($module == "input_formats" && $api == "input_formats") {
    return array("version" => 1);
  }
  elseif ($module == "page_manager" && $api == "pages_default") {
    return array("version" => 1);
  }
  elseif ($module == "panels_mini" && $api == "panels_default") {
    return array("version" => 1);
  }
  elseif ($module == "strongarm" && $api == "strongarm") {
    return array("version" => 1);
  }
  elseif ($module == "wysiwyg" && $api == "wysiwyg") {
    return array("version" => 2);
  }
}

/**
 * Implementation of hook_flag_default_flags().
 */
function boinc_standard_flag_default_flags() {
  $flags = array();
  // Exported flag: "Comment Abuse".
  $flags['abuse_comment'] = array(
    'content_type' => 'comment',
    'title' => 'Comment Abuse',
    'global' => FALSE,
    'types' => array(
      '0' => 'forum',
      '1' => 'news',
      '2' => 'team_forum',
    ),
    'flag_short' => 'Report',
    'flag_long' => 'Report this content as offensive.',
    'flag_message' => '',
    'unflag_short' => 'Cancel report',
    'unflag_long' => 'Cancel reporting this content as offensive.',
    'unflag_message' => '',
    'unflag_denied_text' => '',
    'link_type' => 'toggle',
    'roles' => array(
      'flag' => array(
        '0' => 2,
      ),
      'unflag' => array(
        '0' => 2,
      ),
    ),
    'weight' => 0,
    'access_author' => '',
    'show_on_comment' => 1,
    'module' => 'boinc_standard',
    'locked' => array(
      '0' => 'name',
    ),
    'api_version' => 2,
  );
  // Exported flag: "Node Abuse".
  $flags['abuse_node'] = array(
    'content_type' => 'node',
    'title' => 'Node Abuse',
    'global' => FALSE,
    'types' => array(
      '0' => 'forum',
      '1' => 'profile',
      '2' => 'team_forum',
    ),
    'flag_short' => 'Report',
    'flag_long' => 'Report this content as offensive.',
    'flag_message' => '',
    'unflag_short' => 'Cancel report',
    'unflag_long' => 'Cancel reporting this content as offensive.',
    'unflag_message' => '',
    'unflag_denied_text' => '',
    'link_type' => 'toggle',
    'roles' => array(
      'flag' => array(
        '0' => 2,
      ),
      'unflag' => array(
        '0' => 2,
      ),
    ),
    'weight' => 0,
    'show_on_page' => 1,
    'show_on_teaser' => 0,
    'show_on_form' => 0,
    'access_author' => '',
    'i18n' => '0',
    'module' => 'boinc_standard',
    'locked' => array(
      '0' => 'name',
    ),
    'api_version' => 2,
  );
  // Exported flag: "User Abuse".
  $flags['abuse_user'] = array(
    'content_type' => 'user',
    'title' => 'User Abuse',
    'global' => '0',
    'types' => array(),
    'flag_short' => 'Report User',
    'flag_long' => 'Report this user as offensive.',
    'flag_message' => '',
    'unflag_short' => 'Remove Report',
    'unflag_long' => 'Remove your report about this user.',
    'unflag_message' => '',
    'unflag_denied_text' => '',
    'link_type' => 'confirm',
    'roles' => array(
      'flag' => array(
        '0' => '2',
      ),
      'unflag' => array(
        '0' => '2',
      ),
    ),
    'weight' => 0,
    'show_on_profile' => TRUE,
    'access_uid' => '',
    'flag_confirmation' => 'Are you sure you want to report this user as offensive?',
    'unflag_confirmation' => 'Are you sure you want to remove your report of this user?',
    'module' => 'boinc_standard',
    'locked' => array(
      '0' => 'name',
    ),
    'api_version' => 2,
  );
  // Exported flag: "Subscriptions".
  $flags['subscriptions'] = array(
    'content_type' => 'node',
    'title' => 'Subscriptions',
    'global' => '0',
    'types' => array(
      '0' => 'forum',
      '1' => 'news',
      '2' => 'team_forum',
    ),
    'flag_short' => 'subscribe',
    'flag_long' => 'Subscribe to this topic',
    'flag_message' => 'You are now subscribed to this topic.',
    'unflag_short' => 'unsubscribe',
    'unflag_long' => 'Unsubscribe to this topic',
    'unflag_message' => 'You are no longer subscribed to this topic.',
    'unflag_denied_text' => '',
    'link_type' => 'toggle',
    'roles' => array(
      'flag' => array(
        '0' => 2,
      ),
      'unflag' => array(
        '0' => 2,
      ),
    ),
    'weight' => 0,
    'show_on_page' => 1,
    'show_on_teaser' => 1,
    'show_on_form' => 0,
    'access_author' => '',
    'i18n' => '0',
    'module' => 'boinc_standard',
    'locked' => array(
      '0' => 'name',
    ),
    'api_version' => 2,
  );
  return $flags;

}

/**
 * Implementation of hook_node_info().
 */
function boinc_standard_node_info() {
  $items = array(
    'page' => array(
      'name' => t('Page'),
      'module' => 'features',
      'description' => t('A <em>page</em>, similar in form to a <em>story</em>, is a simple method for creating and displaying information that rarely changes, such as an "About us" section of a website. By default, a <em>page</em> entry does not allow visitor comments and is not featured on the site\'s initial home page.'),
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
function boinc_standard_rules_defaults() {
  return array(
    'rules' => array(
      'rules_forum_topic_marked_sticky_by_moderator_admin' => array(
        '#type' => 'rule',
        '#set' => 'event_node_update',
        '#label' => 'Forum topic marked sticky by moderator/admin',
        '#active' => 1,
        '#weight' => '0',
        '#categories' => array(
          '0' => 'boinc_standard',
          '1' => 'moderator notification',
        ),
        '#status' => 'default',
        '#conditions' => array(
          '0' => array(
            '#type' => 'condition',
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
              'label' => 'User has role(s): administrator or moderator',
              'label callback' => FALSE,
              'arguments' => array(
                'user' => array(
                  'type' => 'user',
                  'label' => 'User',
                ),
              ),
              'module' => 'User',
            ),
            '#weight' => 0,
          ),
          '1' => array(
            '#weight' => 0,
            '0' => array(
              '#weight' => 0,
              '#type' => 'condition',
              '#settings' => array(
                'type' => array(
                  'forum' => 'forum',
                ),
                '#argument map' => array(
                  'node' => 'node',
                ),
              ),
              '#name' => 'rules_condition_content_is_type',
              '#info' => array(
                'label' => 'Updated content is Forum topic',
                'arguments' => array(
                  'node' => array(
                    'type' => 'node',
                    'label' => 'Content',
                  ),
                ),
                'module' => 'Node',
              ),
            ),
            '#type' => 'OR',
            '1' => array(
              '#type' => 'condition',
              '#settings' => array(
                'type' => array(
                  'team_forum' => 'team_forum',
                ),
                '#argument map' => array(
                  'node' => 'node',
                ),
              ),
              '#name' => 'rules_condition_content_is_type',
              '#info' => array(
                'label' => 'Updated content is Team forum topic',
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
          ),
          '3' => array(
            '#weight' => 0,
            '#info' => array(
              'label' => 'PHP code: node content unchanged',
              'label callback' => FALSE,
              'module' => 'PHP',
              'eval input' => array(
                '0' => 'code',
              ),
            ),
            '#name' => 'rules_condition_custom_php',
            '#settings' => array(
              'code' => 'return $node->body == $node_unchanged->body;',
              'vars' => array(
                '0' => 'node',
                '1' => 'node_unchanged',
              ),
              '#eval input' => array(
                'token_rules_input_evaluator' => array(
                  'code' => array(
                    '0' => ':global',
                  ),
                ),
              ),
            ),
            '#type' => 'condition',
          ),
          '4' => array(
            '#weight' => 0,
            '#info' => array(
              'label' => 'PHP code: node made sticky',
              'label callback' => FALSE,
              'module' => 'PHP',
              'eval input' => array(
                '0' => 'code',
              ),
            ),
            '#name' => 'rules_condition_custom_php',
            '#type' => 'condition',
            '#settings' => array(
              'code' => 'return $node_unchanged->sticky == 0 && $node->sticky == 1;',
              'vars' => array(
                '0' => 'node',
                '1' => 'node_unchanged',
              ),
              '#eval input' => array(
                'token_rules_input_evaluator' => array(
                  'code' => array(
                    '0' => ':global',
                  ),
                ),
              ),
            ),
          ),
        ),
        '#actions' => array(
          '0' => array(
            '#info' => array(
              'label' => 'Notify moderators via email',
              'module' => 'BOINC core',
              'eval input' => array(
                '0' => 'subject',
                '1' => 'message',
                '2' => 'from',
              ),
            ),
            '#name' => 'boinccore_rules_action_mail_to_moderators',
            '#settings' => array(
              'from' => '',
              'subject' => 'Forum topic at [:global:site-name] marked sticky by moderator/admin',
              'message' => "[node:type] topic '[node:title]' has been marked sticky by moderator/admin [user:display-name].\r\n\r\nLink: [node:node-url]",
              '#eval input' => array(
                'token_rules_input_evaluator' => array(
                  'subject' => array(
                    '0' => ':global',
                  ),
                  'message' => array(
                    '0' => 'node',
                    '1' => 'user',
                    '2' => ':global',
                  ),
                  'from' => array(
                    '0' => ':global',
                  ),
                ),
              ),
            ),
            '#type' => 'action',
            '#weight' => 0,
          ),
        ),
        '#version' => 6003,
      ),
      'rules_forum_topic_marked_unsticky_by_moderator_admin' => array(
        '#type' => 'rule',
        '#set' => 'event_node_update',
        '#label' => 'Forum topic marked unsticky by moderator/admin',
        '#active' => 1,
        '#weight' => '0',
        '#categories' => array(
          '0' => 'boinc_standard',
          '1' => 'moderator notification',
        ),
        '#status' => 'default',
        '#conditions' => array(
          '0' => array(
            '#type' => 'condition',
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
              'label' => 'User has role(s): administrator or moderator',
              'label callback' => FALSE,
              'arguments' => array(
                'user' => array(
                  'type' => 'user',
                  'label' => 'User',
                ),
              ),
              'module' => 'User',
            ),
            '#weight' => 0,
          ),
          '1' => array(
            '#weight' => 0,
            '0' => array(
              '#weight' => 0,
              '#type' => 'condition',
              '#settings' => array(
                'type' => array(
                  'forum' => 'forum',
                ),
                '#argument map' => array(
                  'node' => 'node',
                ),
              ),
              '#name' => 'rules_condition_content_is_type',
              '#info' => array(
                'label' => 'Updated content is Forum topic',
                'arguments' => array(
                  'node' => array(
                    'type' => 'node',
                    'label' => 'Content',
                  ),
                ),
                'module' => 'Node',
              ),
            ),
            '#type' => 'OR',
            '1' => array(
              '#type' => 'condition',
              '#settings' => array(
                'type' => array(
                  'team_forum' => 'team_forum',
                ),
                '#argument map' => array(
                  'node' => 'node',
                ),
              ),
              '#name' => 'rules_condition_content_is_type',
              '#info' => array(
                'label' => 'Updated content is Team forum topic',
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
          ),
          '3' => array(
            '#weight' => 0,
            '#info' => array(
              'label' => 'PHP code: node content unchanged',
              'label callback' => FALSE,
              'module' => 'PHP',
              'eval input' => array(
                '0' => 'code',
              ),
            ),
            '#name' => 'rules_condition_custom_php',
            '#settings' => array(
              'code' => 'return $node->body == $node_unchanged->body;',
              'vars' => array(
                '0' => 'node',
                '1' => 'node_unchanged',
              ),
              '#eval input' => array(
                'token_rules_input_evaluator' => array(
                  'code' => array(
                    '0' => ':global',
                  ),
                ),
              ),
            ),
            '#type' => 'condition',
          ),
          '4' => array(
            '#weight' => 0,
            '#info' => array(
              'label' => 'PHP code: node made sticky',
              'label callback' => FALSE,
              'module' => 'PHP',
              'eval input' => array(
                '0' => 'code',
              ),
            ),
            '#name' => 'rules_condition_custom_php',
            '#type' => 'condition',
            '#settings' => array(
              'code' => 'return $node_unchanged->sticky == 1 && $node->sticky == 0;',
              'vars' => array(
                '0' => 'node',
                '1' => 'node_unchanged',
              ),
              '#eval input' => array(
                'token_rules_input_evaluator' => array(
                  'code' => array(
                    '0' => ':global',
                  ),
                ),
              ),
            ),
          ),
        ),
        '#actions' => array(
          '0' => array(
            '#info' => array(
              'label' => 'Notify moderators via email',
              'module' => 'BOINC core',
              'eval input' => array(
                '0' => 'subject',
                '1' => 'message',
                '2' => 'from',
              ),
            ),
            '#name' => 'boinccore_rules_action_mail_to_moderators',
            '#settings' => array(
              'from' => '',
              'subject' => 'Forum topic at [:global:site-name] marked unsticky by moderator/admin',
              'message' => "[node:type] topic '[node:title]' has been marked unsticky by moderator/admin [user:display-name].\r\n\r\nLink: [node:node-url]",
              '#eval input' => array(
                'token_rules_input_evaluator' => array(
                  'subject' => array(
                    '0' => ':global',
                  ),
                  'message' => array(
                    '0' => 'node',
                    '1' => 'user',
                    '2' => ':global',
                  ),
                  'from' => array(
                    '0' => ':global',
                  ),
                ),
              ),
            ),
            '#type' => 'action',
            '#weight' => 0,
          ),
        ),
        '#version' => 6003,
      ),
      'rules_forum_topic_locked_by_moderator_admin' => array(
        '#type' => 'rule',
        '#set' => 'event_node_update',
        '#label' => 'Forum topic locked by moderator/admin',
        '#active' => 1,
        '#weight' => '0',
        '#categories' => array(
          '0' => 'boinc_standard',
          '1' => 'moderator notification',
        ),
        '#status' => 'default',
        '#conditions' => array(
          '0' => array(
            '#type' => 'condition',
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
              'label' => 'User has role(s): administrator or moderator',
              'label callback' => FALSE,
              'arguments' => array(
                'user' => array(
                  'type' => 'user',
                  'label' => 'User',
                ),
              ),
              'module' => 'User',
            ),
            '#weight' => 0,
          ),
          '1' => array(
            '#weight' => 0,
            '0' => array(
              '#weight' => 0,
              '#type' => 'condition',
              '#settings' => array(
                'type' => array(
                  'forum' => 'forum',
                ),
                '#argument map' => array(
                  'node' => 'node',
                ),
              ),
              '#name' => 'rules_condition_content_is_type',
              '#info' => array(
                'label' => 'Updated content is Forum topic',
                'arguments' => array(
                  'node' => array(
                    'type' => 'node',
                    'label' => 'Content',
                  ),
                ),
                'module' => 'Node',
              ),
            ),
            '#type' => 'OR',
            '1' => array(
              '#type' => 'condition',
              '#settings' => array(
                'type' => array(
                  'team_forum' => 'team_forum',
                ),
                '#argument map' => array(
                  'node' => 'node',
                ),
              ),
              '#name' => 'rules_condition_content_is_type',
              '#info' => array(
                'label' => 'Updated content is Team forum topic',
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
          ),
          '3' => array(
            '#weight' => 0,
            '#info' => array(
              'label' => 'PHP code: node content unchanged',
              'label callback' => FALSE,
              'module' => 'PHP',
              'eval input' => array(
                '0' => 'code',
              ),
            ),
            '#name' => 'rules_condition_custom_php',
            '#settings' => array(
              'code' => 'return $node->body == $node_unchanged->body;',
              'vars' => array(
                '0' => 'node',
                '1' => 'node_unchanged',
              ),
              '#eval input' => array(
                'token_rules_input_evaluator' => array(
                  'code' => array(
                    '0' => ':global',
                  ),
                ),
              ),
            ),
            '#type' => 'condition',
          ),
          '4' => array(
            '#weight' => 0,
            '#info' => array(
              'label' => 'PHP code: node locked',
              'label callback' => FALSE,
              'module' => 'PHP',
              'eval input' => array(
                '0' => 'code',
              ),
            ),
            '#name' => 'rules_condition_custom_php',
            '#type' => 'condition',
            '#settings' => array(
              'code' => 'return $node_unchanged->comment == 2 && $node->comment == 1;',
              'vars' => array(
                '0' => 'node',
                '1' => 'node_unchanged',
              ),
              '#eval input' => array(
                'token_rules_input_evaluator' => array(
                  'code' => array(
                    '0' => ':global',
                  ),
                ),
              ),
            ),
          ),
        ),
        '#actions' => array(
          '0' => array(
            '#info' => array(
              'label' => 'Notify moderators via email',
              'module' => 'BOINC core',
              'eval input' => array(
                '0' => 'subject',
                '1' => 'message',
                '2' => 'from',
              ),
            ),
            '#name' => 'boinccore_rules_action_mail_to_moderators',
            '#settings' => array(
              'from' => '',
              'subject' => 'Forum topic at [:global:site-name] locked by moderator/admin',
              'message' => "[node:type] topic '[node:title]' has been locked by moderator/admin [user:display-name].\r\n\r\nLink: [node:node-url]",
              '#eval input' => array(
                'token_rules_input_evaluator' => array(
                  'subject' => array(
                    '0' => ':global',
                  ),
                  'message' => array(
                    '0' => 'node',
                    '1' => 'user',
                    '2' => ':global',
                  ),
                  'from' => array(
                    '0' => ':global',
                  ),
                ),
              ),
            ),
            '#type' => 'action',
            '#weight' => 0,
          ),
        ),
        '#version' => 6003,
      ),
      'rules_forum_topic_unlocked_by_moderator_admin' => array(
        '#type' => 'rule',
        '#set' => 'event_node_update',
        '#label' => 'Forum topic unlocked by moderator/admin',
        '#active' => 1,
        '#weight' => '0',
        '#categories' => array(
          '0' => 'boinc_standard',
          '1' => 'moderator notification',
        ),
        '#status' => 'default',
        '#conditions' => array(
          '0' => array(
            '#type' => 'condition',
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
              'label' => 'User has role(s): administrator or moderator',
              'label callback' => FALSE,
              'arguments' => array(
                'user' => array(
                  'type' => 'user',
                  'label' => 'User',
                ),
              ),
              'module' => 'User',
            ),
            '#weight' => 0,
          ),
          '1' => array(
            '#weight' => 0,
            '0' => array(
              '#weight' => 0,
              '#type' => 'condition',
              '#settings' => array(
                'type' => array(
                  'forum' => 'forum',
                ),
                '#argument map' => array(
                  'node' => 'node',
                ),
              ),
              '#name' => 'rules_condition_content_is_type',
              '#info' => array(
                'label' => 'Updated content is Forum topic',
                'arguments' => array(
                  'node' => array(
                    'type' => 'node',
                    'label' => 'Content',
                  ),
                ),
                'module' => 'Node',
              ),
            ),
            '#type' => 'OR',
            '1' => array(
              '#type' => 'condition',
              '#settings' => array(
                'type' => array(
                  'team_forum' => 'team_forum',
                ),
                '#argument map' => array(
                  'node' => 'node',
                ),
              ),
              '#name' => 'rules_condition_content_is_type',
              '#info' => array(
                'label' => 'Updated content is Team forum topic',
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
          ),
          '3' => array(
            '#weight' => 0,
            '#info' => array(
              'label' => 'PHP code: node content unchanged',
              'label callback' => FALSE,
              'module' => 'PHP',
              'eval input' => array(
                '0' => 'code',
              ),
            ),
            '#name' => 'rules_condition_custom_php',
            '#settings' => array(
              'code' => 'return $node->body == $node_unchanged->body;',
              'vars' => array(
                '0' => 'node',
                '1' => 'node_unchanged',
              ),
              '#eval input' => array(
                'token_rules_input_evaluator' => array(
                  'code' => array(
                    '0' => ':global',
                  ),
                ),
              ),
            ),
            '#type' => 'condition',
          ),
          '4' => array(
            '#weight' => 0,
            '#info' => array(
              'label' => 'PHP code: node unlocked',
              'label callback' => FALSE,
              'module' => 'PHP',
              'eval input' => array(
                '0' => 'code',
              ),
            ),
            '#name' => 'rules_condition_custom_php',
            '#type' => 'condition',
            '#settings' => array(
              'code' => 'return $node_unchanged->comment == 1 && $node->comment == 2;',
              'vars' => array(
                '0' => 'node',
                '1' => 'node_unchanged',
              ),
              '#eval input' => array(
                'token_rules_input_evaluator' => array(
                  'code' => array(
                    '0' => ':global',
                  ),
                ),
              ),
            ),
          ),
        ),
        '#actions' => array(
          '0' => array(
            '#info' => array(
              'label' => 'Notify moderators via email',
              'module' => 'BOINC core',
              'eval input' => array(
                '0' => 'subject',
                '1' => 'message',
                '2' => 'from',
              ),
            ),
            '#name' => 'boinccore_rules_action_mail_to_moderators',
            '#settings' => array(
              'from' => '',
              'subject' => 'Forum topic at [:global:site-name] unlocked by moderator/admin',
              'message' => "[node:type] topic '[node:title]' has been unlocked by moderator/admin [user:display-name].\r\n\r\nLink: [node:node-url]",
              '#eval input' => array(
                'token_rules_input_evaluator' => array(
                  'subject' => array(
                    '0' => ':global',
                  ),
                  'message' => array(
                    '0' => 'node',
                    '1' => 'user',
                    '2' => ':global',
                  ),
                  'from' => array(
                    '0' => ':global',
                  ),
                ),
              ),
            ),
            '#type' => 'action',
            '#weight' => 0,
          ),
        ),
        '#version' => 6003,
      ),
      'rules_forum_topic_is_hidden_by_moderator_admin' => array(
        '#type' => 'rule',
        '#set' => 'event_node_update',
        '#label' => 'Forum topic is hidden by moderator/admin',
        '#active' => 1,
        '#weight' => '0',
        '#categories' => array(
          '0' => 'boinc_standard',
          '1' => 'moderator notification',
        ),
        '#status' => 'default',
        '#conditions' => array(
          '0' => array(
            '#type' => 'condition',
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
              'label' => 'User has role(s): administrator or moderator',
              'label callback' => FALSE,
              'arguments' => array(
                'user' => array(
                  'type' => 'user',
                  'label' => 'User',
                ),
              ),
              'module' => 'User',
            ),
            '#weight' => 0,
          ),
          '1' => array(
            '#weight' => 0,
            '0' => array(
              '#weight' => 0,
              '#type' => 'condition',
              '#settings' => array(
                'type' => array(
                  'forum' => 'forum',
                ),
                '#argument map' => array(
                  'node' => 'node',
                ),
              ),
              '#name' => 'rules_condition_content_is_type',
              '#info' => array(
                'label' => 'Updated content is Forum topic',
                'arguments' => array(
                  'node' => array(
                    'type' => 'node',
                    'label' => 'Content',
                  ),
                ),
                'module' => 'Node',
              ),
            ),
            '#type' => 'OR',
            '1' => array(
              '#type' => 'condition',
              '#settings' => array(
                'type' => array(
                  'team_forum' => 'team_forum',
                ),
                '#argument map' => array(
                  'node' => 'node',
                ),
              ),
              '#name' => 'rules_condition_content_is_type',
              '#info' => array(
                'label' => 'Updated content is Team forum topic',
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
          ),
          '3' => array(
            '#weight' => 0,
            '#info' => array(
              'label' => 'PHP code: node content unchanged',
              'label callback' => FALSE,
              'module' => 'PHP',
              'eval input' => array(
                '0' => 'code',
              ),
            ),
            '#name' => 'rules_condition_custom_php',
            '#settings' => array(
              'code' => 'return $node->body == $node_unchanged->body;',
              'vars' => array(
                '0' => 'node',
                '1' => 'node_unchanged',
              ),
              '#eval input' => array(
                'token_rules_input_evaluator' => array(
                  'code' => array(
                    '0' => ':global',
                  ),
                ),
              ),
            ),
            '#type' => 'condition',
          ),
          '4' => array(
            '#weight' => 0,
            '#info' => array(
              'label' => 'PHP code: node status changed to hidden',
              'label callback' => FALSE,
              'module' => 'PHP',
              'eval input' => array(
                '0' => 'code',
              ),
            ),
            '#name' => 'rules_condition_custom_php',
            '#type' => 'condition',
            '#settings' => array(
              'code' => 'return $node_unchanged->status == 1 && $node->status == 0;',
              'vars' => array(
                '0' => 'node',
                '1' => 'node_unchanged',
              ),
              '#eval input' => array(
                'token_rules_input_evaluator' => array(
                  'code' => array(
                    '0' => ':global',
                  ),
                ),
              ),
            ),
          ),
        ),
        '#actions' => array(
          '0' => array(
            '#info' => array(
              'label' => 'Notify moderators via email',
              'module' => 'BOINC core',
              'eval input' => array(
                '0' => 'subject',
                '1' => 'message',
                '2' => 'from',
              ),
            ),
            '#name' => 'boinccore_rules_action_mail_to_moderators',
            '#settings' => array(
              'from' => '',
              'subject' => 'Forum topic at [:global:site-name] hidden by moderator/admin',
              'message' => "[node:type] topic '[node:title]' has been hidden by moderator/admin [user:display-name].\r\n\r\nLink: [node:node-url]",
              '#eval input' => array(
                'token_rules_input_evaluator' => array(
                  'subject' => array(
                    '0' => ':global',
                  ),
                  'message' => array(
                    '0' => 'node',
                    '1' => 'user',
                    '2' => ':global',
                  ),
                  'from' => array(
                    '0' => ':global',
                  ),
                ),
              ),
            ),
            '#type' => 'action',
            '#weight' => 0,
          ),
        ),
        '#version' => 6003,
      ),
      'rules_forum_topic_is_unhidden_by_moderator_admin' => array(
        '#type' => 'rule',
        '#set' => 'event_node_update',
        '#label' => 'Forum topic is unhidden by moderator/admin',
        '#active' => 1,
        '#weight' => '0',
        '#categories' => array(
          '0' => 'boinc_standard',
          '1' => 'moderator notification',
        ),
        '#status' => 'default',
        '#conditions' => array(
          '0' => array(
            '#type' => 'condition',
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
              'label' => 'User has role(s): administrator or moderator',
              'label callback' => FALSE,
              'arguments' => array(
                'user' => array(
                  'type' => 'user',
                  'label' => 'User',
                ),
              ),
              'module' => 'User',
            ),
            '#weight' => 0,
          ),
          '1' => array(
            '#type' => 'OR',
            '0' => array(
              '#info' => array(
                'label' => 'Updated content is Forum topic',
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
                  'forum' => 'forum',
                ),
                '#argument map' => array(
                  'node' => 'node',
                ),
              ),
              '#type' => 'condition',
              '#weight' => 0,
            ),
            '#weight' => 0,
            '1' => array(
              '#weight' => 0,
              '#info' => array(
                'label' => 'Updated content is Team forum topic',
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
                  'team_forum' => 'team_forum',
                ),
                '#argument map' => array(
                  'node' => 'node',
                ),
              ),
              '#type' => 'condition',
            ),
          ),
          '3' => array(
            '#type' => 'condition',
            '#settings' => array(
              'code' => 'return $node->body == $node_unchanged->body;',
              'vars' => array(
                '0' => 'node',
                '1' => 'node_unchanged',
              ),
              '#eval input' => array(
                'token_rules_input_evaluator' => array(
                  'code' => array(
                    '0' => ':global',
                  ),
                ),
              ),
            ),
            '#name' => 'rules_condition_custom_php',
            '#info' => array(
              'label' => 'PHP code: node content is unchanged',
              'label callback' => FALSE,
              'module' => 'PHP',
              'eval input' => array(
                '0' => 'code',
              ),
            ),
            '#weight' => 0,
          ),
          '4' => array(
            '#type' => 'condition',
            '#settings' => array(
              'code' => 'return $node_unchanged->status == 0 && $node->status == 1;',
              'vars' => array(
                '0' => 'node',
                '1' => 'node_unchanged',
              ),
              '#eval input' => array(
                'token_rules_input_evaluator' => array(
                  'code' => array(
                    '0' => ':global',
                  ),
                ),
              ),
            ),
            '#name' => 'rules_condition_custom_php',
            '#info' => array(
              'label' => 'PHP code: node status changed to unhidden',
              'label callback' => FALSE,
              'module' => 'PHP',
              'eval input' => array(
                '0' => 'code',
              ),
            ),
            '#weight' => 0,
          ),
        ),
        '#actions' => array(
          '0' => array(
            '#weight' => 0,
            '#info' => array(
              'label' => 'Notify moderators via email',
              'module' => 'BOINC core',
              'eval input' => array(
                '0' => 'subject',
                '1' => 'message',
                '2' => 'from',
              ),
            ),
            '#name' => 'boinccore_rules_action_mail_to_moderators',
            '#settings' => array(
              'from' => '',
              'subject' => 'Forum topic at [:global:site-name] unhidden by moderator/admin',
              'message' => "[node:type] topic '[node:title]' has been unhidden by moderator/admin [user:display-name].\r\n\r\nLink: [node:node-url]",
              '#eval input' => array(
                'token_rules_input_evaluator' => array(
                  'subject' => array(
                    '0' => ':global',
                  ),
                  'message' => array(
                    '0' => 'node',
                    '1' => 'user',
                    '2' => ':global',
                  ),
                  'from' => array(
                    '0' => ':global',
                  ),
                ),
              ),
            ),
            '#type' => 'action',
          ),
        ),
        '#version' => 6003,
      ),
      'rules_forum_topic_is_edited_by_moderator_or_admin' => array(
        '#type' => 'rule',
        '#set' => 'event_node_update',
        '#label' => 'Forum topic is edited by moderator or admin',
        '#active' => 1,
        '#weight' => '0',
        '#categories' => array(
          '0' => 'boinc_standard',
          '1' => 'moderator notification',
        ),
        '#status' => 'default',
        '#conditions' => array(
          '0' => array(
            '#weight' => 0,
            '#type' => 'condition',
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
              'label' => 'User has role(s): administrator or moderator',
              'label callback' => FALSE,
              'arguments' => array(
                'user' => array(
                  'type' => 'user',
                  'label' => 'User',
                ),
              ),
              'module' => 'User',
            ),
          ),
          '1' => array(
            '#weight' => 0,
            '0' => array(
              '#weight' => 0,
              '#info' => array(
                'label' => 'Updated content is Forum topic',
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
                  'forum' => 'forum',
                ),
                '#argument map' => array(
                  'node' => 'node',
                ),
              ),
              '#type' => 'condition',
            ),
            '#type' => 'OR',
            '1' => array(
              '#type' => 'condition',
              '#settings' => array(
                'type' => array(
                  'team_forum' => 'team_forum',
                ),
                '#argument map' => array(
                  'node' => 'node',
                ),
              ),
              '#name' => 'rules_condition_content_is_type',
              '#info' => array(
                'label' => 'Updated content is Team forum topic',
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
          ),
          '2' => array(
            '#weight' => 0,
            '#info' => array(
              'label' => 'PHP code: content changed',
              'label callback' => FALSE,
              'module' => 'PHP',
              'eval input' => array(
                '0' => 'code',
              ),
            ),
            '#name' => 'rules_condition_custom_php',
            '#settings' => array(
              'code' => 'return ($node->body != $node_unchanged->body);',
              'vars' => array(
                '0' => 'node',
                '1' => 'node_unchanged',
              ),
              '#eval input' => array(
                'token_rules_input_evaluator' => array(
                  'code' => array(
                    '0' => ':global',
                  ),
                ),
              ),
            ),
            '#type' => 'condition',
          ),
        ),
        '#actions' => array(
          '0' => array(
            '#weight' => 0,
            '#type' => 'action',
            '#settings' => array(
              'from' => '',
              'subject' => 'Forum topic at [:global:site-name] edited by moderator/admin',
              'message' => "[node:type] topic '[node:title]' has been edited by moderator/admin [user:display-name].\r\n\r\nLink: [node:node-url]",
              '#eval input' => array(
                'token_rules_input_evaluator' => array(
                  'subject' => array(
                    '0' => ':global',
                  ),
                  'message' => array(
                    '0' => 'node',
                    '1' => 'user',
                    '2' => ':global',
                  ),
                  'from' => array(
                    '0' => ':global',
                  ),
                ),
              ),
            ),
            '#name' => 'boinccore_rules_action_mail_to_moderators',
            '#info' => array(
              'label' => 'Notify moderators via email',
              'module' => 'BOINC core',
              'eval input' => array(
                '0' => 'subject',
                '1' => 'message',
                '2' => 'from',
              ),
            ),
          ),
        ),
        '#version' => 6003,
      ),
      'rules_forum_topic_is_moved_to_a_different_forum_by_moderator_or_admin' => array(
        '#type' => 'rule',
        '#set' => 'event_node_update',
        '#label' => 'Forum topic is moved to a different forum by moderator or admin',
        '#active' => 1,
        '#weight' => '0',
        '#categories' => array(
          '0' => 'boinc_standard',
          '1' => 'moderator notification',
        ),
        '#status' => 'default',
        '#conditions' => array(
          '0' => array(
            '#weight' => 0,
            '#type' => 'condition',
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
              'label' => 'User has role(s): administrator or moderator',
              'label callback' => FALSE,
              'arguments' => array(
                'user' => array(
                  'type' => 'user',
                  'label' => 'User',
                ),
              ),
              'module' => 'User',
            ),
          ),
          '1' => array(
            '#weight' => 0,
            '0' => array(
              '#weight' => 0,
              '#info' => array(
                'label' => 'Updated content is Forum topic',
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
                  'forum' => 'forum',
                ),
                '#argument map' => array(
                  'node' => 'node',
                ),
              ),
              '#type' => 'condition',
            ),
            '#type' => 'OR',
            '1' => array(
              '#type' => 'condition',
              '#settings' => array(
                'type' => array(
                  'team_forum' => 'team_forum',
                ),
                '#argument map' => array(
                  'node' => 'node',
                ),
              ),
              '#name' => 'rules_condition_content_is_type',
              '#info' => array(
                'label' => 'Updated content is Team forum topic',
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
          ),
          '2' => array(
            '#weight' => 0,
            '#info' => array(
              'label' => 'PHP code: node moved to new forum parent',
              'label callback' => FALSE,
              'module' => 'PHP',
              'eval input' => array(
                '0' => 'code',
              ),
            ),
            '#name' => 'rules_condition_custom_php',
            '#settings' => array(
              'code' => 'return ($node->tid != $node_unchanged->tid);',
              'vars' => array(
                '0' => 'node',
                '1' => 'node_unchanged',
              ),
              '#eval input' => array(
                'token_rules_input_evaluator' => array(
                  'code' => array(
                    '0' => ':global',
                  ),
                ),
              ),
            ),
            '#type' => 'condition',
          ),
        ),
        '#actions' => array(
          '0' => array(
            '#weight' => 0,
            '#type' => 'action',
            '#settings' => array(
              'from' => '',
              'subject' => 'Forum topic at [:global:site-name] moved by moderator/admin',
              'message' => "[node:type] topic '[node:title]' has been moved by moderator/admin [user:display-name].\r\n\r\nLink: [node:node-url]",
              '#eval input' => array(
                'token_rules_input_evaluator' => array(
                  'subject' => array(
                    '0' => ':global',
                  ),
                  'message' => array(
                    '0' => 'node',
                    '1' => 'user',
                    '2' => ':global',
                  ),
                  'from' => array(
                    '0' => ':global',
                  ),
                ),
              ),
            ),
            '#name' => 'boinccore_rules_action_mail_to_moderators',
            '#info' => array(
              'label' => 'Notify moderators via email',
              'module' => 'BOINC core',
              'eval input' => array(
                '0' => 'subject',
                '1' => 'message',
                '2' => 'from',
              ),
            ),
          ),
        ),
        '#version' => 6003,
      ),
      'rules_forum_topic_nenamed_title_by_moderator_or_admin' => array(
        '#type' => 'rule',
        '#set' => 'event_node_update',
        '#label' => 'Forum topic renamed title by moderator or admin',
        '#active' => 1,
        '#weight' => '0',
        '#categories' => array(
          '0' => 'boinc_standard',
          '1' => 'moderator notification',
        ),
        '#status' => 'default',
        '#conditions' => array(
          '0' => array(
            '#weight' => 0,
            '#type' => 'condition',
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
              'label' => 'User has role(s): administrator or moderator',
              'label callback' => FALSE,
              'arguments' => array(
                'user' => array(
                  'type' => 'user',
                  'label' => 'User',
                ),
              ),
              'module' => 'User',
            ),
          ),
          '1' => array(
            '#weight' => 0,
            '0' => array(
              '#weight' => 0,
              '#info' => array(
                'label' => 'Updated content is Forum topic',
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
                  'forum' => 'forum',
                ),
                '#argument map' => array(
                  'node' => 'node',
                ),
              ),
              '#type' => 'condition',
            ),
            '#type' => 'OR',
            '1' => array(
              '#type' => 'condition',
              '#settings' => array(
                'type' => array(
                  'team_forum' => 'team_forum',
                ),
                '#argument map' => array(
                  'node' => 'node',
                ),
              ),
              '#name' => 'rules_condition_content_is_type',
              '#info' => array(
                'label' => 'Updated content is Team forum topic',
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
          ),
          '2' => array(
            '#weight' => 0,
            '#info' => array(
              'label' => 'PHP code: title changed',
              'label callback' => FALSE,
              'module' => 'PHP',
              'eval input' => array(
                '0' => 'code',
              ),
            ),
            '#name' => 'rules_condition_custom_php',
            '#settings' => array(
              'code' => 'return ($node->title != $node_unchanged->title);',
              'vars' => array(
                '0' => 'node',
                '1' => 'node_unchanged',
              ),
              '#eval input' => array(
                'token_rules_input_evaluator' => array(
                  'code' => array(
                    '0' => ':global',
                  ),
                ),
              ),
            ),
            '#type' => 'condition',
          ),
        ),
        '#actions' => array(
          '0' => array(
            '#weight' => 0,
            '#type' => 'action',
            '#settings' => array(
              'from' => '',
              'subject' => 'Forum topic at [:global:site-name] renamed by moderator/admin',
              'message' => "[node:type] topic '[node:title]' has its title renamed by moderator/admin [user:display-name].\r\n\r\nLink: [node:node-url]",
              '#eval input' => array(
                'token_rules_input_evaluator' => array(
                  'subject' => array(
                    '0' => ':global',
                  ),
                  'message' => array(
                    '0' => 'node',
                    '1' => 'user',
                    '2' => ':global',
                  ),
                  'from' => array(
                    '0' => ':global',
                  ),
                ),
              ),
            ),
            '#name' => 'boinccore_rules_action_mail_to_moderators',
            '#info' => array(
              'label' => 'Notify moderators via email',
              'module' => 'BOINC core',
              'eval input' => array(
                '0' => 'subject',
                '1' => 'message',
                '2' => 'from',
              ),
            ),
          ),
        ),
        '#version' => 6003,
      ),
      'rules_comment_converted_to_new_forum_topic_by_moderator_or_admin' => array(
        '#type' => 'rule',
        '#set' => 'event_boinccore_comment_convert',
        '#label' => 'Comment is converted to new forum topic by moderator or admin',
        '#active' => 1,
        '#weight' => '0',
        '#categories' => array(
          '0' => 'boinc_standard',
          '1' => 'moderator notification',
        ),
        '#status' => 'default',
        '#conditions' => array(),
        '#actions' => array(
          '0' => array(
            '#info' => array(
              'label' => 'Notify moderators via email',
              'module' => 'BOINC core',
              'eval input' => array(
                '0' => 'subject',
                '1' => 'message',
                '2' => 'from',
              ),
            ),
            '#name' => 'boinccore_rules_action_mail_to_moderators',
            '#settings' => array(
              'from' => '',
              'subject' => 'Comment at [:global:site-name] converted to new forum topic',
              'message' => 'Comment has been converted to new forum topic by moderator/admin [user:display-name].
',
              '#eval input' => array(
                'token_rules_input_evaluator' => array(
                  'subject' => array(
                    '0' => ':global',
                  ),
                  'message' => array(
                    '0' => 'user',
                    '1' => ':global',
                  ),
                  'from' => array(
                    '0' => ':global',
                  ),
                ),
              ),
            ),
            '#type' => 'action',
            '#weight' => 0,
          ),
        ),
        '#version' => 6003,
      ),
      'rules_comment_edited_by_moderator_or_admin' => array(
        '#type' => 'rule',
        '#set' => 'event_comment_update',
        '#label' => 'Comment is edited by moderator or admin',
        '#active' => 1,
        '#weight' => '0',
        '#categories' => array(
          '0' => 'boinc_standard',
          '1' => 'moderator notification',
        ),
        '#status' => 'default',
        '#conditions' => array(
          '0' => array(
            '#negate' => 1,
            '#weight' => 0,
            '#info' => array(
              'label' => 'Compare two users: acting user (who edited the comment) is NOT the comment author',
              'label callback' => FALSE,
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
                'user2' => 'comment_author',
              ),
            ),
            '#type' => 'condition',
          ),
        ),
        '#actions' => array(
          '0' => array(
            '#weight' => 0,
            '#type' => 'action',
            '#settings' => array(
              'from' => '',
              'subject' => 'Comment edited at [:global:site-name] by moderator or admin',
              'message' => "Comment has been edited by moderator/admin [user:display-name].\r\n\r\nLink: [:global:site-url]/goto/comment/[comment:comment-cid]",
              '#eval input' => array(
                'token_rules_input_evaluator' => array(
                  'subject' => array(
                    '0' => ':global',
                  ),
                  'message' => array(
                    '0' => 'comment',
                    '1' => 'user',
                    '2' => ':global',
                  ),
                  'from' => array(
                    '0' => ':global',
                  ),
                ),
              ),
            ),
            '#name' => 'boinccore_rules_action_mail_to_moderators',
            '#info' => array(
              'label' => 'Notify moderators via email',
              'module' => 'BOINC core',
              'eval input' => array(
                '0' => 'subject',
                '1' => 'message',
                '2' => 'from',
              ),
            ),
          ),
        ),
        '#version' => 6003,
      ),
      'rules_comment_is_unpublished_hidden_by_moderator_or_admin' => array(
        '#type' => 'rule',
        '#set' => 'event_comment_unpublish',
        '#label' => 'Comment is hidden by moderator or admin',
        '#active' => 1,
        '#weight' => '0',
        '#categories' => array(
          '0' => 'boinc_standard',
          '1' => 'moderator notification',
        ),
        '#status' => 'default',
        '#conditions' => array(
          '0' => array(
            '#weight' => 0,
            '#negate' => 1,
            '#info' => array(
              'label' => 'Compare two users: acting user (who hid the comment) is NOT the comment author',
              'label callback' => FALSE,
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
            '#type' => 'condition',
            '#settings' => array(
              '#argument map' => array(
                'user1' => 'user',
                'user2' => 'comment_author',
              ),
            ),
          ),
        ),
        '#actions' => array(
          '0' => array(
            '#weight' => 0,
            '#type' => 'action',
            '#settings' => array(
              'from' => '',
              'subject' => 'Comment at [:global:site-name] hidden by moderator or admin',
              'message' => "Comment has been hidden by moderator/admin [user:display-name].\r\n\r\nLink: [:global:site-url]/goto/comment/[comment:comment-cid]",
              '#eval input' => array(
                'token_rules_input_evaluator' => array(
                  'subject' => array(
                    '0' => ':global',
                  ),
                  'message' => array(
                    '0' => 'comment',
                    '1' => 'user',
                    '2' => ':global',
                  ),
                  'from' => array(
                    '0' => ':global',
                  ),
                ),
              ),
            ),
            '#name' => 'boinccore_rules_action_mail_to_moderators',
            '#info' => array(
              'label' => 'Notify moderators via email',
              'module' => 'BOINC core',
              'eval input' => array(
                '0' => 'subject',
                '1' => 'message',
                '2' => 'from',
              ),
            ),
          ),
        ),
        '#version' => 6003,
      ),
      'rules_comment_is_published_unhidden_by_moderator_or_admin' => array(
        '#type' => 'rule',
        '#set' => 'event_boinccore_comment_unhidden',
        '#label' => 'Comment is unhidden by moderator or admin',
        '#active' => 1,
        '#weight' => '0',
        '#categories' => array(
          '0' => 'boinc_standard',
          '1' => 'moderator notification',
        ),
        '#status' => 'default',
        '#conditions' => array(
          '0' => array(
            '#type' => 'condition',
            '#settings' => array(
              '#argument map' => array(
                'user1' => 'user',
                'user2' => 'comment_author',
              ),
            ),
            '#name' => 'rules_condition_user_comparison',
            '#info' => array(
              'label' => 'Compare two users: acting user (who unhid the comment) is NOT the comment author',
              'label callback' => FALSE,
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
            '#negate' => 1,
            '#weight' => 0,
          ),
        ),
        '#actions' => array(
          '0' => array(
            '#weight' => 0,
            '#info' => array(
              'label' => 'Notify moderators via email',
              'module' => 'BOINC core',
              'eval input' => array(
                '0' => 'subject',
                '1' => 'message',
                '2' => 'from',
              ),
            ),
            '#name' => 'boinccore_rules_action_mail_to_moderators',
            '#settings' => array(
              'from' => '',
              'subject' => 'Comment at [:global:site-name] unhidden by moderator or admin',
              'message' => "Comment has been unhidden by moderator/admin [user:display-name].\r\n\r\nLink: [:global:site-url]/goto/comment/[comment:comment-cid]",
              '#eval input' => array(
                'token_rules_input_evaluator' => array(
                  'subject' => array(
                    '0' => ':global',
                  ),
                  'message' => array(
                    '0' => 'comment',
                    '1' => 'user',
                    '2' => ':global',
                  ),
                  'from' => array(
                    '0' => ':global',
                  ),
                ),
              ),
            ),
            '#type' => 'action',
          ),
        ),
        '#version' => 6003,
      ),
      'rules_comment_deleted_by_admin' => array(
        '#type' => 'rule',
        '#set' => 'event_comment_delete',
        '#label' => 'Comment deleted by admin',
        '#active' => 1,
        '#weight' => '0',
        '#categories' => array(
          '0' => 'moderator notification',
          '1' => 'boinc_standard',
        ),
        '#status' => 'default',
        '#conditions' => array(),
        '#actions' => array(
          '0' => array(
            '#weight' => 0,
            '#type' => 'action',
            '#settings' => array(
              'from' => '',
              'subject' => 'Comment deleted at [:global:site-name] by admin',
              'message' => "Comment to [node:type] topic '[node:title]' deleted by admin [user:display-name].\r\n\r\nLink: [node:node-url]",
              '#eval input' => array(
                'token_rules_input_evaluator' => array(
                  'subject' => array(
                    '0' => ':global',
                  ),
                  'message' => array(
                    '0' => 'node',
                    '1' => 'user',
                    '2' => ':global',
                  ),
                  'from' => array(
                    '0' => ':global',
                  ),
                ),
              ),
            ),
            '#name' => 'boinccore_rules_action_mail_to_moderators',
            '#info' => array(
              'label' => 'Notify moderators via email',
              'module' => 'BOINC core',
              'eval input' => array(
                '0' => 'subject',
                '1' => 'message',
                '2' => 'from',
              ),
            ),
          ),
        ),
        '#version' => 6003,
      ),
      'rules_offensive_comment_reported' => array(
        '#type' => 'rule',
        '#set' => 'event_flag_flagged_abuse_comment',
        '#label' => 'Offensive comment reported',
        '#active' => 1,
        '#weight' => '0',
        '#categories' => array(
          '0' => 'moderator notification',
          '1' => 'boinc_standard',
        ),
        '#status' => 'default',
        '#conditions' => array(),
        '#actions' => array(
          '1' => array(
            '#info' => array(
              'label' => 'Notify moderators via email',
              'module' => 'BOINC core',
              'eval input' => array(
                '0' => 'subject',
                '1' => 'message',
                '2' => 'from',
              ),
            ),
            '#name' => 'boinccore_rules_action_mail_to_moderators',
            '#settings' => array(
              'from' => '',
              'subject' => 'Report of offensive [node:type] comment at [:global:site-name]',
              'message' => '[flagging_user:display-name] has reported the following comment on [node:type] content as being offensive or inappropriate for the [:global:site-name] site:

[:global:site-url]/goto/comment/[comment:comment-cid]

Total reports of this comment: [comment:flag-abuse-comment-count]',
              '#eval input' => array(
                'token_rules_input_evaluator' => array(
                  'subject' => array(
                    '0' => 'node',
                    '1' => ':global',
                  ),
                  'message' => array(
                    '0' => 'flagging_user',
                    '1' => 'comment',
                    '2' => 'node',
                    '3' => ':global',
                  ),
                  'from' => array(
                    '0' => ':global',
                  ),
                ),
              ),
            ),
            '#type' => 'action',
            '#weight' => 0,
          ),
        ),
        '#version' => 6003,
      ),
      'rules_offensive_content_reported' => array(
        '#type' => 'rule',
        '#set' => 'event_flag_flagged_abuse_node',
        '#label' => 'Offensive content reported',
        '#active' => 1,
        '#weight' => '0',
        '#categories' => array(
          '0' => 'moderator notification',
          'boinc_standard' => 'boinc_standard',
        ),
        '#status' => 'default',
        '#conditions' => array(),
        '#actions' => array(
          '1' => array(
            '#type' => 'action',
            '#settings' => array(
              'from' => '',
              'subject' => 'Report of offensive [node:type] content at [:global:site-name]',
              'message' => '[flagging_user:display-name] has reported the following [node:type] content as being offensive or inappropriate for the [:global:site-name] site:

[:global:site-url]/[node:node-path]

Total reports of this content: [node:flag-abuse-node-count]',
              '#eval input' => array(
                'token_rules_input_evaluator' => array(
                  'subject' => array(
                    '0' => 'node',
                    '1' => ':global',
                  ),
                  'message' => array(
                    '0' => 'flagging_user',
                    '1' => 'node',
                    '2' => ':global',
                  ),
                  'from' => array(
                    '0' => ':global',
                  ),
                ),
              ),
            ),
            '#name' => 'boinccore_rules_action_mail_to_moderators',
            '#info' => array(
              'label' => 'Notify moderators via email',
              'module' => 'BOINC core',
              'eval input' => array(
                '0' => 'subject',
                '1' => 'message',
                '2' => 'from',
              ),
            ),
            '#weight' => 0,
          ),
        ),
        '#version' => 6003,
      ),
      'rules_offensive_user_reported' => array(
        '#type' => 'rule',
        '#set' => 'event_flag_flagged_abuse_user',
        '#label' => 'Offensive user reported',
        '#active' => 1,
        '#weight' => '0',
        '#categories' => array(
          '0' => 'moderator notification',
          'boinc_standard' => 'boinc_standard',
        ),
        '#status' => 'default',
        '#conditions' => array(),
        '#actions' => array(
          '1' => array(
            '#type' => 'action',
            '#settings' => array(
              'from' => '',
              'subject' => 'Report of offensive user at [:global:site-name]',
              'message' => '[flagging_user:display-name] has reported [account:display-name] for inappropriate behavior:

[:global:site-url]/account/[account:uid]

Total current reports of this user: [account:flag-abuse-user-count]',
              '#eval input' => array(
                'token_rules_input_evaluator' => array(
                  'subject' => array(
                    '0' => ':global',
                  ),
                  'message' => array(
                    '0' => 'flagging_user',
                    '1' => 'account',
                    '2' => ':global',
                  ),
                  'from' => array(
                    '0' => ':global',
                  ),
                ),
              ),
            ),
            '#name' => 'boinccore_rules_action_mail_to_moderators',
            '#info' => array(
              'label' => 'Notify moderators via email',
              'module' => 'BOINC core',
              'eval input' => array(
                '0' => 'subject',
                '1' => 'message',
                '2' => 'from',
              ),
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
function boinc_standard_views_api() {
  return array(
    'api' => '2',
  );
}
