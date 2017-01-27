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
    $nid = $result['fields']['entity_id'];
    $node = node_load($nid);
    $account = user_load($node->uid);
    if (isset($account)) {
      $user_image = boincuser_get_user_profile_image($account->uid);
      $url = "{$base_path}account/{$account->uid}";
      if (empty($title)) {
        $title = $account->name;
      }
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
    $nid = $result['fields']['entity_id'];
    $node = node_load($nid);
    // Get the taxonomy for the node, creates a link to the parent forum
    $taxonomy = reset($node->taxonomy);
    if ($vocab = taxonomy_vocabulary_load($taxonomy->vid)) {
      $parent_forum = l($taxonomy->name, "community/forum/{$taxonomy->tid}");
    }
  ?>
  <div class="result forum">
    <dt class="title">
      <?php if ($parent_forum): ?>
        <?php print $parent_forum . " : "; ?>
      <?php endif; ?>
      <a href="<?php print $url; ?>"><?php print $title; ?></a>
    </dt>
    <dd class="details">
      <?php if ($snippet) : ?>
        <p class="search-snippet"><?php print $snippet; ?></p>
      <?php endif; ?>
      <?php if ($info) : ?>
        <p class="search-info"><?php print $info; ?></p>
      <?php endif; ?>
    </dd>
  </div>
  <?php break; ?>

<?php
  case 'Comment':
    // Get the node if for this comment
    $nid = $result['fields']['tos_content_extra'];
    $node = node_load($nid);
    // Link to the parent forum topic
    $parent_topic = l($node->title, drupal_get_path_alias('node/' . $nid) );
    // Get the taxonomy for the node, creates a link to the parent forum
    $taxonomy = reset($node->taxonomy);
    if ($vocab = taxonomy_vocabulary_load($taxonomy->vid)) {
      $parent_forum = l($taxonomy->name, "community/forum/{$taxonomy->tid}");
    }
  ?>
  <div class="result">
    <dt class="title">
      <?php if ($parent_forum): ?>
        <?php print $parent_forum . " : "; ?>
      <?php endif; ?>
      <a href="<?php print $url; ?>"><?php print $node->title; ?></a>
    </dt>
    <dd>
      <?php if ($snippet) : ?>
        <p class="search-snippet"><?php print $snippet; ?></p>
      <?php endif; ?>
      <?php if ($info) : ?>
        <p class="search-info"><?php print $info; ?>
<?php //print " - " . $parent_forum . " : " . $parent_topic; ?>
        </p>
      <?php endif; ?>
    </dd>
  </div>
<?php break; ?>

<?php default: ?>
  <div class="result">
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
  </div>
<?php endswitch; ?>
<?php //print '<pre>'. check_plain(print_r($info_split, 1)) .'</pre>'; ?>