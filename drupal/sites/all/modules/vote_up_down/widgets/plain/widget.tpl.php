<?php
/**
 * @file
 * widget.tpl.php
 *
 * Plain widget theme for Vote Up/Down
 */
?>
<?php if ($show_links): ?>
  <div class="vud-widget vud-widget-plain" id="<?php print $id; ?>">
    <?php if ($show_up_as_link): ?>
      <a href="<?php print $link_up; ?>" rel="nofollow" class="<?php print $link_class_up; ?>">
    <?php endif; ?>
        <span class="<?php print $class_up; ?>" title="<?php print t('Vote up!'); ?>"></span>
        <div class="element-invisible"><?php print t('Vote up!'); ?></div>
    <?php if ($show_up_as_link): ?>
      </a>
    <?php endif; ?>
    <?php if ($show_down_as_link): ?>
      <a href="<?php print $link_down; ?>" rel="nofollow" class="<?php print $link_class_down; ?>">
    <?php endif; ?>
        <span class="<?php print $class_down; ?>" title="<?php print t('Vote down!'); ?>"></span>
        <div class="element-invisible"><?php print t('Vote down!'); ?></div>
    <?php if ($show_down_as_link): ?>
      </a>
    <?php endif; ?>
  </div>
<?php endif; ?>
