<?php
// $Id$

/**
 * @file
 * Administration page callbacks for the boicnteam module.
 */

/**
* Form builder. Configure annotations.
*
* @ingroup forms
* @see system_settings_form().
*/

/**
 * Utility Form to delete teams suspected of being spammers.
 */
function boincteam_utility_delete(&$form_state) {
  $form = array();

  $form['help'] = array(
    '#type' => 'fieldset',
    '#title' => t('Help'),
    '#collapsible' => TRUE,
  );

  $form['help']['text'] = array(
    '#type' => 'item',
    '#description' => t('This utility page will help in identifying BOINC teams created by SPAMMERs which can be deleted. The search will find BOINC teams that meet the following criteria.
<ul>
<li>Team has zero or one members.</li>
<li>The team has zero (0) total credits.</li>
<li>The team description contains a link.</li>
<li>The team is NOT a BOINC-wide team.</li>
</ul>
<p> <b>Warning:</b> There is no way to UNDO or UNDELETE a team after it has been deleted. It is a good idea to make a backup of the BOINC project database before deleting team information.</p>
<p> <i>How teams are deleted</i>: Select the teams to be deleted from the table below. Click "Delete Selected Teams" button to begin a batch operation to delete the teams. If you select teams, and then page through to another page of the table, your selections will be lost. I.e., only the teams select on the current page will be deleted.
<p> The site must be offline/in maintenance mode before searching for and deleting teams. If not, the delete button below is disabled.
'),
  );

  // This variable name has been changed for Drupal 7/8.
  $maintenance_mode = variable_get('site_offline');
  if (!$maintenance_mode) {
    drupal_set_message(t('WARNING: Site is online (not in maintenance mode)! Deleting teams is only allowed when site is offline. Change this setting in ') . l(t('Site maintenance'), '/admin/settings/site-maintenance'), 'warning');
  }

  $form['teamdelete'] = array(
    '#type' => 'fieldset',
    '#title' => t('Teams to delete'),
  );
  $form['teamdelete']['deleteall'] = array(
    '#type' => 'submit',
    '#disabled' => !($maintenance_mode),
    '#value' => t('Delete Selected Teams'),
    '#submit' => array('boincteam_utility_delete_team'),
    '#attributes' => array(
        'onclick' => 'return confirm(\'' . t('You are about to delete the selected teams suspected of being SPAM. Do you want to continue?') . '\')',
    ),
  );
  $form['teamdelete']['results_table'] = array(
    '#value' => drupal_get_form('boincteam_utility_team_table'),
  );

  return $form;
}

function boincteam_utility_team_table() {

  $limit=50;
  $sql = "SELECT team.id AS id, team.name AS name, team.nusers AS nusers FROM {team} WHERE team.nusers<2 AND team.total_credit=0 AND team.seti_id=0 AND team.description REGEXP '<a href'";

  // @todo - add bts() or t()
  $resultheader = array(
    array(
      'data' => 'Team ID',
      'field' => 'id',
      'sort' => 'asc',
    ),
    array(
      'data' => 'Team Name',
      'field' => 'name',
    ),
    array(
      'data' => 'Num Users',
      'field' => 'nusers',
    ),
    array(
      'data' => 'Link to Team',
      'field' => 'link',
    ),
  );

  $tablesort = tablesort_sql($resultheader);
  db_set_active('boinc_rw');
  $db_res = pager_query($sql . $tablesort, $limit);
  db_set_active('default');

  $form = array();
  $form['options'] = array();
  $checkboxes = array();
  if ($db_res) {
    while ($result = db_fetch_object($db_res)) {
      $drupalnid = boincteam_lookup_nid($result->id);
      if ($drupalnid) {
        $tlink = l($result->name, '/community/teams/' . $drupalnid);
      }
      else {
        $tlink = '';
      }
      // only keys no values
      $checkboxes[$result->id] = '';

      $form['id'][$result->id] = array(
        '#value' => $result->id,
      );
      $form['name'][$result->id] = array(
        '#value' => $result->name,
      );
      $form['nusers'][$result->id] = array(
        '#value' => $result->nusers,
      );
      $form['link'][$result->id] = array(
        '#value' => $tlink,
      );
    }

    $form['checkboxes'] = array('#type' => 'checkboxes', '#options' => $checkboxes);
    $form['pager'] = array('#value' => theme('pager', NULL, $limit, 0));

    return $form;
  }
  return "<p>No teams matched the criteria for a SPAM team.</p>";
}

/**
 * Submit handler - also creates the batch job to delete the teams.
 */
function boincteam_utility_delete_team($form, &$form_state) {
  $checkedteamids = $form_state['clicked_button']['#post']['checkboxes'];
  // Use batch to delete teams
  $operations = array();
  foreach ($checkedteamids as $id) {
    $operations[] = array('boincteam_utility_batch_process', array($id));
  }
  $batch = array(
    'title' => t('Processing BOINC Teams'),
    'operations' => $operations,
    'init_message' => t('Starting team deletion'),
    'finished' => 'boincteam_utility_batch_finished',
    'file' => drupal_get_path('module', 'boincteam') . '/boincteam.admin.inc',
  );
  batch_set($batch);
  //batch_process('/admin/boinc/utility-team-delete');
}

/*
 * Theme the boincteam_utility_team_table form/table
 *
 * Parameters:
 *   @params $form
 *     Input form (table)
 */
function theme_boincteam_utility_team_table($form) {
  //define table header
  $header = array(
    theme('table_select_header_cell'), //using that previously empty field
    array('data' => t('Team ID'), 'field' => 'id', 'sort' => 'asc'),
    array('data' => t('Name'), 'field' => 'name'),
    array('data' => t('Num Users'), 'field' => 'nusers'),
    array('data' => t('Link to Team'), 'field' => 'link'),
  );
  $rows = array();
  $output = "";
  if(!empty($form['checkboxes']['#options'])) {
    foreach (element_children($form['id']) as $key) {
      $rows[] = array(
        drupal_render($form['checkboxes'][$key]),
        drupal_render($form['id'][$key]),
        drupal_render($form['name'][$key]),
        drupal_render($form['nusers'][$key]),
        drupal_render($form['link'][$key]),
      );
    }
  }
  else {
    $rows[] = array(array('data' => '<div class="error">No teams found</div>', 'colspan' => 5));
  }
  $output .= theme('table', $header, $rows);
  if ($form['pager']['#value']) {
    $output .= drupal_render($form['pager']);
  }

  $output .= drupal_render($form);
  return $output;
}

/**
 * Function which is run during the batch job, deletes a single team
 * from the procject team table, as well as drupal's boincteam
 * table. It also removes the team's drupal page (node) if found.
 */
function boincteam_utility_batch_process($id, &$context) {
  $boincteam = boincteam_load($id);
  $drupalid = boincteam_lookup_nid($id);

  // Delete entry in team table
  db_set_active('boinc_rw');
  $sql1='DELETE FROM {team} WHERE id=%d';
  $db_res = db_query($sql1, $id);
  db_set_active('default');

  // Delete entry in boincteam table
  $sql2='DELETE FROM {boincteam} WHERE id=%d';
  $db_res = db_query($sql2, $id);

  // Delete the drupal node (if present)
  if ($drupalid) {
    node_delete($drupalid);
  }

  $context['results'][] = $id . ' : ' . check_plain($boincteam->name);
  $context['message'] = t('Processing team @name', array('@name' => $boincteam->name));
}

/**
 * Function runs at the end of the batch operation, reporting success
 * or failure.
 */
function boincteam_utility_batch_finished($success, $results, $operations) {
  if ($success) {
    $message = 'Info: Delete SPAM Teams utility- ' . count($results) . ' team(s) deleted.';
    drupal_set_message($message, 'info');
  }
  else {
    // An error occurred.
    // $operations contains the operations that remained unprocessed.
    $error_operation = reset($operations);
    $message = 'Error: An error occurred while processing ' . $error_operation[0] . ' with arguments :' . print_r($error_operation[0], TRUE);
    drupal_set_message($message, 'error');
  }
}