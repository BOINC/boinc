<?php
/**
 * @file
 * Contains the Aggregator Item RSS row style plugin.
 */

/**
 * Plugin which loads an aggregator item and formats it as an RSS item.
 */
class views_plugin_row_aggregator_rss extends views_plugin_row {
  function option_definition() {
    $options = parent::option_definition();

    $options['item_length'] = array('default' => 'default');

    return $options;
  }

  function options_form(&$form, &$form_state) {
    $form['item_length'] = array(
      '#type' => 'select',
      '#title' => t('Display type'),
      '#options' => array(
        'fulltext' => t('Full text'),
        'teaser' => t('Title plus teaser'),
        'title' => t('Title only'),
        'default' => t('Use default RSS settings'),
      ),
      '#default_value' => $this->options['item_length'],
    );
  }

  function render($row) {
    $sql =  "SELECT ai.iid, ai.fid, ai.title, ai.link, ai.author, ai.description, ";
    $sql .= "ai.timestamp, ai.guid, af.title AS feed_title, ai.link AS feed_LINK ";
    $sql .= "FROM {aggregator_item} ai LEFT JOIN {aggregator_feed} af ON ai.fid = af.fid ";
    $sql .= "WHERE ai.iid = %d";

    $item = db_fetch_object(db_query($sql, $row->iid));

    $item->elements = array(
      array('key' => 'pubDate', 'value' => gmdate('r', $item->timestamp)),
      array(
        'key' => 'dc:creator',
        'value' => $item->author,
        'namespace' => array('xmlns:dc' => 'http://purl.org/dc/elements/1.1/'),
      ),
      array(
        'key' => 'guid',
        'value' => $item->guid,
        'attributes' => array('isPermaLink' => 'false')
      ),
    );

    foreach ($item->elements as $element) {
      if (isset($element['namespace'])) {
        $this->view->style_plugin->namespaces = array_merge($this->view->style_plugin->namespaces, $element['namespace']);
      }
    }

    return theme($this->theme_functions(), $this->view, $this->options, $item);
  }
}

