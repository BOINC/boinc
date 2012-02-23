<?php
// $Id: privatemsg-recipients.tpl.php,v 1.1.2.3 2009/11/06 13:06:26 berdir Exp $
// Each file loads it's own styles because we cant predict which file will be
// loaded.
drupal_add_css(drupal_get_path('module', 'privatemsg') . '/styles/privatemsg-recipients.css');
?>
<div class="message-participants">
  <?php print $participants; ?>
</div>