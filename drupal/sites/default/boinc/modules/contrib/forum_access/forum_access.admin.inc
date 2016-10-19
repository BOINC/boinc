<?php

/**
 * @file forum_access.admin.inc
 *
 * Include file for forum_access.module, containing (sub-)page handling
 * (form_alter) and batch code.
 *
 */

/**
 * Rewrite the forum administration page with our new access rules.
 */
function _forum_access_forum_form(&$form, &$form_state, $is_container) {
  $tid = (isset($form['tid']['#value']) ? $form['tid']['#value'] : NULL);
  if (isset($tid) && !forum_access_access($tid, 'view', NULL, FALSE)) {
    drupal_access_denied();  // Deny access if the user doesn't have View access.
    module_invoke_all('exit');
    exit;
  }

  $roles = user_roles();
  if (isset($tid)) {  // edit
    $template_tid = variable_get('forum_access_default_template_tid', 0);
    $settings = _forum_access_get_settings($tid);
  }
  else {  // create
    $template_tid = variable_get('forum_access_new_template_tid', NULL);
    $settings = _forum_access_get_settings($template_tid);
  }
  $fa_priority = $settings['priority'];

  $form['forum_access'] = array(
    '#type' => 'fieldset',
    '#title' => t('Access control'),
    '#collapsible' => TRUE,
    '#tree' => TRUE,
  );

  $tr = 't';
  $variables = array(
    '!access_content'                 => '<em>'. l($tr('access content'), 'admin/user/permissions', array('fragment' => 'module-node', 'html' => TRUE)) .'</em>',
    '!access_comments'                => '<em>'. l($tr('access comments'), 'admin/user/permissions', array('fragment' => 'module-comment', 'html' => TRUE)) .'</em>',
    '!create_forum_topics'            => '<em>'. l($tr('create forum topics'), 'admin/user/permissions', array('fragment' => 'module-forum', 'html' => TRUE)) .'</em>',
    '!post_comments'                  => '<em>'. l($tr('post comments'), 'admin/user/permissions', array('fragment' => 'module-comment', 'html' => TRUE)) .'</em>',
    '!post_comments_without_approval' => '<em>'. l($tr('post comments without approval'), 'admin/user/permissions', array('fragment' => 'module-comment', 'html' => TRUE)) .'</em>',
    '!edit_own_forum_topics'          => '<em>'. l($tr('edit own forum topics'), 'admin/user/permissions', array('fragment' => 'module-forum', 'html' => TRUE)) .'</em>',
    '!edit_any_forum_topics'          => '<em>'. l($tr('edit any forum topics'), 'admin/user/permissions', array('fragment' => 'module-forum', 'html' => TRUE)) .'</em>',
    '!delete_own_forum_topics'        => '<em>'. l($tr('delete own forum topics'), 'admin/user/permissions', array('fragment' => 'module-forum', 'html' => TRUE)) .'</em>',
    '!delete_any_forum_topics'        => '<em>'. l($tr('delete any forum topics'), 'admin/user/permissions', array('fragment' => 'module-forum', 'html' => TRUE)) .'</em>',
    '!administer_comments'            => '<em>'. l($tr('administer comments'), 'admin/user/permissions', array('fragment' => 'module-comment', 'html' => TRUE)) .'</em>',
    '!administer_forums'              => '<em>'. l($tr('administer forums'), 'admin/user/permissions', array('fragment' => 'module-forum', 'html' => TRUE)) .'</em>',
    '!administer_nodes'               => '<em>'. l($tr('administer nodes'), 'admin/user/permissions', array('fragment' => 'module-node', 'html' => TRUE)) .'</em>',
  );
  if (!$is_container) {
    $form['forum_access']['permissions'] = array(
      '#type' => 'fieldset',
      '#title' => $tr('Permissions information'),
      '#collapsible' => TRUE,
      '#collapsed' => TRUE,
    );
    $form['forum_access']['permissions'][] = array(
      '#type' => 'markup',
      '#value' => '<div>'. t('Note that users need') .'<ul style="margin-top: 0"><li>'.
                           t('the !access_content and !access_comments permissions <strong>AND <em>View</em></strong> to be able to see this forum and its content at all,', $variables) .'</li><li>'.
                           t('the !create_forum_topics (and similar) permissions <strong>AND <em>Post</em></strong> to be able to create forum content, and', $variables) .'</li><li>'.
                           t('the !post_comments and (probably) !post_comments_without_approval permission <!TAG>AND <em>Comment</em></!TAG> to be able to post comments/replies;', $variables + array('!TAG' => (variable_get('forum_access_D5_legacy_mode', FALSE) ? 'del title="'. t('Drupal 5 legacy mode') .'"' : 'strong'))) .'</li><li>'.
                           t('the !edit_own_forum_topics or !edit_any_forum_topics (and similar) permissions <strong>AND (<em>Edit</em> AND <em>Comment</em></strong>) can be added if desired, <strong>plus</strong>', $variables) .'</li><li>'.
                           t('the !delete_own_forum_topics or !delete_any_forum_topics (and similar) permissions (<strong>OR <em>Delete</em></strong>) if desired;', $variables) .'</li><li>'.
                           t('the !administer_comments (global!) permission <strong>OR <em>Edit</em>/<em>Delete</em></strong> to be able to edit/delete comments;', $variables) .'</li><li>'.
                           t('the !administer_forums permission <strong>AND <em>View</em></strong> to be able to administer forums (and change access!).', $variables) .'</li></ul>'.
                           t('Furthermore note that content which is not published is treated in a different way by Drupal: it can be viewed only by its author or by users with the !administer_nodes permission. Unpublished comments and replies are accessible to users with <strong><em>Edit</em> OR <em>Delete</em></strong>, <strong>OR</strong> with the !administer_comments permission, but they are never counted on the forum page.', $variables) .'</div>',
    );
  }

  // Load a template:
  $vid = _forum_access_get_vid();
  $form['forum_access']['template'] = array(
      '#type' => 'fieldset',
      '#title' => $tr('Template'),
      '#collapsible' => TRUE,
      '#collapsed' => empty($template_tid),
  );
  $form['forum_access']['template']['taxonomy'][$vid] = taxonomy_form($vid, array($template_tid), '');
  $form['forum_access']['template']['taxonomy'][$vid]['#description'] = t("Select a forum and click !Load to retrieve that forum's settings as a starting point for this forum or container.", array('!Load' => '['. t('Load') .']'));
  $form['forum_access']['template']['load_button'] = array(
    '#type' => 'button',
    '#name' => 'load_template',
    '#value' => t('Load'),
    '#submit' => FALSE,
  );
  $form['forum_access']['template']['template_tid'] = array(
    '#type' => 'value',
    '#value' => NULL,
  );
  $form['forum_access']['template']['select_by_default'] = array(
    '#type' => 'checkbox',
    '#title' => t('Remember this selection.'),
    '#default_value' => FALSE,
  );
  $form['forum_access']['template']['load_for_new'] = array(
    '#type' => 'checkbox',
    '#title' => t("Use the selected forum's settings as defaults for new forums and containers."),
    '#default_value' => FALSE,
  );
  $form['forum_access']['#after_build'][] = '_forum_access_forum_form_after_build';

  // Column titles:
  $form['forum_access']['headers']['view'] = array(
    '#type' => 'item',
    '#prefix' => '<div class="forum-access-div">',
    '#title' => ($is_container ? t('View this container') : t('View this forum')),
    '#suffix' => '</div>',
  );
  $forum_vocabulary = taxonomy_vocabulary_load(_forum_access_get_vid());
  $form['forum_access']['headers']['create'] = array(
    '#type' => 'item',
    '#prefix' => '<div class="forum-access-div" id="forum-access-div-container">',
    '#title' => t('See this container in the %Forums selection list', array('%Forums' => $forum_vocabulary->name)),
    '#suffix' => '</div>',
  );
  if (!$is_container) {
    $form['forum_access']['headers']['create'] = array(  // overwrite!
      '#type' => 'item',
      '#prefix' => '<div class="forum-access-div">',
      '#title' => t('Post in this forum'),
      '#suffix' => '</div>',
    );
    $form['forum_access']['headers']['comment_create'] = array(
      '#type' => 'item',
      '#prefix' => '<div class="forum-access-div">',
      '#title' => t('Comment on posts'),
      '#suffix' => '</div>',
    );
    $form['forum_access']['headers']['update'] = array(
      '#type' => 'item',
      '#prefix' => '<div class="forum-access-div">',
      '#title' => t('Edit posts'),
      '#suffix' => '</div>',
    );
    $form['forum_access']['headers']['delete'] = array(
      '#type' => 'item',
      '#prefix' => '<div class="forum-access-div">',
      '#title' => t('Delete posts'),
      '#suffix' => '</div>',
    );
  }
  $form['forum_access']['headers']['clearer'] = array(
      '#value' => '<div class="forum-access-clearer"></div>',
  );
  
  // Column content (checkboxes):
  $form['forum_access']['view'] = array(
    '#type' => 'checkboxes',
    '#prefix' => '<div class="forum-access-div">',
    '#suffix' => '</div>',
    '#options' => $roles,
    '#default_value' => $settings['view'],
    '#process' => array('expand_checkboxes', '_forum_access_forum_form_disable_checkboxes'),
  );
  $form['forum_access']['create'] = array(
    '#type' => 'checkboxes',
    '#prefix' => '<div class="forum-access-div">',
    '#suffix' => '</div>',
    '#options' => $roles,
    '#default_value' => $settings['create'],
    '#process' => array('expand_checkboxes', '_forum_access_forum_form_disable_checkboxes'),
  );
  if (!$is_container) {
    $form['forum_access']['comment_create'] = array(
      '#type' => 'checkboxes',
      '#prefix' => '<div class="forum-access-div">',
      '#suffix' => '</div>',
      '#options' => $roles,
      '#default_value' => $settings['comment_create'],
      '#process' => array('expand_checkboxes', '_forum_access_forum_form_disable_checkboxes'),
    );
    $form['forum_access']['update'] = array(
      '#type' => 'checkboxes',
      '#prefix' => '<div class="forum-access-div">',
      '#suffix' => '</div>',
      '#options' => $roles,
      '#default_value' => $settings['update'],
      '#process' => array('expand_checkboxes', '_forum_access_forum_form_disable_checkboxes'),
    );
    $form['forum_access']['delete'] = array(
      '#type' => 'checkboxes',
      '#prefix' => '<div class="forum-access-div">',
      '#suffix' => '</div>',
      '#options' => $roles,
      '#default_value' => $settings['delete'],
      '#process' => array('expand_checkboxes', '_forum_access_forum_form_disable_checkboxes'),
    );
  }
  $form['forum_access']['clearer'] = array(
    '#type' => 'item',
    '#prefix' => '<div class="forum-access-clearer">',
    '#suffix' => '</div>',
    '#description' => t('For explanations of special cases, hover your mouse over role names.'),
  );
  if ($is_container) {
    $form['forum_access']['container_note'] = array(
      '#type' => 'item',
      '#description' => t('Users who can see any forum or container within this one should get the <strong><em>View</em></strong> grant. <br /> Users who can post to a forum within this container should get the <strong><em>See</em></strong> grant, so that this forum appears in the proper context in the selection list.', $variables),
    );
  }

  drupal_add_css(drupal_get_path('module', 'forum_access') .'/forum_access.css');

  // Find our moderator ACL:
  if (isset($tid)) {  // edit, not new
    $acl_id = acl_get_id_by_number('forum_access', $tid);
    if (!$acl_id) { // create one
      $acl_id = acl_create_new_acl('forum_access', NULL, $tid);
      // update every existing node in this forum to use this acl.
      $result = db_query("SELECT nid FROM {term_node} WHERE tid = %d", $tid);
      while ($node = db_fetch_object($result)) {
        // all privs to this ACL.
        acl_node_add_acl($node->nid, $acl_id, 1, 1, 1);
      }
    }
    $form['forum_access']['acl'] = acl_edit_form($acl_id, t('Moderators'));
    $form['forum_access']['acl'][] = array(
      '#type' => 'markup',
      '#value' => '<div>'. t('Moderators receive all grants above.') .'</div>',
      '#weight' => -1,
    );
    $form['forum_access']['acl']['note'] = array(
      '#type' => 'markup',
      '#value' => '<div>'. t('Note: Changes to moderators are not saved until you click [!Save] below.', array('!Save' => $tr('Save'))) .'</div>',
    );
    $form['forum_access']['acl']['#after_build'][] = '_forum_access_forum_form_after_build_acl0';
    $form['forum_access']['acl']['#after_build'] = array_reverse($form['forum_access']['acl']['#after_build']);
    $form['forum_access']['acl']['#after_build'][] = '_forum_access_forum_form_after_build_acl2';
  }

  foreach (module_implements('node_access_records') as $module) {
    $na_modules[$module] = $module;
  }
  unset($na_modules['forum_access']);
  unset($na_modules['acl']);
  if (count($na_modules) && !$is_container) {
    $form['forum_access']['interference'] = array(
      '#type' => 'fieldset',
      '#title' => t('Module interference'),
      '#collapsible' => TRUE,
    );
    $variables = array(
      '%content_type' => node_get_types('name', 'forum'),
      '!Forum_Access' => 'Forum Access',
      '!Content_Access' => l('Content Access', 'http://drupal.org/project/content_access'),
      '@Content_Access' => 'Content Access',
      '!ACL' => 'ACL',
      '!module_list' => '<ul><li>'. implode($na_modules, '</li><li>') .'</li></ul>',
    );
    $form['forum_access']['interference'][] = array(
      '#type' => 'item',
      '#value' => '<p>'. t("Besides !Forum_Access (and !ACL) you have installed the following node access module(s): !module_list   The grants of every module are combined for each node. Access can only be granted, not removed &mdash; if a certain module grants a permission, the other(s) cannot deny it.", $variables) .'</p>',
      '#description' => t('Forums can contain other content types besides %content_type; !Forum_Access will contribute the grants defined above to every node in this forum, but other node access control modules may also contribute their grants, especially to nodes of types other than %content_type.', $variables),
    );

    if (module_exists('content_access')) {
      $ca_settings = variable_get('content_access_settings', array());
      foreach (array('view', 'update', 'delete', 'per_node') as $type) {
        $value = content_access_get_settings($type, 'forum');
        if (!empty($value)) {
          $ca_interferes = TRUE;
        }
      }
      $ca_priority = content_access_get_settings('priority', 'forum');
      $is_conflict = $ca_priority >= $fa_priority && !empty($ca_interferes) || $ca_priority > $fa_priority;
      $variables += array(
        '!link' => l(t('@Content_Access configuration for the %content_type type', $variables), 'admin/content/node-type/forum/access', array('html' => TRUE)),
        '%Advanced' => $tr('Advanced'),
      );
      $specifically = ($ca_priority == $fa_priority ? t('Specifically, any grants given by !Content_Access cannot be taken back by !Forum_Access.', $variables) : '');
      if ($is_conflict) {
        $form['forum_access']['interference']['by_content_access'] = array(
          '#type' => 'fieldset',
          '#title' => 'Content Access',
          '#collapsible' => FALSE,
          '#attributes' => array('class' => 'error'),
        );
        $form['forum_access']['interference']['by_content_access'][] = array(
          '#value' => '<div>'. t('You have set the !Content_Access module to control access to content of type %content_type&mdash;this can interfere with proper operation of !Forum_Access!', $variables) ." $specifically</div>",
        );
        if ($ca_priority == $fa_priority) {
          $form['forum_access']['interference']['by_content_access'][] = array(
            '#value' => '<div>'. t("Unless you really know what you're doing, we recommend that you go to the !link page and clear all checkboxes. This will instruct @Content_Access to leave the %content_type nodes alone. However, if you put nodes of other content types into forums as well, then these content types will continue to have this problem.", $variables) .'</div>',
          );
        }
        else {
          $form['forum_access']['interference']['by_content_access'][] = array(
            '#value' => '<div>'. t("The priority of @Content_Access ($ca_priority) is higher than the priority of !Forum_Access ($fa_priority), which means the latter is <strong>completely disabled</strong> for the %content_type type! Unless you really know what you're doing, we recommend that you go to the !link page, change the priority (under %Advanced) to 0, and clear all checkboxes.", $variables) .'</div>',
          );
        }
        $form['forum_access']['interference']['by_content_access'][] = array(
          '#value' => '<div>'. t("Alternatively, you can give !Forum_Access priority over @Content_Access by either raising the priority of !Forum_Access in every forum above the priority of @Content_Access, or by lowering the priority of @Content_Access for the content types in question below the priority of !Forum_Access.", $variables) .'</div>',
        );
      }
      else {
        $form['forum_access']['interference'][] = array(
          '#value' => '<p>'. t('Note: You have installed the !Content_Access module, which has the capability to grant access to content that would otherwise be protected by !Forum_Access. Be careful when configuring @Content_Access!', $variables) .'</p>',
        );
      }
    }

    $form['forum_access']['interference']['advanced'] = array(
      '#type' => 'fieldset',
      '#title' => t('Advanced'),
      '#collapsible' => TRUE,
      '#collapsed' => !($fa_priority != 0),
    );
    $form['forum_access']['interference']['advanced']['priority'] = array(
      '#type' => 'weight',
      '#title' => t('Priority of !Forum_Access node grants in this forum', $variables),
      '#default_value' => $fa_priority,
      '#description' => t("If you have no other node access control modules installed, you should leave this at the default 0. <br /> Otherwise you can raise or lower the priority of !Forum_Access' grants. Out of all the grants contributed to a node, only those with the highest priority are used, and all others are discarded.", $variables),
    );
  }

  if (!$is_container) {
    $variables = array(
      '!Forum_Access'        => l('Forum Access', 'http://drupal.org/project/forum_access'),
      '!ACL'                 => l('ACL', 'http://drupal.org/project/acl'),
      '%Module_interference' => t('Module interference'),
      '!Forum_Access-dev'    => l('Forum&nbsp;Access&nbsp;6.x-1.x-dev', 'http://drupal.org/node/96795', array('html' => TRUE)),
      '!ACL-dev'             => l('ACL&nbsp;6.x-1.x-dev', 'http://drupal.org/node/96794', array('html' => TRUE)),
      '%devel_node_access'   => 'devel_node_access',
      '!Devel'               => l('Devel', 'http://drupal.org/project/devel'),
      '!DNA'                 => 'DNA',
      '!debug_mode'          => l('debug mode', 'admin/settings/devel', array('fragment' => 'edit-devel-node-access-debug-mode')),
      '!dna_summary'         => l('devel/node_access/summary', 'devel/node_access/summary'),
      '!Rebuild_permissions' => '['. $tr('Rebuild permissions') .']',
      '!Post_settings_link'  => l('admin/content/node-settings', 'admin/content/node-settings'),
      '!Forum_Access_'       => l('Forum Access', 'http://drupal.org/project/issues/forum_access'),
      '!ACL_'                => l('ACL', 'http://drupal.org/project/issues/acl'),
    );
    $form['forum_access']['troubleshooting'] = array(
      '#type' => 'fieldset',
      '#title' => t('Trouble-shooting node access'),
      '#collapsible' => TRUE,
      '#collapsed' => TRUE,
    );
    $form['forum_access']['troubleshooting'][] = array(
      '#type' => 'item',
      '#value' => '<div>'.    t("In case of problems, follow these steps until you've got it worked out:") .'<ol style="margin-top: 0"><li>'.
                              t("Update to the 'recommended' !Forum_Access and !ACL releases for your version of Drupal.", $variables) .'</li><li>'.
        (count($na_modules) ? t("Read %Module_interference above and update your other node access modules.", $variables) .'</li><li>' : '') .
                              t("Check the release notes of the development snapshots for issues that might have been fixed in !Forum_Access-dev or !ACL-dev since the latest release.", $variables) .'</li><li>'.
                              t("Install the %devel_node_access module (!DNA, part of the !Devel module) and enable its !debug_mode: !DNA will show you all the grants that actually control your nodes in a footer block on each node's page.", $variables) .'</li><li>'.
                              t("Additional insight can be gained from !dna_summary and by enabling the second !DNA block.", $variables) .'</li><li>'.
                              t("Click !Rebuild_permissions on !Post_settings_link and check DNA for changes.", $variables) .'</li><li>'.
                              t("Check the issues queues of !Forum_Access_ and !ACL_ for existing reports and possible solutions.", $variables) .'</li><li>'.
                              t("If all of this hasn't helped, then pick ONE node that is misbehaving, look at it using an account that can see the node (and that should NOT have access if that's your problem!), create a new issue in the issues queue, describe the problem... <ul><li> what did you do? </li><li> what did you expect? </li><li> what happened instead? </li></ul> ... and <strong>attach a screenshot of all the DNA records</strong> for that one node. <br /> Be sure to indicate paths (URLs) for every page and module that you mention.") .'</li></ol></div>',
    );
    $form['forum_access']['troubleshooting'][] = array(
      '#type' => 'item',
      '#value' => '<div>'. t("Note: You should not keep the !Devel module enabled on a production site.", $variables) .'</div>',
    );
  }

  if (!$is_container && isset($tid) && !node_access_needs_rebuild()) {
    $count = db_result(db_query("SELECT COUNT(DISTINCT n.nid) FROM {node} n INNER JOIN {term_node} tn ON tn.vid = n.vid WHERE tn.tid = %d", $tid));
    $limit = 20;   // from _node_access_rebuild_batch_operation()
    $threshold = variable_get('forum_access_batch_threshold', $limit);  // change the variable if you want
    $form['forum_access']['update_limit'] = array(
      '#type' => 'value',
      '#value' => $limit,
    );
    $form['forum_access']['update_choice'] = array(
      '#type' => 'radios',
      '#title' => 'Update the permissions',
      '#description' => t('<em>If</em> you make any node access changes, then each node in this forum needs to be updated. Hover over the radiobuttons for details.'),
      '#options' => NULL,
      0 => array(
        '#type' => 'radio',
        '#title' => t('for all %count nodes immediately', array('%count' => $count)),
        '#attributes' => array('title' => t('This option is the fastest, but with many nodes it can still take considerable time and memory. If it fails, it will leave your !node_access table in an inconsistent state.', array('!node_access' => '{node_access}'))),
        '#return_value' => 0,
        '#default_value' => ($count <= $threshold ? 0 : 1),
        '#parents' => array('forum_access', 'update_choice'),
      ),
      1 => array(
        '#type' => 'radio',
        '#title' => t('in batches of !limit now', array('!limit' => $limit)),
        '#attributes' => array('title' => t('The batch option will always work reliably, but it takes longer to complete.')),
        '#return_value' => 1,
        '#default_value' => ($count <= $threshold ? 0 : 1),
        '#parents' => array('forum_access', 'update_choice'),
      ),
      2 => array(
        '#type' => 'radio',
        '#title' => t('rebuild <strong>all</strong> permissions later'),
        '#attributes' => array('title' => t("This option will only set a flag to remind you to rebuild all permissions later; this is useful if you want to make multiple changes to your node access settings quickly and delay the updating until you're done.")),
        '#return_value' => 2,
        '#default_value' => ($count <= $threshold ? 0 : 1),
        '#parents' => array('forum_access', 'update_choice'),
      ),
      '#attributes' => array('class' => 'forum-access-flowed'),
    );
  }
  if (isset($tid)) {
    $form['forum_access']['force_update'] = array(
      '#type' => 'checkbox',
      '#title' => t('Update even if unchanged'),
    );
  }

  // Move some stuff down so our block goes in a nice place.
  $form['submit']['#weight'] = 10;
  $form['delete']['#weight'] = 10;

  $form['#validate'][] = '_forum_access_form_validate';
  $form['#submit'][] = '_forum_access_form_submit';
}

