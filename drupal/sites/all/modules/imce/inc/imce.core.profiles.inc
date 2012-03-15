<?php

/**
 * @file
 * Creates the default configuration profiles.
 */

/**
 * Create core profiles and import settings from 5.x
 */
function imce_install_profiles() {

  //already installed
  if (in_array(1, array_keys(variable_get('imce_profiles', array())))) {
    return FALSE;
  }

  $profiles = array(1 => imce_user1_profile(), 2 => imce_sample_profile());
  $index = 2;
  $role_profile = array();

  //import user-1 settings
  if ($u1p = variable_get('imce_settings_user1', NULL)) {
    $profiles[1]['dimensions'] = $u1p['width'] .'x'. $u1p['height'];
  }

  //role settings
  $user_roles = user_roles();
  //determine weights
  $weights = array();
  $sort = variable_get('imce_settings_rank', array());
  foreach ($sort as $i => $rid) {
    if (isset($user_roles[$rid])) {
      $weights[$rid] = $i-10;
    }
  }

  //import role settings.
  foreach (variable_get('imce_settings_roles', array()) as $rid => $set) {
    if (isset($user_roles[$rid])) {
      $dirs = $thumbs = array();

      //directories
      $set['shared'] = $set['shared'] == '/' ? '.' : $set['shared'];
      $dirs = array(array($set['shared'] == '' ? $set['prefix'] .'%uid' : $set['shared'], $set['subnav'] && $set['subdirs'] == '', 1, $set['upload'], $set['twidth'] && $set['theight'], $set['delete'], $set['resize']));

      //subdirectories
      if ($set['subnav'] && $set['subdirs'] != '') {
        foreach (explode(',', $set['subdirs']) as $subdir) {
          $subdir = trim($subdir);
          if ($subdir != '') {
            $dirs[] = array($dirs[0][0] .'/'. $subdir, 0, 1, $set['upload'], $dirs[0][4], $set['delete'], $set['resize']);
          }
        }
      }

      //thumbnails
      if ($set['twidth'] && $set['theight']) {
        $thumbs = array(array('Thumbnail-1', $set['twidth'] .'x'. $set['theight'], '', ''));
      }

      //extensions
      $ext = 'gif png jpg jpeg';
      $ext .= $set['extensions'] ? ' '. str_replace(array(',', '.', '  '), array(' ', '', ' '), $set['extensions']) : '';

      //file size - quota
      $fsize = $set['nolimit'] ? 0 : round($set['filesize']/1024, 1);
      $quota = $set['nolimit'] ? 0 : round($set['quota']/1024, 1);

      //create profile
      $profile_name = $user_roles[$rid];
      $profiles[$index] = imce_construct_profile($profile_name, 1, $fsize, $quota, 0, $ext, $set['width'] .'x'. $set['height'], 1, $dirs, $thumbs);

      //assign the profile to the role.
      $role_profile[$rid] = array('pid' => $index, 'weight' => $weights[$rid]);
      $index++;
    }
  }

  //delete old variables
  variable_del('imce_settings_roles');
  variable_del('imce_settings_user1');
  variable_del('imce_settings_rank');
  variable_del('imce_settings_tinymce');
  variable_del('imce_settings_fck');

  //set new variables
  variable_set('imce_profiles', $profiles);
  variable_set('imce_roles_profiles', $role_profile);

  return TRUE;
}

/**
 * Construct a profile based on arguments.
 */
function imce_construct_profile($n, $u, $f, $q, $tq, $e, $d, $fn, $ds, $ts) {
  $profile = array('name' => $n, 'usertab' => $u, 'filesize' => $f, 'quota' => $q, 'tuquota' => $tq, 'extensions' => $e, 'dimensions' => $d, 'filenum' => $fn, 'directories' => array(), 'thumbnails' => array());
  foreach ($ds as $d) {
    $profile['directories'][] = array('name' => $d[0], 'subnav' => $d[1], 'browse' => $d[2], 'upload' => $d[3], 'thumb' => $d[4], 'delete' => $d[5], 'resize' => $d[6]);
  }
  foreach ($ts as $t) {
    $profile['thumbnails'][] = array('name' => $t[0], 'dimensions' => $t[1], 'prefix' => $t[2], 'suffix' => $t[3]);
  }
  return $profile;
}

/**
 * User1's profile.
 */
function imce_user1_profile() {
  $profiles = variable_get('imce_profiles', array());
  if (isset($profiles[1])) {
    return $profiles[1];
  }
  return  imce_construct_profile('User-1', 1, 0, 0, 0, '*', '1200x1200', 0, array(array('.', 1, 1, 1, 1, 1, 1)), array(array('Small', '90x90', 'small_', ''), array('Medium', '120x120', 'medium_', ''), array('Large', '180x180', 'large_', '')));
}

/**
 * Default profile.
 */
function imce_sample_profile() {
  return imce_construct_profile('Sample profile', 1, 1, 2, 0, 'gif png jpg jpeg', '800x600', 1, array(array('u%uid', 0, 1, 1, 1, 0, 0)), array(array('Thumb', '90x90', 'thumb_', '')));
}
