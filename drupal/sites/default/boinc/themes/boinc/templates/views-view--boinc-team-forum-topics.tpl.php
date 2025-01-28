<?php
/**
 * @file views-view.tpl.php
 * Main view template
 *
 * Variables available:
 * - $classes_array: An array of classes determined in
 *   template_preprocess_views_view(). Default classes are:
 *     .view
 *     .view-[css_name]
 *     .view-id-[view_name]
 *     .view-display-id-[display_name]
 *     .view-dom-id-[dom_id]
 * - $classes: A string version of $classes_array for use in the class attribute
 * - $css_name: A css-safe version of the view name.
 * - $css_class: The user-specified classes names, if any
 * - $header: The view header
 * - $footer: The view footer
 * - $rows: The results of the view query, if any
 * - $empty: The empty text to display if the view is empty
 * - $pager: The pager next/prev links to display, if any
 * - $exposed: Exposed widget form/info to display
 * - $feed_icon: Feed icon to display, if any
 * - $more: A link to view more, if any
 * - $admin_links: A rendered list of administrative links
 * - $admin_links_raw: A list of administrative links suitable for theme('links')
 *
 * @ingroup views_templates
 */
?>
<div class="<?php print $classes; ?>">
  <?php if ($admin_links): ?>
    <div class="views-admin-links views-hide">
      <?php print $admin_links; ?>
    </div>
  <?php endif; ?>
  <?php if ($header): ?>
    <div class="view-header">
      <?php print $header; ?>
    </div>
  <?php endif; ?>

  <?php if ($exposed): ?>
    <div class="view-filters">
      <?php print $exposed; ?>
    </div>
  <?php endif; ?>

  <?php if ($attachment_before): ?>
    <div class="attachment attachment-before">
      <?php print $attachment_before; ?>
    </div>
  <?php endif; ?>

  <?php if ($rows OR $empty): ?>
    <div class="view-content">
      <?php

        $team_forum_id = arg(4);
        $team_forum = boincteam_forum_load($team_forum_id);

        // Grab a sample forum topic node to get the forum vocabulary name
        $sample = db_result(db_query("
          SELECT nid FROM {node} WHERE type = 'forum' LIMIT 1"
        ));
        $forum_node = node_load($sample);
        // Get vocabulary name and use that as the page title
        $taxonomy = taxonomy_get_term($forum_node->tid);
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

      ?>

      <h1 class="title"><?php print $forum_vocab->name; ?></h1>

      <div id="forum">
        <h2 class="title">
          <?php print l($forum_vocab->name, 'community/forum') . ' &rsaquo; '; ?>
          <?php print $team_forum->title; ?>
        </h2>
        <?php if ($rows): ?>
          <?php print $rows; ?>
        <?php elseif ($empty): ?>
          <?php print $empty; ?>
        <?php endif; ?>
      </div>

    </div>
  <?php endif; ?>

  <?php if ($pager): ?>
    <?php print $pager; ?>
  <?php endif; ?>

  <ul class="links">
    <li class="forum first last">
      <?php $account=user_load($user->uid); ?>
      <?php if ($account->team AND $account->team == $team_forum->nid AND (user_access('create team_forum content'))): ?>
        <?php print l(bts('Post new topic', array(), NULL, 'boinc:forum-post-new-topic'), "node/add/team-forum/{$team_forum_id}"); ?>
      <?php endif; ?>
    </li>
  </ul>
  <div class="clearfix"></div>

  <?php if ($attachment_after): ?>
    <div class="attachment attachment-after">
      <?php print $attachment_after; ?>
    </div>
  <?php endif; ?>

  <?php if ($more): ?>
    <?php print $more; ?>
  <?php endif; ?>

  <?php if ($footer): ?>
    <div class="view-footer">
      <?php print $footer; ?>
    </div>
  <?php endif; ?>

  <?php if ($feed_icon): ?>
    <div class="feed-icon">
      <?php print $feed_icon; ?>
    </div>
  <?php endif; ?>

</div> <?php /* class view */ ?>
