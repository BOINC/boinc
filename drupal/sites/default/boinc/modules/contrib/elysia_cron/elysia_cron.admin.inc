<?php

/*******************************************************************************
 * INTERFACE
 *
 * WARN: Below this point the word "context" should be avoided (use channel)
 *   Disabled should always be referenced as "disabled" (in db is "disable" for 
 *   compatibility with Ctools )
 ******************************************************************************/

function elysia_cron_admin_page() {
  $aoutput = array();
  $aoutput[] = drupal_get_form('elysia_cron_run_form');

  $output = '';

  elysia_cron_initialize();

  global $elysia_cron_settings, $elysia_cron_settings_by_channel, $elysia_cron_current_channel, $cron_completed, $cron_completed_time;

  $v = variable_get('elysia_cron_disabled', false);
  $output .= '<p>' . t('Global disable') . ': <i>' . ($v ? '<span class="warn">' . t('YES') . '</span>' : 'no') . '</i></p>';
  $output .= '<p>' . t('Last channel executed') . ': <i>' . (($c = elysia_cron_last_channel()) ? $c : t('n/a')) . '</i></p>';

  if (EC_DRUPAL_VERSION < 7) {
    if (_ec_variable_get('elysia_cron_semaphore', 0)) {
      $output .= '<p><span class="warn">' . t('Global semaphore active since !date', array('!date' => elysia_cron_date(_ec_variable_get('elysia_cron_semaphore', 0)))) . '</span></p>';
    }
  }

  $running = '';
  foreach ($elysia_cron_settings_by_channel as $channel => $data) {
    if (elysia_cron_is_channel_running($channel)) {
      $running .= $channel . ' ';
    }
  }
  if ($running) {
    $output .= '<p>' . t('Running channels') . ': <span class="warn">' . $running . '</span></p>';
  }

  $output .= '<p>' . t('Last run') . ': ' . elysia_cron_date(_ec_variable_get('elysia_cron_last_run', 0)) . '</p>';

  foreach ($elysia_cron_settings_by_channel as $channel => $data) {
    $running = elysia_cron_is_channel_running($channel);
    $output .= '<h3>' . t('Channel') . ': ' . $channel . ($running ? ' (' . t('Running since !date', array('!date' => elysia_cron_date($running))) . ')' : '') . ($data['#data']['disabled'] ? ' <span class="warn">(' . t('DISABLED') . ')</span>' : '') . '</h3>';
    $output .= '<p>' . t('Last run') . ': ' . elysia_cron_date($data['#data']['last_run']) . '</p>';
    $output .= '<p>' . t('Last execution time') . ': ' . $data['#data']['last_execution_time'] . 's ' . t('(Shutdown: !shutdown) (Avg total: !avgtotal, Max total: !maxtotal)', array('!shutdown' => $data['#data']['last_shutdown_time'] . 's', '!avgtotal' => $data['#data']['avg_execution_time'] . 's', '!maxtotal' => $data['#data']['max_execution_time'] . 's')) . '</p>';
    $output .= '<p>' . t('Execution count') . ': ' . $data['#data']['execution_count'] . '</p>';
    if ($data['#data']['last_aborted']) {
      $output .= '<p>' . t('Last aborted') . ': <span class="warn">' . t('On function !function', array('!function' => $data['#data']['last_abort_function'])) . '</span></p>';
    }
    if ($data['#data']['abort_count']) {
      $output .= '<p>' . t('Abort count') . ': <span class="warn">' . $data['#data']['abort_count'] . '</span></p>';
    }

    $rows = array();
    foreach ($data as $job => $conf) {
      if ($job != '#data') {
        $rows[] = array(
          (!empty($conf['disabled']) ? '<strike>' . $job . '</strike>' : (empty($conf['running']) ? '<b>' . $job . '</b>' : '<b><u>' . $job . '</u></b> <small title="' . t('Running') . '">(R)</small>')) . (elysia_cron_should_run($conf) ? ' <small title="' . t('Waiting for execution') . '">(w)</small>' : ''),
          array(
            'data' => '<i>' . elysia_cron_description($job) . '</i> ' .
          '[' . _dco_l(t('run'), _dcf_internal_path('admin/config/system/cron/execute/') . $job, array('attributes' => array('onclick' => 'return confirm("' . t('Force execution of !job?', array('!job' => $job)) . '");'))) . ']',
            'colspan' => 4,
          ),
        );
        $rows[] = array(
          $conf['rule'] . (!empty($conf['weight']) ? ' <small>(' . t('Weight') . ': ' . $conf['weight'] . ')</small>' : ''),
          elysia_cron_date($conf['last_run']),
          $conf['last_execution_time'] . 's',
          $conf['execution_count'],
          $conf['avg_execution_time'] . 's / ' . $conf['max_execution_time'] . 's',
        );
      }
    }
    $output .= _dco_theme('table', array('header' => array(t('Job / Rule'), t('Last run'), t('Last exec time'), t('Exec count'), t('Avg/Max Exec time')), 'rows' => $rows));
  }

  $output .= '<p>' . t('Notes: job times don\'t include shutdown times (only shown on channel times).') . '</p>';
  $output .= '<p>' . t('If an abort occours usually the job is not properly terminated, and so job timings can be inaccurate or wrong.') . '</p>';

  $aoutput[] = array(
    '#type' => 'markup',
    '#markup' => $output,
  );

  return _dcr_render_array($aoutput);
}

