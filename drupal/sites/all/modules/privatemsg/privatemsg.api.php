<?php
// $Id: privatemsg.api.php,v 1.1.2.9 2009/11/06 13:06:26 berdir Exp $

/**
 * @file
 * Privatemsg API Documentation
 */

/**
 * @mainpage Privatemsg API Documentation
 * This is the (currently inofficial) API documentation of the privatemsg 6.x
 * API.
 *
 * - Topics:
 *  - @link api API functions @endlink
 *  - @link sql Query builder and related hooks @endlink
 *  - @link message_hooks Message hooks @endlink
 *  - @link generic_hooks Generic hooks @endlink
 *  - @link theming Theming @endlink
 */

/**
 * @defgroup sql Query Builder
 * Privatemsg does use its own simple query builder which allows to extend
 * SELECT-Queries in an easy way. The function _privatemsg_assemble_query
 * creates the query, based on an array $fragments with the following content.
 * Except primary_table, each key is an array itself to allow multiple values
 *
 *  - primary_table: The main table to select from
 *  - select: The fields that should be selected. This can be a simple field, a
 *    field with alias or even a subquery.
 *  - inner_join: The tables that should be joined. This is not specific to
 *    inner joins.
 *    Example: INNER JOIN pm_index pmi ON (pmi.mid = pm.mid)
 *  - where: The where conditions. The conditions are always AND, but it is
 *    possible to use OR inside a condition.
 *    Example: ⁽pmi.is_new = 1 OR pmi.deleted = 1)
 *  - order_by: Order By values, example: pm.timestamp ASC
 *  - query_args: It is possible to use the placeholders like %s in each part of
 *    the query. The values of query_args are then inserted into these.
 *    query_args consists of three arrays (join, where, having), one for each
 *    key that currently supports arguments.
 *
 * Use _privatemsg_assemble_query
 *
 * The privatemsg_assemble_query function takes a query_id as first argument
 * and optionally one or multiple arguments. query_id can either be a
 * string ('some_id') or an array('group_name', 'query_id'), if a string
 * is supplied, group_name defaults to 'privatemsg'. Returned is an array
 * with the keys 'query' (normal query) and 'count' (count query for pager).
 *
 * For the actual query data, the function group_name_sql_query_id is executed,
 * this functions does have $fragments as first parameter and then the
 * optional parameters.
 *
 * A short example:
 * @code
 * // First, create the sql function.
 * function privatemsg_sql_getsubject(&$fragments, $mid, $uid) {
 *   // Set the primary table.
 *   $fragments['primary_table'] = '{pm_message} pm';
 *
 *   // Add a field.
 *   $fragments['select'][] = 'pm.subject';
 *
 *   // Join another table.
 *   $fragment['inner_join'][] = 'JOIN {pm_index} pi ON (pi.mid = pm.mid)';
 *   $fragment['query_args']['join'][] $uid;
 *
 *   // And finally add a condition.
 *   $fragments['where'][] = 'pm.mid = %d';
 *   $fragments['query_args']['where'][] = $mid;
 * }
 *
 * // Now we can use that query everywhere.
 * $query = _privatemsg_assemble_query('getsubject', 5);
 * $result = db_query($query['query']);
 * @endcode
 * Extend existing queries
 *
 * To extend a privatemsg query, use hook_group_name_sql_query_id_alter.
 * This hook does use the same parameters as the sql function.
 *
 * Example:
 * @code
 * function mymodule_privatemsg_sql_getsubject_alter(&$fragments, $mid) {
 * // we want to load the body too..
 *  $fragments['select'][] = 'pm.body';
 * }
 * @endcode
 *
 * - List of sql query hooks.
 *  - list: List of messages, Parameters: $fragments, $accounty
 *  - list_sent: List of sent messages, Parameters: $fragments, $accounty
 *  - load: Load a single message, Parameters: $fragments, $pmid, $account
 *  - messages: Load the messages of a thread,
 *    Parameters: $fragments, $thread_id, $account
 *  - autocomplete: Searching usernames for the autocomplete feature,
 *    Parameters: $fragments, $search, $names
 *  - participants: Loads all participants of a thread,
 *    Parameters: $fragments, $thread_id
 *  - unread_count: Number of unread messages for a user,
 *    Parameters: $fragments, $account
 *
 * - The following query_id's are used in pm_block_user
 *  - threadautors: Return all authors of one or multiple threads,
 *    Parameters: $fragments, $threads
 */

/**
 * @addtogroup sql
 * @{
 */

