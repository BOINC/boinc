<?php
/**
 * @file
 * Applies the filter functions.
 */

/**
 * The table of contents object used to save all the data as
 * we go along (i.e. anchors info, table of contents variables, etc.)
 */
class TOC {
};

/**
 * Each header get an object with its level and identifier. We reuse
 * those to generate the TOC.
 *
 * The title (i.e. what appears between the opening and closing header
 * tags) is taken as the title appearing in your TOC.
 */
class TOC_Header {
};

/**
 * Replace a <!-- tableofcontents ... --> comment with [toc ...]
 */
function _tableofcontents_replace_comments($matches) {
  return '[toc ' . $matches[1] . ']';
}

/**
 * This function changes a header attributes. It adds an identifier in case
 * there are none. It registers the identifier if there is already one.
 *
 * \note
 * Note that the attributes (2nd match) always starts with a space if it
 * exists.
 *
 * @param[in] $matches The matches (level, attributes and title)
 *
 * @return The header with an identifier.
 */
function _tableofcontents_replace_headers($matches) {
  global $_tableofcontents_toc;
  static $toc_translations, $duplicates_error;

  $h = new TOC_Header;
  $h->level = $matches[2];
  $h->attributes = $matches[3];
  $h->title = $matches[4];

  // increase this level and reset all the sub-levels
  ++$_tableofcontents_toc->counters[$h->level];
  for ($l = $h->level + 1; $l <= 6; ++$l) {
    $_tableofcontents_toc->counters[$l] = 0;
  }
  if ($_tableofcontents_toc->level_from > $h->level) {
    $_tableofcontents_toc->level_from = $h->level;
  }
  $_tableofcontents_toc->level_to = $h->level;

  // determine the min./max. on the spot
  if ($_tableofcontents_toc->header_min) {
    if ($_tableofcontents_toc->header_min > $h->level) {
      $_tableofcontents_toc->header_min = $h->level;
    }
  }
  else {
    $_tableofcontents_toc->header_min = $h->level;
  }
  if ($_tableofcontents_toc->header_max) {
    if ($_tableofcontents_toc->header_max < $h->level) {
      $_tableofcontents_toc->header_max = $h->level;
    }
  }
  else {
    $_tableofcontents_toc->header_max = $h->level;
  }

  // check for existing id and use that if found
  if (preg_match('/\sid=(?:"([^"]*?)"|\'([^\']*?)\'|([^\s"\'>]))/i', $h->attributes, $id)) {
    // id[1] is with "
    // id[2] is with '
    // id[3] is without any quotes
    $h->identifier = $id[1] ? $id[1] : ($id[2] ? $id[2] : $id[3]);

    // check for unicity
    foreach ($_tableofcontents_toc->headers as $header) {
      if ($header->identifier == $h->identifier) {
        if (!isset($duplicates_error)) {
          $duplicates_error = TRUE;
          drupal_set_message(t('Two or more anchor identifiers match each others. One of them will be modified. This error happens when some anchor identifiers are generated automatically and others are predefined. Or both are predefined and the operator made a mistake (maybe a copy &amp; paste?)'), 'warning');
        }
        // make it unique anyway
        $h->identifier .= $_tableofcontents_toc->id_separator . $_tableofcontents_toc->header_id++;
        $h->attributes = preg_replace('/\sid=(?:"([^"]*?)"|\'([^\']*?)\'|([^\s"\'>]))/i',
          ' id="' . $h->identifier . '"', $h->attributes);
        break;
      }
    }
  }
  else {
    switch ($_tableofcontents_toc->id_generator) {
    case 'random':
      // generate a random ID and then ensure unicity
      do {
        $id = user_password(8); // system function to generate a password from letters/digits
        $found = FALSE;
        foreach ($_tableofcontents_toc->headers as $header) {
          if ($header->identifier == $id) {
            $found = TRUE;
            break;
          }
        }
      } while ($found);
      break;

    case 'increment':
      $id = $_tableofcontents_toc->identifier_introducer . $_tableofcontents_toc->id_separator . $_tableofcontents_toc->header_id++;
      break;

    case 'sections':
      $id = $_tableofcontents_toc->identifier_introducer;
      for ($idx = $_tableofcontents_toc->level_from; $idx <= $_tableofcontents_toc->level_to; ++$idx) {
        $id .= $_tableofcontents_toc->id_separator . $_tableofcontents_toc->counters[$idx];
      }
      break;

    case 'custom':
      // the callee has to edit the $h->identifier and $h->attributes fields
      module_invoke_all('anchor_identifier', $_tableofcontents_toc, $h);
      $id = '';
      break;

    default: // case 'title'
      // no existing identifier, create one using the header title
      //
      // HTML 4.01
      //
      // http://www.w3.org/TR/html4/types.html#h-6.2
      //
      // ID and NAME tokens must begin with a letter ([A-Za-z]) and
      // may be followed by any number of letters, digits ([0-9]),
      // hyphens ("-"), underscores ("_"), colons (":"), and periods (".").
      //
      // 1. convert &nbsp; and other spaces into underscores
      // 2. convert &mdash; or &ndash; to '--'
      // 3. convert &amp; to 'and'
      // 4. remove any other entities
      // 5. remove any incompatible character
      // 6. remove digits at the start of the name (we could also add a letter?)
      //
      // sanitize accents by luron & deviantintegral (e.g. é=>e)
      // thanks to pathauto module for i18n-ascii.txt file
      if (!isset($toc_translations)) {
        $path = drupal_get_path('module', 'tableofcontents');
        $toc_translations = parse_ini_file($path . '/i18n-ascii.txt');
      }
      $title = strtr(strip_tags($h->title, $_tableofcontents_toc->allowed_tags), $toc_translations);

      $allowed_chars = '';
      if (empty($_tableofcontents_toc->id_stripping['dashes'])) {
        $allowed_chars = '-';
      }
      $allowed_chars .= 'A-Za-z';
      if (empty($_tableofcontents_toc->id_stripping['digits'])) {
        $allowed_chars .= '0-9';
      }
      if (empty($_tableofcontents_toc->id_stripping['periods'])) {
        $allowed_chars .= '.';
      }
      if (empty($_tableofcontents_toc->id_stripping['underscores'])) {
        $allowed_chars .= '_';
      }
      if (empty($_tableofcontents_toc->id_stripping['colons'])) {
        $allowed_chars .= ':';
      }
      $id = preg_replace(
        array(
          '/&nbsp;|\s/',
          '/\'/',
          '/&mdash;/',
          '/&ndash;/',
          '/&amp;/',
          '/&[a-z]+;/',
          '/[^' . $allowed_chars . ']/',
          '/^[-0-9._:]+/',
          '/__+/',
        ),
        array(
          '_',     // &nbsp; and spaces
          '-',     // apostrophe, so it makes things slightly more readable
          '--',    // &mdash;
          '--',    // &ndash;
          'and',   // &amp;
          '',      // any other entity
          '',      // any character that is invalid as an ID name
          '',      // any digits at the start of the name
          '_',     // reduce multiple underscores to just one
        ),
        strip_tags($title)
      );
      if (!$id) {
        // no identifier (i.e. title is composed exclusively of digits, entities, etc.)
        $id = $_tableofcontents_toc->identifier_introducer . $_tableofcontents_toc->id_separator . $_tableofcontents_toc->header_id++;
      }
      break;

    }
    if ($id != '') {
      // ensure unicity
      foreach ($_tableofcontents_toc->headers as $header) {
        if ($header->identifier == $id) {
          $id .= $_tableofcontents_toc->id_separator . $_tableofcontents_toc->header_id++;
          break;
        }
      }

      $h->identifier = $id;

      // create a new header including the generated identifier
      $h->attributes .= ' id="' . $id . '"';
    }
  }

  $h->number = theme('tableofcontents_number', $_tableofcontents_toc);
  if ($_tableofcontents_toc->number_headers) {
    $number = $h->number;
  }
  else {
    $number = '';
  }

  $result = $matches[1] . '<h' . $h->level . $h->attributes . '>' . $number . $h->title . '</h' . $h->level . '>';

  // save that header
  $_tableofcontents_toc->headers[] = $h;

  // Add a back to top before the header?
  if ($_tableofcontents_toc->back_to_top
      && $h->level >= $_tableofcontents_toc->back_to_top_minlevel
      && $h->level <= $_tableofcontents_toc->back_to_top_maxlevel) {
    switch ($_tableofcontents_toc->back_to_top_location) {
    case 'header':
      $result .= $_tableofcontents_toc->back_to_top_link;
      break;

    default: //case 'bottom':
      if (!$_tableofcontents_toc->first_header) {
        $result = $_tableofcontents_toc->back_to_top_link . $result;
      }
      break;

    }
  }
  $_tableofcontents_toc->first_header = FALSE;

  return $result;
}