function elysia_cron_settings_form() {
  global $elysia_cron_settings, $elysia_cron_settings_by_channel;
  elysia_cron_initialize();

  $form = array();

  $form['prefix_1'] = array(
    '#type' => 'fieldset',
    '#title' => t('Click for help and cron rules and script syntax'),
    '#collapsible' => true,
    '#collapsed' => true,
    '#description' => t(<<<EOT
<h3>Fields order</h3>
<pre>
 +---------------- minute (0 - 59)
 |  +------------- hour (0 - 23)
 |  |  +---------- day of month (1 - 31)
 |  |  |  +------- month (1 - 12)
 |  |  |  |  +---- day of week (0 - 6) (Sunday=0)
 |  |  |  |  |
 *  *  *  *  *
</pre>
<p>Each of the patterns from the first five fields may be either * (an asterisk),
which matches all legal values, or a list of elements separated by commas (see below).</p>
<p>For "day of the week" (field 5), 0 is considered Sunday, 6 is Saturday
(7 is an illegal value)</p>
<p>A job is executed when the time/date specification fields all match the current
time and date. There is one exception: if both "day of month" and "day of week"
are restricted (not "*"), then either the "day of month" field (3) or the "day of week"
field (5) must match the current day (even though the other of the two fields
need not match the current day).</p>

<h3>Fields operators</h3>
<p>There are several ways of specifying multiple date/time values in a field:</p>
<ul>
<li>The comma (',') operator specifies a list of values, for example: "1,3,4,7,8"</li>
<li>The dash ('-') operator specifies a range of values, for example: "1-6", which is equivalent to "1,2,3,4,5,6"</li>
<li>The asterisk ('*') operator specifies all possible values for a field. For example, an asterisk in the hour time field would be equivalent to 'every hour' (subject to matching other specified fields).</li>
<li>The slash ('/') operator (called "step") can be used to skip a given number of values. For example, "*/3" in the hour time field is equivalent to "0,3,6,9,12,15,18,21".</li>
</ul>

<h3>Examples</h3>
<pre>
 */15 * * * * : Execute job every 15 minutes
 0 2,14 * * *: Execute job every day at 2:00 and 14:00
 0 2 * * 1-5: Execute job at 2:00 of every working day
 0 12 1 */2 1: Execute job every 2 month, at 12:00 of first day of the month OR at every monday.
</pre>

<h3>Script</h3>
<p>You can use the script section to easily create new jobs (by calling a php function)
or to change the scheduling of an existing job.</p>
<p>Every line of the script can be a comment (if it starts with #) or a job definition.</p>
<p>The syntax of a job definition is:</p>
<code>
&lt;-&gt; [rule] &lt;ch:CHANNEL&gt; [job]
</code>
<p>(Tokens betweens [] are mandatory)</p>
<ul>
<li>&lt;-&gt;: a line starting with "-" means that the job is DISABLED.</li>
<li>[rule]: a crontab schedule rule. See above.</li>
<li>&lt;ch:CHANNEL&gt;: set the channel of the job.</li>
<li>[job]: could be the name of a supported job (for example: 'search_cron') or a function call, ending with ; (for example: 'process_queue();').</li>
</ul>
<p>A comment on the line just preceding a job definition is considered the job description.</p>
<p>Remember that script OVERRIDES all settings on single jobs sections or channel sections of the configuration</p>

<h3>Examples of script</h3>
<pre>
# Search indexing every 2 hours (i'm setting this as the job description)
0 */2 * * * search_cron

# I'll check for module status only on sunday nights
# (and this is will not be the job description, see the empty line below)

0 2 * * 0 update_status_cron

# Trackback ping process every 15min and on a channel called "net"
*/15 * * * * ch:net trackback_cron

# Disable node_cron (i must set the cron rule even if disabled)
- */15 * * * * node_cron

# Launch function send_summary_mail('test@test.com', false); every night
# And set its description to "Send daily summary"
# Send daily summary
0 1 * * *  send_summary_mail('test@test.com', false);
</pre>
EOT
),
  );

  $form['prefix_2'] = array(
    '#markup' => '<hr>',
  );

  $form['main'] = array(
    '#title' => t('Main'),
    '#type' => 'fieldset',
    '#collapsible' => false,
    '#collapsed' => false,
  );
  $form['main']['elysia_cron_disabled'] = array(
    '#title' => t('Global disable'),
    '#type' => 'checkbox',
    '#default_value' => variable_get('elysia_cron_disabled', false),
  );

  $form['installation'] = array(
    '#title' => t('Installation settings'),
    '#type' => 'fieldset',
    '#collapsible' => true,
    '#collapsed' => true,
  );

  if (EC_DRUPAL_VERSION >= 7) {
    $form['installation']['cron_safe_threshold'] = array(
      '#type' => 'select',
      '#title' => t('Run cron on visitor\'s requests, every'),
      '#default_value' => variable_get('cron_safe_threshold', DRUPAL_CRON_DEFAULT_THRESHOLD),
      '#description' => t('Setting a time here will enable the "poormanscron" method, which runs the Drupal cron operation using normal browser/page requests instead of having to set up a crontab to request the cron.php script. This approach requires that your site gets regular traffic/visitors in order to trigger the cron request.') . '<br />' .
        t('This way is fine if you don\'t need a great control over job starting times and execution frequency.') . '<br />' .
        t('If you need a fine grained control over cron timings use the crontab metod, as <a href="!cron_url">described in Drupal installation guide</a>.', array('!cron_url' => url('http://drupal.org/cron'))) . '<br />' .
        t('If you have a very large site, or you need to execute some jobs very often (more than once an hour) refer to Elysia cron\'s INSTALL.TXT to improve main cron setup.'),
      '#options' => array(0 => t('Never / Use external crontab')) + drupal_map_assoc(array(3600, 10800, 21600, 43200, 86400, 604800), 'format_interval'),
    );
  }

  $form['installation']['cron_key'] = array(
    '#title' => t('Cron key'),
    '#type' => 'textfield',
    '#default_value' => variable_get('cron_key', ''),
    '#description' => t('This is used to avoid external cron calling. If you set this cron will by accessible only by calling http://site/cron.php?cron_key=XXX, so you\'ll need to modify system crontab to support this (Logged user with [administer elysia_cron] permission avoid this check).'),
  );

  $form['installation']['elysia_cron_allowed_hosts'] = array(
    '#title' => t('Allowed hosts'),
    '#type' => 'textfield',
    '#default_value' => variable_get('elysia_cron_allowed_hosts', ''),
    '#description' => t('Insert a list of ip addresses separated by , that can run cron.php (Logged user with [administer elysia_cron] permission avoid this check).'),
  );

  $form['installation']['elysia_cron_default_rule'] = array(
    '#title' => t('Default schedule rule'),
    '#type' => 'textfield',
    '#default_value' => variable_get('elysia_cron_default_rule', false),
    '#description' => t('If you don\'t specify a rule for a process, and if it has not a module specified one, this rule will apply'),
  );

  if (!ini_get('safe_mode')) {
    $form['installation']['elysia_cron_time_limit'] = array(
      '#title' => t('Time limit'),
      '#type' => 'textfield',
      '#default_value' => variable_get('elysia_cron_time_limit', 240),
      '#description' => t('Set the number of seconds a channel is allowed to run. If you have some jobs that needs more time to execute increase it or set to 0 to disable the limit (WARN: that way a stuck job will block the channel forever!).'),
    );
  }

  $form['installation']['elysia_cron_stuck_time'] = array(
    '#title' => t('Stuck time'),
    '#type' => 'textfield',
    '#default_value' => variable_get('elysia_cron_stuck_time', 3600),
    '#description' => t('How many seconds the process should wait to consider the job as stuck (so the channel can run again)'),
  );

  $form['installation']['elysia_cron_debug_messages'] = array(
    '#title' => t('Debug'),
    '#type' => 'select',
    '#default_value' => variable_get('elysia_cron_debug_messages', 0),
    '#options' => array(
      0 => t('Disabled'),
      1 => t('Enabled'),
    ),
    '#description' => t('Enable extended logging (in watchdog)'),
  );

  $default_ruless = '';
  $default_rules = variable_get('elysia_cron_default_rules', $GLOBALS['elysia_cron_default_rules']);
  foreach ($default_rules as $dk => $dr) {
    $default_ruless .= $dr . ' = ' . $dk . "\n";
  }

  $form['installation']['elysia_cron_default_rules'] = array(
    '#title' => t('Predefined rules'),
    '#type' => 'textarea',
    '#rows' => 5,
    '#default_value' => $default_ruless,
    '#description' => t('You can put here standard rules used in your system, each one with its own caption. Put each rule in a separate line, in the form "caption = rule". For example: <i>"every 15 minutes = */15 * * * *"</i>.'),
  );

  $form['installation']['elysia_cron_alert_fieldset'] = array(
    '#title' => t('External cron tracking'),
    '#type' => 'fieldset',
    '#collapsible' => true,
    '#collapsed' => true,
    '#description' => t('This lets you use an external tracking system like <a href="http://www.host-tracker.com/">Host Tracker</a> to be used to monitor the health of cron on your site. Point the tracking service to <a href="!cron-ping-url">!cron-ping-url</a>. If Elysia cron has been called within the time interval specified below, the ping page will return HTTP 200.  If not, the ping page will throw a 404 (page not found).', array('!cron-ping-url' => url('admin/build/cron/ping'))),
  );
  $form['installation']['elysia_cron_alert_fieldset']['elysia_cron_alert_interval'] = array(
    '#title' => t('Lapse interval (minutes)'),
    '#type' => 'textfield',
    '#size' => 20,
    '#default_value' => variable_get('elysia_cron_alert_interval', 60),
    '#description' => t('Specify the number of minutes to allow to lapse before the cron ping page returns a 404 (page not found).'),
  );

  $form['elysia_cron_script_fieldset'] = array(
    '#title' => t('Script'),
    '#type' => 'fieldset',
    '#collapsible' => true,
    '#collapsed' => !variable_get('elysia_cron_script', ''),
  );
  $form['elysia_cron_script_fieldset']['elysia_cron_script'] = array(
    '#type' => 'textarea',
    '#rows' => 20,
    '#default_value' => variable_get('elysia_cron_script', ''),
    '#description' => t('You can specify new cron jobs or modify existing schedules by adding lines to the script.<br>' .
      '<b>Warning</b> All rules specified in the script will OVERRIDE single job settings and channel settings (sections below).'),
  );
  
  $form['single_job'] = array(
    '#title' => t('Single job settings'),
    '#description' => 
      '<b>'.t('Disabled').'</b>: '.t('Flag this to disable job execution').'<br />'.
      '<b>'.t('Schedule rule').'</b>: '.t('Timing rule for the job. Leave empty to use default rule (shown after the field in parenthesis)').'<br />'.
      '<b>'.t('Weight').'</b>: '.t('Use this to specify execution order: low weights are executed before high weights. Default value shown in parenthesis').'<br />'.
      '<b>'.t('Channel').'</b>: '.t('Specify a channel for the job (create the channel if not exists)').'<br /><br />',
    '#type' => 'fieldset',
    '#collapsible' => true,
    //'#collapsed' => true,
  );
  
  $jobchannels = array(
    '#title' => t('Job channel associations'),
    '#description' => t('Leave empty for default channel'),
    '#type' => 'fieldset',
    '#collapsible' => true,
    '#collapsed' => true,
  );

  foreach ($elysia_cron_settings_by_channel as $channel => $cconf) {
    foreach ($cconf as $job => $conf) {
      if ($job != '#data' && empty($conf['expression'])) {
        $form['single_job']['elysia_cron_' . $job] = array(
          '#title' => $job, // t('Job !job', array('!job' => $job)),
          '#description' => elysia_cron_description($job),
          '#type' => 'fieldset',
          '#collapsible' => true,
          '#collapsed' => !elysia_cron_get_job_rule($job) && !elysia_cron_get_job_weight($job) && !elysia_cron_is_job_disabled($job) && !elysia_cron_get_job_channel($job),
        );
        //if (!$form['single_job']['elysia_cron_'.$job]['#collapsed'])
        //  $form['single_job']['#collapsed'] = false;

        $rule = elysia_cron_get_job_rule($job);
        $options = array_merge(array('default' => t('Default') . ' (' . (!empty($default_rules[$conf['default_rule']]) ? $default_rules[$conf['default_rule']] : $conf['default_rule']) . ')'), $default_rules);
        if ($rule && !isset($options[$rule])) {
          $options[$rule] = $rule;
        }
        $options['custom'] = t('Custom') . ' ...';

        $form['single_job']['elysia_cron_' . $job]['_elysia_cron_seljob_rule_' . $job] = array(
          '#title' => t('Schedule rule'),
          '#type' => 'select',
          '#options' => $options,
          '#default_value' => $rule ? $rule : 'default',
        );

        $form['single_job']['elysia_cron_' . $job]['_elysia_cron_job_rule_' . $job] = array(
          '#title' => t('Schedule rule'),
          '#type' => 'textfield',
          '#size' => 20,
          '#default_value' => $rule ? $rule : $conf['default_rule'],
        );

        $form['single_job']['elysia_cron_' . $job]['_elysia_cron_job_weight_' . $job] = array(
          '#title' => t('Weight'),
          '#type' => 'textfield',
          '#size' => 4,
          '#default_value' => elysia_cron_get_job_weight($job),
          '#description' => '(' . $conf['default_weight'] . ')',
        );

        //$form['single_job']['elysia_cron_'.$job]['elysia_cron_'.$job.'_disabled'] = array(
        $form['single_job']['elysia_cron_' . $job]['_elysia_cron_job_disabled_' . $job] = array(
          '#title' => t('Disabled'),
          '#type' => 'checkbox',
          '#default_value' => elysia_cron_is_job_disabled($job, false),
        );

        //$jobchannels['elysia_cron_'.$job.'_channel'] = array(
        $form['single_job']['elysia_cron_' . $job]['_elysia_cron_job_channel_' . $job] = array(
          '#title' => t('Channel'), // t('Channel for !job', array('!job' => $job)),
          '#type' => 'textfield',
          '#size' => 20,
          '#default_value' => elysia_cron_get_job_channel($job),
        );

        //if (elysia_cron_get_job_channel($job))
        //  $jobchannels['#collapsed'] = false;
      }
    }
  }

  $form['channels'] = array(
    '#title' => t('Channels settings'),
    '#type' => 'fieldset',
    '#collapsible' => true,
    //'#collapsed' => $jobchannels['#collapsed'],
  );

  foreach ($elysia_cron_settings_by_channel as $channel => $conf) {
    $form['channels']['elysia_cron_ch_' . $channel] = array(
      '#title' => $channel, // t('Channel !channel', array('!channel' => $channel)),
      '#type' => 'fieldset',
    );
    $form['channels']['elysia_cron_ch_' . $channel]['_elysia_cron_ch_disabled_' . $channel] = array(
      '#title' => t('Disabled'),
      '#type' => 'checkbox',
      '#default_value' => elysia_cron_is_channel_disabled($channel, ''),
    );
    $form['channels']['elysia_cron_ch_' . $channel]['_elysia_cron_ch_rule_' . $channel] = array(
      '#title' => t('Default schedule rule'),
      '#type' => 'textfield',
      '#size' => 20,
      '#default_value' => elysia_cron_get_channel_rule($channel),
    );
    //if (elysia_cron_is_channel_disabled($channel))
    //  $form['channels']['#collapsed'] = false;
  }

  //$form['channels']['jobchannels'] = $jobchannels;

  $form['buttons'] = array('#type' => 'actions');
  $form['buttons']['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Save configuration'),
  );
  $form['buttons']['reset'] = array(
    '#type' => 'submit',
    '#value' => t('Reset to defaults'),
  );

  if (!empty($_POST) && form_get_errors()) {
    drupal_set_message(t('The settings have not been saved because of the errors.'), 'error');
  }

  return _dcr_form($form);
}

