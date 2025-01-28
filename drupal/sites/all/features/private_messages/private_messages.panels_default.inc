<?php

/**
 * Implementation of hook_default_panels_mini().
 */
function private_messages_default_panels_mini() {
  $export = array();

  $mini = new stdClass;
  $mini->disabled = FALSE; /* Edit this to true to make a default mini disabled initially */
  $mini->api_version = 1;
  $mini->name = 'user_mail_summary';
  $mini->category = 'BOINC';
  $mini->admin_title = 'User mail summary';
  $mini->admin_description = 'Mail widget for user dashboard';
  $mini->requiredcontexts = array();
  $mini->contexts = array();
  $mini->relationships = array();
  $display = new panels_display;
  $display->layout = 'onecol';
  $display->layout_settings = array();
  $display->panel_settings = array(
    'style_settings' => array(
      'default' => NULL,
      'middle' => NULL,
    ),
  );
  $display->cache = array();
  $display->title = 'Messages';
  $display->storage_type = 'panels_mini';
  $display->storage_id = 'user_mail_summary';
  $display->content = array();
  $display->panels = array();
    $pane = new stdClass;
    $pane->pid = 'new-1';
    $pane->panel = 'middle';
    $pane->type = 'custom';
    $pane->subtype = 'custom';
    $pane->shown = TRUE;
    $pane->access = array();
    $pane->configuration = array(
      'admin_title' => 'Mail summary',
      'title' => 'Messages',
      'body' => '<?php
  $l1 = url(\'messages\');
  $l2 = url(\'messages/new\');
?>
<ul class="tab-list">
  <li class="first tab"><div>
    <a href="<?php echo $l1;?>"><?php print bts(\'Inbox\', array(), NULL, \'boinc:account-dashboard\'); ?></a><div class="item-count-wrapper"><span class="item-count"><?php echo privatemsg_unread_count(); ?></span></div></div>
  </li>
  <li class="last tab"><a href="<?php echo $l2?>"><?php print bts(\'Compose new\', array(), NULL, \'boinc:account-dashboard\'); ?></a></li>
</ul>',
      'format' => '3',
      'substitute' => TRUE,
    );
    $pane->cache = array();
    $pane->style = array(
      'settings' => NULL,
    );
    $pane->css = array();
    $pane->extras = array();
    $pane->position = 0;
    $display->content['new-1'] = $pane;
    $display->panels['middle'][0] = 'new-1';
  $display->hide_title = PANELS_TITLE_NONE;
  $display->title_pane = 'new-1';
  $mini->display = $display;
  $export['user_mail_summary'] = $mini;

  return $export;
}
