<?php

/**
 * @file
 * Include the different setup (administration forms) for the
 * table of contents module.
 */


/**
 * Return the form used for the filter settings.
 *
 * @param $format
 *   The currently selected input format.
 */
function _tableofcontents_settings($format) {
  $form['tableofcontents'] = array(
    '#type' => 'fieldset',
    '#title' => t('Table of Contents'),
    '#collapsible' => TRUE,
    '#description' => t('This filter adds a table of contents referencing headers found in a node. If no headers are defined in the node content, then no table of content is added (whether or not a [toc ...] is defined.)'),
  );


  // On/Off features
  $form['tableofcontents']['tableofcontents_on_off'] = array(
    '#type' => 'fieldset',
    '#title' => t('Table of Contents On/Off features'),
    '#collapsible' => TRUE,
    '#description' => t('Select when and how the Table of Contents is added or removed to your pages.'),
  );

  $form['tableofcontents']['tableofcontents_on_off']['tableofcontents_hide_table_' . $format] = array(
    '#title' => t('Hide the table of contents tags'),
    '#type' => 'checkbox',
    '#default_value' => variable_get('tableofcontents_hide_table_' . $format, FALSE),
    '#description' => 'When checked, all the table of contents are removed without any additional tests.',
  );

  $form['tableofcontents']['tableofcontents_on_off']['tableofcontents_automatic_' . $format] = array(
    '#title' => t('Whether an automatic table of content should be added'),
    '#type' => 'select',
    '#options' => array(
      0 => t('Do not automatically add a table of content'),
      1 => t('If no [toc ...], automatically add one at the top of the page'),
      2 => t('If no [toc ...], automatically add one at the bottom of the page'),
    ),
    '#default_value' => variable_get('tableofcontents_automatic_' . $format, 0),
    '#description' => t('Please, note that there is a minimum limit (see below) that needs to be reached for the table of content to be added.'),
  );

  $form['tableofcontents']['tableofcontents_on_off']['tableofcontents_min_limit_' . $format] = array(
    '#title' => t('Number of headers before an automatic table of content is added'),
    '#type' => 'textfield',
    '#size' => 10,
    '#default_value' => variable_get('tableofcontents_min_limit_' . $format, 5),
    '#description' => t('When no [toc ...] is defined and the automatic feature is turned on, a table of contents will automatically be added only if at least that many headers are present.'),
  );

  $form['tableofcontents']['tableofcontents_on_off']['tableofcontents_remove_teaser_' . $format] = array(
    '#title' => t('Remove Table of Contents tags from teasers'),
    '#type' => 'checkbox',
    '#default_value' => variable_get("tableofcontents_remove_teaser_$format", TRUE),
    '#description' => t('When this setting is changed, each node need to be re-edited to reflect the new setting. Therefore, you should decide at the start what will work for you and not change this flag later. This flag also causes every node with a Table of Contents to have a split teaser (DO NOT CHECK to avoid the auto-teaser addition.) If you have many nodes, the !retease can help to work on teasers automatically. If you also want to use the automatic table of content feature, however, it requires the addition of [toc hide:1] in your teaser, which the !retease does not do. Instead, you may want to consider the [vtoc] option (see your !content_type settings instead.)',
      array(
        '!retease' => l(t('Retease module'), 'http://drupal.org/project/retease',
                        array('attributes' => array('target' => '_blank'))),
        '!content_type' => l(t('Content types'), 'admin/content/types'),
      )
    ),
  );


  // Security flags
  $allow_override = variable_get('tableofcontents_allow_override_' . $format, TRUE);

  $form['tableofcontents']['tableofcontents_security_options'] = array(
    '#type' => 'fieldset',
    '#title' => t('Table of Contents Security Options'),
    '#collapsible' => TRUE,
    '#collapsed' => !$allow_override,
    '#description' => t('The filter can be setup to let you and your users enter parameters directly in the tag. This is considered a security risk on a system allowing customers to edit nodes with the <strong>Table of Contents</strong> capability.'),
  );

  $form['tableofcontents']['tableofcontents_security_options']['tableofcontents_allow_override_' . $format] = array(
    '#title' => t('Allow users to override the settings within the table of contents tag itself'),
    '#type' => 'checkbox',
    '#default_value' => $allow_override,
  );

  $form['tableofcontents']['tableofcontents_security_options']['tableofcontents_safe_title_' . $format] = array(
    '#title' => t('Ensure title is safe (i.e. use check_plain() to avoid XSS attacks.)'),
    '#type' => 'checkbox',
    '#description' => '<span style="color: red;">' . t('WARNING') . ':</span> '
      . t('When this option is unchecked (i.e. "safe" is not turned on) and your users have the right to override the table of contents options, you are letting your users enter full HTML tags in your Table of Contents titles. This means they can include code that may attack a viewer computer. It is recommanded that you never uncheck this flag.'),
    '#default_value' => variable_get('tableofcontents_safe_title_' . $format, TRUE),
  );

  // Table of Contents box
  $form['tableofcontents']['tableofcontents_box'] = array(
    '#type' => 'fieldset',
    '#title' => t('Table of Contents Box'),
    '#collapsible' => TRUE,
    '#description' => t('How the box looks like.'),
  );

  $form['tableofcontents']['tableofcontents_box']['tableofcontents_title_' . $format] = array(
    '#title' => t('Table of Contents Title'),
    '#type' => 'textfield',
    '#size' => 64,
    '#maxlength' => 1024,
    '#default_value' => variable_get('tableofcontents_title_' . $format, t('Table of Contents')),
    '#description' => t('Enter a default title for each Table of Contents. This will be translated for each individual page.'),
  );

  $headers = array(
      1 => "H1",
      2 => "H2",
      3 => "H3",
      4 => "H4",
      5 => "H5",
      6 => "H6",
  );
  $form['tableofcontents']['tableofcontents_box']['tableofcontents_minlevel_' . $format] = array(
    '#title' => t('Minimum heading level'),
    '#type' => 'select',
    '#multiple' => FALSE,
    '#options' => $headers,
    '#default_value' => variable_get('tableofcontents_minlevel_' . $format, 2),
  );

  $form['tableofcontents']['tableofcontents_box']['tableofcontents_maxlevel_' . $format] = array(
    '#title' => t('Maximum heading level'),
    '#type' => 'select',
    '#multiple' => FALSE,
    '#options' => $headers,
    '#default_value' => variable_get('tableofcontents_maxlevel_' . $format, 3),
  );

  $form['tableofcontents']['tableofcontents_box']['tableofcontents_hide_show_' . $format] = array(
    '#title' => t('Include link to hide/show table of contents'),
    '#type' => 'checkbox',
    '#default_value' => variable_get('tableofcontents_hide_show_' . $format, TRUE),
  );

  $form['tableofcontents']['tableofcontents_box']['tableofcontents_collapsed_' . $format] = array(
    '#title' => t('Start with the table of content collapsed'),
    '#type' => 'checkbox',
    '#default_value' => variable_get('tableofcontents_collapsed_' . $format, FALSE),
  );

  // Headers
  $form['tableofcontents']['tableofcontents_header'] = array(
    '#type' => 'fieldset',
    '#title' => t('Table of Contents Header Handling'),
    '#collapsible' => TRUE,
    '#description' => t('Handling of the headers by the filter.'),
  );

  $id_options = array(
    'digits' => 'Strip all digits (0-9)',
    'dashes' => 'Strip all dash characters (-)',
    'periods' => 'Strip all period characters (.)',
    'underscores' => 'Strip all underscore characters (_)',
    'colons' => 'Strip all colon characters (:)',
  );
  $form['tableofcontents']['tableofcontents_header']['tableofcontents_id_stripping_' . $format] = array(
    '#title' => t('Select what is stripped from the header titles'),
    '#type' => 'checkboxes',
    '#options' => $id_options,
    '#default_value' => variable_get('tableofcontents_id_stripping_' . $format, array()),
    '#description' => t('By default, identifiers are valid HTML identifiers. All are not valid JavaScript identifiers and can cause other side effects. You may therefore need to constrain those identifiers some more. Identifiers must start with a letter (A-Z and a-z) and can be followed by letters (A-Z and a-z), dashes (-), digits (0-9), periods (.), colons (:), and underscores (_).<br /><span style="color: red;">WARNING:</span> existing identifier are not being changed by this module. In other words, if it looks like this stripping feature does not work, please, edit your page(s) and check whether you already defined identifiers...'),
  );

  $form['tableofcontents']['tableofcontents_header']['tableofcontents_identifier_introducer_' . $format] = array(
    '#title' => t('Identifier introducer'),
    '#description' => t('Word prepended to the identifier whenever nothing else is available or when you do not use the header title or random schemes.'),
    '#type' => 'textfield',
    '#default_value' => variable_get('tableofcontents_identifier_introducer_' . $format, 'header'),
  );

  $form['tableofcontents']['tableofcontents_header']['tableofcontents_id_separator_' . $format] = array(
    '#title' => t('Identifier and number separator'),
    '#description' => t('The separator used whenever an identifier is appended at the end of the identifier. Also used as the identifier section separator.'),
    '#type' => 'textfield',
    '#default_value' => variable_get('tableofcontents_id_separator_' . $format, '-'),
  );

  $options = array(
    'title'     => 'Use header title',
    'random'    => 'Create a random identifier',
    'increment' => 'Use the "Identifier introducer" and an incrementing number',
    'sections'  => 'Use the "Identifier introducer" and the sections',
    'custom'    => 'Custom: call module_invoke("tableofcontents", $id) -- requires a module or theme that understands this API.',
  );
  $form['tableofcontents']['tableofcontents_header']['tableofcontents_id_generator_' . $format] = array(
    '#title' => t('How to generate missing header identifiers'),
    '#type' => 'radios',
    '#options' => $options,
    '#default_value' => variable_get('tableofcontents_id_generator_' . $format, 'title'),
  );

  $form['tableofcontents']['tableofcontents_header']['tableofcontents_allowed_tags_' . $format] = array(
    '#title' => t('List of tags allowed in table headers'),
    '#type' => 'textfield',
    '#default_value' => variable_get('tableofcontents_allowed_tags_' . $format, TABLEOFCONTENTS_ALLOWED_TAGS),
    '#maxlength' => 2048,
  );


  // Other
  $other = array();

  if (module_exists('upload')) {
    $other['tableofcontents_attachments_' . $format] = array(
      '#title' => t('Show attachments in the table of contents'),
      '#type' => 'checkbox',
      '#default_value' => variable_get('tableofcontents_attachments_' . $format, FALSE),
    );
  }

  if (module_exists('comment')) {
    $other['tableofcontents_comments_' . $format] = array(
      '#title' => t('Add the comments to the table of contents'),
      '#type' => 'checkbox',
      '#default_value' => variable_get('tableofcontents_comments_' . $format, FALSE),
    );

    $other['tableofcontents_comments_level_' . $format] = array(
      '#title' => t('Select header level at which comments start'),
      '#type' => 'select',
      '#options' => $headers,
      '#default_value' => variable_get('tableofcontents_comments_level_' . $format, 3),
    );
  }

  if (module_exists('print')) {
    $other['tableofcontents_show_on_print_pages_' . $format] = array(
      '#title' => t('Show the table of content on Print pages'),
      '#type' => 'checkbox',
      '#default_value' => variable_get('tableofcontents_show_on_print_pages_' . $format, FALSE),
    );
  }

  if (!empty($other)) {
    // create the fieldset only if we got any content for it
    $form['tableofcontents']['tableofcontents_other'] = array(
      '#type' => 'fieldset',
      '#title' => t('Table of Contents Other Modules Support'),
      '#collapsible' => TRUE,
      '#description' => t('The following options are in link with optional modules. The number of options will vary depending on your installation.'),
    );

    foreach ($other as $key => $data) {
      $form['tableofcontents']['tableofcontents_other'][$key] = $data;
    }
  }

  // Back to Top
  $form['tableofcontents']['tableofcontents_back_to_top'] = array(
    '#type' => 'fieldset',
    '#title' => t('Table of Contents Back to Top Links'),
    '#collapsible' => TRUE,
    '#description' => t('Defines the Back to Top link information (i.e. label, location, etc.)'),
  );

  $form['tableofcontents']['tableofcontents_back_to_top']['tableofcontents_back_to_top_' . $format] = array(
    '#title' => t('Back to top label'),
    '#type' => 'textfield',
    '#size' => 35,
    '#default_value' => variable_get('tableofcontents_back_to_top_' . $format, ''),
    '#description' => t('The message to display at the top or bottom of each paragraph with a link back to the table of contents. Leave empty to avoid this link.'),
  );

  $form['tableofcontents']['tableofcontents_back_to_top']['tableofcontents_back_to_top_location_' . $format] = array(
    '#title' => t('Back to top location'),
    '#type' => 'select',
    '#options' => array(
      'header' => t('Under the headers'),
      'bottom' => t('At the bottom of the paragraph'),
    ),
    '#default_value' => variable_get('tableofcontents_back_to_top_location_' . $format, 'bottom'),
  );

  $form['tableofcontents']['tableofcontents_back_to_top']['tableofcontents_back_to_top_minlevel_' . $format] = array(
    '#title' => t('Minimum level where Back to Top appears'),
    '#type' => 'select',
    '#multiple' => FALSE,
    '#options' => $headers,
    '#default_value' => variable_get('tableofcontents_back_to_top_minlevel_' . $format, 2),
  );

  $form['tableofcontents']['tableofcontents_back_to_top']['tableofcontents_back_to_top_maxlevel_' . $format] = array(
    '#title' => t('Maximum level where Back to Top appears'),
    '#type' => 'select',
    '#multiple' => FALSE,
    '#options' => $headers,
    '#default_value' => variable_get('tableofcontents_back_to_top_maxlevel_' . $format, 4),
  );

  $form['tableofcontents']['tableofcontents_back_to_top']['tableofcontents_back_to_top_anchor_' . $format] = array(
    '#title' => t('Back to top anchor'),
    '#type' => 'textfield',
    '#size' => 35,
    '#default_value' => variable_get('tableofcontents_back_to_top_anchor_' . $format, 'toc'),
    '#description' => t('Specify the name of the anchor where you want the Back to top links to go. The default is <em>toc</em> which is the table of contents. Other anchor often used are <em>header, page, screen, wrapper</em>.'),
  );

  $form['tableofcontents']['tableofcontents_back_to_top']['tableofcontents_scroll_back_to_top_' . $format] = array(
    '#title' => t('Scroll back to the table of contents'),
    '#type' => 'checkbox',
    '#default_value' => variable_get('tableofcontents_scroll_back_to_top_' . $format, FALSE),
    '#description' => t('Use the local scroll with jquery to scroll back to the table of contents.'),
  );

  // Numbering
  $form['tableofcontents']['tableofcontents_numbering'] = array(
    '#type' => 'fieldset',
    '#title' => t('Table of Contents Numbering'),
    '#collapsible' => TRUE,
    '#description' => t('Defines the table of contents and headers numbering method.'),
  );

  $form['tableofcontents']['tableofcontents_numbering']['tableofcontents_numbering_' . $format] = array(
    '#title' => t('Numbering method'),
    '#type' => 'select',
    '#options' => array(
      0 => t('No number'),
      4 => t('Browser numbering (<ol> tag)'),
      1 => t('Numbers 1., 2., 3. (like <ol>)'),
      2 => t('Sub-numbers 1., 1.1, 1.2, 2., 2.1, 2.2, etc.'),
      3 => t('Sub-numbers with zero 1.0, 1.1, 1.2, 2.0, 2.1, 2.2, etc.'),
    ),
    '#default_value' => variable_get('tableofcontents_numbering_' . $format, 0),
    '#description' => t('Select the type of numbering.'),
  );

  $form['tableofcontents']['tableofcontents_numbering']['tableofcontents_number_headers_' . $format] = array(
    '#title' => t('Add the number to the headers'),
    '#type' => 'checkbox',
    '#default_value' => variable_get('tableofcontents_number_headers_' . $format, FALSE),
    '#description' => t('Also add the numbers shown in the table of contents to the headers.'),
  );

  $form['tableofcontents']['tableofcontents_numbering']['tableofcontents_number_mode_' . $format] = array(
    '#title' => t('Numbering mode'),
    '#type' => 'select',
    '#options' => array(
      0 => t('Decimal (1, 2, 3...)'),
      1 => t('Roman (I, II, III, ...)'),
      2 => t('Roman (i, ii, iii, ...)'),
      3 => t('Letters (A, B, C, ...)'),
      4 => t('Letters (a, b, c, ...)'),
      5 => t('Hexadecimal (0x00, 0x01, 0x02, ...)'),
    ),
    '#default_value' => variable_get('tableofcontents_number_mode_' . $format, 0),
    '#description' => t('Select decimal, roman, or letters.'),
  );

  $form['tableofcontents']['tableofcontents_numbering']['tableofcontents_number_start_letter_' . $format] = array(
    '#title' => t('Numbering prefix'),
    '#type' => 'textfield',
    '#size' => 10,
    '#default_value' => variable_get('tableofcontents_number_start_letter_' . $format, ''),
    '#description' => t('The characters added at the start. Often used for an opening parenthesis.'),
  );

  $form['tableofcontents']['tableofcontents_numbering']['tableofcontents_number_separator_' . $format] = array(
    '#title' => t('Numbering separator'),
    '#type' => 'textfield',
    '#size' => 10,
    '#default_value' => variable_get('tableofcontents_number_separator_' . $format, '.'),
    '#description' => t('The characters added between each number.'),
  );

  $form['tableofcontents']['tableofcontents_numbering']['tableofcontents_number_end_letter_' . $format] = array(
    '#title' => t('Numbering suffix'),
    '#type' => 'textfield',
    '#size' => 10,
    '#default_value' => variable_get('tableofcontents_number_end_letter_' . $format, '.'),
    '#description' => t('The characters added at the end. Often used for a closing parenthesis.'),
  );

  return $form;
}

