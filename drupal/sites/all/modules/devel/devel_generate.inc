<?php
// $Id: devel_generate.inc,v 1.14.2.24 2010/10/26 05:11:54 weitzman Exp $
// If not in 'safe mode', increase the maximum execution time:
if (!ini_get('safe_mode')) {
  set_time_limit(240);
}

/**
 * Generate some random users.
 *
 * @param $num
 *  Number of users to generate.
 * @param $kill
 *  Boolean that indicates if existing users should be removed first.
 * @param $age
 *  The max age of each randomly-generated user, in seconds.
 */
function devel_create_users($num, $kill, $age = 0) {
  $url = parse_url($GLOBALS['base_url']);
  if ($kill) {
    db_query('DELETE FROM {users} WHERE uid > 1');
    drupal_set_message(t('Users deleted.'));
  }
  if ($num > 0) {
    $names = array();
    while (count($names) < $num) {
      $length = mt_rand(6, 12);
      $name = devel_generate_word($length);
      $names[$name] = '';
    }
    foreach ($names as $name => $value) {
      $pass = md5(user_password());
      $mail = $name .'@'. $url['host'];
      $now = time();
      $created = ($age ? mt_rand($now - $age, $now) : $now - 1);
      $account = user_save(NULL, array('name' => $name, 'pass' => $pass, 'mail' => $mail, 'status' =>'1', 'created' => $created, 'access' => $now, 'devel_generate' => TRUE));
    }
  drupal_set_message(t('!num_users created.', array('!num_users' => format_plural($num, '1 user', '@count users'))));
  }
}

/**
 * The main API function for creating content. 
 * 
 * See devel_generate_content_form() for the supported keys in $form_state['values'].
 * Other modules may participate by form_alter() on that form and then handling their data during hook_nodeapi('pre_save') or in own submit handler for the form.
 *
 * @param string $form_state 
 * @return void
 */
function devel_generate_content($form_state) {
  if ($form_state['values']['kill_content']) {
    devel_generate_content_kill($form_state['values']);
  }

  if (count($form_state['values']['node_types'])) {
    // Generate nodes.
    devel_generate_content_pre_node($form_state['values']);
    for ($i = 1; $i <= $form_state['values']['num_nodes']; $i ++) {
      devel_generate_content_add_node($form_state['values']);
    }
  }

  drupal_set_message(format_plural($form_state['values']['num_nodes'], '1 node created.', '@count nodes created'));
}

function devel_generate_add_comments($node, $users, $max_comments, $title_length = 8) {
  // Insert new data:
  $num_comments = mt_rand(1, $max_comments);
  for ($i = 1; $i <= $num_comments; $i++) {
    $comment->nid = $node->nid;
    $comment->cid = NULL;
    $comment->format = FILTER_FORMAT_DEFAULT;
    $comment->name = 'devel generate';
    $comment->mail = 'devel_generate@example.com';
    $comment->timestamp = mt_rand($node->created, time());

    switch ($i % 3) {
      case 1:
        $comment->pid = db_result(db_query_range("SELECT cid FROM {comments} WHERE pid = 0 AND nid = %d ORDER BY RAND()", $comment->nid, 0, 1));
        break;
      case 2:
        $comment->pid = db_result(db_query("SELECT cid FROM {comments} WHERE pid > 0 AND nid = %d ORDER BY RAND()", $comment->nid, 0, 1));
        break;
      default:
        $comment->pid = 0;
    }

    $comment->subject = devel_create_greeking(mt_rand(1, $title_length), TRUE);
    $comment->comment = devel_create_content();
    $comment->uid = $users[array_rand($users)];

    // this is slow but gets the threading right.
    comment_save((array)$comment);

  }
}

function devel_generate_vocabs($records, $maxlength = 12, $types = array('story', 'blog', 'forum', 'page')) {
  $vocs = array();

  // Insert new data:
  for ($i = 1; $i <= $records; $i++) {
    $voc = array();
    $voc['name'] = devel_generate_word(rand(2, $maxlength));
    $voc['description'] = "description of ". $voc['name'];
    $voc['nodes'] = array_flip(array($types[array_rand($types)]));
    foreach ($voc['nodes'] as $key => $value) {
      $voc['nodes'][$key] = $key;
    }
    $voc['multiple'] = 1;
    $voc['required'] = 0;
    $voc['relations'] = 1;
    $voc['hierarchy'] = 1;
    $voc['weight'] = mt_rand(0,10);

    taxonomy_save_vocabulary($voc);
    $vocs[] = $voc['name'];
  }
  return $vocs;
}