function theme_elysia_cron_settings_form($_dco_variables) {
  extract(_dcf_theme_form($_dco_variables));
  $form = &$variables['form'];

  $output = '<script type="text/javascript"><!--' . "\n" .
    /*'function _ec_select(editid, select) { if (select.value == \'custom\') {'.
      '$ = jQuery; $(select).hide();$("#"+editid).show();$("#"+editid).focus();'.
    '}}'.*/
    'function _ec_select(key, select) { if (select.value == \'custom\') {' .
      '$ = jQuery; $("#_ec_select_"+key).hide();$("#_ec_custom_"+key).show();$("#_ec_custom_"+key).focus();' .
    '}}' .
    "\n" . '--></script>';

  $coutput = '<table>';

  $i = 0;
  foreach (element_children($form['single_job']) as $c) {
    $key = substr($c, 12);
    //print_r($form['single_job'][$c]);
    if ($i++ == 0) {
      $coutput .= '<tr>' .
        '<th>' . $form['single_job'][$c]['_elysia_cron_job_disabled_' . $key]['#title'] . '</th>' .
        '<th>' . $form['single_job'][$c]['_elysia_cron_job_rule_' . $key]['#title'] . '</th>' .
        '<th colspan="2">' . $form['single_job'][$c]['_elysia_cron_job_weight_' . $key]['#title'] . '</th>' .
        '<th>' . $form['single_job'][$c]['_elysia_cron_job_channel_' . $key]['#title'] . '</th>' .
      '</tr>';
    }

    //$def_rule = $form['single_job'][$c]['_elysia_cron_job_rule_'.$key]['#description'];
    $def_weight = $form['single_job'][$c]['_elysia_cron_job_weight_' . $key]['#description'];

    $posted_key = $form['single_job'][$c]['_elysia_cron_seljob_rule_' . $key]['#name'];
    $posted_val = !empty($_REQUEST[$posted_key]) ? $_REQUEST[$posted_key] : false;

    $form['single_job'][$c]['_elysia_cron_job_rule_' . $key]['#prefix'] = '<span id="_ec_custom_' . $key . '" style="' . ($posted_val != 'custom' ? 'display: none;' : '') . '">';
    $form['single_job'][$c]['_elysia_cron_job_rule_' . $key]['#suffix'] = '</span>';
    $form['single_job'][$c]['_elysia_cron_job_rule_' . $key]['#title'] = NULL;
    $form['single_job'][$c]['_elysia_cron_job_rule_' . $key]['#description'] = NULL;
    //$form['single_job'][$c]['_elysia_cron_job_rule_'.$key]['#attributes']['style'] = ($posted_val != 'custom' ? 'display: none;' : '').'width: 20em; margin: 0';
    $form['single_job'][$c]['_elysia_cron_seljob_rule_' . $key]['#prefix'] = '<span id="_ec_select_' . $key . '" style="' . ($posted_val == 'custom' ? 'display: none;' : '') . '">';
    $form['single_job'][$c]['_elysia_cron_seljob_rule_' . $key]['#suffix'] = '</span>';
    $form['single_job'][$c]['_elysia_cron_seljob_rule_' . $key]['#title'] = NULL;
    $form['single_job'][$c]['_elysia_cron_seljob_rule_' . $key]['#description'] = NULL;
    //$form['single_job'][$c]['_elysia_cron_seljob_rule_'.$key]['#attributes']['style'] = ($posted_val == 'custom' ? 'display: none;' : '').'width: 20em; margin: 0';
    //$form['single_job'][$c]['_elysia_cron_seljob_rule_'.$key]['#attributes']['onchange'] = '_ec_select(\''.$form['single_job'][$c]['_elysia_cron_job_rule_'.$key]['#id'].'\', this)';
    $form['single_job'][$c]['_elysia_cron_seljob_rule_' . $key]['#attributes']['onchange'] = '_ec_select(\'' . $key . '\', this)';

    $form['single_job'][$c]['_elysia_cron_job_weight_' . $key]['#title'] = NULL;
    $form['single_job'][$c]['_elysia_cron_job_weight_' . $key]['#description'] = NULL;
    $form['single_job'][$c]['_elysia_cron_job_weight_' . $key]['#attributes']['style'] = 'margin: 0';
    $form['single_job'][$c]['_elysia_cron_job_disabled_' . $key]['#title'] = NULL;
    $form['single_job'][$c]['_elysia_cron_job_disabled_' . $key]['#attributes']['style'] = 'margin: 0';
    $form['single_job'][$c]['_elysia_cron_job_channel_' . $key]['#title'] = NULL;
    $form['single_job'][$c]['_elysia_cron_job_channel_' . $key]['#attributes']['style'] = 'margin: 0';

    $coutput .= '<tr><td colspan="6"><b>' . $form['single_job'][$c]['#title'] . '</b>' . (($d = $form['single_job'][$c]['#description']) && $d != '-' ? ' <i>(' . $d . ')</i>' : '' ) . '</td></tr>';
    $coutput .= '<tr>' .
      '<td align="center">' . drupal_render($form['single_job'][$c]['_elysia_cron_job_disabled_' . $key]) . '</td>' .
      '<td>' . drupal_render($form['single_job'][$c]['_elysia_cron_seljob_rule_' . $key]) . drupal_render($form['single_job'][$c]['_elysia_cron_job_rule_' . $key]) . '</td>' .//'<td><small>'.$def_rule.'</small></td>'.
      '<td>' . drupal_render($form['single_job'][$c]['_elysia_cron_job_weight_' . $key]) . '</td><td><small>' . $def_weight . '</small></td>' .
      '<td>' . drupal_render($form['single_job'][$c]['_elysia_cron_job_channel_' . $key]) . '</td>' .
    '</tr>';
    drupal_render($form['single_job'][$c]);
  }
  $coutput .= '</table>';

  $form['single_job']['#children'] = $coutput;
  //$form['single_job'][] = array('#type' => 'markup', '#markup' => $output);

  $coutput = '<table>';

  $i = 0;

  foreach (element_children($form['channels']) as $c) {
    $key = substr($c, 15);
    if ($i++ == 0) {
      $coutput .= '<tr>' .
        '<th>' . t('Name') . '</th>' .
        '<th>' . $form['channels'][$c]['_elysia_cron_ch_disabled_' . $key]['#title'] . '</th>' .
        '<th>' . $form['channels'][$c]['_elysia_cron_ch_rule_' . $key]['#title'] . '</th>' .
      '</tr>';
    }

    $form['channels'][$c]['_elysia_cron_ch_disabled_' . $key]['#title'] = NULL;
    $form['channels'][$c]['_elysia_cron_ch_disabled_' . $key]['#attributes']['style'] = 'margin: 0';
    $form['channels'][$c]['_elysia_cron_ch_rule_' . $key]['#title'] = NULL;
    $form['channels'][$c]['_elysia_cron_ch_rule_' . $key]['#attributes']['style'] = 'margin: 0';

    $coutput .= '<tr>' .
      '<td><b>' . $form['channels'][$c]['#title'] . '</b></td>' .
      '<td>' . drupal_render($form['channels'][$c]['_elysia_cron_ch_disabled_' . $key]) . '</td>' .
      '<td>' . drupal_render($form['channels'][$c]['_elysia_cron_ch_rule_' . $key]) . '</td>' .
    '</tr>';
    drupal_render($form['channels'][$c]);
  }
  $coutput .= '</table>';

  $form['channels']['#children'] = $coutput;

  return $output . drupal_render_children($form);
  //$form['channels'][] = array('#type' => 'markup', '#markup' => $output);
  //return drupal_render(_dcr_form($form));
}