/**
 * This function makes sure that the table of content is hidden in the teaser
 * if so requested by the user.
 *
 * @param[in] $node The node which teaser will be tweaked
 */
function _tableofcontents_hide_in_teaser(&$node) {
  // node got a teaser?
  if (!$node->teaser) {
    return;
  }

  // node uses our filter?
  $filters = filter_list_format($node->format);
  if (!isset($filters['tableofcontents/0'])) {
    return;
  }

  // any [toc ...] tag in the teaser?
  $new_teaser = preg_replace('%(<!--\s*tableofcontents([^>]*)-->|\[\[TOC.*?\]\]|(<(div|p)(\s+[^>]*)?'.'>\s*)?\[toc.*?\](\s*</(div|p)>)?)%', '', $node->teaser);
  if ($node->teaser == $new_teaser) {
    // okay, no [toc ...], but maybe we need to hide the table of content?
    if (!variable_get('tableofcontents_automatic_' . $node->format, 0)) {
      return;
    }
  }

  if (strpos($node->body, '<!--break-->') !== FALSE) {
    // We've specified the split, but the teaser is shown in full
    // view. So, we now have to convert it so that the table of
    // content is hidden in the teaser.
    $body = '<!--break-->' . str_replace('<!--break-->', '', $node->body);
    if ($body != $node->body) {
      $node->body = $body;
      drupal_set_message(t("The content you saved contains a table of contents. A separate summary (teaser) has been automatically created without the table of contents at the top. If you make any further changes, be sure to check the summary field to see if your changes apply there as well."));
    }
  }
  else {
    // This is the case where no teaser was specified by the user.
    // We add an explicit split teaser.
    $node->body = '<!--break-->' . $node->body;
    drupal_set_message(t("The summary (teaser) was automatically split from the body as site settings does not allow table of contents in summaries. If you make any further changes, be sure to check the summary field to see if your changes apply there as well."));
  }

  // Remove toc from teasers.
  $node->teaser = $new_teaser . '<div>[toc hidden:1]</div>';
}

