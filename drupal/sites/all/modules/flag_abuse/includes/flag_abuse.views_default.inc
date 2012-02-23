<?php
// $Id: flag_abuse.views_default.inc,v 1.1.2.1 2009/03/20 01:15:43 sirkitree Exp $

/**
 * Implementation of hook_views_default_views().
 */
function flag_abuse_views_default_views() {
  $views = array();
  if (module_exists('flag')) {
    $flags = array('abuse_comment', 'abuse_node', 'abuse_user');
    foreach ($flags as $flag_name) {
      if ($flag = flag_get_flag($flag_name)) {
        $function = '_flag_abuse_views_default_'. $flag_name;
        $function($views);
      }
    }
  }
  return $views;
}

function _flag_abuse_views_default_abuse_comment(&$views) {
  $view = new view;
  $view->name = 'all_flag_abuse_comment';
  $view->description = 'View for flag: Comment Abuse ';
  $view->tag = 'flag.abuse';
  $view->view_php = '';
  $view->base_table = 'comments';
  $view->is_cacheable = FALSE;
  $view->api_version = 2;
  $view->disabled = FALSE; /* Edit this to true to make a default view disabled initially */
  $handler = $view->new_display('default', 'Defaults', 'default');
  $handler->override_option('relationships', array(
    'flag_content_rel' => array(
      'label' => 'flag',
      'required' => 1,
      'flag' => 'abuse_comment',
      'user_scope' => 'any',
      'id' => 'flag_content_rel',
      'table' => 'comments',
      'field' => 'flag_content_rel',
      'relationship' => 'none',
    ),
    'nid' => array(
      'label' => 'Node',
      'required' => 0,
      'id' => 'nid',
      'table' => 'comments',
      'field' => 'nid',
      'relationship' => 'none',
    ),
    'uid' => array(
      'label' => 'Flag user',
      'required' => 0,
      'id' => 'uid',
      'table' => 'flag_content',
      'field' => 'uid',
      'relationship' => 'flag_content_rel',
    ),
  ));
  $handler->override_option('fields', array(
    'name_1' => array(
      'label' => 'Reported by',
      'link_to_user' => 1,
      'exclude' => 0,
      'id' => 'name_1',
      'table' => 'users',
      'field' => 'name',
      'relationship' => 'uid',
    ),
    'title' => array(
      'label' => 'Parent Node',
      'link_to_node' => 1,
      'exclude' => 0,
      'id' => 'title',
      'table' => 'node',
      'field' => 'title',
      'relationship' => 'nid',
    ),
    'name' => array(
      'label' => 'Comment author',
      'link_to_user' => 1,
      'exclude' => 0,
      'id' => 'name',
      'table' => 'comments',
      'field' => 'name',
      'relationship' => 'none',
    ),
    'comment' => array(
      'label' => 'Comment',
      'exclude' => 0,
      'id' => 'comment',
      'table' => 'comments',
      'field' => 'comment',
      'relationship' => 'none',
    ),
    'edit_comment' => array(
      'label' => 'Actions',
      'text' => '',
      'exclude' => 0,
      'id' => 'edit_comment',
      'table' => 'comments',
      'field' => 'edit_comment',
      'relationship' => 'none',
    ),
    'delete_comment' => array(
      'label' => 'Delete link',
      'text' => 'delete',
      'exclude' => 0,
      'id' => 'delete_comment',
      'table' => 'comments',
      'field' => 'delete_comment',
      'relationship' => 'none',
    ),
    'ops' => array(
      'id' => 'ops',
      'table' => 'flag_content',
      'field' => 'ops',
      'relationship' => 'flag_content_rel',
    ),
  ));
  $handler->override_option('filters', array(
    'status' => array(
      'operator' => '=',
      'value' => 0,
      'group' => '0',
      'exposed' => FALSE,
      'expose' => array(
        'operator' => FALSE,
        'label' => '',
      ),
      'id' => 'status',
      'table' => 'comments',
      'field' => 'status',
      'relationship' => 'none',
    ),
  ));
  $handler->override_option('access', array(
    'type' => 'role',
    'role' => array(
      '2' => 2,
    ),
  ));
  $handler->override_option('title', 'Comment Abuse');
  $handler->override_option('empty', 'No comments have been reported.');
  $handler->override_option('empty_format', '1');
  $handler->override_option('items_per_page', 25);
  $handler->override_option('use_pager', '1');
  $handler->override_option('style_plugin', 'table');
  $handler->override_option('style_options', array(
    'grouping' => '',
    'override' => 1,
    'sticky' => 0,
    'order' => 'asc',
    'columns' => array(
      'name_1' => 'name_1',
      'title' => 'title',
      'name' => 'name',
      'comment' => 'comment',
      'edit_comment' => 'edit_comment',
      'delete_comment' => 'edit_comment',
      'ops' => 'edit_comment',
    ),
    'info' => array(
      'name_1' => array(
        'sortable' => 1,
        'separator' => '',
      ),
      'title' => array(
        'sortable' => 0,
        'separator' => '',
      ),
      'name' => array(
        'sortable' => 1,
        'separator' => '',
      ),
      'comment' => array(
        'separator' => '',
      ),
      'edit_comment' => array(
        'separator' => ' | ',
      ),
      'delete_comment' => array(
        'separator' => '',
      ),
      'ops' => array(
        'separator' => '',
      ),
    ),
    'default' => '-1',
  ));
  $handler = $view->new_display('page', 'Page', 'page_1');
  $handler->override_option('path', 'admin/content/flags/abuse_comment');
  $handler->override_option('menu', array(
    'type' => 'tab',
    'title' => 'Reported comment abuse',
    'description' => '',
    'weight' => '0',
    'name' => 'navigation',
  ));
  $handler->override_option('tab_options', array(
    'type' => 'none',
    'title' => '',
    'description' => '',
    'weight' => 0,
  ));
  $views[$view->name] = $view;
}

