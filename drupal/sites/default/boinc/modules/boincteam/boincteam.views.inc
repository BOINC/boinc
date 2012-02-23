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
      'database' => 'boinc'
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
    )
  );

  // Describe each of the individual fields in this table to Views. For
  // each field, you may define what field, sort, argument, and/or filter
  // handlers it supports. This will determine where in the Views interface you
  // may use the field.
  
  // Primary keys allowed as arguments
  
  $data['team']['id'] = array(
    'title' => t('Id'),
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
    'filter' => array(
      'handler' => 'views_handler_filter_numeric'
    ),
    'sort' => array(
      'handler' => 'views_handler_sort_numeric'
    )
  );
  
  // Foreign key fields
  
  $data['team']['userid'] = array(
    'title' => t('Founder'),
    'help' => t('The founder of this team.'),
    // Because this is a foreign key to the {user} table. This allows us to
    // have, when the view is configured with this relationship, all the fields
    // for the related node available.
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
    'title' => t('Team established'),
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
    'title' => t('URL'),
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
    'title' => t('Country'),
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
    'title' => t('Recent average credit'),
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
    'title' => t('Name'),
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
    'title' => t('Members'),
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
    'title' => t('Total Credit'),
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
