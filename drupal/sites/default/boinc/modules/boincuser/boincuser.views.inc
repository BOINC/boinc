<?php
// $Id$

/**
 * @file
 * Exposes BOINC user data to Views.
 */

/**
 * Implementation of hook_views_data().
 * Defines the structure of data and how Views should handle it (i.e. which
 * relationships exist; is data text, integer, float; what sort options exist).
 */
function boincuser_views_data() {
  
  // -----------------------------------------------------------------------------------------------
  // Definition for user table
  // -----------------------------------------------------------------------------------------------
  
  $data['user']['table']['group'] = t('BOINC');
  
  $data['user']['table']['base'] = array(
      'field' => 'id',
      'title' => t('BOINC user'),
      'help' => t('BOINC account data for a user'),
      'database' => 'boinc'
  );

  // Describe each of the individual fields in this table to Views. For
  // each field, you may define what field, sort, argument, and/or filter
  // handlers it supports. This will determine where in the Views interface you
  // may use the field.
  
  // Primary keys allowed as arguments
  
  $data['user']['id'] = array(
    'title' => bts('Id'),
    'help' => t('The BOINC ID of the user account.'),
    'field' => array(
      'handler' => 'views_handler_field_numeric',
      'click sortable' => TRUE
    ),
    'argument' => array(
      'handler' => 'views_handler_argument_boincuser_id', // custom handler
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
  $data['user']['teamid'] = array(
    'title' => bts('Team ID'),
    'help' => t('The ID of the user\'s team.'),
    'field' => array(
      'handler' => 'views_handler_field_numeric',
      'click sortable' => TRUE,
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
  
  // Descriptions of general host fields (alphabetized)
  
  $data['user']['authenticator'] = array(
    'title' => bts('Account Key'),
    'help' => t('The BOINC account key of the user.'),
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
  $data['user']['country'] = array(
    'title' => bts('Country'),
    'help' => t('The country of the user.'),
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
  $data['user']['create_time'] = array(
    'title' => bts('Member since'),
    'help' => t('When the BOINC account was created.'),
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
  $data['user']['cross_project_id'] = array(
    'title' => bts('CPID'),
    'help' => t('The cross project user identifier.'),
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
  $data['user']['email_addr'] = array(
    'title' => bts('Email Address'),
    'help' => t('The BOINC account email address.'),
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
  $data['user']['expavg_credit'] = array(
    'title' => bts('Recent average credit'),
    'help' => t('A decaying average of credit per day for the user.'),
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
  $data['user']['name'] = array(
    'title' => bts('Name'),
    'help' => t('The BOINC account user name.'),
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
  $data['user']['passwd_hash'] = array(
    'title' => bts('Password Hash'),
    'help' => t('The user password hash'),
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
  $data['user']['postal_code'] = array(
    'title' => bts('Postal Code'),
    'help' => t('The postal code of the user.'),
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
  $data['user']['send_email'] = array(
    'title' => bts('Send Email'),
    'help' => t('User preference to allow email communications.'),
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
  $data['user']['show_hosts'] = array(
    'title' => bts('Show Hosts'),
    'help' => t('User preference to display owned computers.'),
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
  $data['user']['total_credit'] = array(
    'title' => bts('Total Credit'),
    'help' => t('The total accumulated BOINC credit for the user.'),
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
  $data['user']['url'] = array(
    'title' => bts('URL'),
    'help' => t('The URL provided by the user'),
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
  
  return $data;
}

/*
 * hook_views_handlers(): Reference custom handlers for data.
 * Custom handlers can manipulate data in useful ways. The boincuser_id
 * argument takes a Drupal user ID and converts it to BOINC ID via a
 * reference table for seamless access of user data in the BOINC database.
 */

function boincuser_views_handlers() {
  return array(
    'info' => array(
      'path' => drupal_get_path('module', 'boincuser') . '/views',
    ),
    'handlers' => array(
      'views_handler_argument_boincuser_id' => array(
        'parent' => 'views_handler_argument_numeric'
      ),
      'views_handler_argument_boincteam_id' => array(
        'parent' => 'views_handler_argument_numeric'
      ),
    )
  );
}
