<?php
/**
 * @file query.inc
 * Defines the query object which is the underlying layer in a View.
 */

/**
 * Object used to create a SELECT query.
 */
class views_query {

  /**
   * A list of tables in the order they should be added, keyed by alias.
   */
  var $table_queue = array();

  /**
   * Holds an array of tables and counts added so that we can create aliases
   */
  var $tables = array();

  /**
   * Holds an array of relationships, which are aliases of the primary
   * table that represent different ways to join the same table in.
   */
  var $relationships = array();

  /**
   * An array of sections of the WHERE query. Each section is in itself
   * an array of pieces and a flag as to whether or not it should be AND
   * or OR.
   */
  var $where = array();
  /**
   * An array of sections of the HAVING query. Each section is in itself
   * an array of pieces and a flag as to whether or not it should be AND
   * or OR.
   */
  var $having = array();
  /**
   * The default operator to use when connecting the WHERE groups. May be
   * AND or OR.
   */
  var $group_operator = 'AND';

  /**
   * A simple array of order by clauses.
   */
  var $orderby = array();

  /**
   * A simple array of group by clauses.
   */
  var $groupby = array();

  /**
   * The table header to use for tablesort. This matters because tablesort
   * needs to modify the query and needs the header.
   */
  var $header = array();

  /**
   * A flag as to whether or not to make the primary field distinct.
   */
  var $distinct = FALSE;

  /**
   * Constructor; Create the basic query object and fill with default values.
   */
  function __construct($base_table = 'node', $base_field = 'nid') {
    $this->base_table = $base_table;  // Predefine these above, for clarity.
    $this->base_field = $base_field;
    $this->relationships[$base_table] = array(
      'link' => NULL,
      'table' => $base_table,
      'alias' => $base_table,
      'base' => $base_table
    );

    // init the table queue with our primary table.
    $this->table_queue[$base_table] = array(
      'alias' => $base_table,
      'table' => $base_table,
      'relationship' => $base_table,
      'join' => NULL,
    );

    // init the tables with our primary table
    $this->tables[$base_table][$base_table] = array(
      'count' => 1,
      'alias' => $base_table,
    );

    if ($base_field) {
      $this->fields[$base_field] = array(
        'table' => $base_table,
        'field' => $base_field,
        'alias' => $base_field,
      );
    }

    $this->count_field = array(
      'table' => $base_table,
      'field' => $base_field,
      'alias' => $base_field,
      'count' => TRUE,
    );
  }

  function views_query($base_table = 'node', $base_field = 'nid') {
    self::__construct($base_table, $base_field);
  }

  // ----------------------------------------------------------------
  // Utility methods to set flags and data.

  /**
   * Set the base field to be distinct.
   */
  function set_distinct($value = TRUE) {
    if (!(isset($this->no_distinct) && $value)) {
      $this->distinct = $value;
    }
  }

  /**
   * Set what field the query will count() on for paging.
   */
  function set_count_field($table, $field, $alias = NULL) {
    if (empty($alias)) {
      $alias = $table . '_' . $field;
    }
    $this->count_field = array(
      'table' => $table,
      'field' => $field,
      'alias' => $alias,
      'count' => TRUE,
    );
  }

  /**
   * Set the table header; used for click-sorting because it's needed
   * info to modify the ORDER BY clause.
   */
  function set_header($header) {
    $this->header = $header;
  }

  // ----------------------------------------------------------------
  // Table/join adding