/**
 * Helper function to return a rid-indexed array of arrays, whose keys
 * are the permissions of the corresponding role.
 */
function _forum_access_get_role_permissions() {
  static $permissions;
  if (empty($permissions)) {
    $permissions[DRUPAL_AUTHENTICATED_RID] = array();
    $result = db_query('SELECT r.rid, p.perm FROM {role} r INNER JOIN {permission} p ON r.rid = p.rid ORDER BY r.rid');
    while ($role = db_fetch_object($result)) {
      $permissions[$role->rid] = ($role->rid == DRUPAL_ANONYMOUS_RID ? array() : $permissions[DRUPAL_AUTHENTICATED_RID]);
      $permissions[$role->rid] += array_flip(explode(', ', $role->perm));
    }
  }
  return $permissions;
}

function _forum_access_forum_form_disable_checkboxes($element) {
  global $user;
  $tr = 't';
  $moderator_rid = forum_access_query_moderator_rid();

  $permissions = _forum_access_get_role_permissions();
  $element_children = element_children($element);
  foreach ($element_children as $rid) {
    if ($rid == $moderator_rid) {
      $element[$rid]['#prefix'] = '<span title="'. t("This is the (temporary) @Forum_Moderator role; it has the '@administer_nodes' and '@administer_comments' permissions and is only used temporarily by @Forum_Access &mdash; do NOT give this role to any user!", array('@Forum_Moderator' => t('Forum Moderator'), '@administer_nodes' => $tr('administer nodes'), '@administer_comments' => $tr('administer comments'), '@Forum_Access' => 'Forum Access')) .'" class="forum-access-temporary-moderator">';
      $element[$rid]['#suffix'] = "</span>";
      $element[$rid]['#default_value'] = TRUE;
      $element[$rid]['#disabled'] = TRUE;
    }
    elseif ($element['#parents'][1] == 'create') {
      // Do nothing (Post is always mutable).
    }
    elseif ($element['#parents'][1] == 'comment_create') {
    }
    elseif ($element['#parents'][1] == 'view' && isset($permissions[$rid]['administer forums'])) {
      $element[$rid]['#title'] = '<em>'. $element[$rid]['#title'] .'</em>';
      $element[$rid]['#prefix'] = '<span title="'. t("This role has the '@administer_forums' permission, and granting '@View' enables the role holders to change the settings on this page, including @Access_control!", array('@administer_forums' => $tr('administer forums'), '@View' => t('View'), '@Access_control' => t('Access control'))) .'">';
      if (isset($permissions[$rid]['administer nodes'])) {
        $element[$rid]['#prefix'] = str_replace('">', ' '. t("Because the role also has the '@administer_nodes' permission, it has full access to all nodes either way.", array('@administer_nodes' => $tr('administer nodes'))) .'">', $element[$rid]['#prefix']);
      }
      $element[$rid]['#suffix'] = "</span>";
    }
    elseif (isset($permissions[$rid]['administer nodes'])) {
      $element[$rid]['#disabled'] = TRUE;
      $element[$rid]['#default_value'] = TRUE;
      $element[$rid]['#prefix'] = '<span title="'. ($rid != $moderator_rid
                                                    ? t("This role has the '@administer_nodes' permission and thus full access to all nodes.", array('@administer_nodes' => $tr('administer nodes')))
                                                    : t("This is the @Forum_Moderator role; it has the '@administer_nodes' and '@administer_comments' permissions and thus full access to all nodes and comments.", array('@Forum_Moderator' => t('Forum Moderator'), '@administer_nodes' => $tr('administer nodes'), '@administer_comments' => $tr('administer comments')))) .'">';
      $element[$rid]['#suffix'] = "</span>";
    }
  }
  return $element;
}

