<?php

/**
 * @file
 * Autocomplete callback and friends.
 */

/**
 * Page/autocomplete callback to grab results from Solr.
 */
function islandora_authority_autocomplete($build_id, $element, $string = '') {
  drupal_json_output(array(
    'match' => IslandoraAuthorityAbstractController::create($build_id, $element)->search($string),
  ));
  return;
}

/**
 * Callback to validate the contents of an autocomplete field against Solr.
 */
function islandora_authority_validate($build_id, $element, $string = '') {
  drupal_json_output(array(
    'match' => IslandoraAuthorityAbstractController::create($build_id, $element)->validate($string),
  ));
  return;
}

abstract class IslandoraAuthorityAbstractController {
  const NAME = 'abstract';

  /**
   * The element hash of the target element.
   *
   * @var string
   */
  protected $elementHash;

  /**
   * The target element.
   *
   * @var FormElement
   */
  protected $element;

  /**
   * The parent of the target element.
   *
   * Just for convenience.
   *
   * @var FormElement
   */
  protected $parent;

  /**
   * Constructor.
   */
  public function __construct($element_hash, $element) {
    $this->elementHash = $element_hash;
    $this->element = $element;
    $this->parent = $this->element->parent;
  }

  public static function create($build_id, $element_hash) {
    $form_state = form_state_defaults();
    $form = form_get_cache($build_id, $form_state);
    module_load_include('inc', 'objective_forms', 'Form');
    $obj_form = new Form($form, $form_state);
    $element = $obj_form->findElement($element_hash);

    $controller = $element->user_data['islandora_authority_controller'];
    if (!$controller) {
      // Default to the Solr controller.
      $controller = IslandoraAuthoritySolrController::NAME;
    }

    $class = module_invoke_all('islandora_authority_controllers')[$controller];

    return new $class(
      $element_hash,
      $element
    );
  }

  /**
   * Get results for the given field.
   *
   * @param string $string
   *   The string being searched.
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
   */
  public function search($string);

  /**
   * Validate a string in the current field.
   *
   * @param string $string
   *   The string being validated.
   *
   * @return bool|NULL
   *   TRUE if the value appears to be valid; otherwise, FALSE. NULL if
   *   validation is not configured for this field.
   */
  public function validate($string);
}

class IslandoraAuthoritySolrController extends IslandoraAuthorityAbstractController {
  const NAME = 'solr';

  /**
   * Associative array of info parsed from the target elements.
   *
   * @var array
   */
  protected $fieldInfo;

  public function __construct($build_id, $element_hash) {
    parent::__construct($build_id, $element_hash);

    $this->fieldInfo = array(
      'pattern' => $this->parent->user_data['islandora_authority_t_pattern'],
      'current' => $element_hash,
      'fl' => array(),
      'fq' => (isset($this->parent->user_data['islandora_authority_fq']) ?
        explode('~', $this->parent->user_data['islandora_authority_fq']) :
        array()),
      'fields' => array(),
    );
    $element_types = module_invoke('islandora_authority', 'element_info');
    foreach ($this->parent->toArray() as $sibling) {
      if (is_array($sibling) && isset($sibling['#type']) && array_key_exists($sibling['#type'], $element_types)) {
        $this->fieldInfo['fields'][$sibling['#hash']] = array_filter(array(
          'search_field_name' => $sibling['#user_data']['islandora_authority_solr_search_field'],
          'display_field_name' => (!empty($sibling['#user_data']['islandora_authority_solr_display_field']) ?
            $sibling['#user_data']['islandora_authority_solr_display_field'] :
            $sibling['#user_data']['islandora_authority_solr_search_field']),
          'validate_field_name' => (!empty($sibling['#user_data']['islandora_authority_solr_validate_field']) ?
            $sibling['#user_data']['islandora_authority_solr_validate_field'] :
            NULL),
          'mapping' => drupal_array_get_nested_value($sibling['#user_data'], array('islandora_authority_mapping')),
          'search_without_dismax' => (isset($sibling['#user_data']['islandora_authority_search_without_dismax']) ?
            $sibling['#user_data']['islandora_authority_search_without_dismax'] :
            FALSE),
        ));
        $this->fieldInfo['fl'][] = $this->fieldInfo['fields'][$sibling['#hash']]['display_field_name'];
      }
    }
  }

  /**
   * {@inheritdoc}
   */
  public function search($string) {
    module_load_include('inc', 'islandora_solr', 'includes/utilities');
    $qp = new IslandoraSolrQueryProcessor();
    $escaped_field = islandora_solr_lesser_escape($this->fieldInfo['fields'][$this->elementHash]['search_field_name']);
    if (isset($this->fieldInfo['fields'][$this->elementHash]['search_without_dismax'])) {
      $qp->buildQuery(format_string('!field:(!value*)', array(
        '!field' => $escaped_field,
        '!value' => $string,
      )));
    }
    else {
      $qp->buildQuery($string);
      $qp->solrParams['qf'] = $escaped_field;
      $qp->solrParams['defType'] = 'dismax';
    }
    $qp->solrParams['fl'] = implode(',', array_merge((array) $this->fieldInfo['fl'], array('PID')));
    $qp->solrParams['fq'] = $fields['fq'];
    $qp->executeQuery(FALSE, TRUE);
    $matches = array();
    foreach ($qp->islandoraSolrResult['response']['objects'] as $object) {
      $doc = $object['solr_doc'];
      $matches[$doc['PID']] = $this->buildMatch($doc);
    }

    return $matches;
  }

  /**
   * Helper; transform a Solr document into what we need.
   *
   * @param array $solr_doc
   *   An associative array representing a result from Solr.
   *
   * @return array
   */
  protected function buildMatch($solr_doc) {
    // An associative array mapping Solr field names to format_string()
    // placeholders.
    $map = array();
    // An associative array mapping format_string() placeholders to the values
    // each should recieve.
    $mapped = array();

    $fields = array();
    $keys = array();

    foreach ($this->fieldInfo['fields'] as $hash => $field) {
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
      foreach (array_combine($keys, (array) $value) as $hash => $content) {
        if (isset($map[$hash])) {
          $mapped[$map[$hash]] = $content;
        }
        $to_merge[$hash] = $content;
      }

      $to_merge['full-display'] = format_string($this->fieldInfo['pattern'], $mapped);

      if ($index == 0) {
        $to_return = array_merge($to_return, $to_merge);
      }
      else {
        $to_return['alts'][] = $to_merge;
      }
    }
    return $to_return;
  }
}
