<?php

//FIXME (minor): Doesn't really need $results if it's not doing hightlighting...  Anyway.
function _build_match(&$results, &$solr_doc, &$field_info) {
  $map = array(); //fieldname => !variable_name
  $mapped = array(); //!variable_name => field value
  $fields = array();
  $keys = array();

  foreach ($field_info['fields'] as $hash => &$field) {
    if (isset($field['mapping'])) {
      $map[$hash] = $field['mapping'];
    }

    $keys[] = $hash;
    $fields[] = (array) $solr_doc->{$field['display_field_name']};
  }

  $toReturn = array('alts' => array());
  $zipped = call_user_func_array('array_map', array_merge(array(NULL), $fields));

  foreach ($zipped as $index => &$value) {
    $toMerge = array();
    foreach (array_combine($keys, $value) as $hash => $content) {
      if (isset($map[$hash])) {
        $mapped[$map[$hash]] = $content;
      }
      $toMerge[$hash] = $content;
    }

    $toMerge['full-display'] = format_string($field_info['pattern'], $mapped);

    if ($index == 0) {
      $toReturn = array_merge($toReturn, $toMerge);
    }
    else {
      $toReturn['alts'][$index - 1] = $toMerge;
    }
  }
  if (empty($toReturn['alts'])) {
    unset($toReturn['alts']);
  }
  return $toReturn;
}

/**
 *
 * @param type $build_id
 * @param type $element
 * @param type $form_state
 * @return array (
 *  'pattern' => as used in t function,
 *  'current' => hash of the current element,
 *  'fields' => array(
 *    array(
 *      'field_name' => name_as_in_solr,
 *      'mapping' => as in the t pattern above.
 *    )
 *    ...
 *  )
 * )
 */
function islandora_authority_get_fields(&$build_id, &$element_hash, &$form_state = NULL) {
  if ($form_state === NULL) {
    $form_state = array('storage' => NULL, 'submitted' => FALSE, 'post' => array());
  }
  $form = form_get_cache($build_id, $form_state);
  module_load_include('inc', 'objective_forms', 'Form');
  //FIXME (minor):  If this could be cached somewhere, it would be great.
  $obj_form = new Form($form, $form_state);
  $element = $obj_form->findElement($element_hash);
  $parent = $element->parent;
  $toReturn = array(
    'pattern' => $parent->user_data['islandora_authority_t_pattern'],
    'current' => $element_hash,
    'fl' => array(),
    'fq' => explode('~', $element->user_data['islandora_authority_fq'])
  );
  $element_types = module_invoke('islandora_authority', 'element_info');
  foreach ($parent->toArray() as $sibling) {
    if (is_array($sibling) && isset($sibling['#type']) && array_key_exists($sibling['#type'], $element_types)) {
      $toReturn['fields'][$sibling['#hash']] = array(
        'search_field_name' => $sibling['#user_data']['islandora_authority_solr_search_field'],
        'display_field_name' => (!empty($sibling['#user_data']['islandora_authority_solr_display_field']) ?
            $sibling['#user_data']['islandora_authority_solr_display_field'] :
            $sibling['#user_data']['islandora_authority_solr_search_field']),
        'mapping' => $sibling['#user_data']['islandora_authority_mapping']);
      $toReturn['fl'][] = & $toReturn['fields'][$sibling['#hash']]['display_field_name'];
    }
  }
  return $toReturn;
}

/**
 * Page/autocomplete callback to grab results from Solr.
 */
function islandora_authority_autocomplete($build_id, $element, $string = '') {
  module_load_include('php', 'islandora_solr', 'SolrPhpClient/Apache/Solr/Service');
  $solr_url = variable_get('islandora_solr_url', 'http://localhost:8080/solr/');
  $path_parts = parse_url($solr_url);
  // TODO: Make this use the IslandoraSolrQueryProcessor class.
  $solr = new Apache_Solr_Service($path_parts['host'], $path_parts['port'], $path_parts['path']);
  $solr->setCreateDocuments(FALSE);
  $matches = array();
  // FIXME (minor): If this could be cached at some level, it would be great...
  $fields = islandora_authority_get_fields($build_id, $element);
  $results = $solr->search($string, 0, 1000, array(
    'qf' => $fields['fields'][$element]['search_field_name'],
    'fl' => implode(',', (array) $fields['fl']),
    'fq' => implode(' ', $fields['fq']),
    'defType' => 'dismax')
  );
  $matches = array();
  if (isset($results)) {
    foreach ($results->response->docs as &$doc) {
      $matches[$doc->PID] = _build_match($results, $doc, $fields);
    }
  }
  drupal_json_output($matches);
}