/**
 * Query to search for autocomplete usernames.
 *
 * @param $fragments
 *   Query fragments
 * @param $search
 *   Search for that username
 * @param $names
 *   Names that are already in the list and are excluded
 */
function hook_privatemsg_sql_autocomplete_alter(&$fragments, $search, $names) {
  global $user;
  // Extend the query that searches for usernames

  // $fragments is explained in the api documentation in detail

  // The query is already set up, it's searching for usernames which start with
  // $search and are not $names (may be empty)
  // the main table is {user} a

  // for example, add a join on a table where the user connections are stored
  // and specify that only users connected with the current user should be
  // loaded.
  $fragments['inner_join'] = 'INNER JOIN {my_table} m ON (m.user1 = u.uid AND m.user2 = %d)';
  $fragments['query_args'][] = $user->uid;
}

/**
 * Display a list of threads.
 *
 * @param $fragments
 *   Query fragments
 * @param $account
 *   User object
 */
function hook_privatemsg_sql_list_alter(&$fragment, $account) {

}

/**
 * Query definition to load a message.
 *
 * @param $fragments
 *   Query fragments array.
 * @param $pmid
 *   the id of the message.
  * @param $account
 *   User object of account for which to load the message.
 */
function hook_privatemsg_sql_load_alter(&$fragments, $pmid, $account = NULL) {

}

/**
 * Query definition to load messages of one or multiple threads.
 *
 * @param $fragments
 *   Query fragments array.
 * @param $threads
 *   Array with one or multiple thread id's.
 * @param $account
 *   User object for which the messages are being loaded.
 * @param $load_all
 *   Deleted messages are only loaded if this is set to TRUE.
 */
function hook_privatemsg_sql_messages_alter(&$fragments, $threads, $account = NULL, $load_all = FALSE) {

}

/**
 * Load the participants of a thread.
 *
 * @param $fragments
 *   Query fragments
 * @param $thread_id
 *   Thread id, pmi.thread_id is the same as the mid of the first
 *   message of that thread
 */
function hook_privatemsg_sql_participants_alter(&$fragment, $thread_id) {

}

/**
 * Loads all unread messages of a user (only the count query is used).
 *
 * @param $fragments
 *   Query fragments
 * @param $account
 *   User object
 */
function hook_privatemsg_sql_unread_count_alter(&$fragment, $account) {

}

/**
 * @}
 */

/**
 * @defgroup api API functions
 *
 * There are two different functions to send messages.
 * Either by starting a @link privatemsg_new_thread new thread @endlink
 * or @link privatemsg_reply reply @endlink to an existing thread.
 *
 * There is also a function which returns a link to the privatemsg new message
 * form with the recipient pre-filled if the user is allowed to.
 * privatemsg_get_link().
 */

/**
 * @defgroup message_hooks Message hooks
 * All message-level hooks look like hook_privatemsg_message_op,
 * where op is one of the following:
 * - @link hook_privatemsg_message_load load @endlink: Called when a full
 *   message is loaded similiar to nodeapi_load, new values can be returned and
 *   will be added to $message, parameter: $message
 * - @link hook_privatemsg_message_validate validate @endlink: Validation,
 *   before the message is sent/saved. Return validation errors as array,
 *   parameter: $message, $form = FALSE
 * - @link hook_privatemsg_message_presave_alter presave_alter @endlink: Last
 *   changes to $message before the message is saved, parameter: $message
 * - @link hook_privatemsg_message_insert insert @endlink: message has been
 *   saved, $message has been updated with the mid and thread_id,
 *   parameter: $message
 * - @link hook_privatemsg_message_delete delete @endlink: the message is
 *   going to be deleted, parameter: $message
 * - @link hook_privatemsg_message_view_alter view_alter @endlink: the message
 *   is going to be displayed, parameter: $vars
 *
 * In hooks with _alter suffix, $message is by reference.
 *
 * $message is an array, with all the relevant information about the message.
 * The information in there can be changed/extended by modules, but looks
 * typically like this:
 * @code
 * array (
 *   'mid' => 3517, // message id, identifies a message
 *   'author' => 27, // author id
 *   'subject' => 'raclebugav', // Message subject
 *   'body' => 'bla bla', // Body of the message
 *   'timestamp' => 351287003, // unix timestamp, creation time
 *   'is_new' => 0, // If the message has been read by the user
 *   'thread_id' => 3341, // thread id, this is actually the mid from the first
 *                           message of the thread
 * )
 * @endcode
 */

/**
 * @addtogroup message_hooks
 * @{
 */

/**
 * Is called after the message has been loaded.
 *
 * Return data will be merged with the $message array.
 *
 * @param $message
 *    Message array
 */