function devel_generate_terms($records, $vocs, $maxlength = 12) {
  $terms = array();

  // Insert new data:
  for ($i = 1; $i <= $records; $i++) {
    switch ($i % 2) {
      case 1:
        $term['vid'] = $vocs[array_rand($vocs)];
        // dont set a parent. handled by taxonomy_save_term()
        // $term->parent = 0;
        break;
      case 2:
      default:
        $parent = db_fetch_object(db_query_range("SELECT t.tid, v.vid FROM {term_data} t INNER JOIN {vocabulary} v ON t.vid = v.vid ORDER BY RAND()", 0, 1));
        $term['parent'] = array($parent->tid);
        $term['vid'] = $parent->vid;
        break;
    }

    $term['name'] = devel_generate_word(mt_rand(2, $maxlength));
    $term['description'] = "description of ". $term['name'];
    $term['weight'] = mt_rand(0,10);

    $status = taxonomy_save_term($term);
    $output = NULL;

    if ($status) {
      $terms[] = $term['name'];
    }

    unset($term);
  }
  return $terms;
}

function devel_generate_get_vocabs() {
  $vocs = array();
  $result = db_query("SELECT vid FROM {vocabulary}");
  while($voc = db_fetch_object($result)){
    $vocs[] = $voc->vid;
  }
  return $vocs;
}

function devel_generate_taxonomy_data($num_vocab, $num_terms, $title_length, $kill) {

  if ($kill) {
    db_query("DELETE FROM {term_data}");
    db_query("DELETE FROM {term_node}");
    db_query("DELETE FROM {term_hierarchy}");
    db_query("DELETE FROM {term_relation}");
    db_query("DELETE FROM {term_synonym}");
    db_query("DELETE FROM {vocabulary}");
    db_query("DELETE FROM {vocabulary_node_types}");
    switch ($GLOBALS['db_type']) {
      case 'mysql':
      case 'mysqli':
        db_query("ALTER TABLE {vocabulary} AUTO_INCREMENT = 1");
        db_query("ALTER TABLE {term_data} AUTO_INCREMENT = 1");
        break;
      case 'pgsql':
        db_query("SELECT setval('{vocabulary}_vid_seq', 1, false)");
        db_query("SELECT setval('{term_data}_tid_seq', 1, false)");
        break;
    }
    drupal_set_message(t('Deleted taxonomy.'));
  }

  $new_vocs = devel_generate_vocabs($num_vocab, $title_length);
  if (!empty($new_vocs)) {
    drupal_set_message(t('Created the following new vocabularies: !vocs', array('!vocs' => theme('item_list', $new_vocs))));
  }
  $vocs = devel_generate_get_vocabs();
  $new_terms = devel_generate_terms($num_terms, $vocs, $title_length);
  if (!empty($new_terms)) {
    drupal_set_message(t('Created the following new terms: !terms', array('!terms' => theme('item_list', $new_terms))));
  }
}

function devel_generate_word($length){
  srand((double)microtime()*1000000);

  $vowels = array("a", "e", "i", "o", "u");
  $cons = array("b", "c", "d", "g", "h", "j", "k", "l", "m", "n", "p", "r", "s", "t", "u", "v", "w", "tr",
  "cr", "br", "fr", "th", "dr", "ch", "ph", "wr", "st", "sp", "sw", "pr", "sl", "cl", "sh");

  $num_vowels = count($vowels);
  $num_cons = count($cons);
  $word = '';

  while(strlen($word) < $length){
    $word .= $cons[mt_rand(0, $num_cons - 1)] . $vowels[mt_rand(0, $num_vowels - 1)];
  }

  return substr($word, 0, $length);
}

function devel_create_content($type = NULL) {
  $nparas = mt_rand(1,12);
  $type = empty($type) ? mt_rand(0,3) : $type;

  $output = "";
  switch($type % 3) {
    case 1: // html
      for ($i = 1; $i <= $nparas; $i++) {
        $output .= devel_create_para(mt_rand(10, 60), 1);
      }
      break;

    case 2: // brs only
      for ($i = 1; $i <= $nparas; $i++) {
        $output .= devel_create_para(mt_rand(10, 60), 2);
      }
      break;

    default: // plain text
      for ($i = 1; $i <= $nparas; $i++) {
        $output .= devel_create_para(mt_rand(10,60)) ."\n\n";
      }
  }

  return $output;
}

// Return an $options array of content types.
function devel_generate_content_types() {
  $options = array();

  if (module_exists('content')) {
    $types = content_types();
    foreach ($types as $type) {
      $warn = '';
      if (count($type['fields'])) {
        $warn = t('. This type contains CCK fields which will only be populated by fields that implement the content_generate hook.');
      }
      $options[$type['type']] = t($type['name']). $warn;
    }
  }
  else {
    $types = node_get_types();
    foreach ($types as $type) {
      $options[$type->type] = t($type->name);
    }
  }
  
  // we cannot currently generate valid polls.
  unset($options['poll']);
  
  return $options;
}