function _forum_access_forum_form_after_build_acl0($form, $form_state) {
  if (isset($form['#post']['forum_access']['template']['taxonomy'])) {
    // Get ACL's user_list for the template and replace it before ACL's after_build function gets its shot at it.
    $imv = array_values($form['#post']['forum_access']['template']['taxonomy']);
    $template_tid = reset($imv);
    if ($acl_id = acl_get_id_by_number('forum_access', $template_tid)) {
      $f = acl_edit_form($acl_id, 'DUMMY');
      $form['user_list']['#value'] = $f['user_list']['#default_value'];
    }
  }
  return $form;
}

function _forum_access_forum_form_after_build_acl2($form, $form_state) {
  if (!count(unserialize($form['user_list']['#default_value'])) && !count(unserialize($form['user_list']['#value']))) {
    $form['#collapsed'] = TRUE;
  }
  if ($form['user_list']['#default_value'] != $form['user_list']['#value']) {
    $form['note']['#value'] = preg_replace('/<div>/', '<div class="warning">', $form['note']['#value']);
  }
  return $form;
}

function _forum_access_forum_form_after_build($form, &$form_state) {
  if (isset($form_state['clicked_button']['#name']) && $form_state['clicked_button']['#name'] == $form['template']['load_button']['#name']) {
    // Load a setting from a template:
    $template_tid = reset(array_values($form['#post']['forum_access']['template']['taxonomy']));
    $form_state['values']['forum_access']['template']['template_tid'] = $template_tid;
    $form['template']['#collapsed'] = FALSE;

    $settings = _forum_access_get_settings($template_tid);
    foreach (array('view', 'create', 'comment_create', 'update', 'delete') as $grant_type) {
      if (empty($form[$grant_type])) {
        continue;
      }
      foreach (element_children($form[$grant_type]) as $tid) {
        $checked = array_search($tid, $settings[$grant_type]) !== FALSE;
        $form[$grant_type][$tid]['#value'] = ($checked ? $tid : 0);
      }
    }
    $form['interference']['advanced']['priority']['#value'] = $settings['priority'];
    if ($settings['priority'] != 0) {
      $form['interference']['advanced']['#collapsed'] = FALSE;
    }
  }
  elseif (is_array(reset($form_state['values']['forum_access']['template']['taxonomy']))) {
      $imv = reset($form_state['values']['forum_access']['template']['taxonomy']);
    $template_tid = reset($imv);
  }
  if (isset($template_tid)) {
    $form['template']['select_by_default']['#value'] = ($template_tid && $template_tid == variable_get('forum_access_default_template_tid', 0));
    $form['template']['load_for_new']['#value'] = ($template_tid && $template_tid == variable_get('forum_access_new_template_tid', 0));
  }
  return $form;
}

