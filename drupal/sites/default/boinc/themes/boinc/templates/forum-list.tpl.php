<?php
// $Id: forum-list.tpl.php,v 1.4 2007/08/30 18:58:12 goba Exp $

/**
 * @file forum-list.tpl.php
 * Default theme implementation to display a list of forums and containers.
 *
 * Available variables:
 * - $forums: An array of forums and containers to display. It is keyed to the
 *   numeric id's of all child forums and containers.
 * - $forum_id: Forum id for the current forum. Parent to all items within
 *   the $forums array.
 *
 * Each $forum in $forums contains:
 * - $forum->is_container: Is TRUE if the forum can contain other forums. Is
 *   FALSE if the forum can contain only topics.
 * - $forum->depth: How deep the forum is in the current hierarchy.
 * - $forum->zebra: 'even' or 'odd' string used for row class.
 * - $forum->name: The name of the forum.
 * - $forum->link: The URL to link to this forum.
 * - $forum->description: The description of this forum.
 * - $forum->new_topics: True if the forum contains unread posts.
 * - $forum->new_url: A URL to the forum's unread posts.
 * - $forum->new_text: Text for the above URL which tells how many new posts.
 * - $forum->old_topics: A count of posts that have already been read.
 * - $forum->num_posts: The total number of posts in the forum.
 * - $forum->last_reply: Text representing the last time a forum was posted or
 *   commented in.
 *
 * @see template_preprocess_forum_list()
 * @see theme_forum_list()
 */
?>
<table id="forum-<?php print $forum_id; ?>">
  <tbody>

  <?php foreach ($forums as $child_id => $forum): ?>

    <?php if ($forum->is_container): ?>
      <tr class="heading">
        <td class="primary"><div class="name"><?php print $forum->name; ?></div></td>
        <td><?php print bts('Threads', array(), NULL, 'boinc:forum-column');?></td>
        <td><?php print bts('Posts', array(), NULL, 'boinc:forum-column'); ?></td>
        <td><?php print bts('Last post', array(), NULL, 'boinc:forum-column'); ?></td>
      </tr>
    <?php else: ?>
      <tr id="forum-list-<?php print $child_id; ?>" class="<?php print $forum->zebra; ?>">
        <td class="forum">
          <?php /* Enclose the contents of this cell with X divs, where X is the
                 * depth this forum resides at. This will allow us to use CSS
                 * left-margin for indenting.
                 */ ?>
          <?php print str_repeat('<div class="indent">', $forum->depth); ?>
            <div class="name"><?php print l($forum->name, "community/forum/{$forum->tid}", array('html' => TRUE)); ?></div>
            <?php if ($forum->description): ?>
              <div class="description"><?php print $forum->description; ?></div>
            <?php endif; ?>
          <?php print str_repeat('</div>', $forum->depth); ?>
        </td>
        <td class="topics">
          <?php if ($forum->new_topics): ?>
            <?php print l($forum->num_topics, "community/forum/{$forum->tid}"); ?>
          <?php else: ?>
            <?php print $forum->num_topics; ?>
          <?php endif; ?>
        </td>
        <td class="posts"><?php print $forum->num_posts ?></td>
        <td class="last-reply"><?php print $forum->last_reply ?></td>
      </tr>
    <?php endif; ?>

  <?php endforeach; ?>

  <?php // Show the team forum, if available ?>

  <?php if (module_exists('boincteam_forum')
      AND $team_forums = boincteam_forum_list()): ?>
      <tr class="heading">
        <td class="primary"><div class="name"><?php echo bts('Team', array(), NULL, 'boinc:team-dashboard'); ?></div></td>
        <td><?php print bts('Threads', array(), NULL, 'boinc:forum-column');?></td>
        <td><?php print bts('Posts', array(), NULL, 'boinc:forum-column'); ?></td>
        <td><?php print bts('Last post', array(), NULL, 'boinc:forum-column'); ?></td>
      </tr>

    <?php foreach ($team_forums as $child_id => $forum): ?>
      <tr id="team-forum-list<?php print $child_id; ?>" class="<?php print $forum->zebra; ?>">
        <td class="forum">
            <div class="name"><a href="<?php print $forum->link; ?>"><?php print $forum->title; ?></a></div>
            <?php if ($forum->description): ?>
              <div class="description"><?php print $forum->description; ?></div>
            <?php endif; ?>
        </td>
        <td class="topics">
          <?php if ($forum->new_topics): ?>
            <a href="<?php print $forum->link; ?>">
          <?php endif; ?>
          <?php print $forum->num_topics ?>
          <?php if ($forum->new_topics): ?>
            <?php //print $forum->new_text; ?>
            </a>
          <?php endif; ?>
        </td>
        <td class="posts"><?php print $forum->num_posts ?></td>
        <td class="last-reply"><?php print $forum->last_reply ?></td>
      </tr>
    <?php endforeach; // Team forum ?>
  <?php endif; // Team forums exist ?>
  </tbody>
</table>

<div class="fine-print">
  <p>
    <?php print bts('Please be responsible in what you write and do not create posts which are offensive or insulting. Offensive posts or threads may be deleted by forum moderators without warning or discussion. Do not respond to offensive postings. Click on the "report" button at the bottom of the post to call it to the attention of the moderators.', array(), NULL, 'boinc:forum-fine-print'); ?>
  </p>
  <p>
    <?php print bts('We also ask that you keep all discussion on the message boards related to @project or BOINC with the small exception of the Science message board where you are free to discuss anything relevant to the underlying science. Participants interested in broader discussions should post to unofficial forums for @project.',
      array('@project' => variable_get('site_name', 'Drupal-BOINC')), NULL, 'boinc:forum-fine-print'); ?>
  </p>
  <p>
    <?php print bts('These message boards now support BBCode tags only.', array(), NULL, 'boinc:forum-fine-print'); ?>
  </p>
</div>
