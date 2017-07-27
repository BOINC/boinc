<?php
// $Id$

/**
 * Form submission handlers and data processing functions are contained
 * herein to prevent littering of the main module file.
 */

/*  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *
 * Filter settings
 *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  */

/**
 * Creates the form for the boinc translate input filter settings in
 * the admin interface.
 */
function boinctranslate_filter_settings_form($format = -1) {
  // Form
  $form = array();
  $form['boinctranslate_filter'] = array(
    '#type' => 'fieldset',
    '#title' => bts('BOINC translate filter', array(), NULL, 'boinc:adminpage-boinctranslate-inputfilter'),
    '#collapsible' => TRUE,
    '#collapsed' => FALSE,
  );
  $form['boinctranslate_filter']['boinctranslate_filter_help'] = array(
    '#type' => 'item',
    '#title' => t('BOINC translate filter'),
    '#value' => t('When a node is created or edited, the content is split into chunks
using a delimiter: #SPLIT_TOKEN#. Each chunk is hashed and save in the
drupal database- tables: locales_source and locales_target. The
location field in the table holds the context of the chunk:
node:(nid):body::(hash).<br><br>

When the node is viewed, the boinctranslate_filter input filter is
called. It also splits the text into chunks. The delimiters are
removed from the output, but retained in the database content. Then
the chunks are hashed and compared to those found in the database. If
the hashes match, then the translated content is provided.<br><br>

The workflow for an editor/admin is to add the token to the text at
semi-regular intervals. The choice is entirely up to the editor. S/he
can add or remove them as the text grows or shrinks. If no delimiter
is found, the entire content of the node body will be hashed as a
single block.<br><br>

Translators should see the node body split into separate chunks. If
only one chunk changes, then only that one translation needs to be
updated.'),
    );
  $form['boinctranslate_filter']["boinctranslate_filter_debug_$format"] = array(
    '#type' => 'select',
    '#title' => bts('Print debugging info', array(), NULL, 'boinc:adminpage-boinctranslate-inputfilter'),
    '#default_value' => variable_get("boinctranslate_filter_debug_$format", 0),
    '#options' => array(t('Disabled'), t('Enabled')),
    '#description' => bts('Prints additional debugging information. This option should be disabled on production sites.', array(), NULL, 'boinc:adminpage-boinctranslate-inputfilter'),
  );
  return $form;
}