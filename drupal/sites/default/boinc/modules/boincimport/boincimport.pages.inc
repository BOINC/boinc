<?php
// $Id$

/**
 * Callback admin/boinc/import/settings
 */
function boincimport_admin_settings() {
  global $conf ;
  global $db_url;
  global $base_url;
  $stage = variable_get('boincimport_config_stage', 0);

  // Start with a quick sanity check on the BOINC environment
  boinc_get_path();

  //We'll be checking that the setup is complete.
  //If it is not complete, the migration will not go ahead.
  $ready_for_migration = 1;

  // Checking that the migration has been tested on a copy first.
  $form['test'] = array(
    '#type' => 'fieldset',
    '#title' => t('Test on copy first'),
    '#collapsible' => TRUE,
    '#collapsed' => TRUE,
  );

  $output = '';
  if (!variable_get('boincimport_tested', 0)) {
    $form['test']['#collapsed']= FALSE;
    $output = '<span class="marker">';
    $ready_for_migration = 0;
  }
  $output .= '<p>' . t('The migration will modify and even destroy some data. You
    must test this module on a copy of your data first, several times if necessary.
    Only use this module on your live data when you are familiar with the procedure
    and you\'re confident everything will run smoothly.') . '</p><p>' . t('Also, you
    should make a full backup of your database just prior to performing the
    migration on the live data. It is better to turn off both the BOINC site and the
    Drupal site during the migration.') . '</p>';
  if (!variable_get('boincimport_tested', 0)) {
    $output .= '</span>';
  }
  $form['test']['intro'] = array('#value' => $output);

  $form['test']['boincimport_tested'] = array(
    '#type' => 'checkbox',
    '#title' => t('I am running a trial migration on a disposable copy of the database
      OR I have already done so and I am running this on my live site, confident that
      the migration will proceed smoothly'),
    '#return_value' => 1,
    '#default_value' => variable_get('boincimport_tested', 0),
    '#description' => '',
  );

  // Where is BOINC data?
  $pre = variable_get('boincimport_table_prefix', '');
  $tables = array($pre .'forum', $pre .'post', $pre .'thread', $pre .'user');
  $db = (is_array($db_url) AND isset($db_url['boinc_rw'])) ? 'boinc_rw' : 'default';
  $result = _boincimport_check_tables($tables, $db, 0);

  $config_path = conf_path();
  $form['boinc_data'] = array(
    '#type' => 'fieldset',
    '#title' => t('Location of BOINC data'),
    '#collapsible' => TRUE,
    '#collapsed' => TRUE,
  );
  $form['boinc_data']['boincimport_db_configured'] = array(
    '#type' => 'checkbox',
    '#title' => t('There is a \'boinc\' database connection string configured in %file',
      array('%file' => "{$config_path}/settings.php")),
    '#return_value' => 1,
    '#default_value' => variable_get('boincimport_db_configured', 1),
    '#description' => '',
    '#disabled' => ($result['result']) ? TRUE : FALSE
  );
  if (variable_get('boincimport_db_configured', 1) != 1) {
    $default_db_url = is_array($db_url) ? end($db_url) : $db_url;
    $url = variable_get('boincimport_db_url', $default_db_url);
    if (!preg_match('{(mysql|pgsql):\/\/(.+):(.*?)@(.+)/(\w+)}i', $url)) {
      $output = '<p class="marker">'. t('Set the address of the BOINC database
        according to the following format:') . '<br /><pre>
        mysql://boinc_username:boinc_password@localhost/boinc_database
        </pre></p>';
      $ready_for_migration = 0;
      $form['boinc_data']['#collapsed'] = FALSE;
      $form['boinc_data']['db_connect_string_note'] = array('#value' => $output);
    }

    $form['boinc_data']['boincimport_db_url'] = array(
      '#type' => 'textfield',
      '#title' => t('BOINC database location'),
      '#default_value' => variable_get('boincimport_db_url', $default_db_url),
      '#size' => 80,
      '#maxlength' => 255,
      '#description' => '',
    );
  }

  $output = '<p>' . t('Note that due to a limitation within Drupal core, we cannot
    import messages from one database type to another database type. E.g, we
    cannot import BOINC from a myqsl DB, to a postgresql DB. You need to import
    the data into the same type of database first (import all the BOINC data, as is,
    from mysql to postgresql).') . '</p><p>' . t('If both BOINC and Drupal are on
    postgresql, see !this_issue.', array('!this_issue' => l(t('this issue'),
    'http://drupal.org/node/64877', array('attributes' => array('target' => '_blank'))))) . '</p>';
  $form['boinc_data']['note'] = array('#value' => $output);

  // BOINC table prefix
  $form['boinc_data']['boincimport_table_prefix'] = array(
    '#type' => 'textfield',
    '#title' => t('BOINC table prefix'),
    '#default_value' => variable_get('boincimport_table_prefix', ''),
    '#size' => 20,
    '#maxlength' => 255,
    '#description' => t('What prefix is used for your BOINC tables?'),
    '#disabled' => ($result['result']) ? TRUE : FALSE
  );

  $output = $result['html'];
  if ($result['result'] != 1) {
    $ready_for_migration = 0;
    $form['boinc_data']['#collapsed']= FALSE;
    $output .= '<span class="marker">';
    $output .= t('Some tables couldn\'t be found. Are the database settings correct?
      Please correct the errors so that the BOINC tables can be accessed.');
    $output .= '</span>';
  }

  $form['boinc_data']['result'] = array('#value' => $output);

  // USERS settings
  $form['users'] = array(
    '#type' => 'fieldset',
    '#title' => t('User import'),
    '#collapsible' => TRUE,
    '#collapsed' => TRUE,
  );

  // User import batch size
  $form['users']['boincimport_user_batch_size'] = array(
    '#type' => 'textfield',
    '#title' => t('Number of accounts to process at a time'),
    '#default_value' => variable_get('boincimport_user_batch_size', 50),
    '#size' => 10,
    '#maxlength' => 10,
    '#description' => t('Due to memory intensive API calls, this value may have to
      be adjusted to avoid exhausting system resources. The default value is a
      balance between performance and reliability.')
  );

  // Import users who have never posted?
  $form['users']['boincimport_import_lurkers'] = array(
    '#type' => 'checkbox',
    '#title' => t('Import users who have never posted?'),
    '#return_value' => 1,
    '#default_value' => variable_get('boincimport_import_lurkers', 0),
    '#description' => t('Do you want to import users with no community
      involvement? This may not be desired, as it can take a very long time to
      import tens or hundreds of thousands of accounts. (note that users who
      are not imported now will still be imported on-the-fly upon login)'),
  );

  // Input format settings
  $form['input'] = array(
    '#type' => 'fieldset',
    '#title' => t('Input format settings'),
    '#collapsible' => TRUE,
    '#collapsed' => TRUE
  );
  $output = '<p>'. t('Select which !input_format should be used for the imported
    messages. It is highly-recommended that this value be left as it is, as the
    Full HTML and PHP code formats can be very dangerous!',
    array('!input_format' => l(t('input format'), 'admin/settings/filters'))) .'</p>';
  $form['input']['intro'] = array('#value' => $output);

  $formats = db_query('SELECT format, name FROM {filter_formats}');
  while ($format = db_fetch_array($formats)) {
    $input_formats[$format['format']]= $format['name'];
  }

  // By default, use the format named "Rich text"; fall back to Filtered HTML
  $default_input_format = db_result(db_query("
    SELECT format FROM {filter_formats} WHERE name = '%s'", 'Rich text'));
  if (!$default_input_format) $default_input_format = 1;

  $input_format = variable_get('boincimport_input_format', $default_input_format);
  $form['input']['boincimport_input_format'] = array(
    '#type' => 'radios',
    '#title' => t('Input format'),
    '#default_value' => $input_format,
    '#options' => $input_formats
  );
  if (!$input_format) {
    $ready_for_migration = 0;
    $form['input']['#collapsed']= FALSE;
  }
  $output = '<p><em>'. t('This setting will be applied to all posts and private
    messages.') .'</em></p>';
  $form['input']['result'] = array('#value' => $output);

  // BBcode settings
  $form['bbcode'] = array(
    '#type' => 'fieldset',
    '#title' => t('BBcode settings'),
    '#collapsible' => TRUE,
    '#collapsed' => TRUE,
  );

  $output = '<p>' . t('BOINC posts are often formatted in BBcode. During the
    import process, BBcode can be handled in two diffent ways') . ':</p>';
  $output .= '<ul><li>' . t('Leave the BBcode as is and use the !bbcode_module
    after the migration to filter the imported posts. In this case, the module
    is not required during the migration, but will need to be installed and
    configured later.',
    array('!bbcode_module' => l(t('bbcode module'), 'http://drupal.org/project/bbcode',
    array('attributes' => array('target' => '_blank'))))) . '</li>';
  $output .= '<li>' . t('Filter the BBcode now and replace it with plain HTML.
    The BBcode module will be required during the migration process, but can
    be disabled afterwards.') . '</li></ul>';
  $output .= '<p>' . t('An appropriate input filter should be chosen, as well,
    as it will affect how the post is displayed. For example, if BBcode is
    removed and replaced by HTML, some HTML tags will be stripped by the
    "Filtered HTML" input format unless they are first added to the list of
    allowed tags in that format. See the !input_formats page.',
    array('!input_formats' => l(t('input formats'), 'admin/settings/filters'))) . '</p>';

  $form['bbcode']['intro'] = array('#value' => $output);
  $form['bbcode']['boincimport_bbcode'] = array(
    '#type' => 'radios',
    '#title' => t('BBcode handling'),
    '#default_value' => variable_get('boincimport_bbcode', 0),
    '#options' => array(t('Leave BBcode as is.'), t('Transform BBcode into HTML.')),
    '#description' => '',
  );

  $module_status = _boincimport_check_module('bbcode');
  $bbcode_disabled = ($module_status['result'] == 0) ? true : false;
  $output = $module_status['html'];

  if (variable_get('boincimport_bbcode', 0)) {
    if ($bbcode_disabled) {
      $ready_for_migration = 0;
      $form['bbcode']['#collapsed'] = FALSE;
      $output .= '<p><span class="marker">';
      $output .= t('The !bbcode_module is required to transform BBcode into HTML.
        You first need to !enable_bbcode', array(
          '!bbcode_module' => l(t('bbcode module'), 'http://drupal.org/project/bbcode', array('attributes' => array('target' => '_blank'))),
          '!enable_bbcode' => l(t('enable the bbcode module'), 'admin/build/modules')));
      $output .= '</span></p>';
    }
    $sql = "SELECT COUNT(*) FROM {filters} WHERE module = 'bbcode' AND format = %d";
    $result = db_query($sql, $input_format);
    if (!db_result($result)) {
      $form['bbcode']['#collapsed'] = FALSE;
      $output .= '<p class="marker">'. t('You need to !enable_bbcode_input and
        then !configure_bbcode_options', array(
          '!enable_bbcode_input' =>  l(t('enable BBcode in the selected input format'), "admin/settings/filters/{$input_format}"),
          '!configure_bbcode_options' => l(t('configure BBcode options'), "admin/settings/filters/{$input_format}/configure"))) .'</p>';
    }
    $output .= '<p>'. t('For best results, !disable_all_options',
    array('!disable_all_options' => l(t('all four BBcode configure options should be disabled'), "admin/settings/filters/{$input_format}/configure"))) . '.</p>';
  } else {
    if ($bbcode_disabled) {
      $form['bbcode']['#collapsed'] = FALSE;
      $output .= '<p>'. t('The !bbcode_module is recommended, but not required.
        You may !enable_bbcode after having installed it.', array(
          '!bbcode_module' => l(t('bbcode module'), 'http://drupal.org/project/bbcode', array('attributes' => array('target' => '_blank'))),
          '!enable_bbcode' => l(t('enable the bbcode module'), 'admin/build/modules'))) .'</p>';
    }
    $output .= '<p>' . t('Don\'t forget to !enable_bbcode_input and then !configure_bbcode_options.', array(
      '!enable_bbcode_input' =>  l(t('enable BBcode in the selected input format'), "admin/settings/filters/{$input_format}"),
      '!configure_bbcode_options' => l(t('configure BBcode options'), "admin/settings/filters/{$input_format}/configure"))) . '</p>';
    $output .= '<p>' . t('You may want to enable it for other input formats, too.') . '</p>';
  }
  $output .= '<p><em>'. t('This setting will be applied to all posts, private
    messages and the users\' signature.') .'</em></p>';

  $form['bbcode']['result'] = array('#value' => $output);

  // URL transform settings
  $form['url_transforms'] = array(
    '#type' => 'fieldset',
    '#title' => t('URL transforms'),
    '#collapsible' => TRUE,
    '#collapsed' => TRUE,
  );
  if (!variable_get('boincimport_base_url_boinc', 0)) {
    $form['url_transforms']['#collapsed']= FALSE;
    $output = '<span class="marker">';
    $ready_for_migration = 0;
  }

  // List of BOINC domains possible in URLs to transform
  $form['url_transforms']['boincimport_base_url_boinc'] = array(
    '#type' => 'textarea',
    '#title' => t('BOINC base URLs to transform'),
    '#default_value' => variable_get('boincimport_base_url_boinc', ''),
    '#description' => t('List URL bases that should be transformed from
      BOINC format to Drupal format (enter one domain per line, including http://).'),
  );

  // The Drupal domain to use in URL transform results
  $form['url_transforms']['boincimport_base_url_drupal'] = array(
    '#type' => 'textfield',
    '#title' => t('Target Drupal base URL'),
    '#default_value' => variable_get('boincimport_base_url_drupal', ''),
    '#description' => t('Specify the URL base to use in transform results. If
      left blank, the base_url configured in this Drupal environment will be
      used (currently: %url)', array('%url' =>$base_url)),
  );

  // Are we ready for migration?
  $form['migration'] = array(
    '#type' => 'fieldset',
    '#title' => 'Migration',
    '#collapsible' => TRUE,
    '#collapsed' => FALSE,
  );
  variable_set('boincimport_ready', $ready_for_migration);
  if ($ready_for_migration) {
    $output = '<p>'. t('Check the settings above one more time. If everything
      is ok, !proceed.', array('!proceed' => l(t('you can now proceed with the migration'), 'admin/boinc/import/process'))) .'</p>';
  } else {
    $output = '<p class="marker">'. t('You are not ready for the migration yet.
      Please check the settings above.') .'</p>';
  }

  $form['migration']['info'] = array('#value' => $output);
  return system_settings_form($form);
}

/**
 * Callback admin/boinc/import/post_configuration
 */
function boincimport_post_configuration() {
  $output = '<p>'. t('This page will help to tie up any loose ends following
    the BOINC data import process. Before continuing, it may be best to have
    already:') .'</p>';
  $output .= '<ul>';
  $output .= '<li>'. t('Imported users, teams, and forums using the BOINC
    import module') .'</li>';
  $output .= '<li>'. t('Imported any additional data using add on modules if
    available.') .'</li>';
  $output .= '</ul>';
  //User Set up
  $output .= '<h2>'. t('User settings') .'</h2>';
  $output .= '<p>'. t('Make sure your !user_settings are correct. Specifically,
    you can enable or disable signatures there. ', array(
      '!user_settings' => l(t('user settings'), 'admin/user/settings')
    )) .'</p>';
  $output .= '<p>' . t('Note that avatar/picture support is not tied to user
    settings Picture support at this time.') . '</p>';
  /*$output .= '<p>' . t('In order to activate avatar/picture support, you need to
    enable pictures on this page and also:') . '</p>';
  $output .= '<ul>';
  $output .= '<li>'. t('Tick the checkboxes to allow "User pictures in posts" and/or
    "User pictures in comments" on the !theme_config page.', array(
      '!theme_config' => l(t('theme configuration'), 'admin/build/themes/settings')
    )) .'</li>';
  $output .= '<li>'. t('Manually copy the avatars from the boinc/images/avatars/
    directory to the @files directory.', array(
      '@files' => file_directory_path(). '/pictures'
    )) .'</li>';
  $output .= '</ul>'; */

  // Forum Set up
  $output .= '<h2>'. t('Forum settings') .'</h2>';
  $output .= '<p>'. t('Check any additional changes to make on the
    !forum_settings page.', array(
      '!forum_settings' =>  l(t('forum settings'), 'admin/content/forum/settings')
    )) .'</p>';
  $output .= '<p>'. t('It also may be a good idea to look at the !forum_list to
    verify that all necessary containers and forums are in place.', array(
      '!forum_list' =>  l(t('forum list'), 'admin/content/forum')
    )) .'</p>';
  /*$output .= '<p>'. t('If need be, set up the private forums as explained in the
    !documentation.', array(
      '!documentation' => l(t('documentation'), 'http://drupal.org/node/58969', array('attributes' => array('target' => '_blank')))
    )) .'</p>';
    */
  /*
  // Redirection from old URLs:
  $output .= '<h2>'. t('Redirection from BOINC forum URLs to new Drupal forum URLs') .'</h2>';
  $output .= '<p>'. t('Enable the boincforum_redirect.module. This way, users
    accessing URLs from your old BOINC installation will be redirected to your new
    Drupal forum.') .'</p>';
  */
  // BBcode
  $bbcode = variable_get('boincimport_bbcode', 0);
  if ($bbcode) {
    $output .= '<h2>'. t('Remove BBcode module') .'</h2>';
    $output .= '<p>'. t('The BBcode has been cleaned from the posts. You shouldn\'t
      need the bbcode module anymore. You can disable it.') .'</p>';
  } else {
    $output .= '<h2>'. t('Configure BBcode module') .'</h2>';
    $output .= '<p>'. t('Most likely, there are many BOINC forum postings
      formatted in BBcode. It would be a good idea to verify that BBcode is
      being formatted correctly in the imported posts. Have a look around
      !the_forum.', array(
        '!the_forum' =>  l(t('the forum'), 'community')
      )) . '</p>';
  }

  return $output;
}

/**
 * Callback admin/boinc/import
 */
function boincimport_main() {
  // Start with a quick sanity check on the BOINC environment
  boinc_get_path();
  // If that's ok, on with the show
  $output .= t('You may want to !check_documentation before starting.', array(
    '!check_documentation' => l(t('check the documentation'), 'http://boinc.berkeley.edu/trac/wiki/InstallDrupal', array('attributes' =>array('target' => '_blank')))
  ));
  $output = '<ol><li>';
  $output .= l(t('Configure the migration'), 'admin/boinc/import/settings');
  $output .= '</li>';
  $output .= '<li>';
  $output .= l(t('Execute the migration'), 'admin/boinc/import/process');
  $output .= '</li>';
  $output .= '<li>';
  $output .= l(t('Post migration configuration'), 'admin/boinc/import/post_configuration');
  $output .= '</li>';
  $output .= '<li>';
  $output .= t('!cleanup (Accessing this link will delete all the saved BOINC forum
    conversion settings: click only when you\'re done with all the above.)', array(
      '!cleanup' => l(t('Cleanup'), 'admin/boinc/import/cleanup')
    ));
  $output .= '</li>';
  /*$output .= '<li>';
  $output .= t('Enable the boincforum_redirect module for seamless redirection
    from the old forums to the new. This module requires !clean_urls to be active.', array(
      '!clean_urls' => l(t('Clean URLs'), 'admin/settings/clean-urls')
    ));
  $output .= '</li>';
  $output .= '<li>';
  $output .= t('If you decide to use the boincforum_redirect module, do NOT
    uninstall BOINC forum - This will delete tables necessary for redirection. You can,
    however, deactivate the module safely.');
  $output .= '</li>';*/
  $output .= '</ol>';

  // process will die() if the link to the BOINC database is wrong:
  $output .= '<p>'. t('If the BOINC data is in another database, but you made a
    mistake while setting up the data base url, you may be unable to access the
    setting page due to some limitations of the core of Drupal. !reset_db_url.', array(
      '!reset_db_url' => l(t('Click here to reset the database url'), 'admin/boinc/import/reset')
    )) .'</p>';

  return $output;
}