/**
 * This function goes through all the headers found in the text.
 *
 * @bug
 * The pattern used assumes that you have at most ONE header per
 * line AND that the whole header is defined on ONE line.
 *
 * @param[in,out] $toc The table of content object
 * @param[in] $format The format being worked on
 * @param[in] $text The text to be parsed
 *
 * @return The text with headers transformed to include an identifier
 */
function _tableofcontents_headers(&$toc, $format, $text) {
  // initialize header variables
  $toc->headers = array();
  $toc->header_id = 1;
  $toc->header_min = 0;
  $toc->header_max = 0;
  $toc->allowed_tags = variable_get('tableofcontents_allowed_tags_' . $format, TABLEOFCONTENTS_ALLOWED_TAGS);
  $toc->id_stripping = variable_get('tableofcontents_id_stripping_' . $format, array());
  $toc->identifier_introducer = variable_get('tableofcontents_identifier_introducer_' . $format, 'header');
  $toc->id_separator = variable_get('tableofcontents_id_separator_' . $format, '-');
  $toc->id_generator = variable_get('tableofcontents_id_generator_' . $format, 'title');
  $toc->back_to_top = check_plain(variable_get('tableofcontents_back_to_top_' . $format, ''));
  $toc->back_to_top_location = check_plain(variable_get('tableofcontents_back_to_top_location_' . $format, 'bottom'));
  $toc->back_to_top_anchor = check_plain(variable_get('tableofcontents_back_to_top_anchor_' . $format, 'toc'));
  $toc->scroll_back_to_top = check_plain(variable_get('tableofcontents_scroll_back_to_top_' . $format, FALSE));
  $toc->back_to_top_minlevel = variable_get('tableofcontents_back_to_top_minlevel_' . $format, 2);
  $toc->back_to_top_maxlevel = variable_get('tableofcontents_back_to_top_maxlevel_' . $format, 4);
  $toc->first_header = TRUE;
  $toc->numbering = variable_get('tableofcontents_numbering_' . $format, 0);
  $toc->number_mode = variable_get('tableofcontents_number_mode_' . $format, 0);
  $toc->number_start_letter = check_plain(variable_get('tableofcontents_number_start_letter_' . $format, ''));
  $toc->number_separator = check_plain(variable_get('tableofcontents_number_separator_' . $format, '.'));
  $toc->number_end_letter = check_plain(variable_get('tableofcontents_number_end_letter_' . $format, '.'));
  $toc->number_headers = variable_get('tableofcontents_number_headers_' . $format, FALSE);
  $toc->level_from = 6;
  $toc->level_to = 6;
  $toc->counters = array(0, 0, 0, 0, 0, 0, 0);  // used to generate a toc with advanced counters

  if (isset($toc->on_print_pages)) {
    $toc->back_to_top_link = '';
  }
  else {
    $toc->back_to_top_link = theme('tableofcontents_back_to_top', $toc);
  }

  // note that the pattern below assumes that the headers
  // are properly defined in your HTML (i.e. a header cannot
  // include another)
  //
  // Note: we support having a [collapse] tag just before a header
  //       and even possibly a [/collapse] just before that!
  $result = preg_replace_callback(
    '%((?:(?:<p(?:\s[^>]*)?'.'>\s*)?\[/collapse\](?:</p\s*>\s*)?)?'
      . '(?:<p(?:\s[^>]*)?'.'>\s*)?\[collapse[^]]*?\](?:</p\s*>\s*)?)?'
      . '<h([1-6])(\s+[^>]*?)?'.'>(.*?)</h[1-6]\s*>%si',
    '_tableofcontents_replace_headers',
    $text
  );

  return $result;
}