  /**
   * A relationship is an alternative endpoint to a series of table
   * joins. Relationships must be aliases of the primary table and
   * they must join either to the primary table or to a pre-existing
   * relationship.
   *
   * An example of a relationship would be a nodereference table.
   * If you have a nodereference named 'book_parent' which links to a
   * parent node, you could set up a relationship 'node_book_parent'
   * to 'node'. Then, anything that links to 'node' can link to
   * 'node_book_parent' instead, thus allowing all properties of
   * both nodes to be available in the query.
   *
   * @param $alias
   *   What this relationship will be called, and is also the alias
   *   for the table.
   * @param $join
   *   A views_join object (or derived object) to join the alias in.
   * @param $base
   *   The name of the 'base' table this relationship represents; this
   *   tells the join search which path to attempt to use when finding
   *   the path to this relationship.
   * @param $link_point
   *   If this relationship links to something other than the primary
   *   table, specify that table here. For example, a 'track' node
   *   might have a relationship to an 'album' node, which might
   *   have a relationship to an 'artist' node.
   */
  function add_relationship($alias, $join, $base, $link_point = NULL) {
    if (empty($link_point)) {
      $link_point = $this->base_table;
    }
    else if (!array_key_exists($link_point, $this->relationships)) {
      return FALSE;
    }

    // Make sure $alias isn't already used; if it, start adding stuff.
    $alias_base = $alias;
    $count = 1;
    while (!empty($this->relationships[$alias])) {
      $alias = $alias_base . '_' . $count++;
    }

    // Make sure this join is adjusted for our relationship.
    if ($link_point && isset($this->relationships[$link_point])) {
      $join = $this->adjust_join($join, $link_point);
    }

    // Add the table directly to the queue to avoid accidentally marking
    // it.
    $this->table_queue[$alias] = array(
      'table' => $join->table,
      'num' => 1,
      'alias' => $alias,
      'join' => $join,
      'relationship' => $link_point,
    );

    $this->relationships[$alias] = array(
      'link' => $link_point,
      'table' => $join->table,
      'base' => $base,
    );

    $this->tables[$this->base_table][$alias] = array(
      'count' => 1,
      'alias' => $alias,
    );

    return $alias;
  }

  /**
   * Add a table to the query, ensuring the path exists.
   *
   * This function will test to ensure that the path back to the primary
   * table is valid and exists; if you do not wish for this testing to
   * occur, use $query->queue_table() instead.
   *
   * @param $table
   *   The name of the table to add. It needs to exist in the global table
   *   array.
   * @param $relationship
   *   An alias of a table; if this is set, the path back to this table will
   *   be tested prior to adding the table, making sure that all intermediary
   *   tables exist and are properly aliased. If set to NULL the path to
   *   the primary table will be ensured. If the path cannot be made, the
   *   table will NOT be added.
   * @param $join
   *   In some join configurations this table may actually join back through
   *   a different method; this is most likely to be used when tracing
   *   a hierarchy path. (node->parent->parent2->parent3). This parameter
   *   will specify how this table joins if it is not the default.
   * @param $alias
   *   A specific alias to use, rather than the default alias.
   *
   * @return $alias
   *   The alias of the table; this alias can be used to access information
   *   about the table and should always be used to refer to the table when
   *   adding parts to the query. Or FALSE if the table was not able to be
   *   added.
   */
  function add_table($table, $relationship = NULL, $join = NULL, $alias = NULL) {
    if (!$this->ensure_path($table, $relationship, $join)) {
      return FALSE;
    }

    if ($join && $relationship) {
      $join = $this->adjust_join($join, $relationship);
    }

    return $this->queue_table($table, $relationship, $join, $alias);
  }

  /**
   * Add a table to the query without ensuring the path.
   *
   * This is a pretty internal function to Views and add_table() or
   * ensure_table() should be used instead of this one, unless you are
   * absolutely sure this is what you want.
   *
   * @param $table
   *   The name of the table to add. It needs to exist in the global table
   *   array.
   * @param $relationship
   *   The primary table alias this table is related to. If not set, the
   *   primary table will be used.
   * @param $join
   *   In some join configurations this table may actually join back through
   *   a different method; this is most likely to be used when tracing
   *   a hierarchy path. (node->parent->parent2->parent3). This parameter
   *   will specify how this table joins if it is not the default.
   * @param $alias
   *   A specific alias to use, rather than the default alias.
   *
   * @return $alias
   *   The alias of the table; this alias can be used to access information
   *   about the table and should always be used to refer to the table when
   *   adding parts to the query. Or FALSE if the table was not able to be
   *   added.
   */
  function queue_table($table, $relationship = NULL, $join = NULL, $alias = NULL) {
    // If the alias is set, make sure it doesn't already exist.
    if (isset($this->table_queue[$alias])) {
      return $alias;
    }

    if (empty($relationship)) {
      $relationship = $this->base_table;
    }

    if (!array_key_exists($relationship, $this->relationships)) {
      return FALSE;
    }

    if (!$alias && $join && $relationship && !empty($join->adjusted) && $table != $join->table) {
      if ($relationship == $this->base_table) {
        $alias = $table;
      }
      else {
        $alias = $relationship . '_' . $table;
      }
    }

    // Check this again to make sure we don't blow up existing aliases for already
    // adjusted joins.
    if (isset($this->table_queue[$alias])) {
      return $alias;
    }

    $alias = $this->mark_table($table, $relationship, $alias);

    // If no alias is specified, give it the default.
    if (!isset($alias)) {
      $alias = $this->tables[$relationship][$table]['alias'] . $this->tables[$relationship][$table]['count'];
    }

    // If this is a relationship based table, add a marker with
    // the relationship as a primary table for the alias.
    if ($table != $alias) {
      $this->mark_table($alias, $this->base_table, $alias);
    }

    // If no join is specified, pull it from the table data.
    if (!isset($join)) {
      $join = $this->get_join_data($table, $this->relationships[$relationship]['base']);
      if (empty($join)) {
        return FALSE;
      }

      $join = $this->adjust_join($join, $relationship);
    }

    $this->table_queue[$alias] = array(
      'table' => $table,
      'num' => $this->tables[$relationship][$table]['count'],
      'alias' => $alias,
      'join' => $join,
      'relationship' => $relationship,
    );

    return $alias;
  }

