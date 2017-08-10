<?php
/**
 * @file
 *
 * Provide views data and handlers for taxonomy.module
 */

/**
 * @defgroup views_taxonomy_module taxonomy.module handlers
 *
 * @{
 */

/**
 * Implementation of hook_views_data()
 */
function taxonomy_views_data() {
  $data = array();

  // ----------------------------------------------------------------------
  // vocabulary table

  $data['vocabulary']['table']['group']  = t('Taxonomy');

  $data['vocabulary']['table']['join'] = array(
    // vocabulary links to term_data directly via vid.
    'term_data' => array(
      'left_field' => 'vid',
      'field' => 'vid',
    ),
    // vocabulary links to node through term_data via vid
    'node' => array(
      'left_table' => 'term_data',
      'left_field' => 'vid',
      'field' => 'vid',
    ),
    // vocabulary links to node_revision via term_data
    'node_revision' => array(
      'left_table' => 'term_data',
      'left_field' => 'vid',
      'field' => 'vid',
    ),
  );

  // vocabulary name
  $data['vocabulary']['name'] = array(
    'title' => t('Vocabulary name'), // The item it appears as on the UI,
    'field' => array(
      'help' => t('Name of the vocabulary a term is a member of. This will be the vocabulary that whichever term the "Taxonomy: Term" field is; and can similarly cause duplicates.'),
      'handler' => 'views_handler_field',
      'click sortable' => TRUE,
    ),
  );
  $data['vocabulary']['vid'] = array(
    'title' => t('Vocabulary ID'), // The item it appears as on the UI,
    'help' => t('The taxonomy vocabulary ID'),
    'argument' => array(
      'handler' => 'views_handler_argument_vocabulary_vid',
      'name field' => 'name',
    ),
  );

  // ----------------------------------------------------------------------
  // term_data table

  $data['term_data']['table']['group']  = t('Taxonomy');
  $data['term_data']['table']['base'] = array(
    'field' => 'tid',
    'title' => t('Term'),
    'help' => t('Taxonomy terms are attached to nodes.'),
  );


  // The term data table
  $data['term_data']['table']['join'] = array(
    'node' => array(
      'left_table' => 'term_node',
      'left_field' => 'tid',
      'field' => 'tid',
    ),
    'node_revision' => array(
      'left_table' => 'term_node',
      'left_field' => 'tid',
      'field' => 'tid',
    ),
    // This is provided for many_to_one argument
    'term_node' => array(
      'field' => 'tid',
      'left_field' => 'tid',
    ),
  );

  // tid field
  $data['term_data']['tid'] = array(
    'title' => t('Term ID'),
    'help' => t('The taxonomy term ID'),
    'field' => array(
      'handler' => 'views_handler_field_numeric',
      'click sortable' => TRUE,
    ),
    'sort' => array(
      'handler' => 'views_handler_sort',
    ),
    'argument' => array(
      'handler' => 'views_handler_argument_numeric',
      'name field' => 'name',
      'skip base' => array('node', 'node_revision'),
      'zero is null' => TRUE,
    ),
    'filter' => array(
      'handler' => 'views_handler_filter_term_node_tid',
      'hierarchy table' => 'term_hierarchy',
      'numeric' => TRUE,
      'skip base' => array('node', 'node_revision'),
    ),
  );

  // Term name field
  $data['term_data']['name'] = array(
    'title' => t('Term'),
    'help' => t('Taxonomy terms. Note that using this can cause duplicate nodes to appear in views; you must add filters to reduce the result set.'),
    'field' => array(
      'handler' => 'views_handler_field_taxonomy',
      'click sortable' => TRUE,
    ),
    'sort' => array(
      'handler' => 'views_handler_sort',
    ),
    'filter' => array(
      'handler' => 'views_handler_filter_string',
      'help' => t('Taxonomy term name.'),
    ),
    'argument' => array(
      'handler' => 'views_handler_argument_string',
      'help' => t('Taxonomy term name.'),
      'many to one' => TRUE,
      'empty field name' => t('Uncategorized'),
    ),
  );

  // taxonomy weight
  $data['term_data']['weight'] = array(
    'title' => t('Weight'),
    'help' => t('The term weight field'),
    'field' => array(
      'handler' => 'views_handler_field',
      'click sortable' => TRUE,
    ),
    'sort' => array(
      'handler' => 'views_handler_sort',
    ),
  );

  // Term description
  $data['term_data']['description'] = array(
    'title' => t('Term description'), // The item it appears as on the UI,
    'help' => t('The description associated with a taxonomy term.'),
    'field' => array(
      'field' => 'description', // the real field
      'group' => t('Taxonomy'), // The group it appears in on the UI,
      'handler' => 'views_handler_field_markup',
      'format' => FILTER_FORMAT_DEFAULT,
    ),
  );

  // Term vocabulary
  $data['term_data']['vid'] = array(
    'title' => t('Vocabulary'),
    'help' => t('Filter the results of "Taxonomy: Term" to a particular vocabulary.'),
    'filter' => array(
      'handler' => 'views_handler_filter_vocabulary_vid',
    ),
  );

  // ----------------------------------------------------------------------
  // term_node table

  $data['term_node']['table']['group']  = t('Taxonomy');

  $data['term_node']['table']['join'] = array(
    'term_data' => array(
      // links directly to term_data via tid
      'left_field' => 'tid',
      'field' => 'tid',
    ),
    'node' => array(
      // links directly to node via vid
      'left_field' => 'vid',
      'field' => 'vid',
    ),
    'node_revisions' => array(
      // links directly to node_revisions via vid
      'left_field' => 'vid',
      'field' => 'vid',
    ),
    'term_hierarchy' => array(
      'left_field' => 'tid',
      'field' => 'tid',
    ),
  );

  $data['term_node']['vid'] = array(
    'title' => t('Node'),
    'help' => t('Get all nodes tagged with a term.'),
    'relationship' => array(
      'handler' => 'views_handler_relationship',
      'base' => 'node',
      'base field' => 'vid',
      'label' => t('node'),
    ),
  );

  // tid field
  $data['term_node']['tid'] = array(
    'title' => t('Term ID'),
    'help' => t('The taxonomy term ID'),
    'field' => array(
      'title' => t('All terms'),
      'help' => t('Display all taxonomy terms associated with a node from specified vocabularies.'),
      'handler' => 'views_handler_field_term_node_tid',
      'skip base' => 'term_data',
    ),
    'argument' => array(
      'handler' => 'views_handler_argument_term_node_tid',
      'name table' => 'term_data',
      'name field' => 'name',
      'empty field name' => t('Uncategorized'),
      'numeric' => TRUE,
      'skip base' => 'term_data',
    ),
    'filter' => array(
      'title' => t('Term'),
      'handler' => 'views_handler_filter_term_node_tid',
      'hierarchy table' => 'term_hierarchy',
      'numeric' => TRUE,
      'skip base' => 'term_data',
      'allow empty' => TRUE,
    ),
  );

  // term_relation

  $data['term_relation']['table']['group']  = t('Taxonomy');

  $data['term_relation']['table']['join'] = array(
    'term_data' => array(
      // links directly to term_data via tid
      'left_field' => 'tid',
      'field' => 'tid1',
    ),
    'term_relation' => array(
      // links to self through left.tid1 = right.tid2
      'left_field' => 'tid1',
      'field' => 'tid2',
    ),
    'node' => array(
      'left_table' => 'term_node',
      'left_field' => 'tid',
      'field' => 'tid1',
    ),
    'node_revisions' => array(
      'left_table' => 'term_node',
      'left_field' => 'tid',
      'field' => 'tid1',
    ),
  );

  $data['term_relation']['tid2'] = array(
    'title' => t('Related terms'),
    'help' => t('The related terms of the term. This can produce duplicate entries if there is more than one related term.'),
    'relationship' => array(
      'base' => 'term_data',
      'field' => 'tid2',
      'label' => t('Related term'),
    ),
    'argument' => array(
      'help' => t('A related term of the term.'),
      'handler' => 'views_handler_argument_numeric',
    ),
  );

  // ----------------------------------------------------------------------
  // term_hierarchy table

  $data['term_hierarchy']['table']['group']  = t('Taxonomy');

  $data['term_hierarchy']['table']['join'] = array(
    'term_hierarchy' => array(
      // links to self through left.parent = right.tid (going down in depth)
      'left_field' => 'tid',
      'field' => 'parent',
    ),
    'term_data' => array(
      // links directly to term_data via tid
      'left_field' => 'tid',
      'field' => 'tid',
    ),
    'node' => array(
      // links to node thorugh term_data
      'left_table' => 'term_data',
      'left_field' => 'tid',
      'field' => 'tid',
    ),
    'node_revisions' => array(
      // links to node_revisions thorugh term_data
      'left_table' => 'term_data',
      'left_field' => 'tid',
      'field' => 'tid',
    ),
  );

  $data['term_hierarchy']['parent'] = array(
    'title' => t('Parent term'),
    'help' => t('The parent term of the term. This can produce duplicate entries if you are using a vocabulary that allows multiple parents.'),
    'relationship' => array(
      'base' => 'term_data',
      'field' => 'parent',
      'label' => t('Parent'),
    ),
    'argument' => array(
      'help' => t('The parent term of the term.'),
      'handler' => 'views_handler_argument_numeric',
    ),
  );

  // ----------------------------------------------------------------------
  // term_synonym table

  $data['term_synonym']['table']['group']  = t('Taxonomy');

  $data['term_synonym']['table']['join'] = array(
    'term_data' => array(
      // links directly to term_data via tid
      'left_field' => 'tid',
      'field' => 'tid',
    ),
    'node' => array(
      'left_table' => 'term_node',
      'left_field' => 'tid',
      'field' => 'tid',
    ),
    'node_revisions' => array(
      'left_table' => 'term_node',
      'left_field' => 'tid',
      'field' => 'tid',
    ),
  );

  $data['term_synonym']['name'] = array(
    'title' => t('Term synonym'),
    'help' => t('Term synonyms may be used to find terms by alternate names.'),
    'argument' => array(
      'handler' => 'views_handler_argument_string',
      'many to one' => TRUE,
      'empty field name' => t('Uncategorized'),
    ),
  );
  return $data;
}

