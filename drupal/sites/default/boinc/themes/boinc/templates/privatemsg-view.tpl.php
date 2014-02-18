<?php
// Each file loads it's own styles because we cant predict which file will be
// loaded.
drupal_add_css(drupal_get_path('module', 'privatemsg') . '/styles/privatemsg-view.css');
?>
<?php print $anchors; ?>

<div id="privatemsg-mid-<?php print $mid; ?>" class="privatemsg-box-fb <?php print $zebra; ?> clearfix">

  <div class="user">
    <?php print $author_picture; ?>
  </div>
  <div class="message-body">
    <div class="submitted">
      <div class="name"><?php print $author_name_link; ?></div>
      <?php print $message_timestamp; ?>
    </div>
    <?php if (isset($new)) : ?>
      <span class="new"><?php print $new ?></span>
    <?php endif ?>
    <?php print $message_body; ?>
    <?php if (isset($message_actions)) : ?>
       <?php //print $message_actions ?>
    <?php endif ?>
  </div> <!-- /.message-body -->
</div>
