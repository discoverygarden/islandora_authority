<?php

/**
 * @file
 * OCLC FAST search controller.
 */

/**
 * OCLC FAST search controller.
 */
class IslandoraAuthorityOCLCFastController extends IslandoraAuthoritySolrController {
  const NAME = 'oclc_fast';
  const ID_FIELD = 'idroot';

  /**
   * {@inheritdoc}
   */
  protected function parseConfig() {
    $config = array(
      'index' => $this->parent->user_data['islandora_authority_oclc_fast_index'],
      'pattern' => $this->parent->user_data['islandora_authority_t_pattern'],
      'current' => $this->elementHash,
      'fl' => array(),
      'fields' => array(),
    );
    $element_types = module_invoke('islandora_authority', 'element_info');
    foreach ($this->parent->toArray() as $sibling) {
      if (is_array($sibling) && isset($sibling['#type']) && array_key_exists($sibling['#type'], $element_types)) {
        $config['fields'][$sibling['#hash']] = array_filter(array(
          'display_field_name' => $sibling['#user_data']['islandora_authority_oclc_fast_field'],
          'mapping' => drupal_array_get_nested_value($sibling['#user_data'], array('islandora_authority_mapping')),
        ));
        $config['fl'][] = $config['fields'][$sibling['#hash']]['display_field_name'];
      }
    }

    return $config;
  }

  /**
   * {@inheritdoc}
   */
  public function search($string) {
    $fields = array_merge($this->fieldInfo['fl'], array(static::ID_FIELD));
    $pseudo_field_info = $this->pseudoFieldInfo($fields);

    $response = drupal_http_request(url('http://fast.oclc.org/searchfast/fastsuggest', array(
      'query' => array(
        'query' => $string,
        'queryIndex' => $this->fieldInfo['index'],
        'queryReturn' => implode(',', array_diff($fields, array_keys($pseudo_field_info))),
        // XXX: Undocument parameter; however, used in their examples.
        'suggest' => 'autoSubject',
        'numRows' => 20,
        // XXX: Undocumented parameter for their API; presumably works based on
        // passing some parameters direct to Solr on their end.
        'sort' => 'score desc',
      ),
    )));
    $matches = array();
    dsm($response->data);
    foreach (drupal_json_decode($response->data)['response']['docs'] as $doc) {
      foreach ($pseudo_field_info as $name => $info) {
        // XXX: Add our pseudo-field(s).
        $doc[$name] = $info['callable']($doc);
      }
      $matches[$doc[static::ID_FIELD]] = $this->buildMatch($doc);
    }

    return $matches;
  }

  /**
   * @param array $fields
   *   A reference to the fields being returned in the present response. If any
   *   pseudo-fields are referenced but missing requirements, we'll add in the
   *   missing requirements.
   *
   * @return array
   *   An associative array mapping pseudo-field to information about their
   *   generation, including:
   *   - requires: An array of fields in the result which are required for this
   *     field to be generated.
   *   accepting the associative array representing the Solr result from 
   */
  protected function pseudoFieldInfo(&$fields) {
    $info = array(
      static::ID_FIELD . '_uri' => array(
        'requires' => array(
          static::ID_FIELD,
        ),
        'callable' => function (array $doc) {
          return format_string('http://experimental.worldcat.org/fast/!id', array(
             '!id' => $doc[static::ID_FIELD],
          ));
        },
      ),
    );

    $to_return = array_intersect_key($info, drupal_map_assoc($fields));
    foreach ($to_return as $field) {
      $missing = array_diff($fields, $field['requires']);
      if ($missing) {
        $fields = array_merge($fields, $missing);
      }
    }

    return $to_return;
  }

  /**
   * {@inheritdoc}
   */
  public function validate($string) {
    return NULL;
  }

  /**
   * {@inheritDoc}
   */
  public function fieldHasValidation() {
    return FALSE;
  }

}