  function mark_table($table, $relationship, $alias) {
    // Mark that this table has been added.
    if (empty($this->tables[$relationship][$table])) {
      if (!isset($alias)) {
        $alias = '';
        if ($relationship != $this->base_table) {
          // double underscore will help prevent accidental name
          // space collisions.
          $alias = $relationship . '__';
        }
        $alias .= $table;
      }
      $this->tables[$relationship][$table] = array(
        'count' => 1,
        'alias' => $alias,
      );
    }
    else {
      $this->tables[$relationship][$table]['count']++;
    }

    return $alias;
  }

  /**
   * Ensure a table exists in the queue; if it already exists it won't
   * do anything, but if it doesn't it will add the table queue. It will ensure
   * a path leads back to the relationship table.
   *
   * @param $table
   *   The unaliased name of the table to ensure.
   * @param $relationship
   *   The relationship to ensure the table links to. Each relationship will
   *   get a unique instance of the table being added. If not specified,
   *   will be the primary table.
   * @param $join
   *   A views_join object (or derived object) to join the alias in.
   *
   * @return
   *   The alias used to refer to this specific table, or NULL if the table
   *   cannot be ensured.
   */
  function ensure_table($table, $relationship = NULL, $join = NULL) {
    // ensure a relationship
    if (empty($relationship)) {
      $relationship = $this->base_table;
    }

    // If the relationship is the primary table, this actually be a relationship
    // link back from an alias. We store all aliases along with the primary table
    // to detect this state, because eventually it'll hit a table we already
    // have and that's when we want to stop.
    if ($relationship == $this->base_table && !empty($this->tables[$relationship][$table])) {
      return $this->tables[$relationship][$table]['alias'];
    }

    if (!array_key_exists($relationship, $this->relationships)) {
      return FALSE;
    }

    if ($table == $this->relationships[$relationship]['base']) {
      return $relationship;
    }

    // If we do not have join info, fetch it.
    if (!isset($join)) {
      $join = $this->get_join_data($table, $this->relationships[$relationship]['base']);
    }

    // If it can't be fetched, this won't work.
    if (empty($join)) {
      return;
    }

    // Adjust this join for the relationship, which will ensure that the 'base'
    // table it links to is correct. Tables adjoined to a relationship
    // join to a link point, not the base table.
    $join = $this->adjust_join($join, $relationship);

    if ($this->ensure_path($table, $relationship, $join)) {
      // Attempt to eliminate redundant joins.  If this table's
      // relationship and join exactly matches an existing table's
      // relationship and join, we do not have to join to it again;
      // just return the existing table's alias.  See
      // http://groups.drupal.org/node/11288 for details.
      //
      // This can be done safely here but not lower down in
      // queue_table(), because queue_table() is also used by
      // add_table() which requires the ability to intentionally add
      // the same table with the same join multiple times.  For
      // example, a view that filters on 3 taxonomy terms using AND
      // needs to join term_data 3 times with the same join.

      // scan through the table queue to see if a matching join and
      // relationship exists.  If so, use it instead of this join.

      // TODO: Scanning through $this->table_queue results in an
      // O(N^2) algorithm, and this code runs every time the view is
      // instantiated (Views 2 does not currently cache queries).
      // There are a couple possible "improvements" but we should do
      // some performance testing before picking one.
      foreach ($this->table_queue as $queued_table) {
        // In PHP 4 and 5, the == operation returns TRUE for two objects
        // if they are instances of the same class and have the same
        // attributes and values.
        if ($queued_table['relationship'] == $relationship && $queued_table['join'] == $join) {
          return $queued_table['alias'];
        }
      }

      return $this->queue_table($table, $relationship, $join);
    }
  }

