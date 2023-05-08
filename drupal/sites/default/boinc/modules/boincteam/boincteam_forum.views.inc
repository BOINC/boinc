<?php
// $Id$

/**
 * @file
 * Exposes BOINC team forum data to Views.
 */

/**
 * Implementation of hook_views_data().
 * Defines the structure of data and how Views should handle it (i.e. which
 * relationships exist; is data text, integer, float; what sort options exist).
 */
function boincteam_forum_views_data() {

  // -----------------------------------------------------------------------------------------------
  // Definition for team table
  // -----------------------------------------------------------------------------------------------

  $data['boincteam_forum_node']['table']['group'] = t('BOINC');

  $data['boincteam_forum_node']['table']['base'] = array(
      'field' => 'nid',
      'title' => t('BOINC team forum topic'),
      'help' => t('Forum topics for a team'),
  );

  // Describe each of the individual fields in this table to Views. For
  // each field, you may define what field, sort, argument, and/or filter
  // handlers it supports. This will determine where in the Views interface you
  // may use the field.

  // Primary keys allowed as arguments

  $data['boincteam_forum_node']['tfid'] = array(
    'title' => t('Team forum ID'),
    'help' => t('The ID of the team forum containing this topic.'),
    'field' => array(
      'handler' => 'views_handler_field_numeric',
      'click sortable' => TRUE
    ),
    'argument' => array(
      'handler' => 'views_handler_argument_numeric',
      'name field' => 'title', // the field to display in the summary.
      'numeric' => TRUE,
      'validate type' => 'id'
    ),
    'relationship' => array(
      'base' => 'boincteam_forum',
      'field' => 'tfid',
      'handler' => 'views_handler_relationship',
      'label' => t('Team Forum')
    ),
    'filter' => array(
      'handler' => 'views_handler_filter_numeric'
    ),
    'sort' => array(
      'handler' => 'views_handler_sort_numeric'
    )
  );

  // Foreign key fields

  $data['boincteam_forum_node']['nid'] = array(
    'title' => t('Team forum node'),
    'help' => t('The node containing the content of this team forum topic.'),
    // This is a foreign key to the {node} table. When the view is configured
    // with this relationship, all the fields for the related user node will be
    // available.
    'relationship' => array(
      'base' => 'node',
      'field' => 'nid',
      'handler' => 'views_handler_relationship',
      'label' => t('Team Forum Topic Node')
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
  // ...

  return $data;
}