function devel_create_para($words, $type = 0) {
  $output = "";
  switch ($type) {
    case 1:
      $output .= "<p>";
      $output .= devel_create_greeking($words);
      $output = trim($output) ."</p>";
      break;

    case 2:
      $output .= devel_create_greeking($words);
      $output = trim($output) ."<br />";
      break;

    default:
      $output .= devel_create_greeking($words);
      $output = trim($output);
  }
  return $output;
}

function devel_create_greeking($words, $title = FALSE) {
  $dictionary = array("abbas", "abdo", "abico", "abigo", "abluo", "accumsan",
    "acsi", "ad", "adipiscing", "aliquam", "aliquip", "amet", "antehabeo",
    "appellatio", "aptent", "at", "augue", "autem", "bene", "blandit",
    "brevitas", "caecus", "camur", "capto", "causa", "cogo", "comis",
    "commodo", "commoveo", "consectetuer", "consequat", "conventio", "cui",
    "damnum", "decet", "defui", "diam", "dignissim", "distineo", "dolor",
    "dolore", "dolus", "duis", "ea", "eligo", "elit", "enim", "erat",
    "eros", "esca", "esse", "et", "eu", "euismod", "eum", "ex", "exerci",
    "exputo", "facilisi", "facilisis", "fere", "feugiat", "gemino",
    "genitus", "gilvus", "gravis", "haero", "hendrerit", "hos", "huic",
    "humo", "iaceo", "ibidem", "ideo", "ille", "illum", "immitto",
    "importunus", "imputo", "in", "incassum", "inhibeo", "interdico",
    "iriure", "iusto", "iustum", "jugis", "jumentum", "jus", "laoreet",
    "lenis", "letalis", "lobortis", "loquor", "lucidus", "luctus", "ludus",
    "luptatum", "macto", "magna", "mauris", "melior", "metuo", "meus",
    "minim", "modo", "molior", "mos", "natu", "neo", "neque", "nibh",
    "nimis", "nisl", "nobis", "nostrud", "nulla", "nunc", "nutus", "obruo",
    "occuro", "odio", "olim", "oppeto", "os", "pagus", "pala", "paratus",
    "patria", "paulatim", "pecus", "persto", "pertineo", "plaga", "pneum",
    "populus", "praemitto", "praesent", "premo", "probo", "proprius",
    "quadrum", "quae", "qui", "quia", "quibus", "quidem", "quidne", "quis",
    "ratis", "refero", "refoveo", "roto", "rusticus", "saepius",
    "sagaciter", "saluto", "scisco", "secundum", "sed", "si", "similis",
    "singularis", "sino", "sit", "sudo", "suscipere", "suscipit", "tamen",
    "tation", "te", "tego", "tincidunt", "torqueo", "tum", "turpis",
    "typicus", "ulciscor", "ullamcorper", "usitas", "ut", "utinam",
    "utrum", "uxor", "valde", "valetudo", "validus", "vel", "velit",
    "veniam", "venio", "vereor", "vero", "verto", "vicis", "vindico",
    "virtus", "voco", "volutpat", "vulpes", "vulputate", "wisi", "ymo",
    "zelus");

  $greeking = "";

  if (!$title) {
	  while ($words > 0) {
	    $sentence_length = mt_rand(3,10);

	    $greeking .= ucfirst($dictionary[array_rand($dictionary)]);
	    for ($i = 1; $i < $sentence_length; $i++) {
	      $greeking .= " " . $dictionary[array_rand($dictionary)];
	    }

	    $greeking .= ". ";
	    $words -= $sentence_length;
	  }
  }
  else {
  	// use different method for titles
  	$title_length = $words;
  	$array = array();
  	for ($i = 0; $i < $words; $i++) {
  		$array[] = $dictionary[array_rand($dictionary)];
  	}
  	$greeking = ucwords(implode(' ', $array));
  }
  return $greeking;
}

function devel_generate_add_terms(&$node) {
  $vocabs = taxonomy_get_vocabularies($node->type);
  foreach ($vocabs as $vocab) {
    $sql = "SELECT tid FROM {term_data} WHERE vid = %d ORDER BY RAND()";
    $result = db_query_range($sql, $vocab->vid, 0, 5);
    while ($row = db_fetch_object($result)) {
      $node->taxonomy[] = $row->tid;
      if (!$vocab->multiple) {
        break;
      }
    }
  }
}

function devel_get_users() {
  $users = array();
  $result = db_query_range("SELECT uid FROM {users}", 0, 50);
  while($user = db_fetch_object($result)){
    $users[] = $user->uid;
  }
  return $users;
}