function _forum_access_form_validate($form, &$form_state) {
  global $user;

  if ($user->uid == 1) {
    return;
  }
  $access = $form_state['values']['forum_access']; // shortcut
  foreach ($access['view'] as $rid => $checked) {
    if ($checked && isset($user->roles[$rid])) {
      return;
    }
  }
  form_set_error('forum_access][view', t('You must assign %View access to a role that you hold.', array('%View' => 'View')));
}

function _forum_access_form_submit($form, &$form_state) {
  $access = $form_state['values']['forum_access']; // shortcut

  // Save template choice:
  $template_tid = reset(array_values($access['template']['taxonomy']));
  if ($access['template']['select_by_default']) {
    variable_set('forum_access_default_template_tid', $template_tid);
  }
  elseif (variable_get('forum_access_default_template_tid', 0) == $template_tid) {
    variable_del('forum_access_default_template_tid');
  }
  if ($access['template']['load_for_new']) {
    variable_set('forum_access_new_template_tid', $template_tid);
  }
  elseif (variable_get('forum_access_new_template_tid', 0) == $template_tid) {
    variable_del('forum_access_new_template_tid');
  }
  module_load_include('node.inc', 'forum_access');
  $moderator_rid = _forum_access_get_moderator_rid(TRUE);  // create the moderators role if it doesn't exist

  // check for changes
  $is_changed = $is_new = strpos($_GET['q'], 'admin/content/forum/add/') === 0;
  $is_changed = $is_changed || !empty($access['force_update']);
  $form_initial_values = $form;  // avoid Coder warning
  $form_initial_values = $form_initial_values['forum_access'];
  foreach (array('view', 'create', 'comment_create', 'update', 'delete') as $grant_type) {
    if (isset($form_initial_values[$grant_type])) {
      $defaults = $form_initial_values[$grant_type]['#default_value'];
      $defaults = array_flip($defaults);
      foreach ($access[$grant_type] as $rid => $checked) {
        $is_changed = $is_changed || (empty($form_initial_values[$grant_type][$rid]['#disabled']) && !empty($checked) != isset($defaults[$rid]));
      }
    }
  }
  if (!$is_changed && $access['acl']['user_list'] == $form_initial_values['acl']['user_list']['#default_value'] && $access['interference']['advanced']['priority'] == $form_initial_values['interference']['advanced']['priority']['#default_value']) {
    drupal_set_message(t('The content access permissions are unchanged.'));
    return;
  }

  $tid = $form_state['values']['tid'];
  db_query("DELETE FROM {forum_access} WHERE tid = %d", $tid);

  $fa_priority = isset($access['interference']['advanced']['priority']) ? $access['interference']['advanced']['priority'] : 0;
  if (array_key_exists('acl', $access)) {
    $moderators = unserialize($access['acl']['user_list']);
    acl_save_form($access['acl'], $fa_priority);
  }
  $permissions = _forum_access_get_role_permissions();
  foreach ($access['view'] as $rid => $checked) {
    if ($rid == $moderator_rid) {
      continue;
    }
    if (isset($permissions[$rid]['administer nodes'])) {
      // We prefer not to save records for node administrators, because these have access anyway.
      if (isset($permissions[$rid]['administer forums']) && $access['view'][$rid]) {
        // For forum administrators, View needs to be saved, ...
      }
      else {
        // ... otherwise forget View.
        $access['view'][$rid] = FALSE;
      }
      if ($access['view'][$rid] || $access['create'][$rid] || $access['comment_create'][$rid]) {
        db_query("INSERT INTO {forum_access} (tid, rid, grant_view, grant_update, grant_delete, grant_create, grant_comment_create, priority) VALUES (%d, %d, %d, %d, %d, %d, %d, %d)",
        $tid, $rid, !empty($access['view'][$rid]), 0, 0, !empty($access['create'][$rid]), !empty($access['comment_create'][$rid]), $fa_priority);
      }
    }
    else {
      db_query("INSERT INTO {forum_access} (tid, rid, grant_view, grant_update, grant_delete, grant_create, grant_comment_create, priority) VALUES (%d, %d, %d, %d, %d, %d, %d, %d)",
      $tid, $rid, (bool) $checked, !empty($access['update'][$rid]), !empty($access['delete'][$rid]), !empty($access['create'][$rid]), !empty($access['comment_create'][$rid]), $fa_priority);
    }
  }
  $tr = 't';
  $link = l($tr('edit'), 'admin/content/forum/edit/forum/'. $tid);
  watchdog('access', 'Changed grants for %forum forum.', array('%forum' => $form_state['values']['name']), WATCHDOG_NOTICE, $link);

  if (!$is_new && $form_state['values']['form_id'] != 'forum_form_container') {
    if (!isset($access['update_choice']) || $access['update_choice'] == 2) {
      node_access_needs_rebuild(TRUE);
    }
    elseif ($access['update_choice'] == 0) {
      // update immediately (but use the batch functions anyway
      $save_redirect = $form_state['redirect'];
      $form_state['redirect'] = $_GET['q'];
      $context = array();
      $pending_error_messages = drupal_get_messages('error', FALSE);
      $our_error_message_index = (isset($pending_error_messages['error']) ? count($pending_error_messages['error']) : 0);
      _forum_access_update_batch_finished(FALSE, array(), array());       // add our error message (in case we die underway)
      _forum_access_update_batch_operation($tid, 999999, 1, $context);
      $pending_error_messages = drupal_get_messages('error', TRUE);       // still alive, get and clear all 'error' messages
      unset($pending_error_messages['error'][$our_error_message_index]);  // remove our error message
      foreach ($pending_error_messages['error'] as $message) {            // replay any others
        drupal_set_message($message, 'error');
      }
      _forum_access_update_batch_finished(TRUE, array(), array());
      $form_state['redirect'] = $save_redirect;
    }
    else {
      // mass update in batch mode, modeled after node.module
      $limit = $access['update_limit'];
      $count = db_result(db_query("SELECT COUNT(DISTINCT n.nid) FROM {node} n INNER JOIN {term_node} tn ON tn.vid = n.vid WHERE tn.tid = %d", $tid));
      $batch = array(
        'title' => t('Updating content access permissions'),
        'file' => drupal_get_path('module', 'forum_access') .'/forum_access.admin.inc',
        'operations' => array(
          array('_forum_access_update_batch_operation', array($tid, $limit, $count)),
        ),
        'finished' => '_forum_access_update_batch_finished'
      );
      batch_set($batch);
    }
  }

  variable_del('forum_access_rids');  // clear cache
}

