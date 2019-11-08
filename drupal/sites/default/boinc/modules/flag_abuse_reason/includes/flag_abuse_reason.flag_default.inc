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
  // Exported flag: "Node Abuse metalink".
  $flags[] = array (
    'content_type' => 'node',
    'name' => 'abuse_node_meta',
    'title' => 'Node Abuse meta',
    'global' => false,
    'types' => array (
      '0' => 'forum',
      '1' => 'news',
      '2' => 'team_forum',
    ),
    'events' => array (
      0 => 'flag',
      1 => 'unflag',
      2 => 'reset',
    ),
    'flag_short' => 'Meta Flag for node',
    'flag_long' => 'Meta Flag for node.',
    'flag_message' => '',
    'unflag_short' => 'Meta Flag for node',
    'unflag_long' => 'Meta Flag for node.',
    'unflag_message' => '',
    'unflag_denied_text' => '',
    'link_type' => 'confirm',
    'roles' => array (
      'flag' => array (
        0 => '2',
      ),
      'unflag' => array (
        0 => '2',
      ),
    ),
    'show_on_page' => true,
    'show_on_teaser' => false,
    'show_on_form' => false,
    'access_author' => '',
    'i18n' => 0,
    'flag_confirmation' => 'Are you sure you want to flag this content?',
    'unflag_confirmation' => 'Are you sure you want to unflag this content?',
    'reset_short' => 'Reset flags',
    'reset_long' => 'Reset all flags on this content',
    'reset_confirmation' => 'Are you sure you want to reset all flags on this content?',
    'status' => false,
    'locked' => array (
      'name' => 'name',
      'global' => 'global',
    ),
    'module' => 'flag_abuse_reason',
    'api_version' => 2,
  );

  // Exported flag: "Node Abuse 1".
  $flags[] = array (
    'content_type' => 'node',
    'name' => 'abuse_node_1',
    'title' => 'Node Abuse 1',
    'global' => false,
    'types' => array (
      '0' => 'forum',
      '1' => 'news',
      '2' => 'team_forum',
    ),
    'events' => array (
      0 => 'flag',
      1 => 'unflag',
      2 => 'reset',
    ),
    'flag_short' => 'Flag as offensive',
    'flag_long' => 'Flag this content as offensive.',
    'flag_message' => '',
    'unflag_short' => 'Flag as non-offensive',
    'unflag_long' => 'Flag this content as non-offensive.',
    'unflag_message' => '',
    'unflag_denied_text' => '',
    'link_type' => 'confirm',
    'roles' => array (
      'flag' => array (
        0 => '2',
      ),
      'unflag' => array (
        0 => '2',
      ),
    ),
    'show_on_page' => true,
    'show_on_teaser' => false,
    'show_on_form' => false,
    'access_author' => '',
    'i18n' => 0,
    'flag_confirmation' => 'Are you sure you want to flag this content as offensive?',
    'unflag_confirmation' => 'Are you sure you want to flag this content as non-offensive?',
    'reset_short' => 'Reset flags',
    'reset_long' => 'Reset all flags on this content',
    'reset_confirmation' => 'Are you sure you want to reset all flags on this content?',
    'status' => false,
    'locked' => array (
      'name' => 'name',
      'global' => 'global',
    ),
    'module' => 'flag_abuse_reason',
    'api_version' => 2,
  );

  // Exported flag: "Node Abuse 2".
  $flags[] = array (
    'content_type' => 'node',
    'name' => 'abuse_node_2',
    'title' => 'Node Abuse 2',
    'global' => false,
    'types' => array (
      '0' => 'forum',
      '1' => 'news',
      '2' => 'team_forum',
    ),
    'events' => array (
      0 => 'flag',
      1 => 'unflag',
      2 => 'reset',
    ),
    'flag_short' => 'Flag as offensive',
    'flag_long' => 'Flag this content as offensive.',
    'flag_message' => '',
    'unflag_short' => 'Flag as non-offensive',
    'unflag_long' => 'Flag this content as non-offensive.',
    'unflag_message' => '',
    'unflag_denied_text' => '',
    'link_type' => 'confirm',
    'roles' => array (
      'flag' => array (
        0 => '2',
      ),
      'unflag' => array (
        0 => '2',
      ),
    ),
    'show_on_page' => true,
    'show_on_teaser' => false,
    'show_on_form' => false,
    'access_author' => '',
    'i18n' => 0,
    'flag_confirmation' => 'Are you sure you want to flag this content as offensive?',
    'unflag_confirmation' => 'Are you sure you want to flag this content as non-offensive?',
    'reset_short' => 'Reset flags',
    'reset_long' => 'Reset all flags on this content',
    'reset_confirmation' => 'Are you sure you want to reset all flags on this content?',
    'status' => false,
    'locked' => array (
      'name' => 'name',
      'global' => 'global',
    ),
    'module' => 'flag_abuse_reason',
    'api_version' => 2,
  );

  // Exported flag: "Node Abuse 3".
  $flags[] = array (
    'content_type' => 'node',
    'name' => 'abuse_node_3',
    'title' => 'Node Abuse 3',
    'global' => false,
    'types' => array (
      '0' => 'forum',
      '1' => 'news',
      '2' => 'team_forum',
    ),
    'events' => array (
      0 => 'flag',
      1 => 'unflag',
      2 => 'reset',
    ),
    'flag_short' => 'Flag as offensive',
    'flag_long' => 'Flag this content as offensive.',
    'flag_message' => '',
    'unflag_short' => 'Flag as non-offensive',
    'unflag_long' => 'Flag this content as non-offensive.',
    'unflag_message' => '',
    'unflag_denied_text' => '',
    'link_type' => 'confirm',
    'roles' => array (
      'flag' => array (
        0 => '2',
      ),
      'unflag' => array (
        0 => '2',
      ),
    ),
    'show_on_page' => true,
    'show_on_teaser' => false,
    'show_on_form' => false,
    'access_author' => '',
    'i18n' => 0,
    'flag_confirmation' => 'Are you sure you want to flag this content as offensive?',
    'unflag_confirmation' => 'Are you sure you want to flag this content as non-offensive?',
    'reset_short' => 'Reset flags',
    'reset_long' => 'Reset all flags on this content',
    'reset_confirmation' => 'Are you sure you want to reset all flags on this content?',
    'status' => false,
    'locked' => array (
      'name' => 'name',
      'global' => 'global',
    ),
    'module' => 'flag_abuse_reason',
    'api_version' => 2,
  );

  // Exported flag: "Node Abuse 4".
  $flags[] = array (
    'content_type' => 'node',
    'name' => 'abuse_node_4',
    'title' => 'Node Abuse 4',
    'global' => false,
    'types' => array (
      '0' => 'forum',
      '1' => 'news',
      '2' => 'team_forum',
    ),
    'events' => array (
      0 => 'flag',
      1 => 'unflag',
      2 => 'reset',
    ),
    'flag_short' => 'Flag as offensive',
    'flag_long' => 'Flag this content as offensive.',
    'flag_message' => '',
    'unflag_short' => 'Flag as non-offensive',
    'unflag_long' => 'Flag this content as non-offensive.',
    'unflag_message' => '',
    'unflag_denied_text' => '',
    'link_type' => 'confirm',
    'roles' => array (
      'flag' => array (
        0 => '2',
      ),
      'unflag' => array (
        0 => '2',
      ),
    ),
    'show_on_page' => true,
    'show_on_teaser' => false,
    'show_on_form' => false,
    'access_author' => '',
    'i18n' => 0,
    'flag_confirmation' => 'Are you sure you want to flag this content as offensive?',
    'unflag_confirmation' => 'Are you sure you want to flag this content as non-offensive?',
    'reset_short' => 'Reset flags',
    'reset_long' => 'Reset all flags on this content',
    'reset_confirmation' => 'Are you sure you want to reset all flags on this content?',
    'status' => false,
    'locked' => array (
      'name' => 'name',
      'global' => 'global',
    ),
    'module' => 'flag_abuse_reason',
    'api_version' => 2,
  );

  // Exported flag: "Node Abuse 5".
  $flags[] = array (
    'content_type' => 'node',
    'name' => 'abuse_node_5',
    'title' => 'Node Abuse 5',
    'global' => false,
    'types' => array (
      '0' => 'forum',
      '1' => 'news',
      '2' => 'team_forum',
    ),
    'events' => array (
      0 => 'flag',
      1 => 'unflag',
      2 => 'reset',
    ),
    'flag_short' => 'Flag as offensive',
    'flag_long' => 'Flag this content as offensive.',
    'flag_message' => '',
    'unflag_short' => 'Flag as non-offensive',
    'unflag_long' => 'Flag this content as non-offensive.',
    'unflag_message' => '',
    'unflag_denied_text' => '',
    'link_type' => 'confirm',
    'roles' => array (
      'flag' => array (
        0 => '2',
      ),
      'unflag' => array (
        0 => '2',
      ),
    ),
    'show_on_page' => true,
    'show_on_teaser' => false,
    'show_on_form' => false,
    'access_author' => '',
    'i18n' => 0,
    'flag_confirmation' => 'Are you sure you want to flag this content as offensive?',
    'unflag_confirmation' => 'Are you sure you want to flag this content as non-offensive?',
    'reset_short' => 'Reset flags',
    'reset_long' => 'Reset all flags on this content',
    'reset_confirmation' => 'Are you sure you want to reset all flags on this content?',
    'status' => false,
    'locked' => array (
      'name' => 'name',
      'global' => 'global',
    ),
    'module' => 'flag_abuse_reason',
    'api_version' => 2,
  );

  return $flags;
}