/**
 * Parse one option and save its value.
 *
 * The function returns NULL since we don't need to replace
 * anything in the source.
 *
 * @param[in] $opt An array with the option name and value.
 */
function _tableofcontents_parse_option($opt) {
  global $_tableofcontents_toc;

  $opt[1] = trim($opt[1]);
  $opt[2] = trim($opt[2]);
  switch (drupal_strtolower($opt[1])) {
  case 'id':
  case 'title':
    // check_plain() on $opt[2] is applied later as required by the administrator
    $_tableofcontents_toc->{$opt[1]} = $opt[2];
    break;

  // We may want to, at some point, look into supporting such things
  // for the headers... then this would be valid again.
  case 'list':
    // ignore since some users may have been using it...
    break;
  //  switch (drupal_strtolower($opt[2])) {
  //  case 'ol':
  //  case 'ordered':
  //    $_tableofcontents_toc->numbering = 4;
  //    break;

  //  case 'ul':
  //  case 'unordered':
  //    // in this case, just make sure we don't have 'ol'
  //    if ($_tableofcontents_toc->numbering == 4) {
  //      $_tableofcontents_toc->numbering = 0;
  //    }
  //    break;

  //  case 'none':
  //    $_tableofcontents_toc->numbering = 0;
  //    break;

  //  case 'numbering':
  //    $_tableofcontents_toc->numbering = 1;
  //    break;

  //  case 'sub-numbering':
  //    $_tableofcontents_toc->numbering = 2;
  //    break;

  //  case 'sub-numbering-with-zero':
  //    $_tableofcontents_toc->numbering = 3;
  //    break;

  //  default:
  //    drupal_set_message(t("Table of contents error: !opt is not a valid list type. Expected 'ul', 'unordered', 'ol', or 'ordered'",
  //      array('!opt' => $opt[2])), 'error');
  //    break;

  //  }
  //  break;

  case 'minlevel':
  case 'maxlevel':
  case 'back_to_top_minlevel':
  case 'back_to_top_maxlevel':
    if (!is_numeric($opt[2]) || $opt[2] < 1 || $opt[2] > 6) {
      drupal_set_message(t("Table of contents error: @opt is not a valid level. Expected a number from 1 to 6",
        array('@opt' => $opt[2])), 'error');
    }
    else {
      $_tableofcontents_toc->{$opt[1]} = $opt[2];
    }
    break;

  case 'hidden':
  case 'hide': // support bug from previous versions...
  case 'attachments':
  case 'hideshow':
  case 'collapsed':
    switch (drupal_strtolower($opt[2])) {
    case '0':
    case 'false':
      $_tableofcontents_toc->{$opt[1]} = FALSE;
      break;

    case '1':
    case 'true':
      $_tableofcontents_toc->{$opt[1]} = TRUE;
      break;

    default:
      drupal_set_message(t("Table of contents error: @val is not a valid boolean value for @opt. Expected 0, false, 1, or true.",
        array('@opt' => $opt[1], '@val' => $opt[2])), 'error');
      break;

    }
    break;

  default:
    drupal_set_message(t("Table of contents error: @opt is not a valid option.",
      array('@opt' => $opt[1])), 'error');
    break;

  }
}

/**
 * Parse options and save them in your $toc variable.
 *
 * @warning
 * This function does NOT check whether you are authorized to parse
 * the options. It parses them. Period.
 *
 * @param[in,out] $toc The TOC object where the options are saved
 * @param[in] $options A string of options
 */
function _tableofcontents_get_options(&$toc, $options) {
  static $id_counter = 0;

  // A default identifier for this table of contents
  $toc->id = 'toc';
  if ($id_counter) {
    $toc->id .= $id_counter;
  }
  ++$id_counter;

  // Any options?
  if (!$options) {
    return;
  }
  // Make sure we don't miss the last parameter even if not properly ended
  $options .= ';';

  // We use a replace just so we get the callback
  preg_replace_callback(
    '/([A-Za-z0-9]+)[:=]\s*((?:&[^;]+;|[^;])+);/',
    '_tableofcontents_parse_option',
    $options
  );

  if ($toc->minlevel > $toc->maxlevel) {
    drupal_set_message(t("Table of contents error: minlevel (!minlevel) is larger than maxlevel (!maxlevel), reverting to defaults.",
        array('!minlevel' => $toc->minlevel, '!maxlevel' => $toc->maxlevel)), 'error');
    $toc->minlevel = 2;
    $toc->maxlevel = 3;
  }

  // adjust the levels according to the available headers
  if ($toc->minlevel < $toc->header_min) { // avoid empty indentations
    $toc->minlevel = $toc->header_min;
    if ($toc->minlevel > $toc->maxlevel) {
      $toc->maxlevel = $toc->minlevel;
    }
  }
  if ($toc->maxlevel > $toc->header_max) { // this is much less important
    $toc->maxlevel = $toc->header_max;
  }
}