  /**
   * Make sure that the specified table can be properly linked to the primary
   * table in the JOINs. This function uses recursion. If the tables
   * needed to complete the path back to the primary table are not in the
   * query they will be added, but additional copies will NOT be added
   * if the table is already there.
   */
  function ensure_path($table, $relationship = NULL, $join = NULL, $traced = array(), $add = array()) {
    if (!isset($relationship)) {
      $relationship = $this->base_table;
    }

    if (!array_key_exists($relationship, $this->relationships)) {
      return FALSE;
    }

    // If we do not have join info, fetch it.
    if (!isset($join)) {
      $join = $this->get_join_data($table, $this->relationships[$relationship]['base']);
    }

    // If it can't be fetched, this won't work.
    if (empty($join)) {
      return FALSE;
    }

    // Does a table along this path exist?
    if (isset($this->tables[$relationship][$table]) ||
      ($join && $join->left_table == $relationship) ||
      ($join && $join->left_table == $this->relationships[$relationship]['table'])) {

      // Make sure that we're linking to the correct table for our relationship.
      foreach (array_reverse($add) as $table => $path_join) {
        $this->queue_table($table, $relationship, $this->adjust_join($path_join, $relationship));
      }
      return TRUE;
    }

    // Have we been this way?
    if (isset($traced[$join->left_table])) {
      // we looped. Broked.
      return FALSE;
    }

    // Do we have to add this table?
    $left_join = $this->get_join_data($join->left_table, $this->relationships[$relationship]['base']);
    if (!isset($this->tables[$relationship][$join->left_table])) {
      $add[$join->left_table] = $left_join;
    }

    // Keep looking.
    $traced[$join->left_table] = TRUE;
    return $this->ensure_path($join->left_table, $relationship, $left_join, $traced, $add);
  }

  /**
   * Fix a join to adhere to the proper relationship; the left table can vary
   * based upon what relationship items are joined in on.
   */
  function adjust_join($join, $relationship) {
    if (!empty($join->adjusted)) {
      return $join;
    }

    if (empty($relationship) || empty($this->relationships[$relationship])) {
      return $join;
    }

    // Adjusts the left table for our relationship.
    if ($relationship != $this->base_table) {
      // If we're linking to the primary table, the relationship to use will
      // be the prior relationship. Unless it's a direct link.

      // Safety! Don't modify an original here.
      $join = drupal_clone($join);

      // Do we need to try to ensure a path?
      if ($join->left_table != $this->relationships[$relationship]['table'] &&
          $join->left_table != $this->relationships[$relationship]['base'] &&
          !isset($this->tables[$relationship][$join->left_table]['alias'])) {
        $this->ensure_table($join->left_table, $relationship);
      }

      // First, if this is our link point/anchor table, just use the relationship
      if ($join->left_table == $this->relationships[$relationship]['table']) {
        $join->left_table = $relationship;
      }
      // then, try the base alias.
      else if (isset($this->tables[$relationship][$join->left_table]['alias'])) {
        $join->left_table = $this->tables[$relationship][$join->left_table]['alias'];
      }
      // But if we're already looking at an alias, use that instead.
      else if (isset($this->table_queue[$relationship]['alias'])) {
        $join->left_table = $this->table_queue[$relationship]['alias'];
      }
    }

    $join->adjusted = TRUE;
    return $join;
  }