/**
 * Implementation of hook_views_data_alter().
 */
function taxonomy_views_data_alter(&$data) {
  $data['node']['term_node_tid'] = array(
    'group' => t('Taxonomy'),
    'title' => t('Terms on node'),
    'help' => t('Relate nodes to taxonomy terms, specifiying which vocabulary or vocabularies to use. This relationship will cause duplicated records if there are multiple terms.'),
    'relationship' => array(
      'handler' => 'views_handler_relationship_node_term_data',
      'label' => t('term'),
      'base' => 'term_data',
    ),
  );

  $data['node']['term_node_tid_depth'] = array(
    'group' => t('Taxonomy'),
    'title' => t('Term ID (with depth)'),
    'help' => t('The depth filter is more complex, so provides fewer options.'),
    'real field' => 'vid',
    'argument' => array(
      'handler' => 'views_handler_argument_term_node_tid_depth',
      'accept depth modifier' => TRUE,
    ),
    'filter' => array(
      'handler' => 'views_handler_filter_term_node_tid_depth',
    ),
  );

  $data['node']['term_node_tid_depth_modifier'] = array(
    'group' => t('Taxonomy'),
    'title' => t('Term ID depth modifier'),
    'help' => t('Allows the "depth" for Taxonomy: Term ID (with depth) to be modified via an additional argument.'),
    'argument' => array(
      'handler' => 'views_handler_argument_term_node_tid_depth_modifier',
    ),
  );
}