/**
 * This function replaces one [toc ...] tag.
 *
 * Note that all the toc tags receive the same headers, but each may have
 * a different set of parameters.
 *
 * @bug
 * Unfortunately, since we use the preg_replace_callback() functions
 * to increase speed we run in the problem of having to use a global
 * variable to hold all the TOC information. This function must make
 * a copy of it to avoid problems later.
 *
 * @param[in] $toc The TOC object
 * @param[in] $matches The matches of one [toc ...] tag
 *
 * @return The replacement for that [toc ...] tag
 */
function _tableofcontents_replace_toc_tags($matches) {
  global $_tableofcontents_toc;

  // we do not want options in this [toc ...] tag to affect the following [toc ...] tags
  $save_toc = drupal_clone($_tableofcontents_toc);

  // we always want to check the options because hidden:1 is always accepted!
  $options = isset($matches[1]) ? $matches[1] : '';
  _tableofcontents_get_options($_tableofcontents_toc, $options);
  if ($_tableofcontents_toc->hidden) {
    $_tableofcontents_toc = $save_toc;

    // replace that table of contents with nothing (i.e. hiding it!)
    return '';
  }

  // Is user allowed to override options?
  if (!$_tableofcontents_toc->allow_override) {
    // Nope, then restore!
    $_tableofcontents_toc = $save_toc;
  }
  elseif ($_tableofcontents_toc->safe_title && $_tableofcontents_toc->title != $save_toc->title) {
    $_tableofcontents_toc->title = check_plain(strip_tags($_tableofcontents_toc->title));
  }

  // If attachments are enabled, prepare the $_tableofcontents_toc->files variable
  if ($_tableofcontents_toc->attachments && arg(0) == 'node' && is_numeric(arg(1))
      && module_exists('upload')) {
    $node = node_load(arg(1));
    $_tableofcontents_toc->files = $node->files;
  }

  $result = theme('tableofcontents_toc', $_tableofcontents_toc);

  $_tableofcontents_toc = $save_toc;

  if ($result) {
    $_tableofcontents_toc->has_toc = TRUE;
  }

  return $result;
}

/**
 * \brief This function adds comments to the toc.
 *
 * Comments follow the table of content.
 *
 * \param[in,out] $toc The toc to update
 * \param[in] $format The format used to parse the node comments
 */
function _tableofcontents_comments(&$toc, $format) {
  // hack to get the $nid...
  if (arg(0) != 'node' || !is_numeric(arg(1))) {
    return;
  }
  $nid = arg(1);
  $node = node_load($nid);
  $mode = _comment_get_display_setting('mode', $node);
  $order = _comment_get_display_setting('sort', $node);
  $comments_per_page = _comment_get_display_setting('comments_per_page', $node);

  // define query
  $query = 'SELECT c.cid, c.subject, c.thread, c.status'
    . ' FROM {comments} c WHERE c.nid = %d AND c.status = %d';
  $query_count = 'SELECT COUNT(*) FROM {comments} c WHERE c.nid = %d AND c.status = %d';
  $query_args = array($nid, COMMENT_PUBLISHED);

  // define ordering
  if ($order == COMMENT_ORDER_NEWEST_FIRST) {
    if ($mode == COMMENT_MODE_FLAT_COLLAPSED || $mode == COMMENT_MODE_FLAT_EXPANDED) {
      $query .= ' ORDER BY c.cid DESC';
    }
    else {
      $query .= ' ORDER BY c.thread DESC';
    }
  }
  elseif ($order == COMMENT_ORDER_OLDEST_FIRST) {
    if ($mode == COMMENT_MODE_FLAT_COLLAPSED || $mode == COMMENT_MODE_FLAT_EXPANDED) {
      $query .= ' ORDER BY c.cid';
    }
    else {
      $query .= ' ORDER BY SUBSTRING(c.thread, 1, (LENGTH(c.thread) - 1))';
    }
  }

  $query = db_rewrite_sql($query, 'c', 'cid');
  $query_count = db_rewrite_sql($query_count, 'c', 'cid');

  $result = pager_query($query, $comments_per_page, 0, $query_count, $query_args);

  $comment_level = variable_get('tableofcontents_comments_level_' . $format, 3);
  while ($comment = db_fetch_object($result)) {
    $level = $comment_level;
    if ($mode == COMMENT_MODE_THREADED_COLLAPSED || $mode == COMMENT_MODE_THREADED_EXPANDED) {
      $level += count(explode('.', $comment->thread)) - 1;
    }

    if ($level < 1 || $level > 6) {
      continue;
    }

    // this is the object we return to generate the TOC
    $h = new TOC_Header;
    $h->level = $level;
    $h->title = check_plain($comment->subject);

    // increase this level and reset all the sub-levels
    ++$toc->counters[$level];
    for ($l = $level + 1; $l <= 6; ++$l) {
      $toc->counters[$l] = 0;
    }
    if ($toc->level_from > $level) {
      $toc->level_from = $level;
    }
    $toc->level_to = $level;

    // min/max adjustment
    if ($toc->header_min) {
      if ($toc->header_min > $level) {
        $toc->header_min = $level;
      }
    }
    else {
      $toc->header_min = $level;
    }
    if ($toc->header_max) {
      if ($toc->header_max < $level) {
        $toc->header_max = $level;
      }
    }
    else {
      $toc->header_max = $level;
    }

    // identifier (as added by the comment module)
    $h->identifier = 'comment-' . $comment->cid;
    $h->attributes = ' id="' . $h->identifier . '"';

    $h->number = theme('tableofcontents_number', $toc);

    $toc->headers[] = $h;
  }
}

