<?php
?>
<div class="dashboard-block">
  <h3 class="dashboard-title"><?php print $block['title']; ?></h3>
  <div class="dashboard-content <?php print $block['class']; ?>">
    <?php print $block['content']; ?>
    <?php if (!empty($block['link'])): ?>
      <div class="links">
        <?php print $block['link']; ?>
      </div>
    <?php endif; ?>
  </div>
</div>