/**
 * Batch operation for forum_access_form_submit().
 *
 * This is a multistep operation : we go through all nodes by packs of 20.
 * The batch processing engine interrupts processing and sends progress
 * feedback after 1 second execution time.
 */
function _forum_access_update_batch_operation($tid, $limit, $count, &$context) {
  if (empty($context['sandbox'])) {
    // Initiate multistep processing.
    $context['sandbox']['progress'] = 0;
    $context['sandbox']['current_node'] = 0;
    $context['sandbox']['max'] = $count;
  }

  // Process the next 20 nodes.
  $result = db_query_range("SELECT DISTINCT n.nid FROM {node} n INNER JOIN {term_node} tn ON tn.vid = n.vid WHERE n.nid > %d AND tn.tid = %d ORDER BY n.nid ASC", $context['sandbox']['current_node'], $tid, 0, $limit);
  while ($row = db_fetch_array($result)) {
    $loaded_node = node_load($row['nid'], NULL, TRUE);
    // To preserve database integrity, only aquire grants if the node
    // loads successfully.
    if (!empty($loaded_node)) {
      node_access_acquire_grants($loaded_node);
    }
    $context['sandbox']['progress']++;
    $context['sandbox']['current_node'] = $loaded_node->nid;
  }

  // Multistep processing : report progress.
  if ($context['sandbox']['progress'] != $context['sandbox']['max']) {
    $context['finished'] = $context['sandbox']['progress'] / $context['sandbox']['max'];
  }
}