/**
 * This function reokaces the table of contents.
 *
 * It checks all the current flags such as whether the table should
 * be automatically added if no [toc] tag is found.
 *
 * @param[in] $delta The filter number (0- full toc, 1- only anchors)
 * @param[in] $text The text to work on.
 * @param[in] $format The format used to do the conversion.
 *
 * @return The $text with the [toc ...] replaced.
 */
function _tableofcontents_replace_toc($delta, $format, $text) {
  // we just don't have a way to pass that around our callbacks
  // (PHP is lacking on that one) but it is still a lot faster
  // that way.
  global $_tableofcontents_toc;
  global $_tableofcontents_block_toc;

  // this is a special case... do what it takes at once
  if ($delta == 1) {
    $text = preg_replace(TABLEOFCONTENTS_REMOVE_PATTERN, '', $text);
    // note: $_tableofcontents_toc->automatic is only used in this very function and
    //       thus we do not need to define it for _tableofcontents_headers().
    $_tableofcontents_toc = new TOC;
    // pretent that we are on a print page so we do not get back to top links
    $_tableofcontents_toc->on_print_pages = TRUE;
    return _tableofcontents_headers($_tableofcontents_toc, $format, $text);
  }

  // if the user just asked to hide that, hide it and go away
  if (empty($_tableofcontents_block_toc) && variable_get('tableofcontents_hide_table_' . $format, FALSE)) {
    return preg_replace(TABLEOFCONTENTS_REMOVE_PATTERN, '', $text);
  }

  // keep a copy of the original just in case we bail out as is
  $org_text = $text;

  $_tableofcontents_toc = new TOC;
  $_tableofcontents_toc->id = 'toc';

  // first make sure we want a table of contents
  if (empty($_tableofcontents_block_toc) && strpos($text, "[toc") === FALSE) {
    // no [toc], but a [vtoc], we're good
    if (strpos($text, "[vtoc") !== FALSE) {
      return $org_text;
    }
    $_tableofcontents_toc->automatic = variable_get('tableofcontents_automatic_' . $format, 0);
    switch ($_tableofcontents_toc->automatic) {
    case 1:
    case 2:
      // Avoid the table of contents anywhere else than in regular nodes
      // (which is somewhat equivalent to saying don't add in the teasers)
      if (variable_get('tableofcontents_remove_teaser_' . $format, TRUE)) {
        if (arg(0) != 'node' || !is_numeric(arg(1)) || (arg(2) && arg(2) != 'view')) {
          return $org_text;
        }
      }
      break;

    default:
      // if not defined as automatic in the filter, check the node and its type
      if (arg(0) == 'node' && is_numeric(arg(1)) && (!arg(2) || arg(2) == 'view')) {
        $node = node_load(arg(1));
        if ($node) {
          if ($node->tableofcontents_toc_automatic) {
            $_tableofcontents_toc->automatic = $node->tableofcontents_toc_automatic;
          }
          else {
            $_tableofcontents_toc->automatic = variable_get('tableofcontents_nodetype_toc_automatic_' . $node->type, 0);
            if ($_tableofcontents_toc->automatic == 99) {
              $_tableofcontents_toc->automatic = 0;
            }
          }
        }
      }
      break;

    }
    switch ($_tableofcontents_toc->automatic) {
    default: // case 0
      // if ($_tableofcontents_block_toc) -- cannot happen
      return $org_text;

    case 1: // prepend
      $text = '[toc]' . $text;
      break;

    case 2: // append
      $text = $text . '[toc]';
      break;

    }
  }
  else {
    $_tableofcontents_toc->automatic = 0;
  }

  // print module running and creating a print version?
  if (arg(0) == 'print' && is_numeric(arg(1)) && module_exists('print')) {
    // Note: not to sure when the build_mode is set, but here it is still undefined
    //       just to make sure, we return only if not defined to something else than
    //       'print'; if undefined, we assume that it is 'print'
    $node = node_load(arg(1));
    if (!isset($node->build_mode) || $node->build_mode == 'print') {
      if (!variable_get('tableofcontents_show_on_print_pages_' . $format, FALSE)) {
        return preg_replace(TABLEOFCONTENTS_REMOVE_PATTERN, '', $org_text);
      }
      $_tableofcontents_toc->on_print_pages = TRUE;
    }
  }

  // Add an identifier in the existing headers and return info
  // about the tree
  $text = _tableofcontents_headers($_tableofcontents_toc, $format, $text);

  // If we have a block-only table of contents then we do not want to process
  // the body also, just replacing the headers is enough for the body (to get the IDs.)
  if (!$_tableofcontents_block_toc && variable_get('tableofcontents_block_only_table_' . $format, FALSE)) {
    return preg_replace(TABLEOFCONTENTS_REMOVE_PATTERN, '', $text);
  }

  // Include comments?
  if (variable_get('tableofcontents_comments_' . $format, FALSE)
      && module_exists('comment')
      && user_access('access comments')) {
    _tableofcontents_comments($_tableofcontents_toc, $format);
  }

  // Any headers? if not do nothing...
  $max = count($_tableofcontents_toc->headers);
  if ($max == 0) {
    // not even 1 header!!!
    if ($_tableofcontents_block_toc) {
      return '';
    }
    return preg_replace(TABLEOFCONTENTS_REMOVE_PATTERN, '', $org_text);
  }

  // If automatic verify the limit, if not reached, bail out
  if ($_tableofcontents_toc->automatic) {
    $min_limit = variable_get('tableofcontents_min_limit_' . $format, 5);
    if ($max < $min_limit) {
      // Limit not reached!
      if ($_tableofcontents_block_toc) {
        return '';
      }
      return preg_replace(TABLEOFCONTENTS_REMOVE_PATTERN, '', $org_text);
    }
  }

  // get the default flags, these apply for each call to the
  // _tableofcontents_replace_toc_tags() function.
  // (i.e. that function should not be able to change $_tableofcontents_toc)
  $_tableofcontents_toc->hidden         = FALSE;
  $_tableofcontents_toc->title          = variable_get('tableofcontents_title_'          . $format, 'Table of contents');
  $_tableofcontents_toc->safe_title     = variable_get('tableofcontents_safe_title_'     . $format, TRUE);
  $_tableofcontents_toc->minlevel       = variable_get('tableofcontents_minlevel_'       . $format, 2);
  $_tableofcontents_toc->maxlevel       = variable_get('tableofcontents_maxlevel_'       . $format, 3);
  $_tableofcontents_toc->attachments    = variable_get('tableofcontents_attachments_'    . $format, FALSE);
  $_tableofcontents_toc->hideshow       = variable_get('tableofcontents_hide_show_'      . $format, TRUE);
  $_tableofcontents_toc->collapsed      = variable_get('tableofcontents_collapsed_'      . $format, FALSE);
  $_tableofcontents_toc->allow_override = variable_get('tableofcontents_allow_override_' . $format, TRUE);
  $_tableofcontents_toc->has_toc        = FALSE;

  if ($_tableofcontents_toc->minlevel < $_tableofcontents_toc->header_min) { // avoid empty indentations
    $_tableofcontents_toc->minlevel = $_tableofcontents_toc->header_min;
    if ($_tableofcontents_toc->minlevel > $_tableofcontents_toc->maxlevel) {
      $_tableofcontents_toc->maxlevel = $_tableofcontents_toc->minlevel;
    }
  }
  if ($_tableofcontents_toc->maxlevel > $_tableofcontents_toc->header_max) { // this is much less important
    if ($_tableofcontents_toc->minlevel > $_tableofcontents_toc->header_max) {
      $_tableofcontents_toc->maxlevel = $_tableofcontents_toc->minlevel;
    }
    else {
      $_tableofcontents_toc->maxlevel = $_tableofcontents_toc->header_max;
    }
  }

  if ($_tableofcontents_block_toc) {
    $text = $_tableofcontents_block_toc;
  }

  $result = preg_replace_callback(
    '/(?:<p(?:\s[^>]*)?'.'>)?\[toc(\s[^]]*?)?\](?:<\/p\s*>)?/',
    '_tableofcontents_replace_toc_tags',
    $text
  );

  if ($_tableofcontents_toc->has_toc) {
    // Add a back to top at the very end too
    if (!$_tableofcontents_toc->first_header
        && $_tableofcontents_toc->back_to_top
        && $_tableofcontents_toc->back_to_top_location == 'bottom'
        && $_tableofcontents_toc->header_min >= $_tableofcontents_toc->back_to_top_minlevel
        && $_tableofcontents_toc->header_min <= $_tableofcontents_toc->back_to_top_maxlevel) {
      $result .= $_tableofcontents_toc->back_to_top_link;
    }

    // If we have back to top and scroll on, advice script
    if ($_tableofcontents_toc->back_to_top && $_tableofcontents_toc->scroll_back_to_top) {
      $result .= '<script type="text/javascript">toc_scroll_back_to_top = 1;</script>';
    }
  }

  return $result;
}



