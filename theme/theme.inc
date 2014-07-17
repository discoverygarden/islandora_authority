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
  }

  $output = '<input ' . drupal_attributes($element['#attributes']) . ' />';

  return $output . $extra;
}