function _flag_abuse_views_default_abuse_node(&$views) {
  $view = new view;
  $view->name = 'all_flag_abuse_node';
  $view->description = 'View for flag: Node Abuse';
  $view->tag = 'flag.abuse';
  $view->view_php = '';
  $view->base_table = 'node';
  $view->is_cacheable = FALSE;
  $view->api_version = 2;
  $view->disabled = FALSE; /* Edit this to true to make a default view disabled initially */
  $handler = $view->new_display('default', 'Defaults', 'default');
  $handler->override_option('relationships', array(
    'flag_content_rel' => array(
      'label' => 'abuse_node',
      'required' => 1,
      'flag' => 'abuse_node',
      'user_scope' => 'any',
      'id' => 'flag_content_rel',
      'table' => 'node',
      'field' => 'flag_content_rel',
      'relationship' => 'none',
    ),
    'uid' => array(
      'label' => 'user',
      'required' => 0,
      'id' => 'uid',
      'table' => 'flag_content',
      'field' => 'uid',
      'relationship' => 'flag_content_rel',
    ),
  ));
  $handler->override_option('fields', array(
    'name_1' => array(
      'label' => 'Report by',
      'link_to_user' => 1,
      'exclude' => 0,
      'id' => 'name_1',
      'table' => 'users',
      'field' => 'name',
      'relationship' => 'uid',
    ),
    'type' => array(
      'label' => 'Type',
      'link_to_node' => 0,
      'exclude' => 1,
      'id' => 'type',
      'table' => 'node',
      'field' => 'type',
      'relationship' => 'none',
    ),
    'title' => array(
      'id' => 'title',
      'table' => 'node',
      'field' => 'title',
      'label' => 'Title',
      'link_to_node' => 1,
    ),
    'name' => array(
      'label' => 'Author',
      'link_to_user' => 1,
      'id' => 'name',
      'table' => 'users',
      'field' => 'name',
    ),
    'body' => array(
      'label' => 'Body',
      'exclude' => 0,
      'id' => 'body',
      'table' => 'node_revisions',
      'field' => 'body',
      'relationship' => 'none',
    ),
    'edit_node' => array(
      'label' => 'Actions',
      'text' => 'edit',
      'exclude' => 0,
      'id' => 'edit_node',
      'table' => 'node',
      'field' => 'edit_node',
      'override' => array(
        'button' => 'Override',
      ),
      'relationship' => 'none',
    ),
    'delete_node' => array(
      'label' => '',
      'text' => 'remove',
      'exclude' => 0,
      'id' => 'delete_node',
      'table' => 'node',
      'field' => 'delete_node',
      'override' => array(
        'button' => 'Override',
      ),
      'relationship' => 'none',
    ),
    'ops' => array(
      'label' => '',
      'alter' => array(
        'alter_text' => 0,
        'text' => '',
        'make_link' => 0,
        'path' => '',
        'alt' => '',
        'prefix' => '',
        'suffix' => '',
        'help' => '',
        'trim' => 0,
        'max_length' => '',
        'word_boundary' => 1,
        'ellipsis' => 1,
        'html' => 0,
      ),
      'link_type' => '',
      'exclude' => 0,
      'id' => 'ops',
      'table' => 'flag_content',
      'field' => 'ops',
      'relationship' => 'flag_content_rel',
      'override' => array(
        'button' => 'Override',
      ),
    ),
  ));
  $handler->override_option('filters', array(
    'status' => array(
      'operator' => '=',
      'value' => 1,
      'group' => '0',
      'exposed' => FALSE,
      'expose' => array(
        'operator' => FALSE,
        'label' => '',
      ),
      'id' => 'status',
      'table' => 'node',
      'field' => 'status',
      'relationship' => 'none',
    ),
  ));
  $handler->override_option('access', array(
    'type' => 'role',
    'role' => array(
      '2' => '2',
    ),
    'perm' => '',
  ));
  $handler->override_option('title', 'Node abuse');
  $handler->override_option('empty', 'No content has been reported.');
  $handler->override_option('empty_format', '1');
  $handler->override_option('items_per_page', '25');
  $handler->override_option('use_pager', TRUE);
  $handler->override_option('style_plugin', 'table');
  $handler->override_option('style_options', array(
    'grouping' => 'type',
    'override' => 0,
    'sticky' => 0,
    'order' => 'asc',
    'columns' => array(
      'name_1' => 'name_1',
      'type' => 'type',
      'title' => 'title',
      'name' => 'name',
      'body' => 'body',
      'edit_node' => 'edit_node',
      'delete_node' => 'edit_node',
      'ops' => 'edit_node',
    ),
    'info' => array(
      'name_1' => array(
        'sortable' => 1,
        'separator' => '',
      ),
      'type' => array(
        'sortable' => 1,
        'separator' => '',
      ),
      'title' => array(
        'sortable' => 1,
        'separator' => '',
      ),
      'name' => array(
        'sortable' => 1,
        'separator' => '',
      ),
      'body' => array(
        'separator' => '',
      ),
      'edit_node' => array(
        'separator' => ' | ',
      ),
      'delete_node' => array(
        'separator' => '',
      ),
      'ops' => array(
        'separator' => '',
      ),
    ),
    'default' => '-1',
  ));
  $handler = $view->new_display('page', 'Page', 'page_1');
  $handler->override_option('path', 'admin/content/flags/abuse_node');
  $handler->override_option('menu', array(
    'type' => 'default tab',
    'title' => 'Reported node abuse',
    'description' => '',
    'weight' => '0',
    'name' => 'navigation',
  ));
  $handler->override_option('tab_options', array(
    'type' => 'normal',
    'title' => 'Reported abuse',
    'description' => '',
    'weight' => '',
  ));
  $views[$view->name] = $view;
}

