<?php
// $Id$

/**
* @file
* Administration page callbacks for the boinccore module.
*/

/**
  * The BOINC community form allows the setting of requirements for interaction
  * in the community sections of the site.
  */
function boinccore_admin_community(&$form_state) {
  $form = array();
  $default = array(
    'min_credit_to_post' => variable_get('boinc_comment_min_credit', 0),
    'penalty_period' => variable_get('boinc_penalty_period', 7*24*60*60),
    'moderator_mailing_list' => variable_get('boinc_moderator_mailing_list', ''),
    'moderator_force_individual_notifications' => variable_get('boinc_moderator_force_individual_notifications', 0),
  );

  // Define the form
  $form['boinc_comment_min_credit'] = array(
    '#type' => 'textfield',
    '#title' => t('Credit required for community participation'),
    '#default_value' => $default['min_credit_to_post'],
    '#description' => t('Set a minimum threshold of credit a user must have
      earned before being allowed to create a profile, post comments or participate in forums.'),
  );
  $form['boinc_penalty_period'] = array(
    '#type' => 'textfield',
    '#title' => t('Penalty period'),
    '#default_value' => $default['penalty_period'],
    '#description' => t('The amount of time (in seconds) that user privileges
      are revoked when a user is banned from community participation.'),
  );
  $form['boinc_moderator_mailing_list'] = array(
    '#type' => 'textfield',
    '#title' => t('Moderator mailing list'),
    '#default_value' => $default['moderator_mailing_list'],
    '#description' => t('As an alternative to sending moderator notifications
      to individual users with the moderator role, enter a mailing list address
      here to be the only target of the notification emails.'),
  );
  $form['boinc_moderator_force_individual_notifications'] = array(
    '#type' => 'checkbox',
    '#title' => t('Always send individual notifications'),
    '#default_value' => $default['moderator_force_individual_notifications'],
    '#description' => t('Continue to send individual moderator notification
      emails even when a mailing list is provided.'),
  );

  $form['tools'] = array(
    '#title' => t('Tools'),
    '#type' => 'fieldset',
    '#description' => '',
    '#collapsible' => TRUE,
    '#collapsed' => FALSE,
    '#attributes' => array('class' => '')
  );
  $form['tools']['reset_uotd'] = array(
    '#type' => 'submit',
    '#value' => t('Force new UOTD'),
    '#submit' => array('boinccore_reset_user_of_the_day'),
  );

  return system_settings_form($form);
}

/**
  * Validate the BOINC community form.
  */
function boinccore_admin_community_validate($form, &$form_state) {
  $values = $form_state['values'];
  if (!is_numeric($values['boinc_comment_min_credit'])) {
    form_set_error('boinc_comment_min_credit', t('The minimum credit value must be a number.'));
  }
}

/**
  * Handle post-validation submission of BOINC community form.
  */
function boinccore_admin_community_submit($form, &$form_state) {
  drupal_set_message(t('BOINC community settings have been updated.'));
}

/**
 * Force a reset of the user of the day
 */
function boinccore_reset_user_of_the_day() {
  if (boincuser_select_user_of_the_day()) {
    drupal_set_message(t('The user of the day has been reset.'));
  }
}

/**
  * The standard page generator form provides an interface to create standard
  * pages, such as a Help or About page.
  */
function boinccore_admin_default_content(&$form_state) {
  $form = array();

  // Define the form
  $form['page'] = array(
    '#type' => 'select',
    '#title' => t('Generate default content for'),
    '#description' => t('Generate some standard pages for a Drupal-BOINC Web site. You may generate all pages or specific pages. If a page already exists, it will <b>not</b> be overwritten. Pages will be generated with the path \'/page\', e.g., \'/help\'. Note the absense of \'content/\'.<p>The pages generated here will have very limited content. Most will have content such as \'Add a link to your FAQ here.\', and it is the admin or editors responsibility to add the necessary content.'),
    '#options' => array(
      0 => t('All standard pages'),
//      'about' => t('About page'),
      'help' => t('Help page'),
      'rules' => t('Rules and Policies page'),
      'moderation' => t('Moderation information page'),
    ),
  );
  $form['submit'] = array(
    '#type' => 'submit',
    '#value' => 'Generate content',
  );

  return $form;
}

/**
  * Generate content based on the selection made on the form
  */
function boinccore_admin_default_content_submit($form, &$form_state) {
  $option = $form_state['values']['page'];
  $page_list = boinccore_admin_default_content_generate($option);
  $pages_generated = count($page_list);
  if ($pages_generated > 0) {
    switch ($pages_generated) {
    case 1:
      $page_list = reset($page_list);
      break;
    case 2:
      $page_list = implode(' and ', $page_list);
      break;
    default:
      end($page_list);
      $page_list[key($page_list)] = 'and ' . $page_list[key($page_list)];
      $page_list = implode(', ', $page_list);
    }
    drupal_set_message(t('Default content has been generated for the @listed
      @pages.', array(
        '@listed' => $page_list,
        '@pages' => format_plural($pages_generated, 'page', 'pages'),
      )
    ));
  }
  else {
    if ($option) {
      drupal_set_message(t('The @specified page already exists.',
        array('@specified' => $option)), 'warning');
    }
    else {
      drupal_set_message(t('All pages already exist, so none can be recreated
        automatically.'), 'warning');
    }
    drupal_set_message(t('To recreate a page, first delete the existing page (or
      remove its URL alias), then try this function again.'), 'warning');
  }
}

/**
  *
  */
function boinccore_admin_default_content_generate($pages = array()) {
  if (!$pages) {
    // If not specified, attempt to generate all pages by default
    $pages = array(
      'help',
//      'about',
      'rules',
      'moderation',
    );
  }
  elseif (!is_array($pages)) {
    $pages = array($pages);
  }

  $pages_generated = array();
  if (in_array('about', $pages)) {
    // Create the default About page, if it doesn't exist
    // TODO: what is the default About page...?
    if (!drupal_lookup_path('source', 'about')) {
      drupal_set_message('The about page doesn\'t yet have a definition...', 'warning');
      //$pages_generated[] = 'about';
    }
  }
  if (in_array('help', $pages)) {
    // Create the default Help page, if it doesn't exist
    if (!drupal_lookup_path('source', 'help')) {
      $node = array(
        'type' => 'page',
        'title' => t('Help'),
        'uid' => 1,
        'status' => 1,  // published
        'promote' => 0, // not promoted to front page
        'path' => 'help',
        'pathauto_perform_alias' => FALSE,
        'created' => time(),
        'changed' => time(),
        'comment' => 0, // disable comments
        'moderate' => 0,
        'body' => preg_replace('/>\s+</', '><',
          '<p>Use this page to provide help and support for your site\'s
          visitors:</p>
          <ul>
            <li>Add a link to your primary forum for getting help</li>
            <li>Add a link to a page describing your screensaver</li>
            <li>Add links to your social media outlets like Facebook or Twitter</li>
            <li>Get help regarding <a href="http://boinc.berkeley.edu/help.php" target="_blank">BOINC in general</a></li>
          </ul>'),
        'sticky' => 0,
        'format' => 4,
        //'teaser' => $teaser,
      );
      $node = (object) $node; // node_save requires an object
      node_save($node);

      $item = array(
        'link_path' => drupal_get_normal_path('help'),
        'link_title' => t('Help'),
        'menu_name' => 'primary-links',
        'weight' => 50,
      );
      menu_link_save($item);
      $pages_generated[] = 'help';
    }
  }
  if (in_array('rules', $pages)) {
    // Create the default Rules page, if it doesn't exist
    if (!drupal_lookup_path('source', 'rules-and-policies')) {
      // Create node
      $node = array(
        'type' => 'page',
        'title' => t('Rules and Policies'),
        'uid' => 1,
        'status' => 1,  // published
        'promote' => 0, // not promoted to front page
        'path' => 'rules-and-policies',
        'pathauto_perform_alias' => FALSE,
        'created' => time(),
        'changed' => time(),
        'comment' => 0, // disable comments
        'moderate' => 0,
        'body' => preg_replace('/>\s+</', '><',
          '<p>Use this page to list your sites rules and policies.</p>'),
        'sticky' => 0,
        'format' => 4,
        //'teaser' => $teaser,
      );
      $node = (object) $node; // node_save requires an object
      node_save($node);

      // Create menu item to node
      $item = array(
        'link_path' => drupal_get_normal_path('rules-and-policies'),
        'link_title' => t('Rules and Policies'),
        'menu_name' => 'menu-footer-links',
        'weight' => 0,
      );
      menu_link_save($item);
      $pages_generated[] = 'rules-and-policies';
    }
  }
  if (in_array('moderation', $pages)) {
    // Create the default Moderation page, if it doesn't exist
    if (!drupal_lookup_path('source', 'moderation')) {
      // Create node
      $node = array(
        'type' => 'page',
        'title' => t('Moderation'),
        'uid' => 1,
        'status' => 1,  // published
        'promote' => 0, // not promoted to front page
        'path' => 'moderation',
        'pathauto_perform_alias' => FALSE,
        'created' => time(),
        'changed' => time(),
        'comment' => 0, // disable comments
        'moderate' => 0,
        'body' => preg_replace('/>\s+</', '><',
          '<p>Use this page to describe your sites moderation policy.</p>'),
        'sticky' => 0,
        'format' => 4,
        //'teaser' => $teaser,
      );
      $node = (object) $node; // node_save requires an object
      node_save($node);

      // Create menu item to node
      $item = array(
        'link_path' => drupal_get_normal_path('moderation'),
        'link_title' => t('Moderation'),
        'menu_name' => 'menu-footer-links',
        'weight' => 0,
      );
      menu_link_save($item);
      $pages_generated[] = 'moderation';
    }
  }

  // Rebuild drupal menus
  menu_rebuild();
  return $pages_generated;
}
