<?php

/**
 * Implementation of hook_content_default_fields().
 */
function user_profiles_content_default_fields() {
  $fields = array();

  // Exported field: field_background
  $fields['profile-field_background'] = array(
    'field_name' => 'field_background',
    'type_name' => 'profile',
    'display_settings' => array(
      'label' => array(
        'format' => 'above',
        'exclude' => 0,
      ),
      'teaser' => array(
        'format' => 'default',
        'exclude' => 0,
      ),
      'full' => array(
        'format' => 'default',
        'exclude' => 0,
      ),
      '4' => array(
        'format' => 'default',
        'exclude' => 0,
      ),
      '2' => array(
        'format' => 'default',
        'exclude' => 0,
      ),
      '3' => array(
        'format' => 'default',
        'exclude' => 0,
      ),
      'token' => array(
        'format' => 'default',
        'exclude' => 0,
      ),
    ),
    'widget_active' => '1',
    'type' => 'text',
    'required' => '0',
    'multiple' => '0',
    'module' => 'text',
    'active' => '1',
    'text_processing' => '1',
    'max_length' => '',
    'allowed_values' => '',
    'allowed_values_php' => '',
    'widget' => array(
      'rows' => '10',
      'size' => 60,
      'default_value' => array(
        '0' => array(
          'value' => '',
          '_error_element' => 'default_value_widget][field_background][0][value',
        ),
      ),
      'default_value_php' => NULL,
      'label' => bts('Your personal background', array(), NULL, 'boinc:account-profile-edit'),
      'weight' => '3',
      'description' => bts('Tell us about yourself. You could tell us where you\'re from, your age, occupation, hobbies, or anything else about yourself.', array(), NULL, 'boinc:account-profile-edit'),
      'type' => 'text_textarea',
      'module' => 'text',
    ),
  );

  // Exported field: field_country
  $fields['profile-field_country'] = array(
    'field_name' => 'field_country',
    'type_name' => 'profile',
    'display_settings' => array(
      'label' => array(
        'format' => 'above',
        'exclude' => 0,
      ),
      'teaser' => array(
        'format' => 'default',
        'exclude' => 0,
      ),
      'full' => array(
        'format' => 'default',
        'exclude' => 0,
      ),
      '4' => array(
        'format' => 'default',
        'exclude' => 0,
      ),
      '2' => array(
        'format' => 'default',
        'exclude' => 0,
      ),
      '3' => array(
        'format' => 'default',
        'exclude' => 0,
      ),
      'token' => array(
        'format' => 'default',
        'exclude' => 0,
      ),
    ),
    'widget_active' => '1',
    'type' => 'text',
    'required' => '0',
    'multiple' => '0',
    'module' => 'text',
    'active' => '1',
    'text_processing' => '0',
    'max_length' => '',
    'allowed_values' => 'None|None
International|International
Afghanistan|Afghanistan
Albania|Albania
Algeria|Algeria
American Samoa|American Samoa
Andorra|Andorra
Angola|Angola
Anguilla|Anguilla
Antarctica|Antarctica
Antigua and Barbuda|Antigua and Barbuda
Argentina|Argentina
Armenia|Armenia
Aruba|Aruba
Australia|Australia
Austria|Austria
Azerbaijan|Azerbaijan
Bahamas, The|Bahamas, The
Bahrain|Bahrain
Bangladesh|Bangladesh
Barbados|Barbados
Belarus|Belarus
Belgium|Belgium
Belize|Belize
Benin|Benin
Bermuda|Bermuda
Bhutan|Bhutan
Bolivia|Bolivia
Bosnia and Herzegovina|Bosnia and Herzegovina
Botswana|Botswana
Brazil|Brazil
British Virgin Islands|British Virgin Islands
Brunei|Brunei
Bulgaria|Bulgaria
Burkina Faso|Burkina Faso
Burma|Burma
Burundi|Burundi
Cambodia|Cambodia
Cameroon|Cameroon
Canada|Canada
Cape Verde|Cape Verde
Cayman Islands|Cayman Islands
Central African Republic|Central African Republic
Chad|Chad
Channel Islands|Channel Islands
Chile|Chile
China|China
Colombia|Colombia
Comoros|Comoros
Congo, Democratic Republic of the|Congo, Democratic Republic of the
Congo, Republic of the|Congo, Republic of the
Cook Islands|Cook Islands
Costa Rica|Costa Rica
Cote d\'Ivoire|Cote d\'Ivoire
Croatia|Croatia
Cuba|Cuba
Cyprus|Cyprus
Czech Republic|Czech Republic
Denmark|Denmark
Djibouti|Djibouti
Dominica|Dominica
Dominican Republic|Dominican Republic
East Timor|East Timor
Ecuador|Ecuador
Egypt|Egypt
El Salvador|El Salvador
Equatorial Guinea|Equatorial Guinea
Eritrea|Eritrea
Estonia|Estonia
Ethiopia|Ethiopia
Falkland Islands|Falkland Islands
Faroe Islands|Faroe Islands
Fiji|Fiji
Finland|Finland
France|France
French Guiana|French Guiana
French Polynesia|French Polynesia
Gabon|Gabon
Gambia, The|Gambia, The
Gaza Strip|Gaza Strip
Georgia|Georgia
Germany|Germany
Ghana|Ghana
Gibraltar|Gibraltar
Greece|Greece
Greenland|Greenland
Grenada|Grenada
Guadeloupe|Guadeloupe
Guam|Guam
Guatemala|Guatemala
Guinea|Guinea
Guinea-Bissau|Guinea-Bissau
Guyana|Guyana
Haiti|Haiti
Holy See (Vatican City)|Holy See (Vatican City)
Honduras|Honduras
Hong Kong|Hong Kong
Hungary|Hungary
Iceland|Iceland
India|India
Indonesia|Indonesia
Iran|Iran
Iraq|Iraq
Ireland|Ireland
Isle of Man|Isle of Man
Israel|Israel
Italy|Italy
Jamaica|Jamaica
Japan|Japan
Jordan|Jordan
Kazakhstan|Kazakhstan
Korea, North|Korea, North
Korea, South|Korea, South
Kenya|Kenya
Kiribati|Kiribati
Kuwait|Kuwait
Kyrgyzstan|Kyrgyzstan
Laos|Laos
Latvia|Latvia
Lebanon|Lebanon
Lesotho|Lesotho
Liberia|Liberia
Libya|Libya
Liechtenstein|Liechtenstein
Lithuania|Lithuania
Luxembourg|Luxembourg
Macau|Macau
Macedonia|Macedonia
Madagascar|Madagascar
Malawi|Malawi
Malaysia|Malaysia
Maldives|Maldives
Mali|Mali
Malta|Malta
Marshall Islands|Marshall Islands
Martinique|Martinique
Mauritania|Mauritania
Mauritius|Mauritius
Mexico|Mexico
Micronesia|Micronesia
Moldova|Moldova
Monaco|Monaco
Mongolia|Mongolia
Montenegro|Montenegro
Montserrat|Montserrat
Morocco|Morocco
Mozambique|Mozambique
Namibia|Namibia
Nauru|Nauru
Nepal|Nepal
Netherlands|Netherlands
Netherlands Antilles|Netherlands Antilles
New Caledonia|New Caledonia
New Zealand|New Zealand
Nicaragua|Nicaragua
Niger|Niger
Nigeria|Nigeria
Niue|Niue
Northern Mariana Islands|Northern Mariana Islands
Norway|Norway
Oman|Oman
Pakistan|Pakistan
Palau|Palau
Panama|Panama
Papua New Guinea|Papua New Guinea
Paraguay|Paraguay
Peru|Peru
Philippines|Philippines
Pitcairn Islands|Pitcairn Islands
Poland|Poland
Portugal|Portugal
Puerto Rico|Puerto Rico
Qatar|Qatar
Reunion|Reunion
Romania|Romania
Russia|Russia
Rwanda|Rwanda
Saint Kitts and Nevis|Saint Kitts and Nevis
Saint Lucia|Saint Lucia
Saint Pierre and Miquelon|Saint Pierre and Miquelon
Saint Vincent and the Grenadines|Saint Vincent and the Grenadines
Samoa|Samoa
San Marino|San Marino
Sao Tome and Principe|Sao Tome and Principe
Saudi Arabia|Saudi Arabia
Senegal|Senegal
Serbia|Serbia
Seychelles|Seychelles
Sierra Leone|Sierra Leone
Singapore|Singapore
Slovakia|Slovakia
Slovenia|Slovenia
Solomon Islands|Solomon Islands
Somalia|Somalia
South Africa|South Africa
Spain|Spain
Sri Lanka|Sri Lanka
Sudan|Sudan
Suriname|Suriname
Swaziland|Swaziland
Sweden|Sweden
Switzerland|Switzerland
Syria|Syria
Taiwan|Taiwan
Tajikistan|Tajikistan
Tanzania|Tanzania
Thailand|Thailand
Togo|Togo
Tokelau|Tokelau
Tonga|Tonga
Trinidad and Tobago|Trinidad and Tobago
Tunisia|Tunisia
Turkey|Turkey
Turkmenistan|Turkmenistan
Turks and Caicos Islands|Turks and Caicos Islands
Tuvalu|Tuvalu
Uganda|Uganda
Ukraine|Ukraine
United Arab Emirates|United Arab Emirates
United Kingdom|United Kingdom
United States|United States
Uruguay|Uruguay
Uzbekistan|Uzbekistan
Vanuatu|Vanuatu
Venezuela|Venezuela
Vietnam|Vietnam
Virgin Islands|Virgin Islands
Wallis and Futuna|Wallis and Futuna
West Bank|West Bank
Western Sahara|Western Sahara
Yemen|Yemen
Zambia|Zambia
Zimbabwe|Zimbabwe',
    'allowed_values_php' => 'return boinccore_get_country_list();',
    'widget' => array(
      'default_value' => array(
        '0' => array(
          'value' => '',
        ),
      ),
      'default_value_php' => NULL,
      'label' => 'Country',
      'weight' => '5',
      'description' => '',
      'type' => 'optionwidgets_select',
      'module' => 'optionwidgets',
    ),
  );

  // Exported field: field_image
  $fields['profile-field_image'] = array(
    'field_name' => 'field_image',
    'type_name' => 'profile',
    'display_settings' => array(
      'label' => array(
        'format' => 'above',
        'exclude' => 0,
      ),
      'teaser' => array(
        'format' => 'image_plain',
        'exclude' => 0,
      ),
      'full' => array(
        'format' => 'image_plain',
        'exclude' => 0,
      ),
      '4' => array(
        'format' => 'image_plain',
        'exclude' => 0,
      ),
      '2' => array(
        'format' => 'image_plain',
        'exclude' => 0,
      ),
      '3' => array(
        'format' => 'image_plain',
        'exclude' => 0,
      ),
      'token' => array(
        'format' => 'image_plain',
        'exclude' => 0,
      ),
    ),
    'widget_active' => '1',
    'type' => 'filefield',
    'required' => '0',
    'multiple' => '0',
    'module' => 'filefield',
    'active' => '1',
    'list_field' => '0',
    'list_default' => 1,
    'description_field' => '0',
    'widget' => array(
      'file_extensions' => 'png gif jpg jpeg',
      'file_path' => 'profiles',
      'progress_indicator' => 'bar',
      'max_filesize_per_file' => '512K',
      'max_filesize_per_node' => '',
      'max_resolution' => '300x300',
      'min_resolution' => '50x50',
      'alt' => '',
      'custom_alt' => 0,
      'title' => '',
      'custom_title' => 0,
      'title_type' => 'textfield',
      'default_image' => array(
        'filename' => 'default-avatar.png',
        'filepath' => 'sites/default/files/imagefield_default_images/default-avatar.png',
        'filemime' => 'image/png',
        'source' => 'default_image_upload',
        'destination' => 'sites/default/files/imagefield_default_images/default-avatar.png',
        'filesize' => 9733,
        'uid' => '1',
        'status' => 1,
        'timestamp' => 1347855344,
        'fid' => '1',
      ),
      'use_default_image' => 1,
      'label' => 'Avatar',
      'weight' => '7',
      'description' => '',
      'type' => 'imagefield_widget',
      'module' => 'imagefield',
    ),
  );

  // Exported field: field_opinions
  $fields['profile-field_opinions'] = array(
    'field_name' => 'field_opinions',
    'type_name' => 'profile',
    'display_settings' => array(
      'label' => array(
        'format' => 'above',
        'exclude' => 0,
      ),
      'teaser' => array(
        'format' => 'default',
        'exclude' => 0,
      ),
      'full' => array(
        'format' => 'default',
        'exclude' => 0,
      ),
      '4' => array(
        'format' => 'default',
        'exclude' => 0,
      ),
      '2' => array(
        'format' => 'default',
        'exclude' => 0,
      ),
      '3' => array(
        'format' => 'default',
        'exclude' => 0,
      ),
      'token' => array(
        'format' => 'default',
        'exclude' => 0,
      ),
    ),
    'widget_active' => '1',
    'type' => 'text',
    'required' => '0',
    'multiple' => '0',
    'module' => 'text',
    'active' => '1',
    'text_processing' => '1',
    'max_length' => '',
    'allowed_values' => '',
    'allowed_values_php' => '',
    'widget' => array(
      'rows' => '10',
      'size' => 60,
      'default_value' => array(
        '0' => array(
          'value' => '',
          '_error_element' => 'default_value_widget][field_opinions][0][value',
        ),
      ),
      'default_value_php' => NULL,
      'label' => bts('Your opinions about this project', array(), NULL, 'boinc:account-profile-edit'),
      'weight' => '4',
      'description' => bts('Tell us your thoughts about @this_project
   1. Why do you run @this_project?
   2. What are your views about the project?
   3. Any suggestions?',
      array('@this_project' => variable_get('site_name', 'Drupal-BOINC')), NULL, 'boinc:account-profile-edit'),
      'type' => 'text_textarea',
      'module' => 'text',
    ),
  );

  // Exported field: field_profile_image
  $fields['profile-field_profile_image'] = array(
    'field_name' => 'field_profile_image',
    'type_name' => 'profile',
    'display_settings' => array(
      'label' => array(
        'format' => 'above',
        'exclude' => 0,
      ),
      'teaser' => array(
        'format' => 'image_plain',
        'exclude' => 0,
      ),
      'full' => array(
        'format' => 'image_plain',
        'exclude' => 0,
      ),
      '4' => array(
        'format' => 'image_plain',
        'exclude' => 0,
      ),
      '2' => array(
        'format' => 'image_plain',
        'exclude' => 0,
      ),
      '3' => array(
        'format' => 'image_plain',
        'exclude' => 0,
      ),
      'token' => array(
        'format' => 'image_plain',
        'exclude' => 0,
      ),
    ),
    'widget_active' => '1',
    'type' => 'filefield',
    'required' => '0',
    'multiple' => '0',
    'module' => 'filefield',
    'active' => '1',
    'list_field' => '0',
    'list_default' => 1,
    'description_field' => '0',
    'widget' => array(
      'file_extensions' => 'png gif jpg jpeg',
      'file_path' => 'profiles',
      'progress_indicator' => 'bar',
      'max_filesize_per_file' => '512K',
      'max_filesize_per_node' => '',
      'max_resolution' => '640x480',
      'min_resolution' => '100x100',
      'alt' => '',
      'custom_alt' => 0,
      'title' => '',
      'custom_title' => 0,
      'title_type' => 'textfield',
      'default_image' => array(
        'filename' => 'default-user.png',
        'filepath' => 'sites/default/files/imagefield_default_images/default-user.png',
        'filemime' => 'image/png',
        'source' => 'default_image_upload',
        'destination' => 'sites/default/files/imagefield_default_images/default-user.png',
        'filesize' => 9774,
        'uid' => '1',
        'status' => 1,
        'timestamp' => 1362770990,
        'fid' => '5',
      ),
      'use_default_image' => 1,
      'label' => 'Profile image',
      'weight' => '8',
      'description' => '',
      'type' => 'imagefield_widget',
      'module' => 'imagefield',
    ),
  );

  // Exported field: field_url
  $fields['profile-field_url'] = array(
    'field_name' => 'field_url',
    'type_name' => 'profile',
    'display_settings' => array(
      'label' => array(
        'format' => 'above',
        'exclude' => 0,
      ),
      'teaser' => array(
        'format' => 'default',
        'exclude' => 0,
      ),
      'full' => array(
        'format' => 'default',
        'exclude' => 0,
      ),
      '4' => array(
        'format' => 'default',
        'exclude' => 0,
      ),
      '2' => array(
        'format' => 'default',
        'exclude' => 0,
      ),
      '3' => array(
        'format' => 'default',
        'exclude' => 0,
      ),
      'token' => array(
        'format' => 'default',
        'exclude' => 0,
      ),
    ),
    'widget_active' => '1',
    'type' => 'text',
    'required' => '0',
    'multiple' => '0',
    'module' => 'text',
    'active' => '1',
    'text_processing' => '0',
    'max_length' => '',
    'allowed_values' => '',
    'allowed_values_php' => '',
    'widget' => array(
      'rows' => 5,
      'size' => '42',
      'default_value' => array(
        '0' => array(
          'value' => '',
          '_error_element' => 'default_value_widget][field_url][0][value',
        ),
      ),
      'default_value_php' => NULL,
      'label' => 'Website URL',
      'weight' => '9',
      'description' => '',
      'type' => 'text_textfield',
      'module' => 'text',
    ),
  );

  // Exported field: field_zip
  $fields['profile-field_zip'] = array(
    'field_name' => 'field_zip',
    'type_name' => 'profile',
    'display_settings' => array(
      'label' => array(
        'format' => 'above',
        'exclude' => 0,
      ),
      'teaser' => array(
        'format' => 'default',
        'exclude' => 0,
      ),
      'full' => array(
        'format' => 'default',
        'exclude' => 0,
      ),
      '4' => array(
        'format' => 'default',
        'exclude' => 0,
      ),
      '2' => array(
        'format' => 'default',
        'exclude' => 0,
      ),
      '3' => array(
        'format' => 'default',
        'exclude' => 0,
      ),
      'token' => array(
        'format' => 'default',
        'exclude' => 0,
      ),
    ),
    'widget_active' => '1',
    'type' => 'text',
    'required' => '0',
    'multiple' => '0',
    'module' => 'text',
    'active' => '1',
    'text_processing' => '0',
    'max_length' => '10',
    'allowed_values' => '',
    'allowed_values_php' => '',
    'widget' => array(
      'rows' => 5,
      'size' => '10',
      'default_value' => array(
        '0' => array(
          'value' => '',
          '_error_element' => 'default_value_widget][field_zip][0][value',
        ),
      ),
      'default_value_php' => NULL,
      'label' => bts('Postal code', array(), NULL, 'boinc:account-profile-edit'),
      'weight' => '6',
      'description' => '',
      'type' => 'text_textfield',
      'module' => 'text',
    ),
  );

  // Translatables
  // Use bts() function to translate additional strings.
  bts('Avatar', array(), NULL, 'boinc:account-profile-edit');
  bts('Country', array(), NULL, 'boinc:country-of-origin');
  bts('Profile image', array(), NULL, 'boinc:account-profile-edit');
  bts('Website URL', array(), NULL, 'boinc:account-profile-edit');
  bts('Your opinions about this project', array(), NULL, 'boinc:account-profile-edit');
  bts('Your personal background', array(), NULL, 'boinc:account-profile-edit');
  bts('Postal code', array(), NULL, 'boinc:account-profile-edit');

  return $fields;
}
