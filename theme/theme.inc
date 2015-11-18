<?php

/**
 * @file
 * Theme implementations.
 */

/**
 * Theme implementation for custom textfield.
 *
 * Copy/pasta from Drupal core, diverging only when specifying the class and ID
 * of the hidden autocomplete element.
 * ... And code formatting, to find our standards.
 *
 * @see theme_textfield()
 */
function theme_islandora_authority_textfield($variables) {
  $element = $variables['element'];
  $element['#attributes']['type'] = 'text';
  element_set_attributes($element, array(
    'id',
    'name',
    'value',
    'size',
    'maxlength',
  ));
  _form_set_class($element, array('form-text'));

  $extra = '';
  if ($element['#autocomplete_path'] && drupal_valid_path($element['#autocomplete_path'])) {
    $element['#attributes']['class'][] = 'form-autocomplete';

    $attributes = array();
    $attributes['type'] = 'hidden';
    $attributes['id'] = $element['#attributes']['id'] . '--islandora-authority-autocomplete';
    $attributes['value'] = url($element['#autocomplete_path'], array('absolute' => TRUE));
    $attributes['disabled'] = 'disabled';
    $attributes['class'][] = 'islandora-authority-autocomplete';
    $extra = '<input' . drupal_attributes($attributes) . ' />';
    if (isset($element['#validation_path'])) {
      $validation_attributes = array();
      $validation_attributes['type'] = 'hidden';
      $validation_attributes['id'] = $element['#attributes']['id'] . '--islandora-authority-autocomplete-validate';
      $validation_attributes['value'] = url($element['#validation_path'], array('absolute' => TRUE));
      $validation_attributes['disabled'] = 'disabled';
      $validation_attributes['class'][] = 'islandora-authority-autocomplete-validate';
      $extra .= '<input' . drupal_attributes($validation_attributes) . ' />';
    }
  }

  $output = '<input ' . drupal_attributes($element['#attributes']) . ' />';

  return $output . $extra;
}

/**
 * Process callback for islandora_authority elements.
 */
function islandora_authority_element_process($element, &$form_state, $complete_form) {
  $element['#id'] = $element['#attributes']['id'] = implode('--', array(
    $complete_form['#build_id'],
    $element['#hash'],
  ));
  $form_state['cache'] = TRUE;
  return $element;
}

/**
 * Process callback for islandora_authority_textfield.
 */
function islandora_authority_textfield_process($element, &$form_state, $complete_form) {
  $element['#autocomplete_path'] = implode('/', array(
    $element['#autocomplete_path'],
    $complete_form['#build_id'],
    $element['#hash'],
  ));
  if (variable_get('islandora_authority_validate_fields', FALSE)) {
    $element['#validation_path'] = implode('/', array(
      'islandora/authority-validate',
      $complete_form['#build_id'],
      $element['#hash'],
    ));
  }
  return $element;
}
