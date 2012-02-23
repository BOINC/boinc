<?php
// $Id: widget.tpl.php,v 1.1.2.16 2010/07/03 22:08:30 marvil07 Exp $
/**
 * @file
 * widget.tpl.php
 *
 * Alternate widget theme for Vote Up/Down
 */
?>
<div class="vud-widget vud-widget-alternate" id="<?php print $id; ?>">
  <?php if ($class_up) : ?>
    <div class="alternate-votes-display"><?php print $unsigned_points; ?></div>
    <?php if ($show_links): ?>
      <a href="<?php print $link_up; ?>" rel="nofollow" class="<?php print $link_class_up; ?>">
        <div class="<?php print $class_up; ?>" title="<?php print t('Vote up!'); ?>">  </div>
      </a>
    <?php endif; ?>
  <?php endif; ?>
</div>
