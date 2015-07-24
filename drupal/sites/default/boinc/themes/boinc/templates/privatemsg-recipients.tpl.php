<?php
// Each file loads it's own styles because we cant predict which file will be
// loaded.
drupal_add_css(drupal_get_path('module', 'privatemsg') . '/styles/privatemsg-recipients.css');
?>
<?php /*
<div class="message-participants">
  <?php print $participants; ?>
</div> */ ?>