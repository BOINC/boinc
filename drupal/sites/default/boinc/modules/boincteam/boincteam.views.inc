<?php
// $Id$

/**
 * @file
 * Exposes BOINC team data to Views.
 */

/**
 * Implementation of hook_views_data().
 * Defines the structure of data and how Views should handle it (i.e. which
 * relationships exist; is data text, integer, float; what sort options exist).
 */
function boincteam_views_data() {

  // -----------------------------------------------------------------------------------------------
  // Definition for team table
  // -----------------------------------------------------------------------------------------------

  $data['team']['table']['group'] = t('BOINC');

  $data['team']['table']['base'] = array(
      'field' => 'id',
      'title' => t('BOINC team'),
      'help' => t('BOINC data for a team'),
      'database' => 'boinc_rw'
  );

  // This table references the {user} table.
  // This join creates an 'implicit' relationship to the user table, so that when
  // "User" is the base table, the fields are automatically available.

  // Index this array by the table name to which this table refers.
  // 'left_field' is the primary key in the referenced table.
  // 'field' is the foreign key in this table.

  $data['team']['table']['join'] = array(
    'user' => array(
      'left_field' => 'id',
      'field' => 'userid',
    ),
  );

  // Describe each of the individual fields in this table to Views. For
  // each field, you may define what field, sort, argument, and/or filter
  // handlers it supports. This will determine where in the Views interface you
  // may use the field.

  // Primary keys allowed as arguments

  $data['team']['id'] = array(
    'title' => bts('Team ID', array(), NULL, 'boinc:team-id'),
    'help' => t('The BOINC ID of the team.'),
    'field' => array(
      'handler' => 'views_handler_field_numeric',
      'click sortable' => TRUE
    ),
    'argument' => array(
      'handler' => 'views_handler_argument_boincteam_id', // custom handler
      'name field' => 'title', // the field to display in the summary.
      'numeric' => TRUE,
      'validate type' => 'id'
    ),
    'relationship' => array(
      'base' => 'team_delta',
      'field' => 'id',
      'base field' => 'teamid',
      'handler' => 'views_handler_relationship',
      'label' => t('Team History')
    ),
    'filter' => array(
      'handler' => 'views_handler_filter_numeric'
    ),
    'sort' => array(
      'handler' => 'views_handler_sort_numeric'
    )
  );

  // Foreign key fields

  $data['team']['userid'] = array(
    'title' => bts('Founder', array(), NULL, 'boinc:view-team-info'),
    'help' => t('The founder of this team.'),
    // This is a foreign key to the {user} table. When the view is configured
    // with this relationship, all the fields for the related user node will be
    // available.
    'argument' => array(
      'handler' => 'views_handler_argument_boincuser_id',
      'name field' => 'title',
      'numeric' => TRUE,
      'validate type' => 'id'
    ),
    'relationship' => array(
      'base' => 'user',
      'field' => 'userid',
      'handler' => 'views_handler_relationship',
      'label' => t('User')
    ),
    'field' => array(
      'handler' => 'views_handler_field_numeric',
      'click sortable' => TRUE
    ),
    'filter' => array(
      'handler' => 'views_handler_filter_numeric'
    ),
    'sort' => array(
      'handler' => 'views_handler_sort_numeric'
    )
  );

  // Descriptions of general fields (alphabetized)
  /*
  $data['team']['create_time'] = array(
    'title' => bts('Team established', array(), NULL, 'boinc:date-team-established'),
    'help' => t('When the BOINC team was created.'),
    'field' => array(
      'handler' => 'views_handler_field_date',
      'click sortable' => TRUE
    ),
    'filter' => array(
      'handler' => 'views_handler_filter_date'
    ),
    'sort' => array(
      'handler' => 'views_handler_sort_date'
    )
  );
  $data['team']['url'] = array(
    'title' => bts('Website', array(), NULL, 'boinc:website-of-user-or-team'),
    'help' => t('The URL of a team.'),
    'field' => array(
      'handler' => 'views_handler_field',
      'click sortable' => TRUE
    ),
    'filter' => array(
      'handler' => 'views_handler_filter_string'
    ),
    'sort' => array(
      'handler' => 'views_handler_sort_string'
    )
  );
  */

  $data['team']['country'] = array(
    'title' => bts('Country', array(), NULL, 'boinc:country-of-origin'),
    'help' => t('The country of a team.'),
    'field' => array(
      'handler' => 'views_handler_field',
      'click sortable' => TRUE
    ),
    'filter' => array(
      'handler' => 'views_handler_filter_string'
    ),
    'sort' => array(
      'handler' => 'views_handler_sort_string'
    )
  );
  $data['team']['expavg_credit'] = array(
    'title' => bts('Recent average credit', array(), NULL, 'boinc:user-or-team-RAC'),
    'help' => t('A decaying average of team credit per day.'),
    'field' => array(
      'handler' => 'views_handler_field_numeric',
      'click sortable' => TRUE,
      'float' => TRUE
    ),
    'filter' => array(
      'handler' => 'views_handler_filter_numeric'
    ),
    'sort' => array(
      'handler' => 'views_handler_sort_numeric'
    )
  );
  $data['team']['name'] = array(
    'title' => bts('Name', array(), NULL, 'boinc:user-or-team-name'),
    'help' => t('The name of the team.'),
    'field' => array(
      'handler' => 'views_handler_field',
      'click sortable' => TRUE
    ),
    'filter' => array(
      'handler' => 'views_handler_filter_string'
    ),
    'sort' => array(
      'handler' => 'views_handler_sort_string'
    )
  );
  $data['team']['nusers'] = array(
    'title' => bts('Members', array(), NULL, 'boinc:team-members'),
    'help' => t('Count of team members.'),
    'field' => array(
      'handler' => 'views_handler_field_numeric',
      'click sortable' => TRUE
    ),
    'filter' => array(
      'handler' => 'views_handler_filter_numeric'
    ),
    'sort' => array(
      'handler' => 'views_handler_sort_numeric'
    )
  );
  $data['team']['total_credit'] = array(
    'title' => bts('Total credit', array(), NULL, 'boinc:user-or-team-total-credits'),
    'help' => t('The total team accumulated BOINC credit.'),
    'field' => array(
      'handler' => 'views_handler_field_numeric',
      'click sortable' => TRUE,
      'float' => TRUE
    ),
    'filter' => array(
      'handler' => 'views_handler_filter_numeric'
    ),
    'sort' => array(
      'handler' => 'views_handler_sort_numeric'
    )
  );

  // -----------------------------------------------------------------------------------------------
  // Definition for team_admin table
  // -----------------------------------------------------------------------------------------------

  $data['team_admin']['table']['group'] = t('BOINC');

  $data['team_admin']['table']['base'] = array(
      'field' => 'id',
      'title' => t('BOINC team admins'),
      'help' => t('BOINC admins for a team'),
      'database' => 'boinc_rw'
  );

  // This table references the {team} table.
  // This join creates an 'implicit' relationship to the team table, so that when
  // "Team" is the base table, the fields are automatically available.

  // Index this array by the table name to which this table refers.
  // 'left_field' is the primary key in the referenced table.
  // 'field' is the foreign key in this table.

  $data['team_admin']['table']['join'] = array(
    'team' => array(
      'left_field' => 'id',
      'field' => 'teamid',
    ),
  );

  // Describe each of the individual fields in this table to Views. For
  // each field, you may define what field, sort, argument, and/or filter
  // handlers it supports. This will determine where in the Views interface you
  // may use the field.

  // Primary keys allowed as arguments

  $data['team_admin']['teamid'] = array(
    'title' => bts('Team ID', array(), NULL, 'boinc:team-id'),
    'help' => t('The BOINC ID of the team on which this user is an admin.'),
    'field' => array(
      'handler' => 'views_handler_field_numeric',
      'click sortable' => TRUE
    ),
    'argument' => array(
      'handler' => 'views_handler_argument_boincteam_id', // custom handler
      'name field' => 'title', // the field to display in the summary.
      'numeric' => TRUE,
      'validate type' => 'id'
    ),
    'relationship' => array(
      'base' => 'team',
      'field' => 'teamid',
      'handler' => 'views_handler_relationship',
      'label' => t('Team')
    ),
    'filter' => array(
      'handler' => 'views_handler_filter_numeric'
    ),
    'sort' => array(
      'handler' => 'views_handler_sort_numeric'
    )
  );

  // Foreign key fields

  $data['team_admin']['userid'] = array(
    'title' => bts('User ID', array(), NULL, 'boinc:user-details'),
    'help' => t('The user that is a team admin.'),
    // This is a foreign key to the {user} table. When the view is configured
    // with this relationship, all the fields for the related user node will be
    // available.
    'argument' => array(
      'handler' => 'views_handler_argument_boincteam_id',
      'name field' => 'title',
      'numeric' => TRUE,
      'validate type' => 'id'
    ),
    'relationship' => array(
      'base' => 'user',
      'field' => 'userid',
      'handler' => 'views_handler_relationship',
      'label' => bts('User', array(), NULL, 'boinc:user-on-team')
    ),
    'field' => array(
      'handler' => 'views_handler_field_numeric',
      'click sortable' => TRUE
    ),
    'filter' => array(
      'handler' => 'views_handler_filter_numeric'
    ),
    'sort' => array(
      'handler' => 'views_handler_sort_numeric'
    )
  );

  // Descriptions of general fields (alphabetized)

  $data['team_admin']['create_time'] = array(
    'title' => bts('Admin since', array(), NULL, 'boinc:team-when-user-became-an-admin'),
    'help' => t('When the BOINC user became a team admin.'),
    'field' => array(
      'handler' => 'views_handler_field_date',
      'click sortable' => TRUE
    ),
    'filter' => array(
      'handler' => 'views_handler_filter_date'
    ),
    'sort' => array(
      'handler' => 'views_handler_sort_date'
    )
  );

  // -----------------------------------------------------------------------------------------------
  // Definition for team_delta table
  // -----------------------------------------------------------------------------------------------

  $data['team_delta']['table']['group'] = t('BOINC');

  // Describe each of the individual fields in this table to Views. For
  // each field, you may define what field, sort, argument, and/or filter
  // handlers it supports. This will determine where in the Views interface you
  // may use the field.

  // Primary keys allowed as arguments

  $data['team_delta']['teamid'] = array(
    'title' => bts('Team ID', array(), NULL, 'boinc:team-id'),
    'help' => t('The team ID for this team history event.'),
    // This is a foreign key to the {team} table. When the view is configured
    // with this relationship, all the fields for the related user node will be
    // available.
    'argument' => array(
      'handler' => 'views_handler_argument_boincteam_id',
      'name field' => 'title',
      'numeric' => TRUE,
      'validate type' => 'id'
    ),
    'field' => array(
      'handler' => 'views_handler_field_numeric',
      'click sortable' => TRUE
    ),
    'filter' => array(
      'handler' => 'views_handler_filter_numeric'
    ),
    'sort' => array(
      'handler' => 'views_handler_sort_numeric'
    )
  );

  // Foreign key fields

  $data['team_delta']['userid'] = array(
    'title' => bts('User ID', array(), NULL, 'boinc:user-details'),
    'help' => t('The user for this team history event.'),
    'relationship' => array(
      'base' => 'user',
      'field' => 'userid',
      'handler' => 'views_handler_relationship',
      'label' => t('User')
    ),
    'field' => array(
      'handler' => 'views_handler_field_numeric',
      'click sortable' => TRUE
    ),
    'filter' => array(
      'handler' => 'views_handler_filter_numeric'
    ),
    'sort' => array(
      'handler' => 'views_handler_sort_numeric'
    )
  );

  // Descriptions of general fields (alphabetized)

  $data['team_delta']['joining'] = array(
    'title' => bts('User joined', array(), NULL, 'boinc:user-joined-or-leaving-team'),
    'help' => t('Whether the event is the user joining the team or leaving.'),
    'field' => array(
      'handler' => 'views_handler_field_numeric',
      'click sortable' => TRUE
    ),
    'filter' => array(
      'handler' => 'views_handler_filter_numeric'
    ),
    'sort' => array(
      'handler' => 'views_handler_sort_numeric'
    )
  );
  $data['team_delta']['timestamp'] = array(
    'title' => bts('Timestamp', array(), NULL, 'boinc:team-history-timestamp'),
    'help' => t('When the BOINC team history event took place.'),
    'field' => array(
      'handler' => 'views_handler_field_date',
      'click sortable' => TRUE
    ),
    'filter' => array(
      'handler' => 'views_handler_filter_date'
    ),
    'sort' => array(
      'handler' => 'views_handler_sort_date'
    )
  );
  $data['team_delta']['total_credit'] = array(
    'title' => bts('Total credit', array(), NULL, 'user-or-team-total-credits'),
    'help' => t('The total accumulated BOINC credit of the user when the
      history event took place.'),
    'field' => array(
      'handler' => 'views_handler_field_numeric',
      'click sortable' => TRUE,
      'float' => TRUE
    ),
    'filter' => array(
      'handler' => 'views_handler_filter_numeric'
    ),
    'sort' => array(
      'handler' => 'views_handler_sort_numeric'
    )
  );

  return $data;
}

/*
 * hook_views_handlers(): Reference custom handlers for data.
 * Custom handlers can manipulate data in useful ways. The boincteam_id
 * argument takes a Drupal user ID and converts it to BOINC ID via a
 * reference table for seamless access of user data in the BOINC database.
 */

function boincteam_views_handlers() {
  return array(
    'info' => array(
      'path' => drupal_get_path('module', 'boincteam') . '/views',
    ),
    'handlers' => array(
      'views_handler_argument_boincteam_id' => array(
        'parent' => 'views_handler_argument_numeric'
      )
    )
  );
}