/**
 * Post-processing for forum_access_form_submit().
 */
function _forum_access_update_batch_finished($success, $results, $operations) {
  if ($success) {
    drupal_set_message(t('The content access permissions have been updated.'));
    cache_clear_all();
  }
  else {
    drupal_set_message(t('The content access permissions have not been properly updated.'), 'error');
  }
}

/**
 * Add our settings to the forum administration settings page.
 */
function _forum_access_forum_admin_settings_form(&$form, &$form_state) {
  $variables = array(
    '!Forum_Access' => 'Forum Access',
    '%View' => 'View',
    '%Post' => 'Post',
    '%post_comments' => 'post comments',
  );
  $form['forum_access'] = array(
    '#type' => 'fieldset',
    '#title' => 'Forum Access',
    '#attributes' => array('id' => 'edit-forum-admin-settings-forum-access'),
  );
  $form['forum_access']['note'] = array(
    '#type'          => 'item',
    '#value'         => t('Note: All other !Forum_Access controls are on the administration pages of the individual forums.', $variables),
  );
  $form['forum_access']['forum_access_D5_legacy_mode'] = array(
    '#type'          => 'checkbox',
    '#title'         => t('Drupal 5 legacy mode', $variables),
    '#default_value' => variable_get('forum_access_D5_legacy_mode', FALSE),
    '#description'   => t('In Drupal 5, comment posting was not restricted by !Forum_Access; users with %View access (and the %post_comments permission) were always allowed to post forum comments. Starting with Drupal 6, posting comments is now restricted to users with %Post access. Turn this option on to revert to the old behavior. The default is OFF.', $variables),
  );
  $buttons = $form['buttons'];
  unset($form['buttons']);
  $form['buttons'] = $buttons;
}