function devel_generate_add_upload(&$node) {
  $source = 'misc/blog.png';
  $size = filesize($source);
  
  // $after this call, $source contains the new path.
  file_copy($source);
  $file = new stdClass();
  $file->filename = 'blog.png';
  $file->filepath = $source;
  $file->filemime = 'image/png';
  $file->list = variable_get('upload_list_default', TRUE);
  $file->description = 'b log.png was here';
  $file->filesize = $size;
  $file->weight = mt_rand(0,10);
  $file->new = TRUE;

  // If we made it this far it's safe to record this file in the database.
  db_query("INSERT INTO {files} (uid, filename, filepath, filemime, filesize, status, timestamp) VALUES (%d, '%s', '%s', '%s', %d, %d, %d)", $node->uid, $file->filename, $file->filepath, $file->filemime, $file->filesize, FILE_STATUS_TEMPORARY, time());
  $file->fid = db_last_insert_id('files', 'fid');
  $node->files[$file->fid] = $file;
}

/**
 * Generate statistics information for a node.
 *
 * @param $node
 *   A node object.
 */
function devel_generate_add_statistics($node) {
  $totalcount = mt_rand(0, 500);
  $daycount = mt_rand(0, $totalcount);
  $timestamp = time() - mt_rand(0, $node->created);
  db_query('INSERT INTO {node_counter} (nid, daycount, totalcount, timestamp) VALUES (%d, %d, %d, %d)', $node->nid, $daycount, $totalcount, $timestamp);
}

/**
 * Handle the devel_generate_content_form request to kill all of the content.
 * This is used by both the batch and non-batch branches of the code.
 *
 * @param $num
 *   array of options obtained from devel_generate_content_form.
 */
function devel_generate_content_kill($results) {
  $sql = 'SELECT nid FROM {node} WHERE type IN ('. db_placeholders($results['node_types'], 'text'). ')';
  $result = db_query($sql, $results['node_types']);
  $i=0;
  while ($row = db_fetch_object($result)) {
    node_delete($row->nid);
    $i++;
  }
  drupal_set_message(format_plural($i, 'Deleted one post', 'Deleted @count posts'));
}

/**
 * Pre-process the devel_generate_content_form request.  This is needed so
 * batch api can get the list of users once.  This is used by both the batch
 * and non-batch branches of the code.
 *
 * @param $num
 *   array of options obtained from devel_generate_content_form.
 */
function devel_generate_content_pre_node(&$results) {
  // Get user id.
  $users = devel_get_users();
  $users = array_merge($users, array('0'));
  $results['users'] = $users;
}

/**
 * Create one node.  This is used by both the batch and non-batch branches of
 * the code.
 *
 * @param $num
 *   array of options obtained from devel_generate_content_form.
 */
function devel_generate_content_add_node(&$results) {
  // Insert new data:
  $node->type = array_rand($results['node_types']);
  module_load_include('inc', 'node', 'node.pages');
  $type = node_get_types('type', $node->type);
  node_object_prepare($node);
  $users = $results['users'];
  $node->uid = $users[array_rand($users)];

  $node->title = devel_create_greeking(mt_rand(1, $results['title_length']), TRUE);
  if ($type->has_body) {
    $node->body = "node ($node->type) - ". devel_create_content();
    $node->teaser = node_teaser($node->body);
    $node->filter = variable_get('filter_default_format', 1);
    $node->format = FILTER_FORMAT_DEFAULT;
  }
  $node->language = '';
  $node->revision = mt_rand(0,1);
  $node->promote = mt_rand(0, 1);
  $node->created = time() - mt_rand(0, $results['time_range']);
  
  // A flag to let hook_nodeapi() implementations know that this is a generated node.
  $node->devel_generate = $results;
  // See devel_generate_nodeapi() for actions that happen before and after this save.
  node_save($node);

}

function devel_generate_nodeapi(&$node, $op, $a3 = NULL, $a4 = NULL) {
  if (isset($node->devel_generate)) {
    $results = $node->devel_generate;
    switch ($op) {
      // Modules that want to affect generated nodes may implement hook_nodeapi('presave'). See OG module or CCK.
      // A few implementations live here because core doesn't do bulk node generation.
      case 'presave':
        if ($results['add_upload']) {
          devel_generate_add_upload($node);
        }

        if ($results['add_terms']) {
          devel_generate_add_terms($node);
        }
        if (isset($results['add_language']) && variable_get('language_content_type_' . $node->type, 0)) {
          $languages = array_keys($results['add_language']);
          $node->language = $languages[array_rand($languages)];
        }
        break;
    case 'insert':
      if ($results['max_comments'] && $node->comment >= COMMENT_NODE_READ_WRITE) {
        devel_generate_add_comments($node, $results['users'], $results['max_comments'], $results['title_length']);
      }
    
      // Add an url alias. Cannot happen before save becasue we don't know the nid.
      if ($results['add_alias']) {
        path_set_alias("node/$node->nid", "node-$node->nid-$node->type");
      }

      // Add node statistics.
      if ($results['add_statistics']) {
        devel_generate_add_statistics($node);
      }
      break;  
    }
  }
}
