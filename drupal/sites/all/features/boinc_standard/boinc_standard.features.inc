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
}

/**
 * Implementation of hook_flag_default_flags().
 */
function boinc_standard_flag_default_flags() {
  $flags = array();
  // Exported flag: "Comment Abuse 1 SPAM".
  $flags['abuse_comment_1'] = array(
    'content_type' => 'comment',
    'title' => 'Comment Abuse 1 SPAM',
    'global' => FALSE,
    'types' => array(
      '0' => 'forum',
      '1' => 'news',
      '2' => 'team_forum',
    ),
    'flag_short' => 'Flag as SPAM',
    'flag_long' => 'Flag this comment as SPAM.',
    'flag_message' => '',
    'unflag_short' => 'Unflag as SPAM',
    'unflag_long' => 'Unflag this comment as SPAM.',
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
    'show_on_comment' => 0,
    'module' => 'boinc_standard',
    'locked' => array(
      '0' => 'name',
    ),
    'api_version' => 2,
  );
  // Exported flag: "Comment Abuse 2 LINK".
  $flags['abuse_comment_2'] = array(
    'content_type' => 'comment',
    'title' => 'Comment Abuse 2 LINK',
    'global' => FALSE,
    'types' => array(
      '0' => 'forum',
      '1' => 'news',
      '2' => 'team_forum',
    ),
    'flag_short' => 'Flag as abusive link',
    'flag_long' => 'Flag this comment with abusive link.',
    'flag_message' => '',
    'unflag_short' => 'Unflag as abusive link',
    'unflag_long' => 'Unflag this comment with abusive link.',
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
    'show_on_comment' => 0,
    'module' => 'boinc_standard',
    'locked' => array(
      '0' => 'name',
    ),
    'api_version' => 2,
  );
  // Exported flag: "Comment Abuse 3 Hostile".
  $flags['abuse_comment_3'] = array(
    'content_type' => 'comment',
    'title' => 'Comment Abuse 3 Hostile',
    'global' => FALSE,
    'types' => array(
      '0' => 'forum',
      '1' => 'news',
      '2' => 'team_forum',
    ),
    'flag_short' => 'Flag as hostile',
    'flag_long' => 'Flag this comment as hostile.',
    'flag_message' => '',
    'unflag_short' => 'Unflag as hostile',
    'unflag_long' => 'Unflag this comment as hostile.',
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
    'show_on_comment' => 0,
    'module' => 'boinc_standard',
    'locked' => array(
      '0' => 'name',
    ),
    'api_version' => 2,
  );
  // Exported flag: "Comment Abuse 4 Not Kid Friendly".
  $flags['abuse_comment_4'] = array(
    'content_type' => 'comment',
    'title' => 'Comment Abuse 4 Not Kid Friendly',
    'global' => FALSE,
    'types' => array(
      '0' => 'forum',
      '1' => 'news',
      '2' => 'team_forum',
    ),
    'flag_short' => 'Flag as not kid-friendly',
    'flag_long' => 'Flag this comment as not kid-friendly.',
    'flag_message' => '',
    'unflag_short' => 'Unflag as not kid-friendly',
    'unflag_long' => 'Unflag this comment as not kid-friendly.',
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
    'show_on_comment' => 0,
    'module' => 'boinc_standard',
    'locked' => array(
      '0' => 'name',
    ),
    'api_version' => 2,
  );
  // Exported flag: "Comment Abuse 5 Other".
  $flags['abuse_comment_5'] = array(
    'content_type' => 'comment',
    'title' => 'Comment Abuse 5 Other',
    'global' => FALSE,
    'types' => array(
      '0' => 'forum',
      '1' => 'news',
      '2' => 'team_forum',
    ),
    'flag_short' => 'Flag as other',
    'flag_long' => 'Flag this comment - other reason',
    'flag_message' => '',
    'unflag_short' => 'Unflag as other',
    'unflag_long' => 'Unflag this comment - other reason',
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
    'show_on_comment' => 0,
    'module' => 'boinc_standard',
    'locked' => array(
      '0' => 'name',
    ),
    'api_version' => 2,
  );
  // Exported flag: "Comment Abuse meta".
  $flags['abuse_comment_meta'] = array(
    'content_type' => 'comment',
    'title' => 'Comment Abuse meta',
    'global' => '0',
    'types' => array(
      '0' => 'forum',
      '1' => 'news',
      '2' => 'team_forum',
    ),
    'flag_short' => 'Report',
    'flag_long' => 'Report comment.',
    'flag_message' => '',
    'unflag_short' => 'Cancel Report',
    'unflag_long' => 'Cancel Report on comment.',
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
  // Exported flag: "Node Abuse 1 SPAM".
  $flags['abuse_node_1'] = array(
    'content_type' => 'node',
    'title' => 'Node Abuse 1 SPAM',
    'global' => FALSE,
    'types' => array(
      '0' => 'forum',
      '1' => 'news',
      '2' => 'team_forum',
    ),
    'flag_short' => 'Flag as SPAM',
    'flag_long' => 'Flag this content SPAM.',
    'flag_message' => '',
    'unflag_short' => 'Unflag as SPAM',
    'unflag_long' => 'Unflag this content as SPAM.',
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
    'show_on_page' => 0,
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
  // Exported flag: "Node Abuse 2 Link".
  $flags['abuse_node_2'] = array(
    'content_type' => 'node',
    'title' => 'Node Abuse 2 Link',
    'global' => FALSE,
    'types' => array(
      '0' => 'forum',
      '1' => 'news',
      '2' => 'team_forum',
    ),
    'flag_short' => 'Report as abusive link',
    'flag_long' => 'Flag this content as abusive link.',
    'flag_message' => '',
    'unflag_short' => 'Unflag as abusive link',
    'unflag_long' => 'Unflag this content as abusive link.',
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
    'show_on_page' => 0,
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
  // Exported flag: "Node Abuse 3 Hostile".
  $flags['abuse_node_3'] = array(
    'content_type' => 'node',
    'title' => 'Node Abuse 3 Hostile',
    'global' => FALSE,
    'types' => array(
      '0' => 'forum',
      '1' => 'news',
      '2' => 'team_forum',
    ),
    'flag_short' => 'Flag as hostile',
    'flag_long' => 'Flag this content as hostile.',
    'flag_message' => '',
    'unflag_short' => 'Unflag as hostile',
    'unflag_long' => 'Unflag this content as hostile.',
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
    'show_on_page' => 0,
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
  // Exported flag: "Node Abuse 4 Not Kid Friendly".
  $flags['abuse_node_4'] = array(
    'content_type' => 'node',
    'title' => 'Node Abuse 4 Not Kid Friendly',
    'global' => FALSE,
    'types' => array(
      '0' => 'forum',
      '1' => 'news',
      '2' => 'team_forum',
    ),
    'flag_short' => 'Flag as not kid-friendly',
    'flag_long' => 'Flag this content as not kid-friendly.',
    'flag_message' => '',
    'unflag_short' => 'Unflag as kid-friendly',
    'unflag_long' => 'Unflag this content as not kid-friendly.',
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
    'show_on_page' => 0,
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
  // Exported flag: "Node Abuse 5 Other".
  $flags['abuse_node_5'] = array(
    'content_type' => 'node',
    'title' => 'Node Abuse 5 Other',
    'global' => FALSE,
    'types' => array(
      '0' => 'forum',
      '1' => 'news',
      '2' => 'team_forum',
    ),
    'flag_short' => 'Flag as other',
    'flag_long' => 'Flag this content - other reason.',
    'flag_message' => '',
    'unflag_short' => 'Unflag as other',
    'unflag_long' => 'Unflag this content - other reason.',
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
    'show_on_page' => 0,
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
  // Exported flag: "Node Abuse meta".
  $flags['abuse_node_meta'] = array(
    'content_type' => 'node',
    'title' => 'Node Abuse meta',
    'global' => FALSE,
    'types' => array(
      '0' => 'forum',
      '1' => 'news',
      '2' => 'team_forum',
    ),
    'flag_short' => 'Report',
    'flag_long' => 'Report content',
    'flag_message' => '',
    'unflag_short' => 'Cancel Report',
    'unflag_long' => 'Cancel report of content',
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
  // Exported flag: "User Abuse 1 SPAM".
  $flags['abuse_user_1'] = array(
    'content_type' => 'user',
    'title' => 'User Abuse 1 SPAM',
    'global' => FALSE,
    'types' => array(),
    'flag_short' => 'Report User SPAM',
    'flag_long' => 'Report this user profile as SPAM.',
    'flag_message' => '',
    'unflag_short' => 'Remove Report',
    'unflag_long' => 'Remove your report about this user.',
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
    'show_on_profile' => 0,
    'access_uid' => '',
    'module' => 'boinc_standard',
    'locked' => array(
      '0' => 'name',
    ),
    'api_version' => 2,
  );
  // Exported flag: "User Abuse 2 Link".
  $flags['abuse_user_2'] = array(
    'content_type' => 'user',
    'title' => 'User Abuse 2 Link',
    'global' => FALSE,
    'types' => array(),
    'flag_short' => 'Report as abusive link',
    'flag_long' => 'Report user profile as abusive link.',
    'flag_message' => '',
    'unflag_short' => 'Remove Report',
    'unflag_long' => 'Remove your report about this user.',
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
    'show_on_profile' => 0,
    'access_uid' => '',
    'module' => 'boinc_standard',
    'locked' => array(
      '0' => 'name',
    ),
    'api_version' => 2,
  );
  // Exported flag: "User Abuse 3 Not Kid Friendly".
  $flags['abuse_user_3'] = array(
    'content_type' => 'user',
    'title' => 'User Abuse 3 Not Kid Friendly',
    'global' => FALSE,
    'types' => array(),
    'flag_short' => 'Report as not kid-friendly',
    'flag_long' => 'Report this user profile as not kid-friendly.',
    'flag_message' => '',
    'unflag_short' => 'Remove Report',
    'unflag_long' => 'Remove your report about this user.',
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
    'show_on_profile' => 0,
    'access_uid' => '',
    'module' => 'boinc_standard',
    'locked' => array(
      '0' => 'name',
    ),
    'api_version' => 2,
  );
  // Exported flag: "User Abuse 4 Other".
  $flags['abuse_user_4'] = array(
    'content_type' => 'user',
    'title' => 'User Abuse 4 Other',
    'global' => FALSE,
    'types' => array(),
    'flag_short' => 'Other',
    'flag_long' => 'Report this user profile - other reason.',
    'flag_message' => '',
    'unflag_short' => 'Remove Report',
    'unflag_long' => 'Remove your report about this user.',
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
    'show_on_profile' => 0,
    'access_uid' => '',
    'module' => 'boinc_standard',
    'locked' => array(
      '0' => 'name',
    ),
    'api_version' => 2,
  );
  // Exported flag: "User Abuse Meta".
  $flags['abuse_user_meta'] = array(
    'content_type' => 'user',
    'title' => 'User Abuse Meta',
    'global' => FALSE,
    'types' => array(),
    'flag_short' => 'Report User',
    'flag_long' => 'Report User',
    'flag_message' => '',
    'unflag_short' => 'Cancel Report',
    'unflag_long' => 'Cancel report of user',
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
    'show_on_profile' => 1,
    'access_uid' => '',
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
      'rules_notify_admins_about_circular_merge_error' => array(
        '#type' => 'rule',
        '#set' => 'event_boincwork_circular_merge_error',
        '#label' => 'Notify admins about circular merge error',
        '#active' => 1,
        '#weight' => '0',
        '#categories' => array(
          '0' => 'boinc_standard',
          '1' => 'admin notification',
        ),
        '#status' => 'default',
        '#conditions' => array(),
        '#actions' => array(
          '0' => array(
            '#type' => 'action',
            '#settings' => array(
              'from' => '',
              'subject' => '[subject_tag:string] Circular host merging error',
              'message' => 'User tried to merge a host (old_host) into a target host where the target host\'s rpc_seqno is equal to the old_host\'s id.

Information about this host merge attempt:
<?php
print "Old host id: " . $oldhost_id . "\\n";
print "Target host id: " . $targethost_id . "\\n";
$account = user_load($user->uid);
print "Username: " . $account->boincuser_name . "\\n";
print "BOINC id: " . $account->boincuser_id . "\\n";
print "Drupal UID: " . $user->uid . "\\n";
?>',
              '#eval input' => array(
                'token_rules_input_evaluator' => array(
                  'subject' => array(
                    '0' => 'subject_tag',
                    '1' => ':global',
                  ),
                  'message' => array(
                    '0' => ':global',
                  ),
                  'from' => array(
                    '0' => ':global',
                  ),
                ),
                'rules_input_evaluator_php' => array(
                  'message' => array(
                    '0' => 'oldhost_id',
                    '1' => 'targethost_id',
                    '2' => 'user',
                  ),
                ),
              ),
            ),
            '#name' => 'boinccore_rules_action_mail_to_admins',
            '#info' => array(
              'label' => 'Notify admins via email',
              'module' => 'BOINC core',
              'eval input' => array(
                '0' => 'subject',
                '1' => 'message',
                '2' => 'from',
              ),
            ),
            '#weight' => 0.0,
          ),
        ),
        '#version' => 6003,
      ),
      'rules_notify_admins_about_zombie_merge_attempt' => array(
        '#type' => 'rule',
        '#set' => 'event_boincwork_zombie_merge_error',
        '#label' => 'Notify admins about zombie merge attempt',
        '#active' => 1,
        '#weight' => '0',
        '#categories' => array(
          '0' => 'boinc_standard',
          '1' => 'admin notification',
        ),
        '#status' => 'default',
        '#conditions' => array(),
        '#actions' => array(
          '0' => array(
            '#type' => 'action',
            '#settings' => array(
              'from' => '',
              'subject' => '[subject_tag:string] Zombie host merging error',
              'message' => 'User tried to merge a host (old_host) into a target host where the target host\'s userid is equal to 0, a zombie host.
I.e., $target_host->userid == 0

Information about this host merge attempt:
<?php
print "Old host id: " . $oldhost_id . "\\n";
print "Target host id: " . $targethost_id . "\\n";
$account = user_load($user->uid);
print "Username: " . $account->boincuser_name . "\\n";
print "BOINC id: " . $account->boincuser_id . "\\n";
print "Drupal UID: " . $user->uid . "\\n";
?>',
              '#eval input' => array(
                'token_rules_input_evaluator' => array(
                  'subject' => array(
                    '0' => 'subject_tag',
                    '1' => ':global',
                  ),
                  'message' => array(
                    '0' => ':global',
                  ),
                  'from' => array(
                    '0' => ':global',
                  ),
                ),
                'rules_input_evaluator_php' => array(
                  'message' => array(
                    '0' => 'oldhost_id',
                    '1' => 'targethost_id',
                    '2' => 'user',
                  ),
                ),
              ),
            ),
            '#name' => 'boinccore_rules_action_mail_to_admins',
            '#info' => array(
              'label' => 'Notify admins via email',
              'module' => 'BOINC core',
              'eval input' => array(
                '0' => 'subject',
                '1' => 'message',
                '2' => 'from',
              ),
            ),
            '#weight' => 0.0,
          ),
        ),
        '#version' => 6003,
      ),
      'rules_notify_admins_error_when_user_consenting_with_missing_consent_type' => array(
        '#type' => 'rule',
        '#set' => 'event_boincuser_general_consent_type_error',
        '#label' => 'Notify admins error when user consenting with missing consent type',
        '#active' => 1,
        '#weight' => '0',
        '#categories' => array(
          '0' => 'boinc_standard',
          '1' => 'admin notification',
        ),
        '#status' => 'default',
        '#conditions' => array(),
        '#actions' => array(
          '0' => array(
            '#type' => 'action',
            '#settings' => array(
              'from' => '',
              'subject' => '[subject_tag:string] Error from user consenting to terms of use, missing consent type',
              'message' => '[user:display-name], [user:mail], attempted to consent to terms-of-use form and the function \'check_consent_type\' returned an error. Consent type [consent_type:string] seems to be generating an error.',
              '#eval input' => array(
                'token_rules_input_evaluator' => array(
                  'subject' => array(
                    '0' => 'subject_tag',
                    '1' => ':global',
                  ),
                  'message' => array(
                    '0' => 'consent_type',
                    '1' => 'user',
                    '2' => ':global',
                  ),
                  'from' => array(
                    '0' => ':global',
                  ),
                ),
              ),
            ),
            '#name' => 'boinccore_rules_action_mail_to_admins',
            '#info' => array(
              'label' => 'Notify admins via email',
              'module' => 'BOINC core',
              'eval input' => array(
                '0' => 'subject',
                '1' => 'message',
                '2' => 'from',
              ),
            ),
            '#weight' => 0.0,
          ),
        ),
        '#version' => 6003,
      ),
      'rules_notify_admins_user_changing_email_address_to_existing_user' => array(
        '#type' => 'rule',
        '#set' => 'event_boincuser_change_email_error',
        '#label' => 'Notify admins user changing email address to existing user',
        '#active' => 1,
        '#weight' => '0',
        '#categories' => array(
          '0' => 'boinc_standard',
          '1' => 'admin notification',
        ),
        '#status' => 'default',
        '#conditions' => array(),
        '#actions' => array(
          '0' => array(
            '#type' => 'action',
            '#settings' => array(
              'from' => '',
              'subject' => '[subject_tag:string] User attempted to change email address to one used by existing user.',
              'message' => '[user:display-name]	with email [user:mail] attempted to change their email to [new_email_addr:string], which is already in use by another user.',
              '#eval input' => array(
                'token_rules_input_evaluator' => array(
                  'subject' => array(
                    '0' => 'subject_tag',
                    '1' => ':global',
                  ),
                  'message' => array(
                    '0' => 'new_email_addr',
                    '1' => 'user',
                    '2' => ':global',
                  ),
                  'from' => array(
                    '0' => ':global',
                  ),
                ),
              ),
            ),
            '#name' => 'boinccore_rules_action_mail_to_admins',
            '#info' => array(
              'label' => 'Notify admins via email',
              'module' => 'BOINC core',
              'eval input' => array(
                '0' => 'subject',
                '1' => 'message',
                '2' => 'from',
              ),
            ),
            '#weight' => 0.0,
          ),
        ),
        '#version' => 6003,
      ),
      'rules_notify_admins_user_reverting_to_previous_email_address_that_is_now_used_by_another_user' => array(
        '#type' => 'rule',
        '#set' => 'event_boincuser_revert_email_error',
        '#label' => 'Notify admins user reverting to previous email address that is now used by another user',
        '#active' => 1,
        '#weight' => '0',
        '#categories' => array(
          '0' => 'boinc_standard',
          '1' => 'admin notification',
        ),
        '#status' => 'default',
        '#conditions' => array(),
        '#actions' => array(
          '0' => array(
            '#type' => 'action',
            '#settings' => array(
              'from' => '',
              'subject' => '[subject_tag:string] User attempted to revert to previous email address to one used by existing user.',
              'message' => '[user:display-name]	with email [user:mail] attempted to revert their email to their previous email address, [new_email_addr:string], which is already in use by another user.',
              '#eval input' => array(
                'token_rules_input_evaluator' => array(
                  'subject' => array(
                    '0' => 'subject_tag',
                    '1' => ':global',
                  ),
                  'message' => array(
                    '0' => 'new_email_addr',
                    '1' => 'user',
                    '2' => ':global',
                  ),
                  'from' => array(
                    '0' => ':global',
                  ),
                ),
              ),
            ),
            '#name' => 'boinccore_rules_action_mail_to_admins',
            '#info' => array(
              'label' => 'Notify admins via email',
              'module' => 'BOINC core',
              'eval input' => array(
                '0' => 'subject',
                '1' => 'message',
                '2' => 'from',
              ),
            ),
            '#weight' => 0.0,
          ),
        ),
        '#version' => 6003,
      ),
      'rules_notify_admins_when_user_fails_to_delete_their_own_account' => array(
        '#type' => 'rule',
        '#set' => 'event_boincuser_delete_general_error',
        '#label' => 'Notify admins when user fails to delete their own account',
        '#active' => 1,
        '#weight' => '0',
        '#categories' => array(
          '0' => 'boinc_standard',
          '1' => 'admin notification',
        ),
        '#status' => 'default',
        '#conditions' => array(),
        '#actions' => array(
          '0' => array(
            '#type' => 'action',
            '#settings' => array(
              'from' => '',
              'subject' => '[subject_tag:string] Error when user trying to delete their account',
              'message' => 'User [user:display-name], [user:mail], attempted to delete their account. Error from \'boincuser_load\'.',
              '#eval input' => array(
                'token_rules_input_evaluator' => array(
                  'subject' => array(
                    '0' => 'subject_tag',
                    '1' => ':global',
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
            '#name' => 'boinccore_rules_action_mail_to_admins',
            '#info' => array(
              'label' => 'Notify admins via email',
              'module' => 'BOINC core',
              'eval input' => array(
                '0' => 'subject',
                '1' => 'message',
                '2' => 'from',
              ),
            ),
            '#weight' => 0.0,
          ),
        ),
        '#version' => 6003,
      ),
      'rules_notify_admins_when_user_tries_to_delete_account_with_invalid_uid' => array(
        '#type' => 'rule',
        '#set' => 'event_boincuser_delete_uid_invalid_error',
        '#label' => 'Notify admins when user tries to delete account with invalid uid',
        '#active' => 1,
        '#weight' => '0',
        '#categories' => array(
          '0' => 'boinc_standard',
          '1' => 'admin notification',
        ),
        '#status' => 'default',
        '#conditions' => array(),
        '#actions' => array(
          '0' => array(
            '#type' => 'action',
            '#settings' => array(
              'from' => '',
              'subject' => '[subject_tag:string] invalid uid when user trying to delete their account',
              'message' => 'User [user:display-name], [user:mail], attempted to delete their account. Error is Drupal UID=[user:uid] cannot be located.',
              '#eval input' => array(
                'token_rules_input_evaluator' => array(
                  'subject' => array(
                    '0' => 'subject_tag',
                    '1' => ':global',
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
            '#name' => 'boinccore_rules_action_mail_to_admins',
            '#info' => array(
              'label' => 'Notify admins via email',
              'module' => 'BOINC core',
              'eval input' => array(
                '0' => 'subject',
                '1' => 'message',
                '2' => 'from',
              ),
            ),
            '#weight' => 0.0,
          ),
        ),
        '#version' => 6003,
      ),
      'rules_notify_admin_error_when_user_consenting_to_terms_of_use' => array(
        '#type' => 'rule',
        '#set' => 'event_boincuser_general_consent_error',
        '#label' => 'Notify admins error when user consenting to terms of use',
        '#active' => 1,
        '#weight' => '0',
        '#categories' => array(
          '0' => 'boinc_standard',
          '1' => 'admin notification',
        ),
        '#status' => 'default',
        '#conditions' => array(),
        '#actions' => array(
          '0' => array(
            '#type' => 'action',
            '#settings' => array(
              'from' => '',
              'subject' => '[subject_tag:string] Error when user attempted to consent to terms of use',
              'message' => '[user:display-name], [user:mail], attempted to consent to terms-of-use form and an error was generated from function \'consent_to_a_policy\'.

asdf jkl;',
              '#eval input' => array(
                'token_rules_input_evaluator' => array(
                  'subject' => array(
                    '0' => 'subject_tag',
                    '1' => ':global',
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
            '#name' => 'boinccore_rules_action_mail_to_admins',
            '#info' => array(
              'label' => 'Notify admins via email',
              'module' => 'BOINC core',
              'eval input' => array(
                '0' => 'subject',
                '1' => 'message',
                '2' => 'from',
              ),
            ),
            '#weight' => 0.0,
          ),
        ),
        '#version' => 6003,
      ),
      'rules_offensive_comment_reported_hostile' => array(
        '#type' => 'rule',
        '#set' => 'event_flag_flagged_abuse_comment_3',
        '#label' => 'Offensive comment reported - Hostile',
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
            '#type' => 'action',
            '#settings' => array(
              'from' => '',
              'subject' => 'Report of hostile language in [node:type] comment at [:global:site-name]',
              'message' => '[flagging_user:display-name] has reported the following comment on [node:type] content as
containing hostile language for the [:global:site-name] site:

<?php print url(\'<front>\', array(\'absolute\' => TRUE, \'language\' => \'en\')); ?>/goto/comment/[comment:comment-cid]

Total reports of this comment: [comment:flag-abuse-comment-meta-count]',
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
                'rules_input_evaluator_php' => array(
                  'message' => array(),
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
            '#weight' => 0.0,
          ),
        ),
        '#version' => 6003,
      ),
      'rules_offensive_comment_reported_link' => array(
        '#type' => 'rule',
        '#set' => 'event_flag_flagged_abuse_comment_2',
        '#label' => 'Offensive comment reported - Link',
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
            '#type' => 'action',
            '#settings' => array(
              'from' => '',
              'subject' => 'Report of inappropriate link in  [node:type] comment at [:global:site-name]',
              'message' => '[flagging_user:display-name] has reported the following comment on [node:type] content as
containing an offensive or inappropriate link for the [:global:site-name] site:

<?php print url(\'<front>\', array(\'absolute\' => TRUE, \'language\' => \'en\')); ?>/goto/comment/[comment:comment-cid]

Total reports of this comment: [comment:flag-abuse-comment-meta-count]',
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
                'rules_input_evaluator_php' => array(
                  'message' => array(),
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
            '#weight' => 0.0,
          ),
        ),
        '#version' => 6003,
      ),
      'rules_offensive_comment_reported_not_kid_friendly' => array(
        '#type' => 'rule',
        '#set' => 'event_flag_flagged_abuse_comment_4',
        '#label' => 'Offensive comment reported - Not Kid Friendly',
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
            '#type' => 'action',
            '#settings' => array(
              'from' => '',
              'subject' => 'Report of non Kid-friendly language in [node:type] comment at [:global:site-name]',
              'message' => '[flagging_user:display-name] has reported the following comment on [node:type] content as
containing non Kid-friendly language for the [:global:site-name] site:

<?php print url(\'<front>\', array(\'absolute\' => TRUE, \'language\' => \'en\')); ?>/goto/comment/[comment:comment-cid]

Total reports of this comment: [comment:flag-abuse-comment-meta-count]',
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
                'rules_input_evaluator_php' => array(
                  'message' => array(),
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
            '#weight' => 0.0,
          ),
        ),
        '#version' => 6003,
      ),
      'rules_offensive_comment_reported_other' => array(
        '#type' => 'rule',
        '#set' => 'event_flag_flagged_abuse_comment_5',
        '#label' => 'Offensive comment reported - Other',
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
            '#type' => 'action',
            '#settings' => array(
              'from' => '',
              'subject' => 'Report of offensive [node:type] comment at [:global:site-name]',
              'message' => '[flagging_user:display-name] has reported the following comment on [node:type] content as
being offensive or inappropriate for the [:global:site-name] site, with reason \'Other\':

<?php print url(\'<front>\', array(\'absolute\' => TRUE, \'language\' => \'en\')); ?>/goto/comment/[comment:comment-cid]

Total reports of this comment: [comment:flag-abuse-comment-meta-count]',
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
                'rules_input_evaluator_php' => array(
                  'message' => array(),
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
            '#weight' => 0.0,
          ),
        ),
        '#version' => 6003,
      ),
      'rules_offensive_comment_reported_spam' => array(
        '#type' => 'rule',
        '#set' => 'event_flag_flagged_abuse_comment_1',
        '#label' => 'Offensive comment reported - SPAM',
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
            '#type' => 'action',
            '#settings' => array(
              'from' => '',
              'subject' => 'Report of SPAM [node:type] comment at [:global:site-name]',
              'message' => '[flagging_user:display-name] has reported the following comment on [node:type] content
as being SPAM for the [:global:site-name] site:

<?php print url(\'<front>\', array(\'absolute\' => TRUE, \'language\' => \'en\')); ?>/goto/comment/[comment:comment-cid]

Total reports of this comment: [comment:flag-abuse-comment-meta-count]',
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
                'rules_input_evaluator_php' => array(
                  'message' => array(),
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
            '#weight' => 0.0,
          ),
        ),
        '#version' => 6003,
      ),
      'rules_offensive_content_reported_hostile' => array(
        '#type' => 'rule',
        '#set' => 'event_flag_flagged_abuse_node_3',
        '#label' => 'Offensive content reported - Hostile',
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
            '#type' => 'action',
            '#settings' => array(
              'from' => '',
              'subject' => 'Report of hostile language in [node:type] content at [:global:site-name]',
              'message' => '[flagging_user:display-name] has reported the following [node:type] content
as hostile language for the [:global:site-name] site:

<?php print url(\'node/\' . \\$node->nid, array(\'absolute\' => TRUE, \'language\' => \'en\')); ?>

Total reports of this content: [node:flag-abuse-node-meta-count]',
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
                'rules_input_evaluator_php' => array(
                  'message' => array(
                    '0' => 'node',
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
            '#weight' => 0.0,
          ),
        ),
        '#version' => 6003,
      ),
      'rules_offensive_content_reported_link' => array(
        '#type' => 'rule',
        '#set' => 'event_flag_flagged_abuse_node_2',
        '#label' => 'Offensive content reported - Link',
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
            '#type' => 'action',
            '#settings' => array(
              'from' => '',
              'subject' => 'Report of inappropriate link in [node:type] content at [:global:site-name]',
              'message' => '[flagging_user:display-name] has reported the following [node:type] content
as containing an offensive or inappropriate link for the [:global:site-name] site:

<?php print url(\'node/\' . \\$node->nid, array(\'absolute\' => TRUE, \'language\' => \'en\')); ?>

Total reports of this content: [node:flag-abuse-node-meta-count]',
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
                'rules_input_evaluator_php' => array(
                  'message' => array(
                    '0' => 'node',
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
            '#weight' => 0.0,
          ),
        ),
        '#version' => 6003,
      ),
      'rules_offensive_content_reported_not_kid_friendly' => array(
        '#type' => 'rule',
        '#set' => 'event_flag_flagged_abuse_node_4',
        '#label' => 'Offensive content reported - Not Kid Friendly',
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
            '#type' => 'action',
            '#settings' => array(
              'from' => '',
              'subject' => 'Report of non Kid-friendly language in [node:type] content at [:global:site-name]',
              'message' => '[flagging_user:display-name] has reported the following [node:type] content
as containing non Kid-friendly language for the [:global:site-name] site:

<?php print url(\'node/\' . \\$node->nid, array(\'absolute\' => TRUE, \'language\' => \'en\')); ?>

Total reports of this content: [node:flag-abuse-node-meta-count]',
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
                'rules_input_evaluator_php' => array(
                  'message' => array(
                    '0' => 'node',
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
            '#weight' => 0.0,
          ),
        ),
        '#version' => 6003,
      ),
      'rules_offensive_content_reported_other' => array(
        '#type' => 'rule',
        '#set' => 'event_flag_flagged_abuse_node_4',
        '#label' => 'Offensive content reported - Other',
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
            '#type' => 'action',
            '#settings' => array(
              'from' => '',
              'subject' => 'Report of offensive [node:type] content at [:global:site-name]',
              'message' => '[flagging_user:display-name] has reported the following [node:type] content
as being offensive or inappropriate for the [:global:site-name] site:

<?php print url(\'node/\' . \\$node->nid, array(\'absolute\' => TRUE, \'language\' => \'en\')); ?>

Total reports of this content: [node:flag-abuse-node-meta-count]',
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
                'rules_input_evaluator_php' => array(
                  'message' => array(
                    '0' => 'node',
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
            '#weight' => 0.0,
          ),
        ),
        '#version' => 6003,
      ),
      'rules_offensive_content_reported_spam' => array(
        '#type' => 'rule',
        '#set' => 'event_flag_flagged_abuse_node_1',
        '#label' => 'Offensive content reported - SPAM',
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
            '#type' => 'action',
            '#settings' => array(
              'from' => '',
              'subject' => 'Report of SPAM [node:type] content at [:global:site-name]',
              'message' => '[flagging_user:display-name] has reported the following [node:type] content
as being SPAM for the [:global:site-name] site:

<?php print url(\'node/\' . \\$node->nid, array(\'absolute\' => TRUE, \'language\' => \'en\')); ?>

Total reports of this content: [node:flag-abuse-node-meta-count]',
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
                'rules_input_evaluator_php' => array(
                  'message' => array(
                    '0' => 'node',
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
            '#weight' => 0.0,
          ),
        ),
        '#version' => 6003,
      ),
      'rules_offensive_user_reported_link' => array(
        '#type' => 'rule',
        '#set' => 'event_flag_flagged_abuse_user_2',
        '#label' => 'Offensive user reported - Link',
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
            '#type' => 'action',
            '#settings' => array(
              'from' => '',
              'subject' => 'Report of inappropriate link in user profile at [:global:site-name]',
              'message' => '[flagging_user:display-name] has reported [account:display-name] for
inappropriate link in a user profile:

<?php print url(\\\'<front>\\\', array(\\\'absolute\\\' => TRUE, \\\'language\\\' => \\\'en\\\')); ?>/account/[account:uid]

Total current reports of this user: [account:flag-abuse-user-meta-count]',
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
                'rules_input_evaluator_php' => array(
                  'message' => array(),
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
            '#weight' => 0.0,
          ),
        ),
        '#version' => 6003,
      ),
      'rules_offensive_user_reported_not_kid_friendly' => array(
        '#type' => 'rule',
        '#set' => 'event_flag_flagged_abuse_user_3',
        '#label' => 'Offensive user reported - Not Kid Friendly',
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
            '#type' => 'action',
            '#settings' => array(
              'from' => '',
              'subject' => 'Report of hostile language in user profile at [:global:site-name]',
              'message' => '[flagging_user:display-name] has reported [account:display-name] for
hostile language in a user profile:

<?php print url(\\\'<front>\\\', array(\\\'absolute\\\' => TRUE, \\\'language\\\' => \\\'en\\\')); ?>/account/[account:uid]

Total current reports of this user: [account:flag-abuse-user-meta-count]',
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
                'rules_input_evaluator_php' => array(
                  'message' => array(),
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
            '#weight' => 0.0,
          ),
        ),
        '#version' => 6003,
      ),
      'rules_offensive_user_reported_other' => array(
        '#type' => 'rule',
        '#set' => 'event_flag_flagged_abuse_user_4',
        '#label' => 'Offensive user reported - Other',
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
            '#type' => 'action',
            '#settings' => array(
              'from' => '',
              'subject' => 'Report of offensive user at [:global:site-name]',
              'message' => '[flagging_user:display-name] has reported [account:display-name] for
inappropriate behavior:

<?php print url(\\\'<front>\\\', array(\\\'absolute\\\' => TRUE, \\\'language\\\' => \\\'en\\\')); ?>/account/[account:uid]

Total current reports of this user: [account:flag-abuse-user-meta-count]',
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
                'rules_input_evaluator_php' => array(
                  'message' => array(),
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
            '#weight' => 0.0,
          ),
        ),
        '#version' => 6003,
      ),
      'rules_offensive_user_reported_spam' => array(
        '#type' => 'rule',
        '#set' => 'event_flag_flagged_abuse_user_1',
        '#label' => 'Offensive user reported - SPAM',
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
            '#type' => 'action',
            '#settings' => array(
              'from' => '',
              'subject' => 'Report of SPAM user at [:global:site-name]',
              'message' => '[flagging_user:display-name] has reported [account:display-name] for
a SPAM user profile:

<?php print url(\\\'<front>\\\', array(\\\'absolute\\\' => TRUE, \\\'language\\\' => \\\'en\\\')); ?>/account/[account:uid]

Total current reports of this user: [account:flag-abuse-user-meta-count]',
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
                'rules_input_evaluator_php' => array(
                  'message' => array(),
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
            '#weight' => 0.0,
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
              'message' => "Comment has been converted to new forum topic by moderator/admin [user:display-name].\r\n\r\nLink: <?php print url('node/' . \$node->nid, array('absolute' => TRUE, 'language' => 'en')); ?>",
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
                'rules_input_evaluator_php' => array(
                  'message' => array(
                    '0' => 'node',
                  ),
                ),
              ),
            ),
            '#type' => 'action',
            '#weight' => 0.0,
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
            '#type' => 'action',
            '#settings' => array(
              'from' => '',
              'subject' => 'Comment deleted at [:global:site-name] by admin',
              'message' => "Comment to [node:type] topic '[node:title]' deleted by admin [user:display-name].\r\n\r\nLink: <?php print url('node/' . \$node->nid, array('absolute' => TRUE, 'language' => 'en')); ?>",
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
                'rules_input_evaluator_php' => array(
                  'message' => array(
                    '0' => 'node',
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
            '#weight' => 0.0,
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
            '#weight' => 0.0,
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
            '#type' => 'action',
            '#settings' => array(
              'from' => '',
              'subject' => 'Comment edited at [:global:site-name] by moderator or admin',
              'message' => "Comment has been edited by moderator/admin [user:display-name]\r\n\r\nLink: <?php print url('<front>', array('absolute' => TRUE, 'language' => 'en')); ?>/goto/comment/[comment:comment-cid]",
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
                'rules_input_evaluator_php' => array(
                  'message' => array(),
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
            '#weight' => 0.0,
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
            '#weight' => 0.0,
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
              'subject' => 'Comment at [:global:site-name] unhidden by moderator or admin',
              'message' => "Comment has been unhidden by moderator/admin [user:display-name].\r\n\r\nLink: <?php print url('<front>', array('absolute' => TRUE, 'language' => 'en')); ?>/goto/comment/[comment:comment-cid]",
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
                'rules_input_evaluator_php' => array(
                  'message' => array(),
                ),
              ),
            ),
            '#type' => 'action',
            '#weight' => 0.0,
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
            '#weight' => 0.0,
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
            '#type' => 'action',
            '#settings' => array(
              'from' => '',
              'subject' => 'Comment at [:global:site-name] hidden by moderator or admin',
              'message' => "Comment has been hidden by moderator/admin [user:display-name].\r\n\r\nLink: <?php print url('<front>', array('absolute' => TRUE, 'language' => 'en')); ?>/goto/comment/[comment:comment-cid]",
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
                'rules_input_evaluator_php' => array(
                  'message' => array(),
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
            '#weight' => 0.0,
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
            '#weight' => 0.0,
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
            '#weight' => 0.0,
            '0' => array(
              '#weight' => 0.0,
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
              '#weight' => 0.0,
            ),
          ),
          '2' => array(
            '#weight' => 0.0,
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
            '#type' => 'action',
            '#settings' => array(
              'from' => '',
              'subject' => 'Forum topic at [:global:site-name] edited by moderator/admin',
              'message' => "[node:type] topic '[node:title]' has been edited by moderator/admin [user:display-name].\r\n\r\nLink: <?php print url('node/' . \$node->nid, array('absolute' => TRUE, 'language' => 'en')); ?>",
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
                'rules_input_evaluator_php' => array(
                  'message' => array(
                    '0' => 'node',
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
            '#weight' => 0.0,
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
            '#weight' => 0.0,
          ),
          '1' => array(
            '#weight' => 0.0,
            '0' => array(
              '#weight' => 0.0,
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
              '#weight' => 0.0,
            ),
          ),
          '3' => array(
            '#weight' => 0.0,
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
            '#weight' => 0.0,
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
              'message' => "[node:type] topic '[node:title]' has been hidden by moderator/admin [user:display-name].\r\n\r\nLink: <?php print url('node/' . \$node->nid, array('absolute' => TRUE, 'language' => 'en')); ?>",
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
                'rules_input_evaluator_php' => array(
                  'message' => array(
                    '0' => 'node',
                  ),
                ),
              ),
            ),
            '#type' => 'action',
            '#weight' => 0.0,
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
            '#weight' => 0.0,
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
            '#weight' => 0.0,
            '0' => array(
              '#weight' => 0.0,
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
              '#weight' => 0.0,
            ),
          ),
          '2' => array(
            '#weight' => 0.0,
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
            '#type' => 'action',
            '#settings' => array(
              'from' => '',
              'subject' => 'Forum topic at [:global:site-name] moved by moderator/admin',
              'message' => "[node:type] topic '[node:title]' has been moved by moderator/admin [user:display-name].\r\n\r\nLink: <?php print url('node/' . \$node->nid, array('absolute' => TRUE, 'language' => 'en')); ?>",
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
                'rules_input_evaluator_php' => array(
                  'message' => array(
                    '0' => 'node',
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
            '#weight' => 0.0,
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
            '#weight' => 0.0,
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
              '#weight' => 0.0,
            ),
            '#weight' => 0.0,
            '1' => array(
              '#weight' => 0.0,
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
            '#weight' => 0.0,
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
            '#weight' => 0.0,
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
              'subject' => 'Forum topic at [:global:site-name] unhidden by moderator/admin',
              'message' => "[node:type] topic '[node:title]' has been unhidden by moderator/admin [user:display-name].\r\n\r\nLink: <?php print url('node/' . \$node->nid, array('absolute' => TRUE, 'language' => 'en')); ?>",
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
                'rules_input_evaluator_php' => array(
                  'message' => array(
                    '0' => 'node',
                  ),
                ),
              ),
            ),
            '#type' => 'action',
            '#weight' => 0.0,
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
            '#weight' => 0.0,
          ),
          '1' => array(
            '#weight' => 0.0,
            '0' => array(
              '#weight' => 0.0,
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
              '#weight' => 0.0,
            ),
          ),
          '3' => array(
            '#weight' => 0.0,
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
            '#weight' => 0.0,
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
              'message' => "[node:type] topic '[node:title]' has been locked by moderator/admin [user:display-name].\r\n\r\nLink: <?php print url('node/' . \$node->nid, array('absolute' => TRUE, 'language' => 'en')); ?>",
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
                'rules_input_evaluator_php' => array(
                  'message' => array(
                    '0' => 'node',
                  ),
                ),
              ),
            ),
            '#type' => 'action',
            '#weight' => 0.0,
          ),
        ),
        '#version' => 6003,
      ),
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
            '#weight' => 0.0,
          ),
          '1' => array(
            '#weight' => 0.0,
            '0' => array(
              '#weight' => 0.0,
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
              '#weight' => 0.0,
            ),
          ),
          '3' => array(
            '#weight' => 0.0,
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
            '#weight' => 0.0,
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
              'message' => "[node:type] topic '[node:title]' has been marked sticky by moderator/admin [user:display-name].\r\n\r\nLink: <?php print url('node/' . \$node->nid, array('absolute' => TRUE, 'language' => 'en')); ?>",
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
                'rules_input_evaluator_php' => array(
                  'message' => array(
                    '0' => 'node',
                  ),
                ),
              ),
            ),
            '#type' => 'action',
            '#weight' => 0.0,
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
            '#weight' => 0.0,
          ),
          '1' => array(
            '#weight' => 0.0,
            '0' => array(
              '#weight' => 0.0,
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
              '#weight' => 0.0,
            ),
          ),
          '3' => array(
            '#weight' => 0.0,
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
            '#weight' => 0.0,
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
              'message' => "[node:type] topic '[node:title]' has been marked unsticky by moderator/admin [user:display-name].\r\n\\r\nLink: <?php print url('node/' . \$node->nid, array('absolute' => TRUE, 'language' => 'en')); ?>",
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
                'rules_input_evaluator_php' => array(
                  'message' => array(
                    '0' => 'node',
                  ),
                ),
              ),
            ),
            '#type' => 'action',
            '#weight' => 0.0,
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
            '#weight' => 0.0,
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
            '#weight' => 0.0,
            '0' => array(
              '#weight' => 0.0,
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
              '#weight' => 0.0,
            ),
          ),
          '2' => array(
            '#weight' => 0.0,
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
            '#type' => 'action',
            '#settings' => array(
              'from' => '',
              'subject' => 'Forum topic at [:global:site-name] renamed by moderator/admin',
              'message' => "[node:type] topic '[node:title]' has its title renamed by moderator/admin [user:display-name].\r\n\r\nLink: <?php print url('node/' . \$node->nid, array('absolute' => TRUE, 'language' => 'en')); ?>",
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
                'rules_input_evaluator_php' => array(
                  'message' => array(
                    '0' => 'node',
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
            '#weight' => 0.0,
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
            '#weight' => 0.0,
          ),
          '1' => array(
            '#weight' => 0.0,
            '0' => array(
              '#weight' => 0.0,
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
              '#weight' => 0.0,
            ),
          ),
          '3' => array(
            '#weight' => 0.0,
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
            '#weight' => 0.0,
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
              'message' => "[node:type] topic '[node:title]' has been unlocked by moderator/admin [user:display-name].\r\n\r\nLink: <?php print url('node/' . \$node->nid, array('absolute' => TRUE, 'language' => 'en')); ?>",
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
                'rules_input_evaluator_php' => array(
                  'message' => array(
                    '0' => 'node',
                  ),
                ),
              ),
            ),
            '#type' => 'action',
            '#weight' => 0.0,
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
              'message' => "[flagging_user:display-name] has reported the following comment on [node:type] content as being offensive or inappropriate for the [:global:site-name] site:\r\n\r\n<?php print url('<front>', array('absolute' => TRUE, 'language' => 'en')); ?>/goto/comment/[comment:comment-cid]\r\n\r\nTotal reports of this comment: [comment:flag-abuse-comment-count]",
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
                'rules_input_evaluator_php' => array(
                  'message' => array(),
                ),
              ),
            ),
            '#type' => 'action',
            '#weight' => 0.0,
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
              'message' => "[flagging_user:display-name] has reported the following [node:type] content as being offensive or inappropriate for the [:global:site-name] site:\r\n\r\n<?php print url('node/' . \$node->nid, array('absolute' => TRUE, 'language' => 'en')); ?>\r\n\r\nTotal reports of this content: [node:flag-abuse-node-count]",
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
                'rules_input_evaluator_php' => array(
                  'message' => array(
                    '0' => 'node',
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
            '#weight' => 0.0,
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
              'message' => "[flagging_user:display-name] has reported [account:display-name] for inappropriate behavior:\r\n\r\n<?php print url(\'<front>\', array(\'absolute\' => TRUE, \'language\' => \'en\')); ?>/account/[account:uid]\r\n\r\nTotal current reports of this user: [account:flag-abuse-user-count]",
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
                'rules_input_evaluator_php' => array(
                  'message' => array(),
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
            '#weight' => 0.0,
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
