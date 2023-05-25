<?php
// $Id: node.tpl.php,v 1.10 2009/11/02 17:42:27 johnalbin Exp $

/**
 * @file
 * Theme implementation to display a node.
 *
 * Available variables:
 * - $title: the (sanitized) title of the node.
 * - $content: Node body or teaser depending on $teaser flag.
 * - $user_picture: The node author's picture from user-picture.tpl.php.
 * - $date: Formatted creation date. Preprocess functions can reformat it by
 *   calling format_date() with the desired parameters on the $created variable.
 * - $name: Themed username of node author output from theme_username().
 * - $node_url: Direct url of the current node.
 * - $terms: the themed list of taxonomy term links output from theme_links().
 * - $display_submitted: whether submission information should be displayed.
 * - $links: Themed links like "Read more", "Add new comment", etc. output
 *   from theme_links().
 * - $classes: String of classes that can be used to style contextually through
 *   CSS. It can be manipulated through the variable $classes_array from
 *   preprocess functions. The default values can be one or more of the
 *   following:
 *   - node: The current template type, i.e., "theming hook".
 *   - node-[type]: The current node type. For example, if the node is a
 *     "Blog entry" it would result in "node-blog". Note that the machine
 *     name will often be in a short form of the human readable label.
 *   - node-teaser: Nodes in teaser form.
 *   - node-preview: Nodes in preview mode.
 *   The following are controlled through the node publishing options.
 *   - node-promoted: Nodes promoted to the front page.
 *   - node-sticky: Nodes ordered above other non-sticky nodes in teaser
 *     listings.
 *   - node-unpublished: Unpublished nodes visible only to administrators.
 *   The following applies only to viewers who are registered users:
 *   - node-by-viewer: Node is authored by the user currently viewing the page.
 *
 * Other variables:
 * - $node: Full node object. Contains data that may not be safe.
 * - $type: Node type, i.e. story, page, blog, etc.
 * - $comment_count: Number of comments attached to the node.
 * - $uid: User ID of the node author.
 * - $created: Time the node was published formatted in Unix timestamp.
 * - $classes_array: Array of html class attribute values. It is flattened
 *   into a string within the variable $classes.
 * - $zebra: Outputs either "even" or "odd". Useful for zebra striping in
 *   teaser listings.
 * - $id: Position of the node. Increments each time it's output.
 *
 * Node status variables:
 * - $build_mode: Build mode, e.g. 'full', 'teaser'...
 * - $teaser: Flag for the teaser state (shortcut for $build_mode == 'teaser').
 * - $page: Flag for the full page state.
 * - $promote: Flag for front page promotion state.
 * - $sticky: Flags for sticky post setting.
 * - $status: Flag for published status.
 * - $comment: State of comment settings for the node.
 * - $readmore: Flags true if the teaser content of the node cannot hold the
 *   main body content.
 * - $is_front: Flags true when presented in the front page.
 * - $logged_in: Flags true when the current user is a logged-in member.
 * - $is_admin: Flags true when the current user is an administrator.
 *
 * The following variables are deprecated and will be removed in Drupal 7:
 * - $picture: This variable has been renamed $user_picture in Drupal 7.
 * - $submitted: Themed submission information output from
 *   theme_node_submitted().
 *
 * @see template_preprocess()
 * @see template_preprocess_node()
 * @see zen_preprocess()
 * @see zen_preprocess_node()
 * @see zen_process()
 */
?>
<div id="top"></div>

<div id="node-<?php print $node->nid; ?>" class="<?php print $classes; ?> clearfix<?php echo ($first_page) ? '' : ' not-first-page'; ?>">

  <?php
    if ($page) {
      // Set topic title as page title
      drupal_set_title($title);
      $subtitle = array();
      // Get vocabulary name and taxonomy name for subtitle breadcrumbs
      $taxonomy = taxonomy_get_term($node->forum_tid);
      if ($forum_vocab = taxonomy_vocabulary_load($taxonomy->vid)) {
        $subtitle[] = l($forum_vocab->name, 'community/forum');
      }
      if (isset($taxonomy->name)) {
        $subtitle[] = l($taxonomy->name, "community/forum/{$taxonomy->tid}");
      }
      $subtitle = implode(' &rsaquo; ', $subtitle);
    }
  ?>

  <div class="forum-links">
    <div class="breadcrumb">
      <h2 class="title"><?php print $subtitle; ?></h2>
    </div>
    <div class="subscribe">
      <ul class="links">
        <?php if (user_access('post comments') AND ($comment==COMMENT_NODE_READ_WRITE)): ?>
          <li class="first"><a href="#block-comment_form_block-comment_form">Post new comment</a></li>
          <?php if ($subscribe_link): ?>
            <li class="last"><?php print $subscribe_link; ?></li>
          <?php endif; ?>
        <?php else: ?>
          <?php if ($subscribe_link): ?>
            <li class="first"><?php print $subscribe_link; ?></li>
          <?php endif; ?>
        <?php endif; ?>
      </ul>
    </div>
    <div class="clearfix"></div>
  </div>

  <?php if ($unpublished): ?>
    <div class="unpublished"><?php print bts('Unpublished', array(), NULL, 'boinc:comment-action-links'); ?></div>
  <?php endif; ?>

  <?php
    if (!$oldest_post_first) {
      print comment_render($node);
    }
  ?>
  <?php // Only show this post on the first or last page, depending on sort ?>
  <?php if (($oldest_post_first AND $first_page) OR (!$oldest_post_first AND $last_page)): ?>