function hook_privatemsg_message_load($message) {
  return array('my_key' => 'my_value');
}

/**
 * Is called when a message is flushed.
 *
 * The message will be deleted from the database, remove any related data here.
 *
 * @param $message
 *   Message array
 */
function hook_privatemsg_message_flush($message) {

}

/**
 * Validate a message before it is sent/saved in the database.
 *
 * Validation errors can be returned, either as a string or as array when there
 * are multiple errors. If the $form flag is set, errors should be reported
 * with form_set_error instead.
 *
 * @todo adapt api return value changes
 *
 * @param $message
 *   Message array
 */
function hook_privatemsg_message_validate($message, $form = FALSE) {
  global $_privatemsg_invalid_recipients;
  $_privatemsg_invalid_recipients = array();

  $errors = array();

  foreach ($message['recipients'] as $recipient) {
    if ($recipient->name == 'blocked user') {
      $_privatemsg_invalid_recipients[] = $recipient->uid;
      $errors[] = t('%name has chosen to not recieve any more messages from you.', array('%name' => $recipient->name));
    }
  }
}

/**
 * Change the message before it is stored.
 *
 * Alter the message, for example remove recipients that have been detected as
 * invalid or forbidden in the validate hook.
 *
 * @param $message
 *   Message array
 */
function hook_privatemsg_message_presave_alter(&$message) {
  // delete recipients which have been marked as invalid
  global $_privatemsg_invalid_recipients;
  foreach ($_privatemsg_invalid_recipients as $invalid) {
    unset($message['recipients'][$invalid]);
  }
}
/**
 * Act on the $vars before a message is displayed.
 *
 * This is called in the preprocess hook of the privatemsg-view template.
 * The $message data is aviable in $vars['message'].
 *
 * @param $var
 *   Template variables
 */
function hook_privatemsg_message_view_alter(&$var) {
  // add a link to each message
  $vars['message_links'][] = array('title' => t('My link'), 'href' => '/path/to/my/action/'. $vars['message']['mid']);
}

/**
 * This hook is executed after the message has been saved.
 *
 * $message is updated with mid and thread id. Use this hook to store data,
 * that needs to point to the saved message for example attachments.
 *
 * @param $message
 *   Message array
 */
function hook_privatemsg_message_insert($message) {
  _mymodule_save_data($message['mid']);
}

/**
 * @}
 */

/**
 * @defgroup generic_hooks Generic Hooks
 * @{
 *
 * Some generic hooks that can't be categorized.
 */

/**
 * Check if $author can send $recipient a message.
 *
 * Return a message if it is not alled, nothing if it is. This hook is executed
 * for each recipient.
 *
 * @param $author
 *   Author of the message to be sent
 * @param $recipient
 *   Recipient of the message
 */
function hook_privatemsg_block_message($author, $recipient) {

}
/**
 * Add content to the view thread page.
 *
 * Each element in content contains a 'value' and a '#weight' key, the weight
 * is used set the order of the different parts.
 *
 * @param $content
 *   Content array
 * @param $message_count
 *   Amount of messages
 */
function hook_privatemsg_view_messages($content, $message_count) {

}

/**
 * List of possible templates.
 */
function hook_privatemsg_view_template() {

}

/**
 * Expose operations/actions which can be executed on threads.
 *
 * Return an array of operations to privatemsg, the key of each operation is the
 * operation key or name.
 *
 * @see _privatemsg_action_form()
 * @see privatemsg_list_submit()
 */
function hook_privatemsg_thread_operations() {
  return array(
    'operation key' => array(
      'label' => 'Label of the operation. Only use this if the operation
                  should be displayed automatically in the action form',
      'callback' => 'privatemsg_thread_change_status', // Function callback that will be executed.
      'callback arguments' => array('status' => PRIVATEMSG_READ), // Additional arguments to above function
      'undo callback' => 'privatemsg_thread_change_status',  // Provide a function which can "undo" the operation. Optional.
      'undo callback arguments' => array('status' => PRIVATEMSG_UNREAD), // Additional arguments to above function.
    ),
  );
}

/**
 * Hook which allows to look up a user object.
 *
 * You can try to look up a user object based on the information passed to the
 * hook. The first hook that successfully looks up a specific string wins.
 *
 * Therefore, it is important to only return something if you can actually look
 * up the string.
 */
function hook_privatemsg_name_lookup($string) {
  if ((int)$string > 0) {
    // This is a possible uid, try to load a matching user.
    if ($recipient = user_load(array('uid' => $string))) {
      return $recipient;
    }
  }
}

/**
 * @}
 */