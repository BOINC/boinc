<?php

/**
 * Implementation of hook_fieldgroup_default_groups().
 */
function features_test_fieldgroup_default_groups() {
  $groups = array();

  // Exported group: group_features_test
  $groups['features_test-group_features_test'] = array(
    'group_type' => 'standard',
    'type_name' => 'features_test',
    'group_name' => 'group_features_test',
    'label' => 'Test fieldgroup',
    'settings' => array(
      'form' => array(
        'style' => 'fieldset',
        'description' => '',
      ),
      'display' => array(
        'description' => '',
        'label' => 'above',
        '5' => array(
          'format' => 'fieldset',
          'exclude' => 0,
        ),
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
      ),
    ),
    'weight' => '35',
    'fields' => array(
      '0' => 'field_features_test_child_a',
      '1' => 'field_features_test_child_b',
    ),
  );

  // Translatables
  array(
    t('Test fieldgroup'),
  );

  return $groups;
}
