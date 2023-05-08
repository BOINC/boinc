<?php
// $Id: forum-topic-list.tpl.php,v 1.4.2.1 2008/10/22 18:22:51 dries Exp $

/**
 * @file forum-topic-list.tpl.php
 * Theme implementation to display a list of forum topics.
 *
 * Available variables:
 * - $header: The table header. This is pre-generated with click-sorting
 *   information. If you need to change this, @see template_preprocess_forum_topic_list().
 * - $pager: The pager to display beneath the table.
 * - $topics: An array of topics to be displayed.
 * - $topic_id: Numeric id for the current forum topic.
 *
 * Each $topic in $topics contains:
 * - $topic->icon: The icon to display.
 * - $topic->moved: A flag to indicate whether the topic has been moved to
 *   another forum.
 * - $topic->title: The title of the topic. Safe to output.
 * - $topic->message: If the topic has been moved, this contains an
 *   explanation and a link.
 * - $topic->zebra: 'even' or 'odd' string used for row class.
 * - $topic->num_comments: The number of replies on this topic.
 * - $topic->new_replies: A flag to indicate whether there are unread comments.
 * - $topic->new_url: If there are unread replies, this is a link to them.
 * - $topic->new_text: Text containing the translated, properly pluralized count.
 * - $topic->created: An outputtable string represented when the topic was posted.
 * - $topic->last_reply: An outputtable string representing when the topic was
 *   last replied to.
 * - $topic->timestamp: The raw timestamp this topic was posted.
 *
 * @see template_preprocess_forum_topic_list()
 * @see theme_forum_topic_list()
 */
?>
<table id="forum-topic-<?php print $topic_id; ?>">

  <?php
    // Get vocabulary name and use that as the title
    $topic = current($topics);
    $taxonomy = taxonomy_get_term($topic->tid);
    if (module_exists('internationalization')) {
      $imv = i18ntaxonomy_localize_terms(array($taxonomy));
      $taxonomy = reset($imv);
    }
    if ($forum_vocab = taxonomy_vocabulary_load($taxonomy->vid)) {
      if (module_exists('internationalization')) {
        $forum_vocab->name = i18ntaxonomy_translate_vocabulary_name($forum_vocab);
      }
      drupal_set_title($forum_vocab->name);
    }
    // Get the count of topics on this page
    $topic_count = count($topics);
    $topic_index = 0;
    $first_non_sticky = FALSE;
  ?>

  <h2 class="title">
    <?php print l($forum_vocab->name, 'community/forum') . ' &rsaquo; '; ?>
    <?php print $taxonomy->name; ?>
  </h2>

  <thead>
    <tr><?php print $header; ?></tr>
  </thead>
  <tbody>
  <?php foreach ($topics as $topic): ?>
    <?php
      node_load($topic->id);
      $author = user_load($topic->uid);
      $topic_index++;
      $row_class = 'topic ' . $topic->zebra;
      if ($topic_index == 1) {
        $row_class .= ' first';
      }
      if ($topic->sticky) {
        $row_class .= ' sticky';
      }
      elseif (!$first_non_sticky AND !$topic->sticky) {
        $row_class .= ' first-non-sticky';
        $first_non_sticky = TRUE;
      }
      if ($topic_index == $topic_count) {
        $row_class .= ' last';
      }
      if ($topic->new) {
        $row_class .= ' updated';
      }
    ?>
    <tr class="<?php print $row_class;?>">
      <td class="icon"><?php //print $topic->icon; ?>
        <?php if ($topic->sticky): ?>
          <i class='fas fa-thumbtack'></i>
        <?php endif; ?>
        <?php if ($topic->comment_mode != COMMENT_NODE_READ_WRITE): ?>
          <i class='fas fa-lock'></i>
        <?php endif; ?>
        <?php if (($topic->new) AND ($topic->new_replies)): ?>
            <i class='far fa-bell'></i>
        <?php elseif ($topic->new): ?>
            <i class='far fa-star'></i>
        <?php endif; ?>
      </td>
      <td class="title" title="<?php print $author->boincuser_name; ?>">
        <?php print $topic->title; ?>
      </td>
    <?php if ($topic->moved): ?>
      <td colspan="3"><?php print $topic->message; ?></td>
    <?php else: ?>
      <td class="replies">
        <?php if ($topic->new_replies): ?>
          <a href="<?php print $topic->new_url; ?>">
        <?php endif; ?>
        <?php print $topic->num_comments; ?>
        <?php if ($topic->new_replies): ?>
          <?php //print $topic->new_text; ?>
          </a>
        <?php endif; ?>
      </td>
      <td class="created">
        <?php print $topic->created; ?></td>
      <td class="last-reply">
        <?php print $topic->last_reply; ?>
      </td>
    <?php endif; ?>
    </tr>
  <?php endforeach; ?>
  </tbody>
</table>
<?php print $pager; ?>
