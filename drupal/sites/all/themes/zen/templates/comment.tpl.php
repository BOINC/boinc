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
  <?php print $picture; ?>

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
    <div class="unpublished"><?php print t('Unpublished'); ?></div>
  <?php endif; ?>

  <div class="submitted">
    <?php
      print t('Submitted by !username on !datetime.',
        array('!username' => $author, '!datetime' => $created));
    ?>
  </div>

  <div class="content">
    <?php print $content; ?>
    <?php if ($signature): ?>
      <div class="user-signature clearfix">
        <?php print $signature; ?>
      </div>
    <?php endif; ?>
  </div>

  <?php print $links; ?>
</div> <!-- /.comment -->
