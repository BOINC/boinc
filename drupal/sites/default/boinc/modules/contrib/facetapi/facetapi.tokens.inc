<?php

/**
 * @file
 * Builds placeholder replacement tokens for searches.
 */

/**
 * Implements hook_token_info().
 */
function facetapi_token_info() {
  $types = $results = $active = $facet = array();

  // The types of tokens.
  $types['facetapi_results'] = array(
    'name' => t('Search results'),
    'description' => t('Tokens related to the search query and response.'),
    'needs-data' => 'facetapi_adapter',
  );

  $types['facetapi_active'] = array(
    'name' => t('Active facet items'),
    'description' => t('Tokens related to active facet items.'),
    'needs-data' => 'facetapi_active_item',
  );

  $types['facetapi_facet'] = array(
    'name' => t('Facet field'),
    'description' => t('Tokens related to a facet field.'),
    'needs-data' => 'facetapi_facet',
  );

  // Tokens related to search results and response.
  $results['keys'] = array(
    'name' => t('Search keywords'),
    'description' => t('The search keywords entered by the user.'),
  );

  $results['page-number'] = array(
    'name' => t('Page number'),
    'description' => t('The page number of the result set.'),
  );

  $results['page-limit'] = array(
    'name' => t('Page limit'),
    'description' => t('The number of results displayed per page.'),
  );

  $results['page-total'] = array(
    'name' => t('Page total'),
    'description' => t('The total number of pages in the result set.'),
  );

  $results['offset'] = array(
    'name' => t('Offset'),
    'description' => t('The zero-based offset of the first element on the search page.'),
  );

  $results['start-count'] = array(
    'name' => t('Start count'),
    'description' => t('The number of the first item on the page.'),
  );

  $results['end-count'] = array(
    'name' => t('End count'),
    'description' => t('The number of the last item on the page.'),
  );

  $results['result-count'] = array(
    'name' => t('Result count'),
    'description' => t('The total number of results matched by the search query.'),
  );

  // Tokens related to active facet items.
  $active['active-value'] = array(
    'name' => t('Mapped value'),
    'description' => t('The mapped value of the active item.'),
  );

  $active['active-value-raw'] = array(
    'name' => t('Raw value'),
    'description' => t('The raw value of the active item as stored in the index.'),
  );

  $active['active-pos'] = array(
    'name' => t('Position'),
    'description' => t('The zero-based position of the active item.'),
  );

  $active['facet-label'] = array(
    'name' => t('Facet label'),
    'description' => t('The human readable label of the active item\'s facet.'),
  );

  $active['facet-name'] = array(
    'name' => t('Facet name'),
    'description' => t('The machine readable name of the active item\'s facet.'),
  );

  // Tokens related to a facet.
  $facet['facet-label'] = array(
    'name' => t('Facet label'),
    'description' => t('The human readable label of the active item\'s facet.'),
  );

  $facet['facet-name'] = array(
    'name' => t('Facet name'),
    'description' => t('The machine readable name of the active item\'s facet.'),
  );

  return array(
    'types' => $types,
    'tokens' => array(
      'facetapi_results' => $results,
      'facetapi_active' => $active,
      'facetapi_facet' => $facet,
    ),
  );
}

/**
 * Implements hook_tokens().
 */
function facetapi_tokens($type, $tokens, array $data = array(), array $options = array()) {
  $replacements = array();

  if ('facetapi_results' == $type && !empty($data['facetapi_adapter'])) {

    $adapter = $data['facetapi_adapter'];
    foreach ($tokens as $name => $original) {
      switch ($name) {

        case 'keys':
          $replacements[$original] = check_plain($adapter->getSearchKeys());
          break;

        case 'result-count':
          $replacements[$original] = (int) $adapter->getResultCount();
          break;

        case 'page-number':
          $replacements[$original] = (int) $adapter->getPageNumber();
          break;

        case 'page-limit':
          $replacements[$original] = (int) $adapter->getPageLimit();
          break;

        case 'page-total':
          $replacements[$original] = (int) $adapter->getPageTotal();
          break;

        case 'offset':
          $offset = ($adapter->getPageNumber() - 1) * $adapter->getPageLimit();
          $replacements[$original] = $offset;
          break;

        case 'start-count':
          $offset = (($adapter->getPageNumber() - 1) * $adapter->getPageLimit()) + 1;
          $replacements[$original] = $offset;
          break;

        case 'end-count':
          $page_max = $adapter->getPageNumber() * $adapter->getPageLimit();
          $total_max = (int) $adapter->getResultCount();
          $replacements[$original] = min($page_max, $total_max);
          break;
      }
    }
  }
  elseif ('facetapi_active' == $type && !empty($data['facetapi_active_item'])) {

    $item = $data['facetapi_active_item'];
    $adapter = $item['adapter'];
    foreach ($tokens as $name => $original) {
      switch ($name) {

        case 'active-value':
          $markup = $adapter->getMappedValue($item['facets'][0], $item['value']);
          $text = empty($markup['#html']) ? check_plain($markup['#value']) : $markup['#value'];
          $replacements[$original] = $text;
          break;

        case 'active-value-raw':
          $replacements[$original] = check_plain($item['value']);
          break;

        case 'active-pos':
          $replacements[$original] = $item['pos'];
          break;

        case 'facet-label':
          if ($facet = facetapi_facet_load($item['facets'][0], $adapter->getSearcher())) {
            $replacements[$original] = check_plain($facet['label']);
          }
          break;

        case 'facet-name':
          $replacements[$original] = check_plain($item['facets'][0]);
          break;
      }
    }
  }
  elseif ('facetapi_facet' == $type && !empty($data['facetapi_facet'])) {

    $facet = $data['facetapi_facet'];
    foreach ($tokens as $name => $original) {
      switch ($name) {

        case 'facet-label':
          $replacements[$original] = check_plain($facet['label']);
          break;

        case 'facet-name':
          $replacements[$original] = check_plain($facet['name']);
          break;
      }
    }
  }

  return $replacements;
}