/**
 * Add a field so users can ask the TOC to be generated in a
 * node without the use of the filter [toc].
 */
function _tableofcontents_node_form_alter(&$form) {
  $nid = $form['nid']['#value'];
  if ($nid) {
    $node = node_load($nid);
    $toc_automatic = variable_get('tableofcontents_nodetype_toc_automatic_' . $node->type, 0);
  }
  else {
    $node = (object)array(
      'tableofcontents_toc_automatic' => 0,
    );
    $toc_automatic = variable_get('tableofcontents_nodetype_toc_automatic_' . $form['type']['#value'], 0);
  }
  if ($toc_automatic == 99) {
    $options = array(
      0 => t('No table of contents'),
      1 => t('Add the table at the top'),
      2 => t('Add the table at the bottom'),
    );
    $form['tableofcontents_toc_automatic'] = array(
      '#title' => t('Add a table of content to this node'),
      '#type' => 'select',
      '#options' => $options,
      '#default_value' => $node->tableofcontents_toc_automatic,
      '#description' => t('Select where you want the table of content, or no table.<br/><small>Note that the filter automatic feature takes precedence.</small>'),
    );
  }
}

/**
 * Add a field to the node type so one can define the automatic
 * table of content type on a per type basis.
 */
function _tableofcontents_nodetype_form_alter(&$form) {
  $form['tableofcontents'] = array(
    '#type' => 'fieldset',
    '#title' => t('Table of Contents'),
    '#collapsible' => TRUE,
    '#collapsed' => TRUE,
    '#description' => t('Defines whether a table of content is automatically created on those pages and whether the node_view() function is used along with the filter.'),
  );

  $options = array(
     0 => 'No table of contents',
     1 => 'Add the table at the top',
     2 => 'Add the table at the bottom',
    99 => 'Let user choose on node add/edit',
  );
  $form['tableofcontents']['tableofcontents_toc_automatic'] = array(
    '#type' => 'select',
    '#title' => t('Choose where the automatic table of contents is shown'),
    '#options' => $options,
    '#default_value' => variable_get('tableofcontents_nodetype_toc_automatic_' . $form['#node_type']->type, 0),
    '#description' => t('Select whether the table of content should be shown or not on this type of node.'),
  );

  $form['tableofcontents']['tableofcontents_vtoc'] = array(
    '#title' => t('Transform [vtoc] tags'),
    '#type' => 'checkbox',
    '#default_value' => variable_get('tableofcontents_nodetype_toc_vtoc_' . $form['#node_type']->type, FALSE),
    '#description' => t('Select this option to allow [vtoc] (usually instead of [toc], although if you do not use the filter, both [vtoc] and [toc] will be transformed.) The difference is with the teasers. [vtoc] only works with the Node View hook, not the filters. This means we have the $teaser flag available and know exactly whether the teaser is being generated or not. There are several drawbacks, however: (1) no other filter runs after the table of contents changes, although the data should already be sanetize, there could be a security issue; (2) it does not work with Views and many other modules that will get the body content without running the node_prepare() function; (3) this only works within a node, no other type of data whether it supports a filter or not, will not support a table of content.'),
  );

  $form['tableofcontents']['tableofcontents_remove_from_teaser'] = array(
    '#title' => t('Remove table of contents from the teaser'),
    '#type' => 'checkbox',
    '#default_value' => variable_get('tableofcontents_nodetype_toc_remove_from_teaser_' . $form['#node_type']->type, TRUE),
    '#description' => t('This is the main reason to support [vtoc]: a way to remove the table of contents from your teasers without adding a [toc hide=1;] tag in the teaser. However, as specified in the [vtoc] above, there are drawbacks to this method.'),
  );

  $form['#submit'][] = '_tableofcontents_nodetype_form_alter_submit';
}

/**
 * Save the user selection.
 */
function _tableofcontents_nodetype_form_alter_submit($form, &$form_state) {
  variable_set('tableofcontents_nodetype_toc_automatic_' . $form['#node_type']->type, $form_state['values']['tableofcontents_toc_automatic']);
  variable_set('tableofcontents_nodetype_toc_vtoc_' . $form['#node_type']->type, $form_state['values']['tableofcontents_vtoc']);
  variable_set('tableofcontents_nodetype_toc_remove_from_teaser_' . $form['#node_type']->type, $form_state['values']['tableofcontents_remove_from_teaser']);
}

// vim: ts=2 sw=2 et syntax=php