/*  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  * *
 * Comment Flags
 *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  */
function _flag_abuse_reason_abuse_comment_flags(&$flags) {

  // Exported flag: "Comment Abuse meta".
  $flags[] = array(
    'content_type' => 'comment',
    'name' => 'abuse_comment_meta',
    'title' => 'Comment Abuse meta',
    'global' => '0',
    'types' => array(
      '0' => 'forum',
      '1' => 'news',
      '2' => 'team_forum',
    ),
    'events' => array (
      0 => 'flag',
      1 => 'unflag',
    ),
    'flag_short' => 'Meta Flag for comment',
    'flag_long' => 'Meta Flag for comment.',
    'flag_message' => '',
    'unflag_short' => 'Meta Flag for comment',
    'unflag_long' => 'Meta Flag for comment.',
    'unflag_message' => '',
    'unflag_denied_text' => '',
    'link_type' => 'confirm',
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
    'flag_confirmation' => 'Are you sure you want to flag this content?',
    'unflag_confirmation' => 'Are you sure you want to unflag this content?',
    'reset_short' => 'Reset flags',
    'reset_long' => 'Reset all flags on this comment',
    'reset_confirmation' => 'Are you sure you want to reset all flags on this comment?',
    'status' => false,
    'show_on_teaser' => false,
    'locked' => array (
      'name' => 'name',
      'global' => 'global',
    ),
    'module' => 'flag_abuse_reason',
    'api_version' => 2,
  );

  // Exported flag: "Comment Abuse 1".
  $flags[] = array (
    'content_type' => 'comment',
    'name' => 'abuse_comment_1',
    'title' => 'Comment Abuse 1',
    'global' => false,
    'types' => array (
      '0' => 'forum',
      '1' => 'news',
      '2' => 'team_forum',
    ),
    'events' => array (
      0 => 'flag',
      1 => 'unflag',
    ),
    'flag_short' => 'Flag as offensive',
    'flag_long' => 'Flag this content as offensive.',
    'flag_message' => '',
    'unflag_short' => 'Flag as non-offensive',
    'unflag_long' => 'Flag this content as non-offensive.',
    'unflag_message' => '',
    'unflag_denied_text' => '',
    'link_type' => 'confirm',
    'roles' => array (
      'flag' => array (
        0 => '2',
      ),
      'unflag' => array (
        0 => '2',
      ),
    ),
    'access_author' => '',
    'show_on_comment' => true,
    'flag_confirmation' => 'Are you sure you want to flag this content as offensive?',
    'unflag_confirmation' => 'Are you sure you want to flag this content as non-offensive?',
    'reset_short' => 'Reset flags',
    'reset_long' => 'Reset all flags on this comment',
    'reset_confirmation' => 'Are you sure you want to reset all flags on this comment?',
    'status' => false,
    'show_on_teaser' => false,
    'locked' => array (
      'name' => 'name',
      'global' => 'global',
    ),
    'module' => 'flag_abuse_reason',
    'api_version' => 2,
  );

  // Exported flag: "Comment Abuse 2".
  $flags[] = array (
    'content_type' => 'comment',
    'name' => 'abuse_comment_2',
    'title' => 'Comment Abuse 2',
    'global' => false,
    'types' => array (
      '0' => 'forum',
      '1' => 'news',
      '2' => 'team_forum',
    ),
    'events' => array (
      0 => 'flag',
      1 => 'unflag',
    ),
    'flag_short' => 'Flag as offensive',
    'flag_long' => 'Flag this content as offensive.',
    'flag_message' => '',
    'unflag_short' => 'Flag as non-offensive',
    'unflag_long' => 'Flag this content as non-offensive.',
    'unflag_message' => '',
    'unflag_denied_text' => '',
    'link_type' => 'confirm',
    'roles' => array (
      'flag' => array (
        0 => '2',
      ),
      'unflag' => array (
        0 => '2',
      ),
    ),
    'access_author' => '',
    'show_on_comment' => true,
    'flag_confirmation' => 'Are you sure you want to flag this content as offensive?',
    'unflag_confirmation' => 'Are you sure you want to flag this content as non-offensive?',
    'reset_short' => 'Reset flags',
    'reset_long' => 'Reset all flags on this comment',
    'reset_confirmation' => 'Are you sure you want to reset all flags on this comment?',
    'status' => false,
    'show_on_teaser' => false,
    'locked' => array (
      'name' => 'name',
      'global' => 'global',
    ),
    'module' => 'flag_abuse_reason',
    'api_version' => 2,
  );

  // Exported flag: "Comment Abuse 3".
  $flags[] = array (
    'content_type' => 'comment',
    'name' => 'abuse_comment_3',
    'title' => 'Comment Abuse 3',
    'global' => false,
    'types' => array (
      '0' => 'forum',
      '1' => 'news',
      '2' => 'team_forum',
    ),
    'events' => array (
      0 => 'flag',
      1 => 'unflag',
    ),
    'flag_short' => 'Flag as offensive',
    'flag_long' => 'Flag this content as offensive.',
    'flag_message' => '',
    'unflag_short' => 'Flag as non-offensive',
    'unflag_long' => 'Flag this content as non-offensive.',
    'unflag_message' => '',
    'unflag_denied_text' => '',
    'link_type' => 'confirm',
    'roles' => array (
      'flag' => array (
        0 => '2',
      ),
      'unflag' => array (
        0 => '2',
      ),
    ),
    'access_author' => '',
    'show_on_comment' => true,
    'flag_confirmation' => 'Are you sure you want to flag this content as offensive?',
    'unflag_confirmation' => 'Are you sure you want to flag this content as non-offensive?',
    'reset_short' => 'Reset flags',
    'reset_long' => 'Reset all flags on this comment',
    'reset_confirmation' => 'Are you sure you want to reset all flags on this comment?',
    'status' => false,
    'show_on_teaser' => false,
    'locked' => array (
      'name' => 'name',
      'global' => 'global',
    ),
    'module' => 'flag_abuse_reason',
    'api_version' => 2,
  );

  // Exported flag: "Comment Abuse 4".
  $flags[] = array (
    'content_type' => 'comment',
    'name' => 'abuse_comment_4',
    'title' => 'Comment Abuse 4',
    'global' => false,
    'types' => array (
      '0' => 'forum',
      '1' => 'news',
      '2' => 'team_forum',
    ),
    'events' => array (
      0 => 'flag',
      1 => 'unflag',
    ),
    'flag_short' => 'Flag as offensive',
    'flag_long' => 'Flag this content as offensive.',
    'flag_message' => '',
    'unflag_short' => 'Flag as non-offensive',
    'unflag_long' => 'Flag this content as non-offensive.',
    'unflag_message' => '',
    'unflag_denied_text' => '',
    'link_type' => 'confirm',
    'roles' => array (
      'flag' => array (
        0 => '2',
      ),
      'unflag' => array (
        0 => '2',
      ),
    ),
    'access_author' => '',
    'show_on_comment' => true,
    'flag_confirmation' => 'Are you sure you want to flag this content as offensive?',
    'unflag_confirmation' => 'Are you sure you want to flag this content as non-offensive?',
    'reset_short' => 'Reset flags',
    'reset_long' => 'Reset all flags on this comment',
    'reset_confirmation' => 'Are you sure you want to reset all flags on this comment?',
    'status' => false,
    'show_on_teaser' => false,
    'locked' => array (
      'name' => 'name',
      'global' => 'global',
    ),
    'module' => 'flag_abuse_reason',
    'api_version' => 2,
  );

  // Exported flag: "Comment Abuse 5".
  $flags[] = array (
    'content_type' => 'comment',
    'name' => 'abuse_comment_5',
    'title' => 'Comment Abuse 5',
    'global' => false,
    'types' => array (
      '0' => 'forum',
      '1' => 'news',
      '2' => 'team_forum',
    ),
    'events' => array (
      0 => 'flag',
      1 => 'unflag',
    ),
    'flag_short' => 'Flag as offensive',
    'flag_long' => 'Flag this content as offensive.',
    'flag_message' => '',
    'unflag_short' => 'Flag as non-offensive',
    'unflag_long' => 'Flag this content as non-offensive.',
    'unflag_message' => '',
    'unflag_denied_text' => '',
    'link_type' => 'confirm',
    'roles' => array (
      'flag' => array (
        0 => '2',
      ),
      'unflag' => array (
        0 => '2',
      ),
    ),
    'access_author' => '',
    'show_on_comment' => true,
    'flag_confirmation' => 'Are you sure you want to flag this content as offensive?',
    'unflag_confirmation' => 'Are you sure you want to flag this content as non-offensive?',
    'reset_short' => 'Reset flags',
    'reset_long' => 'Reset all flags on this comment',
    'reset_confirmation' => 'Are you sure you want to reset all flags on this comment?',
    'status' => false,
    'show_on_teaser' => false,
    'locked' => array (
      'name' => 'name',
      'global' => 'global',
    ),
    'module' => 'flag_abuse_reason',
    'api_version' => 2,
  );

  return $flags;
}