/**
 * Helper function to retrieve the settings for a forum.
 */
function _forum_access_get_settings($tid = NULL) {
    $return = array('view' => array(), 'create' => array(), 'comment_create' => array(), 'update' => array(), 'delete' => array(), 'priority' => 0);
  if (!isset($tid)) {
    // Default to all users can read; all logged in users can post and comment.
    $return['view'] = array(DRUPAL_ANONYMOUS_RID, DRUPAL_AUTHENTICATED_RID);
    $return['create'] = array(DRUPAL_AUTHENTICATED_RID);
    $return['comment_create'] = array(DRUPAL_AUTHENTICATED_RID);
  }
  else {
    $result = db_query("SELECT * FROM {forum_access} where tid = %d", $tid);
    while ($access = db_fetch_object($result)) {
      if ($access->grant_view) {
        $return['view'][] = $access->rid;
      }
      if ($access->grant_update) {
        $return['update'][] = $access->rid;
      }
      if ($access->grant_delete) {
        $return['delete'][] = $access->rid;
      }
      if ($access->grant_create) {
        $return['create'][] = $access->rid;
      }
      if ($access->grant_comment_create) {
        $return['comment_create'][] = $access->rid;
      }
      if ($access->rid == DRUPAL_AUTHENTICATED_RID) {  // this is our reference
        $return['priority'] = $access->priority;
      }
    }
  }
  return $return;
}

/**
 * Remove unusable 'edit' links from overview form.
 */
function _forum_access_forum_overview(&$form, &$form_state) {
  global $user;
  if ($user->uid == 1) {
    return;
  }
  foreach ($form as $key => $value) {
    if (preg_match('/^tid:(.*):0$/', $key, $matches)) {
      if (!forum_access_access($matches[1], 'view', NULL, FALSE)) {
        $form[$key]['edit']['#access'] = FALSE;
        if (preg_match('|<a [^>]*>([^<]*)</a>|', $form[$key]['view']['#value'], $matches)) {
          $form[$key]['view']['#value'] = $matches[1];
        }
      }
    }
  }
}

/**
 * We must know when a role is deleted.
 */
function _forum_access_user_admin_role_form(&$form, &$form_state) {
  $form['#submit'][] = '_forum_access_user_admin_role_submit';
}

/**
 * If a role is deleted, we remove the grants it provided.
 * Also, we complain if the Forum Moderator role is deleted.
 */
function _forum_access_user_admin_role_submit($form, &$form_state) {
  if ($form_state['values']['op'] == $form_state['values']['delete']) {
    $rid = $form_state['values']['rid'];
    db_query("DELETE FROM {forum_access} WHERE rid = %d", $rid);
    db_query("DELETE FROM {node_access} WHERE gid = %d AND realm = 'forum_access'", $rid);
    if ($rid === forum_access_query_moderator_rid()) {
      drupal_set_message(t('The role you have just deleted is required by !Forum_Access; it will be recreated automatically.', array('!Forum_Access' => 'Forum Access')), 'error');
    }
  }
}

/**
 * Add warnings on Content Access admin forms where CA wants
 * to control the same content types as we do.
 */
