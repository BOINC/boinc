<?php

/**
 * @file
 * widget.tpl.php
 *
 * UpAndDown widget theme for Vote Up/Down
 * TODO use $show_up_as_link and $show_down_as_link
 */
?>
<div class="vud-widget vud-widget-upanddown" id="<?php print $id; ?>">
  <div class="up-score clear-block">
    <?php if ($show_links): ?>
      <?php if ($show_up_as_link): ?>
        <a href="<?php print $link_up; ?>" rel="nofollow" class="<?php print "$link_class_up"; ?>" title="<?php print t('Vote up!'); ?>">
      <?php endif; ?>
          <div class="<?php print $class_up; ?>" title="<?php print t('Vote up!'); ?>"></div>
          <div class="element-invisible"><?php print t('Vote up!'); ?></div>
      <?php if ($show_up_as_link): ?>
        </a>
      <?php endif; ?>
    <?php endif; ?>
    <span class="up-current-score"><?php print $up_points; ?></span>
  </div>

  <div class="down-score clear-block">
    <?php if ($show_links): ?>
      <?php if ($show_down_as_link): ?>
        <a href="<?php print $link_down; ?>" rel="nofollow" class="<?php print "$link_class_down"; ?>" title="<?php print t('Vote down!'); ?>">
      <?php endif; ?>
          <div class="<?php print $class_down; ?>" title="<?php print t('Vote down!'); ?>"></div>
          <div class="element-invisible"><?php print t('Vote down!'); ?></div>
      <?php if ($show_down_as_link): ?>
        </a>
      <?php endif; ?>
    <?php endif; ?>
    <span class="down-current-score"><?php print $down_points; ?></span>
  </div>
</div>
