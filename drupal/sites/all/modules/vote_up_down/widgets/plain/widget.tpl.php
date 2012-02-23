<?php
// $Id: widget.tpl.php,v 1.1.2.15 2010/07/03 22:08:30 marvil07 Exp $
/**
 * @file
 * widget.tpl.php
 *
 * Plain widget theme for Vote Up/Down
 */
?>
<?php if ($show_links): ?>
  <div class="vud-widget vud-widget-plain" id="<?php print $id; ?>">
    <a href="<?php print $link_up; ?>" rel="nofollow" class="<?php print $link_class_up; ?>">
      <span class="<?php print $class_up; ?>" title="<?php print t('Vote up!'); ?>"></span>
    </a>
    <a href="<?php print $link_down; ?>" rel="nofollow" class="<?php print $link_class_down; ?>">
      <span class="<?php print $class_down; ?>" title="<?php print t('Vote down!'); ?>"></span>
    </a>
  </div>
<?php endif; ?>