/**
 * Prepare the text for the table of content.
 *
 * This function simply replaces the comment into a tag with square backets.
 *
 * @param[in] $delta The filter number
 * @param[in] $format The format number
 * @param[in] $text The text to filter
 */
function _tableofcontents_prepare($delta, $format, $text) {
  // support for PEAR wiki (http://drupal.org/node/150067#comment-937519)
  $text = str_replace('[[TOC]]', '[toc]', $text);
  return preg_replace_callback(
    array('%<!--\s*tableofcontents([^>]*?)-->%', '%\[\[TOC(.*?)\]\]%'),
    '_tableofcontents_replace_comments',
    $text
  );
}

/**
 * Transform the [toc ...] tags into the actual table of content.
 *
 * This function parses the headers to determine the content of the
 * table of content and then parses the [toc ...] tags.
 *
 * It uses sub-functions in order to make them reusable by other
 * parts such as the block implementation.
 *
 * @param[in] $delta The filter number
 * @param[in] $format The format number
 * @param[in] $text The text to filter
 */
function _tableofcontents_process($delta, $format, $text) {
  return _tableofcontents_replace_toc($delta, $format, $text);
}

/**
 * Theme the output of a table of contents.
 *
 * @param $toc
 *   A TOC object with the options, table of contents, headers,
 *   and files.
 *
 * @return
 *   Rendered HTML to be displayed.
 */
