<div class='context-block-browser clear-block'>

  <div class='categories'><?php print theme('select', $categories) ?></div>

  <?php foreach ($blocks as $module => $module_blocks): ?>

  <?php if (!empty($module_blocks)): ?>
  <div class='category category-<?php print $module ?> clear-block'>
    <?php foreach ($module_blocks as $block): ?>
      <?php print theme('context_block_browser_item', $block); ?>
    <?php endforeach; ?>
  </div>
  <?php endif; ?>

  <?php endforeach; ?>

</div>