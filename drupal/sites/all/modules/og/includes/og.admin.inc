<?php
// $Id: og.admin.inc,v 1.15.4.2 2009/06/22 15:59:27 weitzman Exp $

/**
 * Menu callback to load settings form.
 */
function og_admin_settings() {
  // content types
  $is_configured = count(og_get_types('group')) && count(og_get_types('group_post'));
  if (!$is_configured) {
    form_set_error('content_types_table', t('You must designate at least one content type to act as a group node and another as a group post. <a href="!create">Create new content type</a> if needed.', array('!create' => url('admin/content/types/add', array('query' => drupal_get_destination())))));
  }
  $form['og_settings']['content_types'] = array(
    '#type' => 'fieldset', 
    '#title' => t('Content types'),
    '#description' => t('You may <a href="!create">create new content types</a> as needed. Note that you may have multiple types acting as groups and multiple types are allowed as group posts.', array('!create' => url('admin/content/types/add', array('query' => drupal_get_destination())))),
    '#collapsible' => TRUE, 
    '#collapsed' => $is_configured,
  );
  $map = og_types_map();
  $header = array(t('Type'), t('Usage'), t('Operations'));
  $rows = array();
  foreach (node_get_types() as $type) {
    $type_url_str = str_replace('_', '-', $type->type);
    $usage = variable_get('og_content_type_usage_'. $type->type, 'omitted');
    $rows[] = array(
      $type->name,
      $map[$usage],
      l(t('Edit'), "admin/content/node-type/$type_url_str", array('query' => drupal_get_destination())),      
    );
  }
  $form['og_settings']['content_types']['content_types_table'] = array('#value' => theme('table', $header, $rows));

  
  $form['og_settings']['group_details'] = array(
    '#type' => 'fieldset', 
    '#title' => t('Group details'), 
    '#collapsible' => TRUE, 
    '#collapsed' => TRUE
  );
  // groups directory visibility
  $options = array(
    t("New groups don't appear in the groups directory. Administrators control the directory exclusively."),
    t('New groups always appear in the groups directory.'),
    t('Group creator chooses whether her group appears in the directory. Defaults to <em>in directory</em>.'),
    t('Group creator chooses whether her group appears in the directory. Defaults to <em>not in directory</em>.'),
  );
  $form['og_settings']['group_details']['og_visibility_directory'] = array(
    '#type' => 'radios', 
    '#title' => t('Groups directory control'), 
    '#default_value' => variable_get('og_visibility_directory', OG_DIRECTORY_CHOOSE_TRUE), 
    '#description' =>t('Site admins always see the checkbox for adding a group to the <em>groups directory</em>. Note that changing this setting has no effect on existing posts. Re-save those posts to acquire this new setting.'), 
    '#options' => $options
  );
  
  // groups registration visibility
  $options = array(
    t("New groups don't appear on the registration form. Administrators control the form exclusively."),
    t('New groups always appear on the registration form.'),
    t('Group creator chooses whether her group appears on the registration form. Defaults to <em>on form</em>.'),
    t('Group creator chooses whether her group appears on the registration form. Defaults to <em>not on form</em>.'),
  );
  $form['og_settings']['group_details']['og_visibility_registration'] = array(
    '#type' => 'radios', 
    '#title' => t('Registration form control'), 
    '#default_value' => variable_get('og_visibility_registration', OG_REGISTRATION_CHOOSE_FALSE), 
    '#description' =>t('OG admins always see the checkbox for adding a group to the <em>registration form</em>. Note that changing this setting has no effect on existing posts. Re-save those posts to acquire this new setting.'), 
    '#options' => $options
  );

  // audience checkboxes
  $form['og_settings']['group_details']['og_audience_checkboxes'] = array(
    '#type' => 'checkbox', 
    '#title' => t('Audience checkboxes'), 
    '#default_value' => variable_get('og_audience_checkboxes', TRUE), 
    '#description' => t('Show each group that the user is a member of as a checkbox in the "Audience" section. This enables the member to place her post into multiple groups. If unchecked, simplify the user interface by omitting the checkboxes and assuming user wants to post into the current group. This simplification only applies to new nodes, and not to edits of existing nodes. Group administrators always see checkboxes.')
  );
  
  // audience required
  $options = array(t('Optional'), t('Required'));
  $form['og_settings']['group_details']['og_audience_required'] = array(
    '#type' => 'radios', 
    '#title' => t('Audience required'), 
    '#default_value' => variable_get('og_audience_required', 0), 
    '#options' => $options, 
    '#description' => t('Do you require that all (non administrator) posts be affiliated with a group? Note that changing this setting will affect existing posts when they are edited.')
  );
  unset($options);

  // Messages fieldset.
  $form['og_settings']['notifications'] = array(
    '#type' => 'fieldset',
    '#title' => t('Messaging & Notifications'),
    '#collapsible' => TRUE,
    '#collapsed' => TRUE
  );
  $form['og_settings']['notifications']['og_email_notification_pattern'] = array(
    '#type' => 'textfield',
    '#title' => t('Format of "From:" field'),
    '#default_value' => variable_get("og_email_notification_pattern", '@user_name  <@site_mail>'),
    '#description' => t('Specify the format of the "From:" field on outgoing notifications. Available variables: @user_mail, @user_name, @site_mail, @site_name. Note that the @user_mail token reveals the author\'s email address. If the admin email examples above appear blank, you need to set your site email in the "Site information" panel.'),
  );
  $form['og_settings']['notifications']['og_new_node_subject'] = array(
    '#type' => 'textfield',
    '#title' => t('New content subject'),
    '#description' => t('Subject of notification message for new content. Available variables: @group, !group_url, @type, @site, !content_url, !reply_url, @title, @subject, @node_full, @node_teaser, @username. %subject contains the comment title in the case of a comment but the node title in the case of a new post. @title is always the node title.'),
    '#default_value' => _og_mail_text('og_new_node_subject')
  );
  $form['og_settings']['notifications']['og_new_node_body'] = array(
    '#type' => 'textarea',
    '#title' => t('New content body'),
    '#rows' => 10,
    '#description' => t('Body of the notification for new content. Available variables: @group, !group_url, @type, @site, !content_url, !reply_url, @title, @subject, @node_full, @node_teaser, @username. @subject contains the comment title in the case of a comment but the node title in the case of a new post. %title is always the node title.'),
    '#default_value' => _og_mail_text('og_new_node_body')
  );
  $form['og_settings']['notifications']['og_admin_email_body'] = array(
    '#type' => 'textarea',
    '#title' => t('Group admin notification body'),
    '#rows' => 10,
    '#description' => t('The body of the message sent to users from the group admin. Available variables: @group, @body, @site, !url_group, !url_unsubscribe'),
    '#default_value' => _og_mail_text('og_admin_email_body')
  );
  $form['og_settings']['notifications']['og_approve_user_subject'] = array(
    '#type' => 'textfield',
    '#title' => t('User approved notification subject'),
    '#description' => t('The subject of the message sent to new approved members. Available variables: !group_url, @title'),
    '#default_value' => _og_mail_text('og_approve_user_subject')
  );
  $form['og_settings']['notifications']['og_approve_user_body'] = array(
    '#type' => 'textarea',
    '#title' => t('User approved notification body'),
    '#rows' => 10,
    '#description' => t('The body of the message sent to new approved members. Available variables: !group_url, @title'),
    '#default_value' => _og_mail_text('og_approve_user_body')
  );   
  $form['og_settings']['notifications']['og_deny_user_subject'] = array(
    '#type' => 'textfield',
    '#title' => t('User denied notification subject'),
    '#description' => t('The subject of the message sent to denied users. Available variables: !group_url, @title'),
    '#default_value' => _og_mail_text('og_deny_user_subject')
  );
  $form['og_settings']['notifications']['og_deny_user_body'] = array(
    '#type' => 'textarea',
    '#title' => t('User denied notification body'),
    '#rows' => 10,
    '#description' => t('The body of the message sent to denied users. Available variables: !group_url, @title'),
    '#default_value' => _og_mail_text('og_deny_user_body')
  );   
  $form['og_settings']['notifications']['og_invite_user_subject'] = array(
    '#type' => 'textfield',
    '#title' => t('Invite user notification subject'),
    '#description' => t('The subject of the message sent to users invited to join a group. Available variables: @group, @site, @description, !group_url, @body'),
    '#default_value' => _og_mail_text('og_invite_user_subject')
  );
  $form['og_settings']['notifications']['og_invite_user_body'] = array(
    '#type' => 'textarea',
    '#title' => t('Invite user notification body'),
    '#rows' => 10,
    '#description' => t('The body of the message sent to users invited to join a group. Available variables: @group, @site, @description, !group_url, @body'),
    '#default_value' => _og_mail_text('og_invite_user_body')
  );   
  $form['og_settings']['notifications']['og_request_user_subject'] = array(
    '#type' => 'textfield',
    '#title' => t('Request user notification subject'),
    '#description' => t("The subject of the message sent to a user's request to join a group. Available variables: @group, @username, !approve_url, !group_url"),
    '#default_value' => _og_mail_text('og_request_user_subject')
  );
  $form['og_settings']['notifications']['og_request_user_body'] = array(
    '#type' => 'textarea',
    '#title' => t('Request user notification body'),
    '#rows' => 10,
    '#description' => t("The body of the message sent to a user's request to join a group. Available variables: @group, @username, !approve_url, !group_url"),
    '#default_value' => _og_mail_text('og_request_user_body')
  );
  $form['og_settings']['notifications']['og_new_admin_subject'] = array(
    '#type' => 'textfield',
    '#title' => t('New admin user notification subject'),
    '#description' => t('The subject of the message sent to a new admin for a group. Available variables: @group, @username, !group_url'),
    '#default_value' => _og_mail_text('og_new_admin_subject')
  );
  $form['og_settings']['notifications']['og_new_admin_body'] = array(
    '#type' => 'textarea',
    '#title' => t('New admin user notification body'),
    '#rows' => 10,
    '#description' => t('The body of the message sent to a new admin for a group. Available variables: @group, @username, !group_url, !invite_url'),
    '#default_value' => _og_mail_text('og_new_admin_body')
  );

  return system_settings_form($form);
}
