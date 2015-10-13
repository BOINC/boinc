<?php
// $Id: search-result.tpl.php,v 1.1.2.1 2008/08/28 08:21:44 dries Exp $

/**
 * @file search-result.tpl.php
 * Default theme implementation for displaying a single search result.
 *
 * This template renders a single search result and is collected into
 * search-results.tpl.php. This and the parent template are
 * dependent to one another sharing the markup for definition lists.
 *
 * Available variables:
 * - $url: URL of the result.
 * - $title: Title of the result.
 * - $snippet: A small preview of the result. Does not apply to user searches.
 * - $info: String of all the meta information ready for print. Does not apply
 *   to user searches.
 * - $info_split: Contains same data as $info, split into a keyed array.
 * - $type: The type of search, e.g., "node" or "user".
 *
 * Default keys within $info_split:
 * - $info_split['type']: Node type.
 * - $info_split['user']: Author of the node linked to users profile. Depends
 *   on permission.
 * - $info_split['date']: Last update of the node. Short formatted.
 * - $info_split['comment']: Number of comments output as "% comments", %
 *   being the count. Depends on comment.module.
 * - $info_split['upload']: Number of attachments output as "% attachments", %
 *   being the count. Depends on upload.module.
 *
 * Since $info_split is keyed, a direct print of the item is possible.
 * This array does not apply to user searches so it is recommended to check
 * for their existance before printing. The default keys of 'type', 'user' and
 * 'date' always exist for node searches. Modules may provide other data.
 *
 *   <?php if (isset($info_split['comment'])) : ?>
 *     <span class="info-comment">
 *       <?php print $info_split['comment']; ?>
 *     </span>
 *   <?php endif; ?>
 *
 * To check for all available data within $info_split, use the code below.
 *
 *   <?php print '<pre>'. check_plain(print_r($info_split, 1)) .'</pre>'; ?>
 *
 * @see template_preprocess_search_result()
 */
 
 global $base_path;
 
?>
<?php switch ($info_split['type']): ?>
<?php
  case 'Profile':
  case 'profile':
    $parsed_url = parse_url($url);
    $base_length = strlen($base_path);
    $core_path = trim(substr($parsed_url['path'], $base_length), '/');
    $path = drupal_lookup_path('source', $core_path);
    $matches = array();
    if (preg_match('/node\/([0-9]+)/', $path, $matches)) {
      $node = node_load($matches[1]);
      $account = user_load($node->uid);
      $user_image = boincuser_get_user_profile_image($account->uid);
      $url = "{$base_path}account/{$account->uid}";
      $forum = $node->taxonomy[$node->forum_tid]->name;
      if ($forum) {
        $title_prefix = "{$forum} : ";
      }
      //echo '<pre>' . print_r($node,1) . '</pre>';
      //echo '<pre>' . print_r($account,1) . '</pre>';
    }
  ?>
  <div class="result user">
    <?php if ($user_image['image']['filepath']): ?>
      <div class="picture">
        <?php print theme('imagefield_image', $user_image['image'], $user_image['alt'], $user_image['alt'], array(), false); ?>
      </div>
    <?php endif; ?>
    <div class="name"><a href="<?php print $url; ?>"><?php print $title; ?></a></div>
    <div class="details">
      <div class="user-stats">
        <div class="join-date"><?php print bts('Joined') . ': ' . date('j M y', $account->created); ?></div>
        <div class="post-count"><?php print bts('Posts') . ': ' . $account->post_count; ?></div>
        <div class="credit"><?php print bts('Credit') . ': ' . $account->boincuser_total_credit; ?></div>
        <div class="rac"><?php print bts('RAC') . ': ' . $account->boincuser_expavg_credit; ?></div>
      </div>
    </div>
    <?php if ($snippet) : ?>
      <p class="search-snippet"><?php print $snippet; ?></p>
    <?php endif; ?>
  </div>
  
  <?php break; ?>
  
<?php
  case 'Forum topic':
    $parsed_url = parse_url($url);
    $base_length = strlen($base_path);
    $core_path = trim(substr($parsed_url['path'], $base_length), '/');
    $path = drupal_lookup_path('source', $core_path);
    $matches = array();
    if (preg_match('/node\/([0-9]+)/', $path, $matches)) {
      $node = node_load($matches[1]);
      $forum = $node->taxonomy[$node->forum_tid]->name;
      if ($forum) {
        $title_prefix = "{$forum} : ";
      }
      //echo '<pre>' . print_r($variables,1) . '</pre>';
      //echo 'node: ' . $variables['result']['node']->nid;
    }
  ?>
  <div class="result forum">
    <div class="title">
      <a href="<?php print $url; ?>"><?php print $title_prefix . $title; ?></a>
    </div>
    <div class="details">
      <?php if ($snippet) : ?>
        <p class="search-snippet"><?php print $snippet; ?></p>
      <?php endif; ?>
      <?php if ($info) : ?>
        <p class="search-info"><?php print $info; ?></p>
      <?php endif; ?>
    </div>
  </div>
  <?php break; ?>
  
<?php default: ?>
  <dt class="title">
    <a href="<?php print $url; ?>"><?php print $title; ?></a>
  </dt>
  <dd>
    <?php if ($snippet) : ?>
      <p class="search-snippet"><?php print $snippet; ?></p>
    <?php endif; ?>
    <?php if ($info) : ?>
    <p class="search-info"><?php print $info; ?></p>
    <?php endif; ?>
  </dd>
<?php endswitch; ?>
                 <?php //print '<pre>'. check_plain(print_r($info_split, 1)) .'</pre>'; ?>