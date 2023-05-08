<?php
/**
 * @file views-view-table.tpl.php
 * Template to display a view as a table.
 *
 * - $title : The title of this group of rows.  May be empty.
 * - $header: An array of header labels keyed by field id.
 * - $fields: An array of CSS IDs to use for each field id.
 * - $class: A class or classes to apply to the table, based on settings.
 * - $row_classes: An array of classes to apply to each row, indexed by row
 *   number. This matches the index in $rows.
 * - $rows: An array of row items. Each row is an array of content.
 *   $rows are keyed by row number, fields within rows are keyed by field ID.
 * @ingroup views_templates
 */
?>
<?php

  $sort_by = isset($_GET['order']) ? $_GET['order'] : NULL;
  $sort_order = isset($_GET['sort']) ? $_GET['sort'] : 'asc';

  $team_forum_id = arg(4);

  $topics = $rows;

  // Get the count of topics on this page
  $topic_count = count($topics);
  $topic_index = 0;
  $first_non_sticky = FALSE;
?>

<table id="forum-topic-<?php print $team_forum_id; ?>">

  <thead>
    <tr>
      <th></th>
      <th class="views-field views-field-title<?php print ($sort_by == 'title') ? " sort-{$sort_order}" : NULL; ?>"><?php print $header['title']; ?></th>
      <th class="views-field views-field-comment-count<?php print ($sort_by == 'comment_count') ? " sort-{$sort_order}" : NULL; ?>"><?php print $header['comment_count']; ?></th>
      <th class="views-field views-field-created<?php print ($sort_by == 'created') ? " sort-{$sort_order}" : NULL; ?>"><?php print $header['created']; ?></th>
      <th class="views-field views-field-last-comment-timestamp<?php print ($sort_by == 'last_comment_timestamp') ? " sort-{$sort_order}" : NULL; ?>"><?php print $header['last_comment_timestamp']; ?></th>
    </tr>
  </thead>
  <tbody>
  <?php foreach ($topics as $id => $topic): ?>
    <?php
      $topic = (object) $topic;
      $author = user_load($topic->uid);
      $topic_index++;
      $row_class = 'topic ' . $topic->zebra;
      if ($topic_index == 1) {
        $row_class .= ' first';
      }
      if ($result[$id]->node_boincteam_forum_node_sticky) {
        $row_class .= ' sticky';
      }
      elseif (!$first_non_sticky AND !($result[$id]->node_boincteam_forum_node_sticky)) {
        $row_class .= ' first-non-sticky';
        $first_non_sticky = TRUE;
      }
      if ($topic_index == $topic_count) {
        $row_class .= ' last';
      }
      if (!empty($topic->timestamp) OR $topic->new_comments) {
        $row_class .= ' updated';
      }
    ?>
    <tr class="<?php print $row_class;?>">
      <td class="icon"><?php //print $topic->icon; ?>
        <?php if ($result[$id]->node_boincteam_forum_node_sticky): ?>
          <i class='fas fa-thumbtack'></i>
        <?php endif; ?>
        <?php if ($result[$id]->node_boincteam_forum_node_comment != COMMENT_NODE_READ_WRITE): ?>
          <i class='fas fa-lock'></i>
        <?php endif; ?>
        <?php if (!empty($topic->timestamp)): ?>
            <i class='far fa-star'></i>
        <?php elseif ($topic->new_comments): ?>
            <i class='far fa-bell'></i>
        <?php endif; ?>
      </td>
      <td class="title" title="<?php print $author->boincuser_name; ?>">
        <?php print $topic->title; ?>
      </td>
    <?php if ($topic->moved): ?>
      <td colspan="3"><?php print $topic->message; ?></td>
    <?php else: ?>
      <td class="replies">
        <?php if ($topic->new_comments): ?>
          <?php preg_match_all('/<a[^>]+href=([\'"])(.+?)\1[^>]*>/i', $topic->new_comments, $myresult); ?>
          <a href="<?php print $myresult[2][0]; ?>">
        <?php endif; ?>
        <?php print $topic->comment_count; ?>
        <?php if ($topic->new_comments): ?>
          </a>
        <?php endif; ?>
      </td>
      <td class="created">
        <?php print $topic->created; ?></td>
      <td class="last-reply">
        <?php print $topic->last_comment_timestamp; ?>
      </td>
    <?php endif; ?>
    </tr>
  <?php endforeach; ?>
  </tbody>
</table>
<?php print $pager; ?>
