<?php

/**
 * @file
 * Abstract controller class.
 */

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