<?// DBOINCP-300: added node comment count condition in order to get Preview working ?>
    <?php if ( (!$oldest_post_first) AND ($comment_count>0) ): ?>
          </div>
        </div>
      </div>
      <div class="section bottom framing container shadow">
        <div id="content-area-alt">
          <div id="node-<?php print $node->nid; ?>-alt" class="<?php print $classes; ?> clearfix<?php echo ($first_page) ? '' : ' not-first-page'; ?>">
    <?php endif; ?>

    <?php
      $ddname = 'flag_abuse_reason-dropdown-node-' . $node->nid;
    ?>
    <div class="user">
      <?php
        $account = user_load(array('uid' => $uid));
        $user_image = boincuser_get_user_profile_image($uid);
        if ($user_image) {
          print '<div class="picture">';
          if (is_array($user_image) AND $user_image['image']['filepath']) {
            //print theme('imagecache', 'thumbnail', $user_image['image']['filepath'], $user_image['alt'], $user_image['alt']);
            print theme('imagefield_image', $user_image['image'], $user_image['alt'], $user_image['alt'], array(), false);
          }
          elseif (is_string($user_image)) {
            print '<img src="' . $user_image . '"/>';
          }
          print '</div>';
        }
        // ignore user link is now generated in preprocess functions.
        //echo '<pre>' . print_r($node->links, TRUE) . '</pre>';
      ?>
      <div class="name"><?php print $name; ?></div>
      <?php if ($account->uid): ?>
      <?php if (in_array('moderator', $account->roles)): ?>
        <div class="moderator"><?php print bts('Moderator', array(), NULL, 'boinc:user-info'); ?></div>
      <?php endif; ?>
      <?php if (in_array('administrator', $account->roles)): ?>
        <div class="administrator"><?php print bts('Administrator', array(), NULL, 'boinc:user-info'); ?></div>
      <?php endif; ?>
        <?php $nf = new NumberFormatter($locality, NumberFormatter::DECIMAL); ;?>
        <?php $nf->setAttribute(NumberFormatter::MIN_FRACTION_DIGITS, 0); ;?>
        <?php $nf->setAttribute(NumberFormatter::MAX_FRACTION_DIGITS, 0); ;?>
        <?php $nf2 = new NumberFormatter($locality, NumberFormatter::DECIMAL); ;?>
        <div class="join-date"><?php print bts('Joined: @join_date', array( '@join_date' => date('j M y', $account->created) ), NULL, 'boinc:mini-user-stats'); ?></div>
        <div class="post-count"><?php print bts('Posts: @post_count', array( '@post_count' => $nf->format($account->post_count) ), NULL, 'boinc:mini-user-stats'); ?></div>
        <div class="credit"><?php print bts('Credit: @user_credits', array( '@user_credits' => $nf->format($account->boincuser_total_credit) ), NULL, 'boinc:mini-user-stats'); ?></div>
        <div class="rac"><?php print bts('RAC: @user_rac', array( '@user_rac' => $nf2->format($account->boincuser_expavg_credit) ), NULL, 'boinc:mini-user-stats'); ?></div>
        <div class="user-links">
          <div class="ignore-link"><?php print l($ignore_link['ignore_user']['title'],
            $ignore_link['ignore_user']['href'],
            array('query' => $ignore_link['ignore_user']['query'])); ?>
          </div>
          <div class="pm-link"><?php
            if ($user->uid AND ($user->uid != $account->uid)) {
              print l(bts('Send message', array(), NULL, 'boinc:private-message'),
              privatemsg_get_link(array($account)),
              array('query' => drupal_get_destination()));
            } ?>
          </div>
        </div>
      <?php endif; ?>
    </div>

    <div class="node-body">

      <?php /* if ($terms): ?>
        <div class="terms terms-inline"><?php print $terms; ?></div>
      <?php endif; */ ?>

      <?php if ($display_submitted): ?>
        <div class="submitted">
          <?php print date('j M Y G:i:s T', $node->created); ?>
        </div>
      <?php endif; ?>
      <div class="topic-id">
        Topic <?php print $node->nid; ?>
      </div>
      <div class="standard-links">
        <?php print $links; ?>
      </div>
      <?php if ($moderator_links): ?>
        <div class="moderator-links">
          <span class="label">(<?php print bts('moderation', array(), NULL, 'boinc:comment-action-links'); ?>:</span>
          <?php print $moderator_links; ?>
          <span class="label">)</span>
        </div>
      <?php endif; ?>

      <div class="dropdown">
        <div id="<?php print $ddname; ?>" class="dropdown-content">
          <?php print flag_create_link('abuse_node_1', $node->nid); ?>
          <?php print flag_create_link('abuse_node_2', $node->nid); ?>
          <?php print flag_create_link('abuse_node_3', $node->nid); ?>
          <?php print flag_create_link('abuse_node_4', $node->nid); ?>
          <?php print flag_create_link('abuse_node_5', $node->nid); ?>
        </div>
      </div>
      <div class="content">
        <?php print $content; ?>
        <?php if ($signature AND $show_signatures): ?>
          <div class="user-signature clearfix">
            <?php print $signature; ?>
          </div>
        <?php endif; ?>
      </div>


    </div> <!-- /.node-body -->

  <?php endif; // page with topic starter post ?>

  <?php
    if ($oldest_post_first) {
      print comment_render($node);
    }
  ?>

  <div class="breadcrumb bottom-breadcrumb">
    <h2 class="title"><?php print $subtitle; ?><br>
  </div>

</div> <!-- /.node -->
