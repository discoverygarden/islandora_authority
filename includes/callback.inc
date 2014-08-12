<?php

/**
 * @file
 * Autocomplete callback and friends.
 */

/**
 * Build the structure expected client-side.
 *
 * @param array $solr_doc
 *   An associative array mapping field names to either strings or arrays.
 * @param array $field_info
 *   An associative array as returned by islandora_authority_get_fields().
 *
 * @return array
 *   An associative array containing:
 *   - full-display: A string to display
 *   - alts: An indexed array of "alternative" entries, each containing:
 *     - full-display: A string to display
 *     And additional keys representing element hashes mapped to values the
 *     relevant elements should recieve.
 *   And additional keys representing element hashes mapped to values the
 *   relevant elements should recieve.
 *
 * @todo Make the structure of things returned here a little more sane.
 */
function islandora_authority_build_match($solr_doc, $field_info) {
  // An associative array mapping Solr field names to format_string()
  // placeholders.
  $map = array();
  // An associative array mapping format_string() placeholders to the values
  // each should recieve.
  $mapped = array();

  $fields = array();
  $keys = array();

  foreach ($field_info['fields'] as $hash => $field) {
    if (isset($field['mapping'])) {
      $map[$hash] = $field['mapping'];
    }

    $keys[] = $hash;
    $fields[] = isset($solr_doc[$field['display_field_name']]) ?
      (array) $solr_doc[$field['display_field_name']] :
      array();
  }

  $to_return = array('alts' => array());
  $counts = array_map('count', $fields);
  $min_count = min($counts);
  $max_count = max($counts);
  if ($min_count != $max_count) {
    foreach ($fields as &$field) {
      $count = count($field);
      $pad_value = '';
      if ($count == 1) {
        // If there happens to be only a single value, assume that it should be
        // repeated for each: associating the PID with each result, for example.
        $pad_value = end($field);
      }
      $field = array_pad($field, $max_count, $pad_value);
    }
    unset($field);
  }
  $zipped = call_user_func_array('array_map', array_merge(array(NULL), $fields));

  foreach ($zipped as $index => &$value) {
    $to_merge = array();
    foreach (array_combine($keys, $value) as $hash => $content) {
      if (isset($map[$hash])) {
        $mapped[$map[$hash]] = $content;
      }
      $to_merge[$hash] = $content;
    }

    $to_merge['full-display'] = format_string($field_info['pattern'], $mapped);

    if ($index == 0) {
      $to_return = array_merge($to_return, $to_merge);
    }
    else {
      $to_return['alts'][] = $to_merge;
    }
  }
  return $to_return;
}

/**
 * Get info for associated fields.
 *
 * @param string $build_id
 *   Build ID of the form, so we can grab the form from cache to be able to
 *   relate back to form elements on the client side.
 * @param string $element_hash
 *   The element in which the user is currently autocompleting.
 *
 * @return array
 *   - pattern: A string to use as the first parameter of "format_string", as a
 *     template of what will be shown to users.
 *   - current: A string containing the hash of the current element.
 *   - fields: An array of associative arrays, each containing:
 *     - field_name: A string containing the Solr field name
 *     - mapping: A string containing a placeholder in the "pattern" above.
 */
function islandora_authority_get_fields($build_id, $element_hash) {
  $form_state = form_state_defaults();
  $form = form_get_cache($build_id, $form_state);
  module_load_include('inc', 'objective_forms', 'Form');
  $obj_form = new Form($form, $form_state);
  $element = $obj_form->findElement($element_hash);
  $parent = $element->parent;
  $to_return = array(
    'pattern' => $parent->user_data['islandora_authority_t_pattern'],
    'current' => $element_hash,
    'fl' => array(),
    'fq' => (isset($parent->user_data['islandora_authority_fq']) ?
      explode('~', $parent->user_data['islandora_authority_fq']) :
      array()),
    'fields' => array(),
  );
  $element_types = module_invoke('islandora_authority', 'element_info');
  foreach ($parent->toArray() as $sibling) {
    if (is_array($sibling) && isset($sibling['#type']) && array_key_exists($sibling['#type'], $element_types)) {
      $to_return['fields'][$sibling['#hash']] = array_filter(array(
        'search_field_name' => $sibling['#user_data']['islandora_authority_solr_search_field'],
        'display_field_name' => (!empty($sibling['#user_data']['islandora_authority_solr_display_field']) ?
          $sibling['#user_data']['islandora_authority_solr_display_field'] :
          $sibling['#user_data']['islandora_authority_solr_search_field']),
        'mapping' => drupal_array_get_nested_value($sibling['#user_data'], array('islandora_authority_mapping')),
      ));
      $to_return['fl'][] = $to_return['fields'][$sibling['#hash']]['display_field_name'];
    }
  }
  return $to_return;
}

/**
 * Page/autocomplete callback to grab results from Solr.
 */
function islandora_authority_autocomplete($build_id, $element, $string = '') {
  $fields = islandora_authority_get_fields($build_id, $element);
  $qp = new IslandoraSolrQueryProcessor();
  $qp->buildQuery($string);
  $qp->solrParams['qf'] = $fields['fields'][$element]['search_field_name'];
  $qp->solrParams['fl'] = implode(',', array_merge((array) $fields['fl'], array('PID')));
  $qp->solrParams['fq'] = $fields['fq'];
  $qp->solrParams['defType'] = 'dismax';
  $qp->executeQuery(FALSE, TRUE);
  $matches = array();
  foreach ($qp->islandoraSolrResult['response']['objects'] as $object) {
    $doc = $object['solr_doc'];
    $matches[$doc['PID']] = islandora_authority_build_match($doc, $fields);
  }
  drupal_json_output($matches);
}
