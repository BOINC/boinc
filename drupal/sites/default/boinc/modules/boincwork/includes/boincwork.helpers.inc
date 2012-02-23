<?php
// $Id$

/**
 * Functions that are shared amongst files and dependent modules go
 * here to keep the clutter down in the main module file.
 */ 


/*  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *
 * Functions for use in displaying special case text in views
 *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  */

/**
  * Determine output for host list views when no hosts are found.
  */
function views_host_list_empty_text($context = null) {
  
  // Pull the BOINC user ID from the view arguments to get show_hosts 
  // preference for that user
  require_boinc('db');
  $view = views_get_current_view();
  $boinc_id = $view->args[0];
  $boincuser = lookup_user_id($boinc_id);
  
  // Determine if hosts are associated at all or just hidden
  $output = '';
  if ($boincuser->show_hosts) {
    if ($context == 'active') {
      $output .=  '<h2>' . t('No active computers') . '</h2>';
      $output .=  '<p>' . t('This user has no computers that have been active in the last 30 days.') . '</p>';
    }
    else {
      $output .=  '<h2>' . t('Computers pending') . '</h2>';
      $output .=  '<p>' . t('This user does not yet have any associated computers. Computers will be displayed when they have earned their first credits.') . '</p>';
    }
  }
  else {
    $output .=  '<h2>' . t('Computers hidden') . '</h2>';
    $output .=  '<p>' . t('This user has chosen not to show information about their computers.') . '</p>';
  }
  return $output;
}

/**
  * Determine output for active host list views when no hosts are found.
  */
function views_active_host_list_empty_text() {
  
  // Pull the BOINC user ID from the view arguments to get show_hosts 
  // preference for that user
  require_boinc('db');
  $view = views_get_current_view();
  $boinc_id = $view->args[0];
  $boincuser = lookup_user_id($boinc_id);
  
  // Determine if hosts are associated at all or just hidden
  $output = '';
  if ($boincuser->show_hosts) {
    $output .= '<h2>' . t('No active computers found') . '</h2>';
    $output .=  '<p>' . t('This user does not have any computers that have made contact with @project recently.', array('@project' => PROJECT)) . '</p>';
  } else {
    $output .=  '<h2>' . t('Computers hidden') . '</h2>';
    $output .=  '<p>' . t('This user has chosen not to show information about their computers.') . '</p>';
  }
  return $output;
}