  /**
   * Retrieve join data from the larger join data cache.
   *
   * @param $table
   *   The table to get the join information for.
   * @param $base_table
   *   The path we're following to get this join.
   *
   * @return
   *   A views_join object or child object, if one exists.
   */
  function get_join_data($table, $base_table) {
    // Check to see if we're linking to a known alias. If so, get the real
    // table's data instead.
    if (!empty($this->table_queue[$table])) {
      $table = $this->table_queue[$table]['table'];
    }
    return views_get_table_join($table, $base_table);

  }

  /**
   * Get the information associated with a table.
   *
   * If you need the alias of a table with a particular relationship, use
   * ensure_table().
   */
  function get_table_info($table) {
    if (!empty($this->table_queue[$table])) {
      return $this->table_queue[$table];
    }

    // In rare cases we might *only* have aliased versions of the table.
    if (!empty($this->tables[$this->base_table][$table])) {
      $alias = $this->tables[$this->base_table][$table]['alias'];
      if (!empty($this->table_queue[$alias])) {
        return $this->table_queue[$alias];
      }
    }
  }

  /**
   * Add a field to the query table, possibly with an alias. This will
   * automatically call ensure_table to make sure the required table
   * exists, *unless* $table is unset.
   *
   * @param $table
   *   The table this field is attached to. If NULL, it is assumed this will
   *   be a formula; otherwise, ensure_table is used to make sure the
   *   table exists.
   * @param $field
   *   The name of the field to add. This may be a real field or a formula.
   * @param $alias
   *   The alias to create. If not specified, the alias will be $table_$field
   *   unless $table is NULL. When adding formulae, it is recommended that an
   *   alias be used.
   *
   * @return $name
   *   The name that this field can be referred to as. Usually this is the alias.
   */
  function add_field($table, $field, $alias = '', $params = NULL) {
    // We check for this specifically because it gets a special alias.
    if ($table == $this->base_table && $field == $this->base_field && empty($alias)) {
      $alias = $this->base_field;
    }

    if ($table && empty($this->table_queue[$table])) {
      $this->ensure_table($table);
    }

    if (!$alias && $table) {
      $alias = $table . '_' . $field;
    }

    $name = $alias ? $alias : $field;

    // @todo FIXME -- $alias, then $name is inconsistent
    if (empty($this->fields[$alias])) {
      $this->fields[$name] = array(
        'field' => $field,
        'table' => $table,
        'alias' => $alias,
      );
    }

    foreach ((array)$params as $key => $value) {
      $this->fields[$name][$key] = $value;
    }

    return $name;
  }

  /**
   * Remove all fields that may've been added; primarily used for summary
   * mode where we're changing the query because we didn't get data we needed.
   */
  function clear_fields() {
    $this->fields = array();
  }

  /**
   * Create a new grouping for the WHERE or HAVING clause.
   *
   * @param $type
   *   Either 'AND' or 'OR'. All items within this group will be added
   *   to the WHERE clause with this logical operator.
   * @param $group
   *   An ID to use for this group. If unspecified, an ID will be generated.
   * @param $where
   *   'where' or 'having'.
   *
   * @return $group
   *   The group ID generated.
   */
  function set_where_group($type = 'AND', $group = NULL, $where = 'where') {
    // Set an alias.
    $groups = &$this->$where;

    if (!isset($group)) {
      $group = empty($groups) ? 1 : max(array_keys($groups)) + 1;
    }

    // Create an empty group
    if (empty($groups[$group])) {
      $groups[$group] = array('clauses' => array(), 'args' => array());
    }

    $groups[$group]['type'] = strtoupper($type);
    return $group;
  }

  /**
   * Control how all WHERE and HAVING groups are put together.
   *
   * @param $type
   *   Either 'AND' or 'OR'
   */
  function set_group_operator($type = 'AND') {
    $this->group_operator = strtoupper($type);
  }

