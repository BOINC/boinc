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
?>
<?php switch ($result['bundle']): ?>
<?php
  case 'Profile':
  case 'profile':
  case 'User':
  case 'user':
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
        <?php $nf = new NumberFormatter($locality, NumberFormatter::DECIMAL); ;?>
        <?php $nf->setAttribute(NumberFormatter::MIN_FRACTION_DIGITS, 0); ;?>
        <?php $nf->setAttribute(NumberFormatter::MAX_FRACTION_DIGITS, 0); ;?>
        <?php $nf2 = new NumberFormatter($locality, NumberFormatter::DECIMAL); ;?>
        <div class="join-date"><?php print bts('Joined: @join_date', array( '@join_date' => date('j M y', $account->created) ), NULL, 'boinc:mini-user-stats'); ?></div>
        <div class="post-count"><?php print bts('Posts: @post_count', array( '@post_count' => $nf->format($account->post_count) ), NULL, 'boinc:mini-user-stats'); ?></div>
        <div class="credit"><?php print bts('Credit: @user_credits', array( '@user_credits' => $nf->format($account->boincuser_total_credit) ), NULL, 'boinc:mini-user-stats'); ?></div>
        <div class="rac"><?php print bts('RAC: @user_rac', array( '@user_rac' => $nf2->format($account->boincuser_expavg_credit) ), NULL, 'boinc:mini-user-stats'); ?></div>
      </div>
    </div>
    <?php if ($snippet) : ?>
      <p class="search-snippet"><?php print $snippet; ?></p>
    <?php endif; ?>
  </div>

  <?php break; ?>

<?php
  case 'Forum':
  case 'forum':
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
  case 'comment':
  ?>
  <div class="result">
    <dt class="title">
      <?php if ($parent_forum): ?>
        <?php print $parent_forum . " : "; ?>
      <?php endif; ?>
      <a href="<?php print $url; ?>"><?php print $parent_title; ?></a>
    </dt>
    <dd>
      <?php if ($snippet) : ?>
        <p class="search-snippet"><?php print $snippet; ?></p>
      <?php endif; ?>
      <?php if ($info) : ?>
        <p class="search-info"><?php print $info; ?>
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
