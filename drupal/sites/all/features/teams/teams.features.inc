<?php

/**
 * Implementation of hook_ctools_plugin_api().
 */
function teams_ctools_plugin_api() {
  list($module, $api) = func_get_args();
  if ($module == "page_manager" && $api == "pages_default") {
    return array("version" => 1);
  }
  elseif ($module == "strongarm" && $api == "strongarm") {
    return array("version" => 1);
  }
}

/**
 * Implementation of hook_node_info().
 */
function teams_node_info() {
  $items = array(
    'team' => array(
      'name' => t('Team'),
      'module' => 'features',
      'description' => t('A grouping of users, centered on a common interest.'),
      'has_title' => '1',
      'title_label' => t('Name'),
      'has_body' => '1',
      'body_label' => t('Description'),
      'min_word_count' => '0',
      'help' => '',
    ),
  );
  return $items;
}

/**
 * Implementation of hook_rules_defaults().
 */
function teams_rules_defaults() {
  return array(
    'rules' => array(
      'rules_notify_admins_about_error_creating_a_boinc_team' => array(
        '#type' => 'rule',
        '#set' => 'event_boincteam_create_team_error',
        '#label' => 'Notify admins about error creating a BOINC team',
        '#active' => 1,
        '#weight' => '0',
        '#categories' => array(
          '0' => 'boinc_team',
          'teams' => 'teams',
        ),
        '#status' => 'default',
        '#conditions' => array(),
        '#actions' => array(
          '0' => array(
            '#type' => 'action',
            '#settings' => array(
              'from' => '',
              'subject' => '[subject_tag:string] Error creating new team',
              'message' => '[user:display-name] attempted to create team [team_name:string], but an error was returned. See function \'boincteam_create_form_submit\'. BOINC function \'make_team\' did not return a valid boinc_team object.',
              '#eval input' => array(
                'token_rules_input_evaluator' => array(
                  'subject' => array(
                    '0' => 'subject_tag',
                    '1' => ':global',
                  ),
                  'message' => array(
                    '0' => 'team_name',
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
      'rules_notify_admins_about_error_creating_a_boinc_team_no_pathauto' => array(
        '#type' => 'rule',
        '#set' => 'event_boincteam_create_team_nopathauto_error',
        '#label' => 'Notify admins about error creating a BOINC team no pathauto',
        '#active' => 1,
        '#weight' => '0',
        '#categories' => array(
          '0' => 'boinc_team',
          'teams' => 'teams',
        ),
        '#status' => 'default',
        '#conditions' => array(),
        '#actions' => array(
          '0' => array(
            '#type' => 'action',
            '#settings' => array(
              'from' => '',
              'subject' => '[subject_tag:string] Error creating BOINC team - no pathauo module',
              'message' => '[user:display-name] attempted to create team [team_name:string], but no pathauto module detected. See function \'boincteam_create_form_submit\'.',
              '#eval input' => array(
                'token_rules_input_evaluator' => array(
                  'subject' => array(
                    '0' => 'subject_tag',
                    '1' => ':global',
                  ),
                  'message' => array(
                    '0' => 'team_name',
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
    ),
  );
}

/**
 * Implementation of hook_views_api().
 */
function teams_views_api() {
  return array(
    'api' => '2',
  );
}
