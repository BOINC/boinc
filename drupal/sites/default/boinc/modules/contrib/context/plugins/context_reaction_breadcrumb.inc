<?php

/**
 * Set the breadcrumb using a context reaction.
 */
class context_reaction_breadcrumb extends context_reaction_menu {
  /**
   * Override of execute().
   */
  function execute(&$vars = NULL) {
    if ($active_paths = $this->get_active_paths()) {
      $breadcrumb = array(l(t('Home'), '<front>', array('purl' =>array('disabled' => TRUE))));
      foreach ($active_paths as $path) {
        $result = db_query("SELECT p1, p2, p3, p4, p5, p6, p7, p8 FROM {menu_links} WHERE hidden = 0 AND link_path = '%s'", $path);
        while ($parents = db_fetch_array($result)) {
          $set = FALSE;
          foreach (array_filter($parents) as $plid) {
            $parent = menu_link_load($plid);
            if ($parent && $parent['access'] && empty($parent['hidden']) && !empty($parent['title'])) {
              $set = TRUE;
              $breadcrumb[] = l($parent['title'], $parent['href']);
            }
          }
          // Only set the breadcrumb if one or more links were added to the
          // trail. If not, continue iterating through possible menu links.
          if ($set) {
            drupal_set_breadcrumb($breadcrumb);
            break;
          }
        }
      }
    }
  }
}

