<?php

/**
 * @file
 * Theme implementations.
 */

function islandora_authority_element_process($element, $edit, &$form_state) {
  $element['#id'] = implode('--', array(
    $form_state['#build_id'],
    $element['#hash'],
  ));
  $form_state['cache'] = TRUE;
  return $element;
}

function islandora_authority_textfield_process($element, $edit, &$form_state) {
  $element['#autocomplete_path'] = implode('/', array(
    $element['#autocomplete_path'],
    $form_state['#build_id'],
    $element['#hash'],
  ));
  return $element;
}

/**
 * Theme implementation for custom textfield...  Only difference from 'regular'
 *  textfield is the use of the 'islandora_authority_id' instead of the 'real' id
 * @param array $element 'preprocessed' element definition.
 * @return string Containing the markup for the given element
 */
function theme_islandora_authority_textfield($variables) {
  $element = $variables['element'];
  $element['#attributes']['type'] = 'text';
  element_set_attributes($element, array('id', 'name', 'value', 'size', 'maxlength'));
  _form_set_class($element, array('form-text'));

  dsm('a buh?');

  $extra = '';
  if ($element['#autocomplete_path'] && drupal_valid_path($element['#autocomplete_path'])) {
    $element['#attributes']['class'][] = 'form-islandora-authority-autocomplete';

    $attributes = array();
    $attributes['type'] = 'hidden';
    $attributes['id'] = $element['#attributes']['id'] . '-islandora-authority-autocomplete';
    $attributes['value'] = url($element['#autocomplete_path'], array('absolute' => TRUE));
    $attributes['disabled'] = 'disabled';
    $attributes['class'][] = 'islandora-authority-autocomplete';
    $extra = '<input' . drupal_attributes($attributes) . ' />';
  }

  $element_copy = $element;
  $element_copy['#autocomplete_path'] = '';
  $element_copy['#theme'] = $element_copy['#type'] = 'textfield';

  $output = drupal_render($element_copy);

  return $output . $extra;
}
