<?php

/**
 * Implementation of hook_fieldgroup_default_groups().
 */
function boinc_standard_fieldgroup_default_groups() {
  $groups = array();

  // Exported group: group_boinctranslate_opts
  $groups['page-group_boinctranslate_opts'] = array(
    'group_type' => 'standard',
    'type_name' => 'page',
    'group_name' => 'group_boinctranslate_opts',
    'label' => 'boinctranslate node options',
    'settings' => array(
      'form' => array(
        'style' => 'fieldset_collapsed',
        'description' => '',
      ),
      'display' => array(
        'description' => '',
        'teaser' => array(
          'format' => 'fieldset',
          'exclude' => 0,
        ),
        'full' => array(
          'format' => 'fieldset',
          'exclude' => 0,
        ),
        '4' => array(
          'format' => 'fieldset',
          'exclude' => 0,
        ),
        '2' => array(
          'format' => 'fieldset',
          'exclude' => 0,
        ),
        '3' => array(
          'format' => 'fieldset',
          'exclude' => 0,
        ),
        'token' => array(
          'format' => 'fieldset',
          'exclude' => 0,
        ),
        'label' => 'above',
      ),
    ),
    'weight' => '104',
    'fields' => array(
      '0' => 'field_boinctranslate_transifex',
    ),
  );

  // Translatables
  // Included for use with string extractors like potx.
  t('boinctranslate node options');

  return $groups;
}
