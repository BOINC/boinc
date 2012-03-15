<?php if ($empty): ?>
<div id='<?php print $block->bid ?>' class='block context-block-hidden'><?php print $block->content ?></div>
<?php else: ?>
<?php print theme('block', $block) ?>
<?php endif; ?>