/*  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  * *
 * User Flags
 *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  */
function _flag_abuse_reason_abuse_user_flags(&$flags) {
  // Exported flag: "User Abuse meta".
  $flags[] = array (
    'content_type' => 'user',
    'name' => 'abuse_user_meta',
    'title' => 'User Abuse Meta',
    'global' => false,
    'types' => array (
    ),
    'events' => array (
      0 => 'flag',
      1 => 'unflag',
    ),
    'flag_short' => 'Meta Flag for User',
    'flag_long' => 'Meta Flag for User.',
    'flag_message' => '',
    'unflag_short' => 'Meta Flag for User',
    'unflag_long' => 'Meta Flag for User.',
    'unflag_message' => '',
    'unflag_denied_text' => '',
    'link_type' => 'confirm',
    'roles' => array (
      'flag' => array (
        0 => '2',
      ),
      'unflag' => array (
        0 => '2',
      ),
    ),
    'show_on_profile' => true,
    'access_uid' => '',
    'flag_confirmation' => 'Are you sure you want to report this user?',
    'unflag_confirmation' => 'Are you sure you want to remove your report of this user?',
    'reset_short' => 'Reset flags',
    'reset_long' => 'Reset all flags on this user',
    'reset_confirmation' => 'Are you sure you want to reset all flags on this user?',
    'status' => false,
    'show_on_teaser' => false,
    'locked' => array (
      'name' => 'name',
      'global' => 'global',
    ),
    'module' => 'flag_abuse_reason',
    'api_version' => 2,
  );

  // Exported flag: "User Abuse 1".
  $flags[] = array (
    'content_type' => 'user',
    'name' => 'abuse_user_1',
    'title' => 'User Abuse 1',
    'global' => false,
    'types' => array (
    ),
    'events' => array (
      0 => 'flag',
      1 => 'unflag',
    ),
    'flag_short' => 'Report User',
    'flag_long' => 'Report this user as offensive.',
    'flag_message' => '',
    'unflag_short' => 'Remove Report',
    'unflag_long' => 'Remove your report about this user.',
    'unflag_message' => '',
    'unflag_denied_text' => '',
    'link_type' => 'confirm',
    'roles' => array (
      'flag' => array (
        0 => '2',
      ),
      'unflag' => array (
        0 => '2',
      ),
    ),
    'show_on_profile' => true,
    'access_uid' => '',
    'flag_confirmation' => 'Are you sure you want to report this user as offensive?',
    'unflag_confirmation' => 'Are you sure you want to remove your report of this user?',
    'reset_short' => 'Reset flags',
    'reset_long' => 'Reset all flags on this user',
    'reset_confirmation' => 'Are you sure you want to reset all flags on this user?',
    'status' => false,
    'show_on_teaser' => false,
    'locked' => array (
      'name' => 'name',
      'global' => 'global',
    ),
    'module' => 'flag_abuse_reason',
    'api_version' => 2,
  );

  // Exported flag: "User Abuse 2".
  $flags[] = array (
    'content_type' => 'user',
    'name' => 'abuse_user_2',
    'title' => 'User Abuse 2',
    'global' => false,
    'types' => array (
    ),
    'events' => array (
      0 => 'flag',
      1 => 'unflag',
    ),
    'flag_short' => 'Report User',
    'flag_long' => 'Report this user as offensive.',
    'flag_message' => '',
    'unflag_short' => 'Remove Report',
    'unflag_long' => 'Remove your report about this user.',
    'unflag_message' => '',
    'unflag_denied_text' => '',
    'link_type' => 'confirm',
    'roles' => array (
      'flag' => array (
        0 => '2',
      ),
      'unflag' => array (
        0 => '2',
      ),
    ),
    'show_on_profile' => true,
    'access_uid' => '',
    'flag_confirmation' => 'Are you sure you want to report this user as offensive?',
    'unflag_confirmation' => 'Are you sure you want to remove your report of this user?',
    'reset_short' => 'Reset flags',
    'reset_long' => 'Reset all flags on this user',
    'reset_confirmation' => 'Are you sure you want to reset all flags on this user?',
    'status' => false,
    'show_on_teaser' => false,
    'locked' => array (
      'name' => 'name',
      'global' => 'global',
    ),
    'module' => 'flag_abuse_reason',
    'api_version' => 2,
  );

  // Exported flag: "User Abuse 3".
  $flags[] = array (
    'content_type' => 'user',
    'name' => 'abuse_user_3',
    'title' => 'User Abuse 3',
    'global' => false,
    'types' => array (
    ),
    'events' => array (
      0 => 'flag',
      1 => 'unflag',
    ),
    'flag_short' => 'Report User',
    'flag_long' => 'Report this user as offensive.',
    'flag_message' => '',
    'unflag_short' => 'Remove Report',
    'unflag_long' => 'Remove your report about this user.',
    'unflag_message' => '',
    'unflag_denied_text' => '',
    'link_type' => 'confirm',
    'roles' => array (
      'flag' => array (
        0 => '2',
      ),
      'unflag' => array (
        0 => '2',
      ),
    ),
    'show_on_profile' => true,
    'access_uid' => '',
    'flag_confirmation' => 'Are you sure you want to report this user as offensive?',
    'unflag_confirmation' => 'Are you sure you want to remove your report of this user?',
    'reset_short' => 'Reset flags',
    'reset_long' => 'Reset all flags on this user',
    'reset_confirmation' => 'Are you sure you want to reset all flags on this user?',
    'status' => false,
    'show_on_teaser' => false,
    'locked' => array (
      'name' => 'name',
      'global' => 'global',
    ),
    'module' => 'flag_abuse_reason',
    'api_version' => 2,
  );

  // Exported flag: "User Abuse 4".
  $flags[] = array (
    'content_type' => 'user',
    'name' => 'abuse_user_4',
    'title' => 'User Abuse 4',
    'global' => false,
    'types' => array (
    ),
    'events' => array (
      0 => 'flag',
      1 => 'unflag',
    ),
    'flag_short' => 'Report User',
    'flag_long' => 'Report this user as offensive.',
    'flag_message' => '',
    'unflag_short' => 'Remove Report',
    'unflag_long' => 'Remove your report about this user.',
    'unflag_message' => '',
    'unflag_denied_text' => '',
    'link_type' => 'confirm',
    'roles' => array (
      'flag' => array (
        0 => '2',
      ),
      'unflag' => array (
        0 => '2',
      ),
    ),
    'show_on_profile' => true,
    'access_uid' => '',
    'flag_confirmation' => 'Are you sure you want to report this user as offensive?',
    'unflag_confirmation' => 'Are you sure you want to remove your report of this user?',
    'reset_short' => 'Reset flags',
    'reset_long' => 'Reset all flags on this user',
    'reset_confirmation' => 'Are you sure you want to reset all flags on this user?',
    'status' => false,
    'show_on_teaser' => false,
    'locked' => array (
      'name' => 'name',
      'global' => 'global',
    ),
    'module' => 'flag_abuse_reason',
    'api_version' => 2,
  );

  return $flags;
}
