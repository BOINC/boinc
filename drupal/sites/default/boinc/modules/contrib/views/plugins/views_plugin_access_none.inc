<?php

/**
 * Access plugin that provides no access control at all.
 */
class views_plugin_access_none extends views_plugin_access {
  function summary_title() {
    return t('Unrestricted');
  }
}
