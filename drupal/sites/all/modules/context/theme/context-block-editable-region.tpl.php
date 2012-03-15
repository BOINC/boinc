<?php if ($editable && (!empty($blocks) || $show_always)): ?>
  <a class='context-block-region' id='context-block-region-<?php print $region ?>'><?php print $region_description ?></a>
  <?php foreach ($blocks as $block): ?>
    <?php print theme('context_block_editable_block', $block); ?>
  <?php endforeach; ?>
<?php else: ?>
<?php /* When themes check to see if there is any content is the region any
         whitespace will make the them think it's got content. Consequently
         we don't nest this following code. */ ?>
<?php foreach ($blocks as $block): ?>
  <?php print theme('block', $block); ?>
<?php endforeach; ?>
<?php endif; ?>
