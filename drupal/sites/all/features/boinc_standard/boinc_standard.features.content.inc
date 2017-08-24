<?php

/**
 * Implementation of hook_content_default_fields().
 */
function boinc_standard_content_default_fields() {
  $fields = array();

  // Exported field: field_boinctranslate_transifex
  $fields['page-field_boinctranslate_transifex'] = array(
    'field_name' => 'field_boinctranslate_transifex',
    'type_name' => 'page',
    'display_settings' => array(
      'weight' => '101',
      'parent' => '',
      'label' => array(
        'format' => 'hidden',
      ),
      'teaser' => array(
        'format' => 'default',
        'exclude' => 0,
      ),
      'full' => array(
        'format' => 'hidden',
        'exclude' => 0,
      ),
      'token' => array(
        'format' => 'hidden',
        'exclude' => 0,
      ),
      '2' => array(
        'format' => 'hidden',
        'exclude' => 0,
      ),
      '3' => array(
        'format' => 'hidden',
        'exclude' => 0,
      ),
      '4' => array(
        'format' => 'hidden',
        'exclude' => 0,
      ),
    ),
    'widget_active' => '1',
    'type' => 'text',
    'required' => '1',
    'multiple' => '0',
    'module' => 'text',
    'active' => '1',
    'text_processing' => '0',
    'max_length' => '',
    'allowed_values' => '0|Do not translate
1|Export for translation',
    'allowed_values_php' => '',
    'widget' => array(
      'default_value' => array(
        '0' => array(
          'value' => 1,
        ),
      ),
      'default_value_php' => NULL,
      'label' => 'Export to Transifex',
      'weight' => '101',
      'description' => 'If selected, this node will be exported to Transifex for translation. Otherwise the node will only be available in the default language.',
      'type' => 'optionwidgets_onoff',
      'module' => 'optionwidgets',
    ),
  );

  // Translatables
  // Included for use with string extractors like potx.
  t('boinctranslate export to transifex');

  return $fields;
}
