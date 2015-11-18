<?php

/**
 * @file
 * Admin form for Islandora Authority.
 */

/**
 * Gets the form definition for the Islandora Authority admin page.
 *
 * @see drupal_get_form().
 */
function islandora_authority_admin_form($form, &$form_state) {
  return system_settings_form(array(
    'islandora_authority_validate_fields' => array(
      '#type' => 'checkbox',
      '#title' => t('Validate Islandora Authority Fields In-Form'),
      '#description' => t('Causes a green checkmark to appear beside any Islandora Authority textfields in XML forms whose value matches an indexed value for that field in Solr.'),
      '#default_value' => isset($form_state['values']['islandora_authority_validate_fields']) ? $form_state['values']['islandora_authority_validate_fields'] : variable_get('islandora_authority_validate_fields', FALSE),
    ),
  ));
}
