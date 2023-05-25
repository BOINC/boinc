<?php
// $Id$

/**
 *
 */

/*  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *
 * Import functions
 *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  */

/**
 * Import new teams and update BOINC-wide teams to pull in any changes from
 * the BOINC database
 */
function boincteam_sync() {

  // Get the list of teams to import
  db_set_active('boinc_rw');
  $boinc_teams = db_query('
    SELECT id, name, description, userid, create_time, seti_id
    FROM team
    WHERE mod_time > FROM_UNIXTIME(%d)',
    variable_get('boincteam_last_sync', 0)
  );
  db_set_active('default');

  $existing_teams = array();

  // Get the list of teams already in Drupal to be sure we're not importing
  // any twice
  $result = db_query('
    SELECT nid, team_id FROM {boincteam}'
  );
  while ($row = db_fetch_object($result)) {
    $existing_teams[$row->team_id] = $row->nid;
  }

  while ($boinc_team = db_fetch_object($boinc_teams)) {
    $success = NULL;
    if (isset($existing_teams[$boinc_team->id])) {
      if ($boinc_team->seti_id > 0) {
        // Sync BOINC-wide teams
        $nid = $existing_teams[$boinc_team->id];
        $success = boincteam_import($boinc_team, $nid);
      }
    }
    else {
      // Import new teams created by RPC or ops/team_import.php
      $success = boincteam_import($boinc_team);
    }
  }
}


/**
 * Import a given BOINC team object into Drupal
 */
function boincteam_import($boincteam, $nid = NULL) {

  $input_format = variable_get('boincimport_input_format', 0);
  $team_type_map = variable_get('boincimport_team_types', array());

  // Save the team type affiliation
  $team_type_tid = $team_type_map[$boincteam->type];

  $boincteam->description = html_entity_decode($boincteam->description, ENT_QUOTES, 'utf-8');
  // Be sure the text is filtered for the default input format
  $boincteam->description = check_markup($boincteam->description, $input_format);

  $teaser = node_teaser($boincteam->description);

  if ($nid) {
    // Update an existing node
    $node = node_load($nid);
    $node->title = $boincteam->name;
    $node->body = $boincteam->description;
    $node->teaser = $teaser;
    $node->uid = boincuser_lookup_uid($boincteam->userid);
  }
  else {
    // Construct the team as a new node
    $node = array(
      'type' => 'team',
      'title' => $boincteam->name,
      'body' => $boincteam->description,
      'teaser' => $teaser,
      'uid' => boincuser_lookup_uid($boincteam->userid),
      'path' => null,
      'status' => 1,  // published or not - always publish
      'promote' => 0,
      'created' => $boincteam->create_time,
      'comment' => 0,  // comments disabled
      'moderate' => 0,
      'sticky' => 0,
      'format' => $input_format
    );
    $node = (object) $node; // node_save requires an object form
  }

  $node->taxonomy[] = taxonomy_get_term($team_type_tid);

  // Save the team node
  node_save($node);
  $success = ($node->nid) ? TRUE : FALSE;

  if (!$nid) {
    // Save the team IDs to a BOINC <--> Drupal reference table, if needed
    db_query('INSERT INTO {boincteam} (team_id, nid) VALUES (%d, %d)', $boincteam->id, $node->nid);
  }

  return $success;
}

/**
 * Obtains an array of BOINC ids or Drupal uids for a given team.
 *
 * @param int $boincteamid
 *   The BOINC team id (not drupal team/node id).
 * @param bool $boincid
 *   A boolean, if TRUE the function will return the BOINC ids. If
 *   FALSE, the function will return the Drupal uids.
 * @param bool $respectprivacy
 *   A boolean, if TRUE the function will only return users who have
 *   flag send_email=1, meaning their notification privacy will be
 *   respected. This is useful the list of users are to be contacted.
 */
function _boincteam_userids($boincteamid, $boincid=TRUE, $respectprivacy=TRUE) {
  $sql = 'SELECT user.id as id FROM {user} user WHERE user.teamid=%s';
  if ($respectprivacy) {
    $sql .= ' AND user.send_email=1';
  }

  db_set_active('boinc_ro');
  $dbres = db_query($sql, $boincteamid);
  db_set_active('default');

  $ids = array();
  while (($result = db_fetch_object($dbres)) != FALSE) {
    $ids[] = $result->id;
  }
  if ($boincid) {
    return $ids;
  }
  else {
    return array_map('boincuser_lookup_uid', $ids);
  }
}

/**
 * Obtains an array of email addresses for a given team
 *
 * @param int $boincteamid
 *    The BOINC team id (not drupal team/node id).
 * @param bool $respectprivacy
 *   A boolean, if TRUE the function will only return users who have
 *   flag send_email=1, meaning their notification privacy will be
 *   respected. This is useful the list of users are to be contacted.
 */
function _boincteam_emails($boincteamid, $respectprivacy=TRUE) {
  $sql = 'SELECT user.email_addr as email_addr FROM {user} user WHERE user.teamid=%s';
  if ($respectprivacy) {
    $sql .= ' AND user.send_email=1';
  }

  db_set_active('boinc_ro');
  $dbres = db_query($sql, $boincteamid);
  db_set_active('default');

  $emails = array();
  while (($result = db_fetch_object($dbres)) != FALSE) {
    $emails[] = $result->email_addr;
  }
  return $emails;
}