function elysia_cron_settings_form_validate($_dco_form, &$_dco_form_state) {
  extract(_dcf_form_validate($_dco_form, $_dco_form_state));
  global $elysia_cron_settings;

  $script = $form_state['values']['elysia_cron_script'];
  if ($script) {
    $errors = elysia_cron_decode_script($script, false);
    if ($errors) {
      form_set_error('elysia_cron_script', t('Invalid lines:') . implode('<br>', $errors));
    }
  }

  foreach ($form_state['values'] as $key => $value) {
    if ($value && preg_match('/^_elysia_cron_([^_]+_[^_]+)_(.*)$/', $key, $r) && ($r[1] == 'job_rule' || $r[1] == 'ch_rule')) {
      if (!preg_match('/^\\s*([0-9*,\/-]+[ ]+[0-9*,\/-]+[ ]+[0-9*,\/-]+[ ]+[0-9*,\/-]+[ ]+[0-9*,\/-]+)\\s*$/', $value)) {
        form_set_error($key, t('Invalid rule: !rule', array('!rule' => $value)));
      }
    }
  }
}

function elysia_cron_settings_form_submit($_dco_form, &$_dco_form_state) {
  extract(_dcf_form_validate($_dco_form, $_dco_form_state));
  $form_values = $form_state['values'];

  $op = isset($form_values['op']) ? $form_values['op'] : '';

  // Exclude unnecessary elements.
  unset($form_values['submit'], $form_values['reset'], $form_values['form_id'], $form_values['op'], $form_values['form_token']);

  $elysia_cron_default_rules = array();
  $rules = explode("\n", $form_values['elysia_cron_default_rules']);
  foreach ($rules as $r) {
    if (trim($r)) {
      $rr = explode("=", $r);
      $elysia_cron_default_rules[trim($rr[1])] = trim($rr[0]);
    }
  }
  variable_set('elysia_cron_default_rules', $elysia_cron_default_rules);

  foreach ($form_values as $key => $value) {
    $value = trim($value);
    if (!preg_match('/^_elysia_cron_([^_]+_[^_]+)_(.*)$/', $key, $r)) {
      if ($op == t('Reset to defaults') || ($key != 'cron_safe_threshold' && !$value)) {
        variable_del($key);
      }
      elseif ($key != 'elysia_cron_default_rules') {
        if (is_array($value) && isset($form_values['array_filter'])) {
          $value = array_keys(array_filter($value));
        }
        variable_set($key, $value);
      }
    }
    else {
      $nullvalue = $r[1] != 'job_weight' ? !$value : !$value && $value !== '0';

      //dprint($r[1].' '.$r[1].' '.$r[2]);
      if ($op == t('Reset to defaults') || $nullvalue) {
        switch ($r[1]) {
          case 'job_channel':
            elysia_cron_reset_job_channel($r[2]);
            break;
          case 'job_rule':
            elysia_cron_reset_job_rule($r[2]);
            break;
          case 'job_weight':
            elysia_cron_reset_job_weight($r[2]);
            break;
          case 'job_disabled':
            elysia_cron_reset_job_disabled($r[2]);
            break;
          case 'ch_disabled':
            elysia_cron_reset_channel_disabled($r[2]);
            break;
          case 'ch_rule':
            elysia_cron_reset_channel_rule($r[2]);
            break;
        }
      }
      else {
        switch ($r[1]) {
          case 'job_channel':
            elysia_cron_set_job_channel($r[2], $value);
            break;
          case 'job_rule':
            if ($form_values['_elysia_cron_seljob_rule_' . $r[2]] == 'custom') {
              elysia_cron_set_job_rule($r[2], $value);
            }
            break;
          case 'seljob_rule':
            if ($value != 'custom') {
              if ($value == 'default') {
                elysia_cron_reset_job_rule($r[2]);
              }
              else {
                elysia_cron_set_job_rule($r[2], $value);
              }
            }
            break;
          case 'job_weight':
            elysia_cron_set_job_weight($r[2], $value);
            break;
          case 'job_disabled':
            elysia_cron_set_job_disabled($r[2], $value);
            break;
          case 'ch_disabled':
            elysia_cron_set_channel_disabled($r[2], $value);
            break;
          case 'ch_rule':
            elysia_cron_set_channel_rule($r[2], $value);
            break;
        }
      }

    }
  }
  if ($op == t('Reset to defaults')) {
    drupal_set_message(t('The configuration options have been reset to their default values.'));
  }
  else {
    drupal_set_message(t('The configuration options have been saved.'));
  }
}

