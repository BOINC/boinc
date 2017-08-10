<?php
/**
 * Create a simple renderer plugin that renders a layout but the content is
 * already rendered, not in panes.
 */
$plugin = array(
  'handler' => array(
    'class' => 'panels_renderer_simple',
    'parent' => 'standard',
  ),
);