  /**
   * Add a simple WHERE clause to the query. The caller is responsible for
   * ensuring that all fields are fully qualified (TABLE.FIELD) and that
   * the table already exists in the query.
   *
   * @param $group
   *   The WHERE group to add these to; groups are used to create AND/OR
   *   sections. Groups cannot be nested. Use 0 as the default group.
   *   If the group does not yet exist it will be created as an AND group.
   * @param $clause
   *   The actual clause to add. When adding a where clause it is important
   *   that all tables are addressed by the alias provided by add_table or
   *   ensure_table and that all fields are addressed by their alias wehn
   *   possible. Please use %d and %s for arguments.
   * @param ...
   *   A number of arguments as used in db_query(). May be many args or one
   *   array full of args.
   */
  function add_where($group, $clause) {
    $args = func_get_args();
    array_shift($args); // ditch $group
    array_shift($args); // ditch $clause

    // Expand an array of args if it came in.
    if (count($args) == 1 && is_array(reset($args))) {
      $args = current($args);
    }

    // Ensure all variants of 0 are actually 0. Thus '', 0 and NULL are all
    // the default group.
    if (empty($group)) {
      $group = 0;
    }

    // Check for a group.
    if (!isset($this->where[$group])) {
      $this->set_where_group('AND', $group);
    }

    // Add the clause and the args.
    if (is_array($args)) {
      $this->where[$group]['clauses'][] = $clause;
      // we use array_values() here to prevent array_merge errors as keys from multiple
      // sources occasionally collide.
      $this->where[$group]['args'] = array_merge($this->where[$group]['args'], array_values($args));
    }
  }

  /**
   * Add a simple HAVING clause to the query. The caller is responsible for
   * ensuring that all fields are fully qualified (TABLE.FIELD) and that
   * the table and an appropriate GROUP BY already exist in the query.
   *
   * @param $group
   *   The HAVING group to add these to; groups are used to create AND/OR
   *   sections. Groups cannot be nested. Use 0 as the default group.
   *   If the group does not yet exist it will be created as an AND group.
   * @param $clause
   *   The actual clause to add. When adding a having clause it is important
   *   that all tables are addressed by the alias provided by add_table or
   *   ensure_table and that all fields are addressed by their alias wehn
   *   possible. Please use %d and %s for arguments.
   * @param ...
   *   A number of arguments as used in db_query(). May be many args or one
   *   array full of args.
   */
  function add_having($group, $clause) {
    $args = func_get_args();
    array_shift($args); // ditch $group
    array_shift($args); // ditch $clause

    // Expand an array of args if it came in.
    if (count($args) == 1 && is_array(reset($args))) {
      $args = current($args);
    }

    // Ensure all variants of 0 are actually 0. Thus '', 0 and NULL are all
    // the default group.
    if (empty($group)) {
      $group = 0;
    }

    // Check for a group.
    if (!isset($this->having[$group])) {
      $this->set_where_group('AND', $group, 'having');
    }

    // Add the clause and the args.
    if (is_array($args)) {
      $this->having[$group]['clauses'][] = $clause;
      $this->having[$group]['args'] = array_merge($this->having[$group]['args'], array_values($args));
    }
  }

  /**
   * Add an ORDER BY clause to the query.
   *
   * @param $table
   *   The table this field is part of. If a formula, enter NULL.
   * @param $field
   *   The field or formula to sort on. If already a field, enter NULL
   *   and put in the alias.
   * @param $order
   *   Either ASC or DESC.
   * @param $alias
   *   The alias to add the field as. In SQL, all fields in the order by
   *   must also be in the SELECT portion. If an $alias isn't specified
   *   one will be generated for from the $field; however, if the
   *   $field is a formula, this alias will likely fail.
   */
  function add_orderby($table, $field, $order, $alias = '') {
    if ($table) {
      $this->ensure_table($table);
    }

    // Only fill out this aliasing if there is a table;
    // otherwise we assume it is a formula.
    if (!$alias && $table) {
      $as = $table . '_' . $field;
    }
    else {
      $as = $alias;
    }

    if ($field) {
      $this->add_field($table, $field, $as);
    }

    $this->orderby[] = "$as " . strtoupper($order);

    // If grouping, all items in the order by must also be in the
    // group by clause. Check $table to ensure that this is not a
    // formula.
    if ($this->groupby && $table) {
      $this->add_groupby($as);
    }
  }

  /**
   * Add a simple GROUP BY clause to the query. The caller is responsible
   * for ensuring that the fields are fully qualified and the table is properly
   * added.
   */
  function add_groupby($clause) {
    // Only add it if it's not already in there.
    if (!in_array($clause, $this->groupby)) {
      $this->groupby[] = $clause;
    }
  }

