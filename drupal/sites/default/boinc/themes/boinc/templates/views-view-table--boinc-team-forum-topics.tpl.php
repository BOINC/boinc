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
  <?php foreach ($topics as $topic): ?>
    <?php
      $topic = (object) $topic;
      
      $topic_index++;
      $row_class = $topic->zebra;
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
    ?>
    <tr class="<?php print $row_class;?>">
      <td class="icon"><?php //print $topic->icon; ?></td>
      <td class="title"><?php print $topic->title; ?></td>
    <?php if ($topic->moved): ?>
      <td colspan="3"><?php print $topic->message; ?></td>
    <?php else: ?>
      <td class="replies">
        <?php if ($topic->new_replies): ?>
          <a href="<?php print $topic->new_url; ?>">
        <?php endif; ?>
        <?php print $topic->comment_count; ?>
        <?php if ($topic->new_replies): ?>
          <?php //print $topic->new_text; ?>
          </a>
        <?php endif; ?>
      </td>
      <td class="created"><?php print $topic->created; ?></td>
      <td class="last-reply">
        <?php if ($topic->sticky AND $topic->comment == 'Read only'): ?>
          <?php print bts('Featured') . ' / ' . bts('Locked'); ?>
        <?php elseif ($topic->sticky): ?>
          <?php print bts('Featured'); ?>
        <?php elseif ($topic->comment == 'Read only'): ?>
          <?php print bts('Locked'); ?>
        <?php else: ?>
          <?php print $topic->last_comment_timestamp; ?>
        <?php endif; ?>
      </td>
    <?php endif; ?>
    </tr>
  <?php endforeach; ?>
  </tbody>
</table>
<?php print $pager; ?>