function elysia_cron_date($timestamp) {
  return $timestamp > 0 ? format_date($timestamp, EC_DRUPAL_VERSION >= 7 ? 'short' : 'small') : t('n/a');
  //return date(variable_get('date_format_short', 'm/d/Y - H:i'), $timestamp);
}

function elysia_cron_run_form() {
  $form = array();
  $form['runf'] = array(
    '#type' => 'fieldset',
  );
  $form['runf']['run'] = array(
    '#type' => 'submit',
    '#value' => t('Run cron'),
  );
  return $form;
}

function elysia_cron_run_form_submit($_dco_form, &$_dco_form_state) {
  // Run cron manually from Cron form.
  if (elysia_cron_run()) {
    drupal_set_message(t('Cron run successfully.'));
  }
  else {
    drupal_set_message(t('Cron run failed.'), 'error');
  }

  drupal_goto(_dcf_internal_path('admin/config/system/cron'));
}

function elysia_cron_execute_page($job = false) {
  global $cron_completed, $cron_executing_job, $conf;

  if (!$job) {
    drupal_set_message(t('No job specified'), 'error');
    drupal_goto(_dcf_internal_path('admin/config/system/cron'));
  }

  $running = false;
  if (elysia_cron_is_job_running($job, false)) {
    if (time() - elysia_cron_get_job_last_run($job, 0) > variable_get('elysia_cron_stuck_time', 3600)) {
      drupal_set_message(t('Job %job already running, but is probably stuck, so i consider it as terminated', array('%job' => $job)));
    }
    else {
      drupal_set_message(t('Job %job already running', array('%job' => $job)));
      $running = true;
    }
  }

  if (!$running) {
    $cron_completed = false;
    $cron_executing_job = $job;

    if (EC_DRUPAL_VERSION < 7) {
      // Some modules (feedapi, ipaper...) uses the internal "cron_semaphore" variable to detect
      // start time of cron process. I'll set this only in memory for that purpose.
      // (In normal drupal cron execution that is done by a variable_set just before this call,
      // but i need to set this manually if drupal cron is bypassed)
      $conf['cron_semaphore'] = time();
    }

    // Register shutdown callback
    register_shutdown_function('elysia_cron_execute_page_cleanup');

    elysia_cron_initialize();
    elysia_cron_execute($job);

    if (EC_DRUPAL_VERSION < 7) {
      _ec_variable_del('cron_semaphore');
      $conf['cron_semaphore'] = false;
    }

    $cron_completed = true;

    drupal_set_message(t('Job executed'));
  }

  drupal_goto(_dcf_internal_path('admin/config/system/cron'));
}