  /**
   * Construct the "WHERE" or "HAVING" part of the query.
   *
   * @param $where
   *   'where' or 'having'.
   */
  function condition_sql($where = 'where') {
    $clauses = array();
    foreach ($this->$where as $group => $info) {
      $clause = implode(") " . $info['type'] . " (", $info['clauses']);
      if (count($info['clauses']) > 1) {
        $clause = '(' . $clause . ')';
      }
      $clauses[] = $clause;
    }

    if ($clauses) {
      $keyword = drupal_strtoupper($where);
      if (count($clauses) > 1) {
        return "$keyword (" . implode(")\n    " . $this->group_operator . ' (', $clauses) . ")\n";
      }
      else {
        return "$keyword " . array_shift($clauses) . "\n";
      }
    }
    return "";
  }

  /**
   * Generate a query and a countquery from all of the information supplied
   * to the object.
   *
   * @param $get_count
   *   Provide a countquery if this is true, otherwise provide a normal query.
   */
  function query($get_count = FALSE) {
    // Check query distinct value.
    if (empty($this->no_distinct) && $this->distinct && !empty($this->fields)) {
      if (!empty($this->fields[$this->base_field])) {
        $this->fields[$this->base_field]['distinct'] = TRUE;
        $this->add_groupby($this->base_field);
      }
    }

    /**
     * An optimized count query includes just the base field instead of all the fields.
     * Determine of this query qualifies by checking for a groupby or distinct.
     */
    $fields_array = $this->fields;
    if ($get_count && !$this->groupby) {
      foreach ($fields_array as $field) {
        if (!empty($field['distinct'])) {
          $get_count_optimized = FALSE;
          break;
        }
      }
    }
    else {
      $get_count_optimized = FALSE;
    }
    if (!isset($get_count_optimized)) {
      $get_count_optimized = TRUE;
    }

    $joins = $fields = $where = $having = $orderby = $groupby = '';
    // Add all the tables to the query via joins. We assume all LEFT joins.
    foreach ($this->table_queue as $table) {
      if (is_object($table['join'])) {
        $joins .= $table['join']->join($table, $this) . "\n";
      }
    }

    $has_aggregate = FALSE;
    $non_aggregates = array();

    foreach ($fields_array as $field) {
      if ($fields) {
        $fields .= ",\n   ";
      }
      $string = '';
      if (!empty($field['table'])) {
        $string .= $field['table'] . '.';
      }
      $string .= $field['field'];

      // store for use with non-aggregates below
      $fieldname = (!empty($field['alias']) ? $field['alias'] : $string);

      if (!empty($field['distinct'])) {
        $string = "DISTINCT($string)";
      }
      if (!empty($field['count'])) {
        $string = "COUNT($string)";
        $has_aggregate = TRUE;
      }
      else if (!empty($field['aggregate'])) {
        $has_aggregate = TRUE;
      }
      elseif ($this->distinct && !in_array($fieldname, $this->groupby)) {
        $string = $GLOBALS['db_type'] == 'pgsql' ? "FIRST($string)" : $string;
      }
      else {
        $non_aggregates[] = $fieldname;
      }
      if ($field['alias']) {
        $string .= " AS $field[alias]";
      }
      $fields .= $string;

      if ($get_count_optimized) {
        // We only want the first field in this case.
        break;
      }
    }

    if ($has_aggregate || $this->groupby) {
      $groupby = "GROUP BY " . implode(', ', array_unique(array_merge($this->groupby, $non_aggregates))) . "\n";
      if ($this->having) {
        $having = $this->condition_sql('having');
      }
    }

    if (!$get_count_optimized) {
      // we only add the groupby if we're not counting.
      if ($this->orderby) {
        $orderby = "ORDER BY " . implode(', ', $this->orderby) . "\n";
      }
    }

    $where = $this->condition_sql();

    $query = "SELECT $fields\n FROM {" . $this->base_table . "} $this->base_table \n$joins $where $groupby $having $orderby";

    $replace = array('&gt;' => '>', '&lt;' => '<');
    $query = strtr($query, $replace);

    return $query;
  }

  /**
   * Get the arguments attached to the WHERE and HAVING clauses of this query.
   */
  function get_where_args() {
    $args = array();
    foreach ($this->where as $group => $where) {
      $args = array_merge($args, $where['args']);
    }
    foreach ($this->having as $group => $having) {
      $args = array_merge($args, $having['args']);
    }
    return $args;
  }
}

