<?php

// $Id:$

/**
 * Description:
 *
 * Define the flags for the flag_abuse_module. There are multiple
 * flags for each content type: node, comment, and user. Each has a
 * 'meta' flag which helps organize the 'subflags' for each type. Each
 * subflag is labeled 1, 2, 3, etc. There are five for node and
 * comment; four for user. More may be added here if necessary.
 *
 * In the admin flag interface, after adding the meta flags and flags
 * 1, 2, 3, etc. The admin may give the 1, 2, 3, flags titles to make
 * them clear what each flag does. The machine readable name will
 * still be use number 1, 2, 3, but the title, description, links,
 * etc should all be changed to be human readable as well as
 * specific. Example: abuse_node_1 should be used for SPAM nodes.
 */

/*  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  * *
 * Node Flags
 *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  */
function _flag_abuse_reason_abuse_node_flags(&$flags) {
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

  return $flags;
}

/*  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  * *
 * Comment Flags
 *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  */
function _flag_abuse_reason_abuse_comment_flags(&$flags) {
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

  return $flags;
}

/*  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  * *
 * User Flags
 *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  */
function _flag_abuse_reason_abuse_user_flags(&$flags) {
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

  return $flags;
}