function _forum_access_content_access_admin_form() {
  $tr = 't';
  $variables = array(
    '!Content_Access' => 'Content Access',
    '!Forum_Access' => 'Forum Access',
    '!Forum_Access_link' => l('Forum Access', 'admin/content/forum'),
    '%anonymous_user' => $tr('anonymous user'),
    '%authenticated_user' => $tr('authenticated user'),
    '%Advanced' => $tr('Advanced'),
  );
  if (arg(3) == 'forum') {
    drupal_set_message(t('Note: In Drupal, access can only be granted, not taken away. Whatever access you grant here will not be reflected in the !Forum_Access_link settings, but !Forum_Access can only allow <i>more</i> access, not less.', $variables)
      .'<br /><span class="error">'. t('Specifically, any rights granted to the %anonymous_user and/or the %authenticated_user will <b>override</b> the settings of !Forum_Access!', $variables) .'</span>'
      .'<br />'. t('To avoid conflicts with !Forum_Access settings, you may want to lower the priority of !Content_Access (under %Advanced below) below the priority of !Forum_Access for the content types that you want to be controlled by !Forum_Access.', $variables), 'warning');
  }
  else {
    $vid = _forum_access_get_vid();
    $vocabulary = taxonomy_vocabulary_load($vid);
    if (isset($vocabulary->nodes[arg(3)])) {
      drupal_set_message(t('Note: Nodes of this content type can be put inside forums, where access to them will also be controlled by !Forum_Access.<br />In Drupal, access can only be granted, not taken away. Whatever access you grant here will not be reflected on the !Forum_Access_link settings, and vice versa, but any node access module can only allow <i>more</i> access, not less.', $variables), 'warning');
    }
  }
}

/**
 * Keep the user from changing the permissions of the Forum Moderator role.
 *
 * Disable the checkboxes in the Forum Moderators column on
 * admin/user/permissions.
 */
function _forum_access_user_admin_perm_form(&$form, &$form_state) {
  $moderator_rid = forum_access_query_moderator_rid();
  if (isset($moderator_rid) && array_key_exists($moderator_rid, $form['role_names'])) {
    drupal_set_message(t('The %role role is used internally by the @Forum_Access module and must not be changed!', array('%role' => $form['role_names'][$moderator_rid]['#value'], '@Forum_Access' => 'Forum Access')), 'warning', FALSE);
    $form['checkboxes'][$moderator_rid]['#disabled'] = TRUE;
  }
  array_unshift($form['#submit'], '_forum_access_user_admin_perm_form_submit');
}

/**
 * Submit handler for the admin/user/permissions form.
 *
 * Avoid losing the Forum Moderators record by removing the (empty!) FM values
 * before the real handler gets them.
 */
function _forum_access_user_admin_perm_form_submit($form, &$form_state) {
  $moderator_rid = forum_access_query_moderator_rid();
  if (isset($moderator_rid)) {
    unset($form_state['values'][$moderator_rid]);
  }
}

/**
 * Keep the user from bulk-assigning the Forum Moderator role.
 *
 * Remove the Forum Moderator role from the selection list on
 * admin/user/user.
 */
function _forum_access_user_admin_account_form(&$form, &$form_state) {
  $moderator_rid = forum_access_query_moderator_rid();
  if (isset($moderator_rid)) {
    unset($form['options']['operation']['#options'][t('Add a role to the selected users')]["add_role-$moderator_rid"]);
  }
}

/**
 * Keep the user from assigning the Forum Moderator role.
 *
 * Disable the Forum Moderator checkbox on user/UID/edit.
 */
function _forum_access_user_profile_form(&$form, &$form_state) {
  //dpm($form, '_forum_access_user_profile_form()');
  $moderator_rid = forum_access_query_moderator_rid();
  if (isset($moderator_rid) && isset($form['account']['roles']['#options'][$moderator_rid])) {
    $form['account']['roles'][$moderator_rid] = array(
      '#type' => 'checkbox',
      '#title' => $form['account']['roles']['#options'][$moderator_rid] .' ('. t('reserved for internal use by the @Forum_Access module', array('@Forum_Access' => 'Forum Access')) .')',
      '#default_value' => in_array($moderator_rid, $form['account']['roles']['#default_value']),
      '#disabled' => TRUE,
    );
    unset($form['account']['roles']['#options'][$moderator_rid]);
  }
}

/**
 * Create the Forum Moderator role.
 */
function _forum_access_create_moderator_rid($verbose = FALSE) {
  $tr = 't';
  $variables = array('!Forum_Access' => 'Forum Access', '%administer_comments' => $tr('administer comments'), '%administer_nodes' => $tr('administer nodes'));
  $role_name = t('Forum Moderator');
  $role = new stdClass();
  $role->name = $role_name;
  for ($i = 2; $i <= 12; ++$i) {
    $variables['%role'] = $role->name;
    if (!db_result(db_query("SELECT COUNT(rid) FROM {role} WHERE name = '%s'", $role->name)) &&
        drupal_write_record('role', $role)) {
      $rid = $role->rid;
      variable_set('forum_access_moderator_rid', $rid);
      $permission = new stdClass();
      $permission->rid = $rid;
      $permission->perm = 'administer comments, administer nodes, post comments, post comments without approval';
      drupal_write_record('permission', $permission);
      $msg = t('!Forum_Access has created a new role named %role and given it the %administer_nodes and %administer_comments permissions. This role is used internally by !Forum_Access. You can change the name of the role as you like, but you must keep it unmodified otherwise.', $variables);
      if ($verbose) {
        drupal_set_message($msg, 'warning');
      }
      watchdog('user', $msg, NULL, WATCHDOG_NOTICE);
      return $rid;
    }
    else {
      $msg = t('!Forum_Access cannot create the %role role!', $variables);
      watchdog('user', $msg, NULL, WATCHDOG_WARNING);
      drupal_set_message($msg .' '. t('Is it already in use?'), 'error');
      $role->name = $role_name .' '. $i;
    }
  }
  $msg = t('!Forum_Access has given up and will not work correctly! Rename one of the roles listed above, so that !Forum_Access can use its name.', $variables);
  drupal_set_message($msg, 'error');
  watchdog('user', $msg, NULL, WATCHDOG_CRITICAL);
}

