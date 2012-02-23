<?php
// $Id: flag_friend.views_default.inc,v 1.1.2.11 2010/08/08 13:04:24 sirkitree Exp $

/**
 * @file flag_friend.views_default.inc
 *
 * Default view for flag friend which provides a friend listing, a pending
 * friends listing, and a listing of people you've requested as friends.
 */

/**
 * Implementation of hook_views_default_views().
 */
function flag_friend_views_default_views() {
  $view = new view;
  $view->name = 'friends';
  $view->description = 'Various page displays for flag_friend.';
  $view->tag = 'flag.friend';
  $view->view_php = '';
  $view->base_table = 'users';
  $view->is_cacheable = FALSE;
  $view->api_version = 2;
  $view->disabled = FALSE; /* Edit this to true to make a default view disabled initially */
  $handler = $view->new_display('default', 'Defaults', 'default');
  $handler->override_option('relationships', array(
    'flag_friend_content_rel' => array(
      'label' => 'flag friend',
      'required' => 1,
      'flag' => 'friend',
      'user_scope' => 'any',
      'id' => 'flag_friend_content_rel',
      'table' => 'users',
      'field' => 'flag_friend_content_rel',
      'override' => array(
        'button' => 'Override',
      ),
      'relationship' => 'none',
    ),
  ));
  $handler->override_option('fields', array(
    'name' => array(
      'label' => 'Name',
      'link_to_user' => 1,
      'exclude' => 0,
      'id' => 'name',
      'table' => 'users',
      'field' => 'name',
      'override' => array(
        'button' => 'Override',
      ),
      'relationship' => 'none',
    ),
    'ops' => array(
      'label' => 'Flag link',
      'link_type' => '',
      'exclude' => 0,
      'id' => 'ops',
      'table' => 'flag_content',
      'field' => 'ops',
      'relationship' => 'flag_friend_content_rel',
      'override' => array(
        'button' => 'Override',
      ),
    ),
    'uid' => array(
      'label' => 'Flag friend links',
      'exclude' => 0,
      'id' => 'uid',
      'table' => 'flag_friend',
      'field' => 'uid',
      'relationship' => 'none',
    ),
  ));
  $handler->override_option('arguments', array(
    'friend_uid' => array(
      'id' => 'friend_uid',
      'table' => 'flag_friend',
      'field' => 'friend_uid',
    ),
  ));
  $handler->override_option('filters', array(
    'status' => array(
      'operator' => '=',
      'value' => '1',
      'group' => '0',
      'exposed' => FALSE,
      'expose' => array(
        'operator' => FALSE,
        'label' => '',
      ),
      'id' => 'status',
      'table' => 'users',
      'field' => 'status',
      'relationship' => 'none',
    ),
  ));
  $handler->override_option('access', array(
    'type' => 'perm',
    'perm' => 'access content',
  ));
  $handler->override_option('cache', array(
    'type' => 'none',
  ));
  $handler->override_option('empty', 'No Friends have been added.');
  $handler->override_option('empty_format', '1');
  $handler->override_option('use_pager', '1');
  $handler->override_option('distinct', 1);
  $handler->override_option('style_plugin', 'table');
  $handler->override_option('style_options', array(
    'grouping' => '',
    'override' => 1,
    'sticky' => 0,
    'order' => 'asc',
    'columns' => array(
      'picture' => 'picture',
      'name' => 'picture',
      'message' => 'message',
      'ops' => 'ops',
      'uid' => 'ops',
    ),
    'info' => array(
      'picture' => array(
        'sortable' => 0,
        'separator' => '',
      ),
      'name' => array(
        'sortable' => 0,
        'separator' => '',
      ),
      'message' => array(
        'separator' => '',
      ),
      'ops' => array(
        'separator' => '',
      ),
      'uid' => array(
        'separator' => '',
      ),
    ),
    'default' => '-1',
  ));
  $handler = $view->new_display('page', 'Page (friends)', 'page_1');
  $handler->override_option('relationships', array());
  $handler->override_option('fields', array(
    'picture' => array(
      'label' => 'Friend',
      'exclude' => 0,
      'id' => 'picture',
      'table' => 'users',
      'field' => 'picture',
      'override' => array(
        'button' => 'Use default',
      ),
      'relationship' => 'none',
    ),
    'name' => array(
      'label' => 'Name',
      'link_to_user' => 1,
      'exclude' => 0,
      'id' => 'name',
      'table' => 'users',
      'field' => 'name',
      'override' => array(
        'button' => 'Use default',
      ),
      'relationship' => 'none',
    ),
    'uid' => array(
      'label' => 'Actions',
      'exclude' => 0,
      'id' => 'uid',
      'table' => 'flag_friend',
      'field' => 'uid',
      'override' => array(
        'button' => 'Use default',
      ),
      'relationship' => 'none',
    ),
  ));
  $handler->override_option('arguments', array(
    'friend_uid' => array(
      'default_action' => 'default',
      'style_plugin' => 'default_summary',
      'style_options' => array(),
      'wildcard' => 'all',
      'wildcard_substitution' => 'All',
      'title' => '',
      'default_argument_type' => 'user',
      'default_argument' => '',
      'validate_type' => 'none',
      'validate_fail' => 'not found',
      'break_phrase' => 0,
      'not' => 0,
      'id' => 'friend_uid',
      'table' => 'flag_friend',
      'field' => 'friend_uid',
      'override' => array(
        'button' => 'Use default',
      ),
      'relationship' => 'none',
      'default_options_div_prefix' => '',
      'default_argument_user' => 0,
      'default_argument_fixed' => '',
      'default_argument_php' => '',
      'validate_argument_node_access' => 0,
      'validate_argument_nid_type' => 'nid',
      'validate_argument_vocabulary' => array(),
      'validate_argument_type' => 'tid',
      'validate_argument_node_flag_name' => '*relationship*',
      'validate_argument_node_flag_test' => 'flaggable',
      'validate_argument_node_flag_id_type' => 'id',
      'validate_argument_user_flag_name' => '*relationship*',
      'validate_argument_user_flag_test' => 'flaggable',
      'validate_argument_user_flag_id_type' => 'id',
      'validate_argument_php' => '',
    ),
  ));
  $handler->override_option('title', 'Friends');
  $handler->override_option('path', 'user/%/friends/all');
  $handler->override_option('menu', array(
    'type' => 'default tab',
    'title' => 'View All Friends',
    'description' => '',
    'weight' => '-10',
    'name' => 'navigation',
  ));
  $handler->override_option('tab_options', array(
    'type' => 'tab',
    'title' => 'Friends',
    'description' => '',
    'weight' => '0',
    'name' => 'navigation',
  ));
  $handler = $view->new_display('page', 'Page (pending)', 'page_2');
  $handler->override_option('fields', array(
    'picture' => array(
      'label' => 'User',
      'exclude' => 0,
      'id' => 'picture',
      'table' => 'users',
      'field' => 'picture',
      'override' => array(
        'button' => 'Use default',
      ),
      'relationship' => 'none',
    ),
    'name' => array(
      'label' => 'Name',
      'link_to_user' => 1,
      'exclude' => 0,
      'id' => 'name',
      'table' => 'users',
      'field' => 'name',
      'override' => array(
        'button' => 'Use default',
      ),
      'relationship' => 'none',
    ),
    'message' => array(
      'label' => 'Message',
      'exclude' => 0,
      'id' => 'message',
      'table' => 'flag_friend_message',
      'field' => 'message',
      'relationship' => 'flag_friend_content_rel',
      'override' => array(
        'button' => 'Use default',
      ),
    ),
    'ops' => array(
      'label' => 'Actions',
      'link_type' => '',
      'exclude' => 0,
      'id' => 'ops',
      'table' => 'flag_content',
      'field' => 'ops',
      'relationship' => 'flag_friend_content_rel',
      'override' => array(
        'button' => 'Use default',
      ),
    ),
    'uid' => array(
      'label' => 'Flag friend links',
      'exclude' => 0,
      'id' => 'uid',
      'table' => 'flag_friend',
      'field' => 'uid',
      'override' => array(
        'button' => 'Use default',
      ),
      'relationship' => 'none',
    ),
  ));
  $handler->override_option('arguments', array(
    'content_id' => array(
      'default_action' => 'default',
      'style_plugin' => 'default_summary',
      'style_options' => array(),
      'wildcard' => 'all',
      'wildcard_substitution' => 'All',
      'title' => '',
      'default_argument_type' => 'user',
      'default_argument' => '',
      'validate_type' => 'php',
      'validate_fail' => 'not found',
      'break_phrase' => 0,
      'not' => 0,
      'id' => 'content_id',
      'table' => 'flag_content',
      'field' => 'content_id',
      'relationship' => 'flag_friend_content_rel',
      'override' => array(
        'button' => 'Use default',
      ),
      'default_options_div_prefix' => '',
      'default_argument_user' => 0,
      'default_argument_fixed' => '',
      'default_argument_php' => '',
      'validate_argument_node_access' => 0,
      'validate_argument_nid_type' => 'nid',
      'validate_argument_vocabulary' => array(),
      'validate_argument_type' => 'tid',
      'validate_argument_node_flag_name' => '*relationship*',
      'validate_argument_node_flag_test' => 'flaggable',
      'validate_argument_node_flag_id_type' => 'id',
      'validate_argument_user_flag_name' => '*relationship*',
      'validate_argument_user_flag_test' => 'flaggable',
      'validate_argument_user_flag_id_type' => 'id',
      'validate_argument_php' => 'if ($argument !== $GLOBALS[\'user\']->uid && !user_access(\'administer users\')) {
                                      return FALSE;
                                    }
                                    return TRUE;',
    ),
  ));
  $handler->override_option('title', 'Friend Requests');
  $handler->override_option('header', 'These are users who would like to be your friend.');
  $handler->override_option('header_format', '1');
  $handler->override_option('header_empty', 0);
  $handler->override_option('empty', 'No Friend Requests.');
  $handler->override_option('path', 'user/%/friends/pending');
  $handler->override_option('menu', array(
    'type' => 'tab',
    'title' => 'Friend Requests',
    'description' => '',
    'weight' => '0',
    'name' => 'navigation',
  ));
  $handler->override_option('tab_options', array(
    'type' => 'none',
    'title' => '',
    'description' => '',
    'weight' => 0,
    'name' => 'navigation',
  ));
  $handler = $view->new_display('page', 'Page (flagged)', 'page_3');
  $handler->override_option('relationships', array(
    'flag_content_rel' => array(
      'label' => 'flag',
      'required' => 1,
      'flag' => 'friend',
      'user_scope' => 'any',
      'id' => 'flag_content_rel',
      'table' => 'users',
      'field' => 'flag_content_rel',
      'override' => array(
        'button' => 'Use default',
      ),
      'relationship' => 'none',
    ),
    'uid' => array(
      'label' => 'Flag user',
      'required' => 0,
      'id' => 'uid',
      'table' => 'flag_content',
      'field' => 'uid',
      'relationship' => 'flag_content_rel',
      'override' => array(
        'button' => 'Use default',
      ),
    ),
  ));
  $handler->override_option('fields', array(
    'picture' => array(
      'label' => 'User',
      'exclude' => 0,
      'id' => 'picture',
      'table' => 'users',
      'field' => 'picture',
      'override' => array(
        'button' => 'Use default',
      ),
      'relationship' => 'none',
    ),
    'name' => array(
      'label' => 'Name',
      'link_to_user' => 1,
      'exclude' => 0,
      'id' => 'name',
      'table' => 'users',
      'field' => 'name',
      'override' => array(
        'button' => 'Use default',
      ),
      'relationship' => 'none',
    ),
    'message' => array(
      'label' => 'Message',
      'exclude' => 0,
      'id' => 'message',
      'table' => 'flag_friend_message',
      'field' => 'message',
      'relationship' => 'flag_content_rel',
      'override' => array(
        'button' => 'Use default',
      ),
    ),
    'ops' => array(
      'label' => 'Actions',
      'link_type' => '',
      'exclude' => 0,
      'id' => 'ops',
      'table' => 'flag_content',
      'field' => 'ops',
      'relationship' => 'flag_content_rel',
      'override' => array(
        'button' => 'Use default',
      ),
    ),
  ));
  $handler->override_option('arguments', array(
    'uid' => array(
      'default_action' => 'default',
      'style_plugin' => 'default_summary',
      'style_options' => array(),
      'wildcard' => 'all',
      'wildcard_substitution' => 'All',
      'title' => '',
      'breadcrumb' => '',
      'default_argument_type' => 'user',
      'default_argument' => '',
      'validate_type' => 'php',
      'validate_fail' => 'not found',
      'break_phrase' => 0,
      'not' => 0,
      'id' => 'uid',
      'table' => 'users',
      'field' => 'uid',
      'validate_user_argument_type' => 'uid',
      'validate_user_roles' => array(
        '2' => 0,
      ),
      'override' => array(
        'button' => 'Use default',
      ),
      'relationship' => 'uid',
      'default_options_div_prefix' => '',
      'default_argument_user' => 0,
      'default_argument_fixed' => '',
      'default_argument_php' => '',
      'validate_argument_node_type' => array(
        'page' => 0,
        'story' => 0,
      ),
      'validate_argument_node_access' => 0,
      'validate_argument_nid_type' => 'nid',
      'validate_argument_vocabulary' => array(),
      'validate_argument_type' => 'tid',
      'validate_argument_transform' => 0,
      'validate_user_restrict_roles' => 0,
      'validate_argument_node_flag_name' => '*relationship*',
      'validate_argument_node_flag_test' => 'flaggable',
      'validate_argument_node_flag_id_type' => 'id',
      'validate_argument_user_flag_name' => '*relationship*',
      'validate_argument_user_flag_test' => 'flaggable',
      'validate_argument_user_flag_id_type' => 'id',
      'validate_argument_php' => 'if ($argument !== $GLOBALS[\'user\']->uid && !user_access(\'administer users\')) {
    return FALSE;
  }
  return TRUE;',
    ),
  ));
  $handler->override_option('title', 'Awaiting Friend Approvals');
  $handler->override_option('header', 'These are users who you have requested to be friends with.');
  $handler->override_option('header_format', '1');
  $handler->override_option('header_empty', 1);
  $handler->override_option('empty', 'No Friend Requests.');
  $handler->override_option('path', 'user/%/friends/flagged');
  $handler->override_option('menu', array(
    'type' => 'tab',
    'title' => 'Awaiting Friend Approvals',
    'description' => '',
    'weight' => '0',
    'name' => 'navigation',
  ));
  $handler->override_option('tab_options', array(
    'type' => 'none',
    'title' => '',
    'description' => '',
    'weight' => 0,
    'name' => 'navigation',
  ));
  $handler = $view->new_display('block', 'Current user\'s Friends block', 'block_1');
  $handler->override_option('relationships', array());
  $handler->override_option('fields', array(
    'name' => array(
      'label' => 'Name',
      'link_to_user' => 1,
      'exclude' => 0,
      'id' => 'name',
      'table' => 'users',
      'field' => 'name',
      'override' => array(
        'button' => 'Override',
      ),
      'relationship' => 'none',
    ),
    'uid' => array(
      'label' => 'Flag friend links',
      'exclude' => 0,
      'id' => 'uid',
      'table' => 'flag_friend',
      'field' => 'uid',
      'relationship' => 'none',
    ),
  ));
  $handler->override_option('arguments', array(
    'friend_uid' => array(
      'default_action' => 'default',
      'style_plugin' => 'default_summary',
      'style_options' => array(),
      'wildcard' => 'all',
      'wildcard_substitution' => 'All',
      'title' => '',
      'default_argument_type' => 'current_user',
      'default_argument' => '',
      'validate_type' => 'none',
      'validate_fail' => 'not found',
      'break_phrase' => 0,
      'not' => 0,
      'id' => 'friend_uid',
      'table' => 'flag_friend',
      'field' => 'friend_uid',
      'override' => array(
        'button' => 'Use default',
      ),
      'relationship' => 'none',
      'default_options_div_prefix' => '',
      'default_argument_user' => 0,
      'default_argument_fixed' => '',
      'default_argument_php' => '',
      'validate_argument_node_access' => 0,
      'validate_argument_nid_type' => 'nid',
      'validate_argument_node_flag_name' => '*relationship*',
      'validate_argument_node_flag_test' => 'flaggable',
      'validate_argument_node_flag_id_type' => 'id',
      'validate_argument_user_flag_name' => '*relationship*',
      'validate_argument_user_flag_test' => 'flaggable',
      'validate_argument_user_flag_id_type' => 'id',
      'validate_argument_php' => '',
    ),
  ));
  $handler->override_option('items_per_page', 6);
  $handler->override_option('use_pager', '0');
  $handler->override_option('use_more', 1);
  $handler->override_option('block_description', '');
  $handler->override_option('block_caching', -1);
  $views[$view->name] = $view;
  return $views;
}