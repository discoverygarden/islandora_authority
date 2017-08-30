<?php

/**
 * @file
 * Abstract controller class.
 */

/**
 * Abstract base controller implementation.
 */
abstract class IslandoraAuthorityAbstractController {
  const NAME = 'abstract';

  /**
   * The element hash of the target element (convenience).
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
   *
   * @param FormElement $element
   *   An objective forms instance of the target element.
   */
  public function __construct(FormElement $element) {
    $this->elementHash = $element->hash;
    $this->element = $element;
    $this->parent = $this->element->parent;
  }

  /**
   * Factory function; instantiate the correct controller.
   *
   * @param string $build_id
   *   A form build ID, so we can grab the relevant info from the form cache.
   * @param string $element_hash
   *   An objective forms element hash, so we can identify the target element
   *   and its associated elements.
   *
   * @return IslandoraAuthorityAbstractController
   *   An instance of a subclass, as indicated by the element's
   *   #user_data/islandora_authority_controller value; defaults to the "solr"
   *   controller.
   */
  public static function createFromBuildAndHash($build_id, $element_hash) {
    $form_state = form_state_defaults();
    $form = form_get_cache($build_id, $form_state);

    return static::createFromFormAndState($element_hash, $form, $form_state);
  }

  /**
   * Factory function; instantiate the correct controller.
   *
   * @param string $element_hash
   *   An objective forms element hash, so we can identify the target element
   *   and its associated elements.
   * @param array $form
   *   The form structure being manipulated.
   * @param array $form_state
   *   The form state of the target form.
   *
   * @return IslandoraAuthorityAbstractController
   *   An instance of a subclass, as indicated by the element's
   *   #user_data/islandora_authority_controller value; defaults to the "solr"
   *   controller.
   */
  public static function createFromFormAndState($element_hash, array $form, array $form_state) {
    module_load_include('inc', 'objective_forms', 'Form');
    $obj_form = new Form($form, $form_state);
    $element = $obj_form->findElement($element_hash);

    // Default to the Solr controller, so existing forms continue to work.
    $controller = isset($element->parent->user_data['islandora_authority_controller']) ?
      $element->parent->user_data['islandora_authority_controller'] :
      IslandoraAuthoritySolrController::NAME;

    $controllers = module_invoke_all('islandora_authority_controllers');
    $class = $controllers[$controller];

    if (!is_subclass_of($class, __CLASS__)) {
      throw new UnexpectedValueException(t('The controller identified by @name does not appear to be a subclass of @class.', array(
        '@name' => $controller,
        '@class' => __CLASS__,
      )));
    }

    return new $class($element);
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
  abstract public function search($string);

  /**
   * Validate a string in the current field.
   *
   * @param string $string
   *   The string being validated.
   *
   * @return bool|null
   *   TRUE if the value appears to be valid; otherwise, FALSE. NULL if
   *   validation is not configured for this field.
   */
  public function validate($string) {
    return NULL;
  }

  /**
   * Determine if the given field should run through the validation process.
   *
   * @return bool
   *   TRUE if it should; otherwise, FALSE.
   */
  public function fieldHasValidation() {
    return FALSE;
  }

}
