<?php
// $Id: comment.tpl.php,v 1.10 2009/11/02 17:42:27 johnalbin Exp $

/**
 * @file
 * Default theme implementation for comments.
 *
 * Available variables:
 * - $author: Comment author. Can be link or plain text.
 * - $content: Body of the comment.
 * - $created: Formatted date and time for when the comment was created.
 *   Preprocess functions can reformat it by calling format_date() with the
 *   desired parameters on the $comment->timestamp variable.
 * - $new: New comment marker.
 * - $picture: Authors picture.
 * - $signature: Authors signature.
 * - $status: Comment status. Possible values are:
 *   comment-unpublished, comment-published or comment-preview.
 * - $title: Linked title.
 * - $links: Various operational links.
 * - $unpublished: An unpublished comment visible only to administrators.
 * - $classes: String of classes that can be used to style contextually through
 *   CSS. It can be manipulated through the variable $classes_array from
 *   preprocess functions. The default values can be one or more of the following:
 *   - comment: The current template type, i.e., "theming hook".
 *   - comment-by-anonymous: Comment by an unregistered user.
 *   - comment-by-node-author: Comment by the author of the parent node.
 *   - comment-preview: When previewing a new or edited comment.
 *   - first: The first comment in the list of displayed comments.
 *   - last: The last comment in the list of displayed comments.
 *   - odd: An odd-numbered comment in the list of displayed comments.
 *   - even: An even-numbered comment in the list of displayed comments.
 *   The following applies only to viewers who are registered users:
 *   - comment-by-viewer: Comment by the user currently viewing the page.
 *   - comment-unpublished: An unpublished comment visible only to administrators.
 *   - comment-new: New comment since the last visit.
 *
 * These two variables are provided for context:
 * - $comment: Full comment object.
 * - $node: Node object the comments are attached to.
 *
 * Other variables:
 * - $classes_array: Array of html class attribute values. It is flattened
 *   into a string within the variable $classes.
 *
 * The following variables are deprecated and will be removed in Drupal 7:
 * - $date: Formatted date and time for when the comment was created.
 * - $submitted: By line with date and time.
 *
 * @see template_preprocess()
 * @see template_preprocess_comment()
 * @see zen_preprocess()
 * @see zen_preprocess_comment()
 * @see zen_process()
 */
?>

<div class="<?php print $classes; ?> clearfix">
  <?php
    static $authors;
    if (_ignore_user_ignored_user($comment->uid)) {
      if (!$authors[$comment->uid]) {
        $authors[$comment->uid] = user_load(array('uid' => $comment->uid));
      }
      // Remove the wrapper the Ignore User module puts around node->content.
      // It should be around the whole comment, not one part...
      // Absurd nested functions to remove wrappers are as follows.
      $wrapper = explode('<div class="ignore-user-content">', $content);
      $wrapper = end($wrapper);
      $content = strstr($wrapper, '</div></div>', TRUE);
      print '<div class="ignore-user-container">';
      print bts('!username is on your !ignore_list. Click !here to view this post.',
        array(
          '!username' => theme('username', $authors[$comment->uid]),
          '!ignore_list' => l(bts('ignore list', array(), NULL, 'boinc:ignore-user-content'), 'ignore_user/list'),
          '!here' => l(bts('here', array(), NULL, 'boinc:ignore-user-content'), "node/{$comment->nid}#comment-{$comment->cid}",
            array(
              'attributes' => array('class' => 'ignore-user-content-link')
            ))
          ),
        NULL, 'boinc:coment-from-ignored-user');
      print '<div class="ignore-user-content">';
    }
    ?>

  <?php
    $ddname = 'flag_abuse_reason-dropdown-comment-' . $comment->cid;
  ?>
  <div class="user">
    <?php
      $account = user_load(array('uid' => $comment->uid));
      $user_image = boincuser_get_user_profile_image($comment->uid);
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
      //echo '<pre>' . print_r($links, TRUE) . '</pre>';
    ?>
    <div class="name"><?php print $author; ?></div>
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
  <div class="comment-body">
    <?php if ($title): ?>
      <h3 class="title">
        <?php print $title; ?>
        <?php if ($new): ?>
          <span class="new"><?php print $new; ?></span>
        <?php endif; ?>
      </h3>
    <?php elseif ($new): ?>
      <div class="new"><?php print $new; ?></div>
    <?php endif; ?>

    <?php if ($unpublished): ?>
      <div class="unpublished"><?php print bts('Unpublished', array(), NULL, 'boinc:comment-action-link'); ?></div>
    <?php endif; ?>

    <div class="submitted">
      <?php print date('j M Y G:i:s T', $comment->timestamp); ?>
    </div>
    <div class="comment-id">
      <?php echo l(bts('Message @id', array('@id' => $comment->cid), NULL, 'boinc:message-header'),
        "goto/comment/{$comment->cid}"); ?>
      <?php
        if ($comment->pid):
          $parent = _comment_load($comment->pid);
          if ($parent->status == COMMENT_PUBLISHED) {
            $parent_link = l(bts('message @id', array('@id' => $comment->pid), NULL, 'boinc:message-header'),
            "goto/comment/{$comment->pid}");
          }
          else {
            $parent_link = '(' . bts('parent removed', array(), NULL, 'boinc:message-header') . ')';
          }
          echo bts(' in response to !parent', array(
            '!parent' => $parent_link
          ), NULL, 'boinc:message-header');
        endif;
      ?>
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
        <?php print flag_create_link('abuse_comment_1', $comment->cid); ?>
        <?php print flag_create_link('abuse_comment_2', $comment->cid); ?>
        <?php print flag_create_link('abuse_comment_3', $comment->cid); ?>
        <?php print flag_create_link('abuse_comment_4', $comment->cid); ?>
        <?php print flag_create_link('abuse_comment_5', $comment->cid); ?>
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
  </div> <!-- /.comment-body -->

  <?php
    if (_ignore_user_ignored_user($comment->uid)) {
      print '</div> <!-- /.ignore-user-content -->';
      print '</div> <!-- /.ignore-user-container -->';
    }
  ?>

</div> <!-- /.comment -->

<?php if ($status == 'comment-preview'): ?>
  <h2 class="title"><?php print bts('Revise or post comment', array(), NULL, 'boinc:comment-preview-title'); ?></h2>
<?php endif; ?>