function theme_tableofcontents_toc($toc) {
  // TODO: at this time, the hideshow works with all TOCs at once
  if ($toc->hideshow && !isset($toc->on_print_pages)) {
    // We add the link with JavaScript so if someone does not have JavaScript, they do
    // not see a link that doesn nothing for them.
    //$hs = '<span class="toc-toggle-message"> [<a href="#" class="toc-toggle">' . $label . '</a>]</span>';
    $hs = '<span class="toc-toggle-message">&nbsp;</span>';
    $js = '<script type="text/javascript">toc_collapse=' . ($toc->collapsed ? 1 : 0) . ';</script>';
  }
  else {
    $hs = '';
    $js = '';
  }
  // TODO: we should only define the id="toc" for the first TOC
  if ($toc->id) {
    $id = ' id="' . check_plain($toc->id) . '"';
  }
  else {
    $id = '';
  }
  $output = $js . "<div class=\"toc\"" . $id . ">\n";
  if ($toc->title != '<none>') {
    $output .= "<div class=\"toc-title\">" . t($toc->title) . $hs . "</div>\n";
  }
  elseif ($hs) {
    $output .= $hs;
  }
  $list = $toc->numbering == 4 ? 'ol' : 'ul';
  $output .= "<div class=\"toc-list\">\n"
      . "<" .  $list . ">\n";

  $has_content = FALSE;

  // Process nested lists.
  $level = $toc->minlevel;
  $first = TRUE;
  foreach ($toc->headers as $h) {

    $cur_level = $h->level;
    if ($cur_level >= $toc->minlevel && $cur_level <= $toc->maxlevel) {
      $has_content = TRUE;

      // Be sure to deal with skipping between non-adjacent h levels.
      if ($cur_level > $level) {
        do {
          $output .= "\n<" . $list . ">\n";
          ++$level;
        } while ($cur_level > $level);
      }
      elseif ($cur_level < $level) {
        do {
          $output .= "</li>\n</" . $list . ">\n";
          --$level;
        } while ($cur_level < $level);
        $output .= "</li>\n";
      }
      elseif (!$first) { // we know that $cur_level == $level here
        // Close list items at the same level (except the very first time)
        $output .= "</li>\n";
      }

      if ($h->number) {
        $number = '<span class="toc-number">' . $h->number . '</span>';
      }
      else {
        $number = '';
      }
      $list_class = 'toc-level-' . ($level - $toc->minlevel + 1);

      $title = strip_tags($h->title, $toc->allowed_tags);

      // insert the li element
      $output .= "\t<li class=\"$list_class\">";
      if (isset($toc->on_print_pages)) {
        $output .= $number . $title;
      }
      else {
        $output .= "<a href=\"#" . $h->identifier . "\">" . $number . $title . "</a>";
      }

      $first = FALSE;
    }
  }
  // Did we recurse back out? If not, close open lists.
  for (; $level > $toc->minlevel; --$level) {
    $output .= "</li>\n</" . $list . ">\n";
  }
  $output .= "</li>\n";

  // If we've been passed an array of files, add them to the table of contents.
  if (!empty($toc->files)) {
    $has_files = FALSE;
    $files_output = '<li><a href="#attachments">' . t("Attachments") . "</a>";
    $files_output .= "<" . $list . ">";
    foreach ($toc->files as $file) {
      if ($file->list && !$file->remove) {
        $has_files = TRUE;

        $url = strpos($file->fid, 'upload') === FALSE ? $file->filepath : file_create_filename($file->filename, file_create_path());
        $file_href = file_create_url($url);
        $file_text = $file->description ? $file->description : $file->filename;

        // If there is no function providing phptemplate_file_icon in
        // template.php or similar then the theme file_icon will fail gracefully.
        $files_output .= '<li>' . theme('file_icon', $file) . l($file_text, $file_href) . "</li>\n";
      }
    }
    if ($has_files) {
      $output .= $files_output . "</" . $list . "></li>\n";
      $has_content = TRUE;
    }
  }

  if (!$has_content) {
    // this happens when all the header levels are either too small or too large
    // and if there is no attachment either
    return '';
  }

  $output .= "</" . $list . ">\n</div>\n</div>";

  return theme('tableofcontents_toc_text', $output);
}

/**
 * Basic formatting of the text generated by the
 * theme_tableofcontents_toc() function. This is the
 * table of contents in the form of a string.
 *
 * @param $text
 *   The text to format
 *
 * @return
 *   The resulting text
 */
function theme_tableofcontents_toc_text($text) {
  return $text;
}

/*****************************************************************************/
/**
 * PHP Roman Numeral Library
 *
 * Copyright (c) 2008, reusablecode.blogspot.com; some rights reserved.
 *
 * This work is licensed under the Creative Commons Attribution License. To view
 * a copy of this license, visit http://creativecommons.org/licenses/by/3.0/ or
 * send a letter to Creative Commons, 559 Nathan Abbott Way, Stanford, California
 * 94305, USA.
 *
 * Roman numbers library from
 * http://snipplr.com/view/6314/roman-numerals/
 * http://reusablecode.blogspot.com/search/label/roman%20numerals
 */
 
// Convert Arabic numerals into Roman numerals.
function _tableofcontents_roman($arabic) {
  //static $fractions = Array("", "¿", "¿¿", "¿¿¿", "¿¿¿¿", "¿¿¿¿¿", "S", "S¿", "S¿¿", "S¿¿¿", "S¿¿¿¿", "S¿¿¿¿¿", "I");
  static $ones = Array("", "I", "II", "III", "IV", "V", "VI", "VII", "VIII", "IX");
  static $tens = Array("", "X", "XX", "XXX", "XL", "L", "LX", "LXX", "LXXX", "XC");
  static $hundreds = Array("", "C", "CC", "CCC", "CD", "D", "DC", "DCC", "DCCC", "CM");
  static $thousands = Array("", "M", "MM", "MMM", "MMMM");

  if ($arabic > 4999) {
    // For large numbers (five thousand and above), a bar is placed above a base numeral to indicate multiplication by 1000.
    // Since it is not possible to illustrate this in plain ASCII, this function will refuse to convert numbers above 4999.
    //die("Cannot represent numbers larger than 4999 in plain ASCII.");
    return $arabic;
  }
  elseif ($arabic == 0) {
    // About 725, Bede or one of his colleagues used the letter N, the initial of nullae,
    // in a table of epacts, all written in Roman numerals, to indicate zero.
    return "N";
  }
  else {
    // Handle fractions that will round up to 1.
    //if (round(fmod($arabic, 1) * 12) == 12) {
    //  $arabic = round($arabic);
    //}

    // With special cases out of the way, we can proceed.
    // NOTE: modulous operator (%) only supports integers, so fmod() had to be used instead to support floating point.
    $m = fmod($arabic, 1000);
    $roman = $thousands[($arabic - $m) / 1000];
    $arabic = $m;
    $m = fmod($arabic, 100);
    $roman .= $hundreds[($arabic - $m) / 100];
    $arabic = $m;
    $m = fmod($arabic, 10);
    $roman .= $tens[($arabic - $m) / 10];
    $arabic = $m;
    $m = fmod($arabic, 1);
    $roman .= $ones[($arabic - $m) / 1];

    // Handling for fractions.
    //$arabic = $m;
    //if ($arabic > 0) {
    //  $roman .= $fractions[round($arabic * 12)];
    //}

    return $roman;
  }
}
 