/**
 * Implementation of hook_views_handlers() to register all of the basic handlers
 * views uses.
 */
function taxonomy_views_handlers() {
  return array(
    'info' => array(
      'path' => drupal_get_path('module', 'views') . '/modules/taxonomy',
    ),
    'handlers' => array(
      'views_handler_argument_term_node_tid_depth' => array(
        'parent' => 'views_handler_argument',
      ),
      'views_handler_argument_term_node_tid_depth_modifier' => array(
        'parent' => 'views_handler_argument',
      ),
      'views_handler_argument_taxonomy' => array(
        'parent' => 'views_handler_argument',
      ),
      'views_handler_argument_vocabulary_vid' => array(
        'parent' => 'views_handler_argument',
      ),
      'views_handler_argument_term_node_tid' => array(
        'parent' => 'views_handler_argument_many_to_one',
      ),
      'views_handler_field_term_node_tid' => array(
        'parent' => 'views_handler_field_prerender_list',
      ),
      'views_handler_field_taxonomy' => array(
        'parent' => 'views_handler_field',
      ),
      'views_handler_filter_vocabulary_vid' => array(
        'parent' => 'views_handler_filter_in_operator',
      ),
      'views_handler_filter_term_node_tid' => array(
        'parent' => 'views_handler_filter_many_to_one',
      ),
      'views_handler_filter_term_node_tid_depth' => array(
        'parent' => 'views_handler_filter_term_node_tid',
      ),
      'views_handler_relationship_node_term_data' => array(
        'parent' => 'views_handler_relationship',
      ),
    ),
  );
}

/**
 * Implementation of hook_views_plugins
 */
function taxonomy_views_plugins() {
  return array(
    'module' => 'views', // This just tells our themes are elsewhere.
    'argument validator' => array(
      'taxonomy_term' => array(
        'title' => t('Taxonomy term'),
        'handler' => 'views_plugin_argument_validate_taxonomy_term',
        'path' => drupal_get_path('module', 'views') . '/modules/taxonomy',
      ),
    ),
   'argument default' => array(
     'taxonomy_tid' => array(
       'title' => t('Taxonomy Term ID from URL'),
       'handler' => 'views_plugin_argument_default_taxonomy_tid',
       'path' => drupal_get_path('module', 'views') . '/modules/taxonomy',
       'parent' => 'fixed',
     ),
   ),
  );
}

/**
 * Helper function to set a breadcrumb for taxonomy.
 */
function views_taxonomy_set_breadcrumb(&$breadcrumb, &$argument) {
  if (empty($argument->options['set_breadcrumb'])) {
    return;
  }

  $args = $argument->view->args;
  $parents = taxonomy_get_parents_all($argument->argument);
  foreach (array_reverse($parents) as $parent) {
    // Unfortunately parents includes the current argument. Skip.
    if ($parent->tid == $argument->argument) {
      continue;
    }
    if ($argument->options['use_taxonomy_term_path']) {
      $path = taxonomy_term_path($parent);
    }
    else {
      $args[$argument->position] = $parent->tid;
      $path = $argument->view->get_url($args);
    }
    $breadcrumb[$path] = check_plain($parent->name);
  }
}

/**
 * @}
 */