function elysia_cron_execute_page_cleanup() {
  global $cron_completed, $cron_executing_job;

  if ($cron_completed) {
    return;
  }

  // See if the semaphore is still locked.
  if (elysia_cron_is_job_running($cron_executing_job)) {
    _dco_watchdog('cron', 'Unexpected termination of cron job %job manually started, aborted.', array('%job' => $cron_executing_job), WATCHDOG_WARNING);

    elysia_cron_set_job_running($cron_executing_job, 0);
  }
}

function elysia_cron_maintenance_page() {
  $output = array();
  $output[] = drupal_get_form('elysia_cron_reset_statistics_form');
  
  return _dcr_render_array($output);
}

function elysia_cron_reset_statistics_form() {
  $form = array();
  $form['fieldset'] = array(
    '#type' => 'fieldset',
    '#title' => t('Reset statistics'),
    '#description' => t('Deletes all cron execution statitics (Last run, last exec time, exec count, avg/max exec time...). Do not touch cron settings.<br /><b>This operation could not be reverted</b><br />'),
  );
  $form['fieldset']['reset'] = array(
    '#type' => 'submit',
    '#value' => t('Reset'),
    '#attributes' => array(
      'onclick' => 'return confirm(\'' . htmlentities(t('Are you sure you want to reset statistics?')) . '\')',
    ),
  );
  return $form;
}

function elysia_cron_reset_statistics_form_submit($_dco_form, &$_dco_form_state) {
  elysia_cron_reset_stats();
  
  drupal_set_message(t('Reset done.'));
  drupal_goto(_dcf_internal_path('admin/config/system/cron/maintenance'));
}

function elysia_cron_reset_page() {
  elysia_cron_reset_statistics_form_submit(false, $res = array());
}