function _flag_abuse_views_default_abuse_user(&$views) {
  $view = new view;
  $view->name = 'all_flag_abuse_user';
  $view->description = 'View for flag: User Abuse';
  $view->tag = 'flag.abuse';
  $view->view_php = '';
  $view->base_table = 'users';
  $view->is_cacheable = FALSE;
  $view->api_version = 2;
  $view->disabled = FALSE; /* Edit this to true to make a default view disabled initially */
  $handler = $view->new_display('default', 'Defaults', 'default');
  $handler->override_option('relationships', array(
    'flag_content_rel' => array(
      'label' => 'abuse_user',
      'required' => 1,
      'flag' => 'abuse_user',
      'user_scope' => 'any',
      'id' => 'flag_content_rel',
      'table' => 'users',
      'field' => 'flag_content_rel',
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
      'relationship' => 'none',
    ),
    'ops' => array(
      'label' => 'Ops',
      'exclude' => 0,
      'id' => 'ops',
      'table' => 'flag_content',
      'field' => 'ops',
      'relationship' => 'flag_content_rel',
    ),
  ));
  $handler->override_option('access', array(
    'type' => 'role',
    'role' => array(
      '2' => '2',
    ),
    'perm' => '',
  ));
  $handler->override_option('title', 'User abuse');
  $handler->override_option('empty', 'No users have been reported.');
  $handler->override_option('empty_format', '1');
  $handler->override_option('items_per_page', 30);
  $handler->override_option('use_pager', TRUE);
  $handler->override_option('style_plugin', 'table');
  $handler->override_option('style_options', array(
    'grouping' => '',
    'override' => 1,
    'sticky' => 0,
    'order' => 'asc',
    'columns' => array(
      'name' => 'name',
      'ops' => 'ops',
    ),
    'info' => array(
      'name' => array(
        'sortable' => 1,
        'separator' => '',
      ),
      'ops' => array(
        'separator' => '',
      ),
    ),
    'default' => 'name',
  ));
  $handler = $view->new_display('page', 'Page', 'page_1');
  $handler->override_option('path', 'admin/content/flags/abuse_user');
  $handler->override_option('menu', array(
    'type' => 'tab',
    'title' => 'Reported user abuse',
    'weight' => '0',
    'name' => 'navigation',
  ));
  $handler->override_option('tab_options', array(
    'type' => 'none',
    'title' => NULL,
    'weight' => 0,
  ));
  $views[$view->name] = $view;
}