// Expand subtractive notation in Roman numerals.
//function _tableofcontents_roman_expand($roman)
//{
//  $roman = str_replace("CM", "DCCCC", $roman);
//  $roman = str_replace("CD", "CCCC", $roman);
//  $roman = str_replace("XC", "LXXXX", $roman);
//  $roman = str_replace("XL", "XXXX", $roman);
//  $roman = str_replace("IX", "VIIII", $roman);
//  $roman = str_replace("IV", "IIII", $roman);
//  return $roman;
//}
 
// Compress Roman numerals using subtractive notation.
//function _tableofcontents_roman_compress($roman)
//{
//  $roman = str_replace("DCCCC", "CM", $roman);
//  $roman = str_replace("CCCC", "CD", $roman);
//  $roman = str_replace("LXXXX", "XC", $roman);
//  $roman = str_replace("XXXX", "XL", $roman);
//  $roman = str_replace("VIIII", "IX", $roman);
//  $roman = str_replace("IIII", "IV", $roman);
//  return $roman;
//}
 
// Convert Roman numerals into Arabic numerals.
//function _tableofcontents_arabic($roman)
//{
//  $result = 0;
//
//  // Remove subtractive notation.
//  $roman = roman_expand($roman);
//
//  // Calculate for each numeral.
//  $result += substr_count($roman, 'M') * 1000;
//  $result += substr_count($roman, 'D') * 500;
//  $result += substr_count($roman, 'C') * 100;
//  $result += substr_count($roman, 'L') * 50;
//  $result += substr_count($roman, 'X') * 10;
//  $result += substr_count($roman, 'V') * 5;
//  $result += substr_count($roman, 'I');
//  return $result;
//}
/*****************************************************************************/

/**
 * Transform a decimal number into a set of letters
 */
function _tableofcontents_letter($number) {
  while ($number > 0) {
    $result = chr($number % 26 + 64) . $result;
    $number = (int) ($number / 26);
  }

  return $result;
}

/**
 * Transform a decimal number in one of:
 *
 * @li decimal number
 * @li roman number
 * @li letters
 *
 * @param[in] $mode The type of number to transform into
 * @param[in] $number The number to transform
 *
 * @return The result
 */
function _tableofcontents_convert_number($mode, $number) {
  switch ($mode) {
  default: //case 0:
    return $number;

  case 1:
    return _tableofcontents_roman($number);

  case 2:
    return drupal_strtolower(_tableofcontents_roman($number));

  case 3:
    return _tableofcontents_letter($number);

  case 4:
    return drupal_strtolower(_tableofcontents_letter($number));

  case 5:
    return sprintf("0x%02x", $number);

  }
}

/**
 * Theme the output of the Back to Top link.
 *
 * This way users can easily add an image instead of using the CSS
 * arrow up as we have in the module.
 *
 * @param $toc
 *   A TOC object with the options and levels.
 *
 * @return
 *   Rendered HTML to be displayed.
 */
function theme_tableofcontents_back_to_top($toc) {
  return '<div class="toc-back-to-top"><a href="#' . $toc->back_to_top_anchor . '">'
    . $toc->back_to_top . '</a></div>';
}

/**
 * Theme the output of a multi-level number.
 *
 * @param $toc
 *   A TOC object with the options and levels.
 *
 * @return
 *   Rendered HTML to be displayed.
 */
function theme_tableofcontents_number($toc) {
  $result = '';
  switch ($toc->numbering) {
  case 0: // no numbering
  case 4: // numbering by browser
    return '';

  case 1:
    // "regular" (like <ol>)
    $result = _tableofcontents_convert_number($toc->number_mode, $toc->counters[$toc->level_to]);
    break;

  case 2:
  case 3:
    // 1., 1.1, 1.2, 2., 2.1, 2.2, ...
    $mode = $toc->number_mode;
    $result = _tableofcontents_convert_number($mode, $toc->counters[$toc->level_from]);
    for ($idx = $toc->level_from + 1; $idx <= $toc->level_to; ++$idx) {
      $result .= $toc->number_separator . _tableofcontents_convert_number($mode, $toc->counters[$idx]);
    }
    if ($toc->numbering == 3 && $toc->level_from == $toc->level_to) {
      $result .= $toc->number_separator . _tableofcontents_convert_number($mode, 0);
    }
    break;

  }

  // we add a space at the end (before the title)
  $output = $toc->number_start_letter . $result . $toc->number_end_letter . ' ';
  return theme('tableofcontents_number_text', $output);
}

/**
 * Basic formatting of the text generated by the
 * theme_tableofcontents_number() function. This is the
 * number in the form of a string.
 *
 * @param $text
 *   The text to format
 *
 * @return
 *   The resulting text
 */
function theme_tableofcontents_number_text($text) {
  return $text;
}

// vim: ts=2 sw=2 et syntax=php
