<?php

/**
 * @file
 * AAT search controller.
 */

/**
 * OCLC FAST search controller.
 */
class IslandoraAuthorityAATController extends IslandoraAuthorityAbstractController {
  const NAME = 'aat';

  /**
   * {@inheritdoc}
   */
  public function search($string) {
    $query = <<<EOT
select distinct ?Term {
  ?Subject luc:term "{$string}*";
           a ?typ.
  ?typ rdfs:subClassOf gvp:Subject;
       rdfs:label ?Type.
  filter (?typ != gvp:Subject)
  ?Subject gvp:prefLabelGVP [xl:literalForm ?Term]
  filter(lang(?Term) = "en")
} limit 20
EOT;
    $ch = curl_init();
    $url = url('http://vocab.getty.edu/sparql.csv', array(
      'absolute' => TRUE,
      'query' => array('query' => $query),
    ));
    curl_setopt($ch, CURLOPT_URL, $url);
    curl_setopt($ch, CURLOPT_RETURNTRANSFER, TRUE);
    curl_setopt($ch, CURLOPT_CONNECTTIMEOUT, 10);
    curl_setopt($ch, CURLOPT_TIMEOUT, 10);
    $output = curl_exec($ch);
    $http_code = curl_getinfo($ch, CURLINFO_HTTP_CODE);
    curl_close($ch);
    $results = array();
    if ($http_code == 200) {
      $results = explode("\r\n", $output);
      // Remove the header and the trailing newline.
      array_shift($results);
      array_pop($results);
      $results = drupal_map_assoc($results);
      $mapped = array_map(array($this, 'buildMatch'), $results);
      return $mapped;
    }
    else {
      watchdog('islandora_authority_oclc_fast', 'Search for term "@term" failed.', array('@term' => $string));
      return array(
        'invalid' => array(
          'full-display' => t('Invalid string for autocomplete: "@string"', array(
            '@string' => $string,
          )),
        ),
      );
    }

  }

  /**
   * Helper; transform a result into the form we expect.
   *
   * @param string $result
   *   The string result of the value autocompleted from the AAT endpoint.
   *
   * @return array
   *   The array for a particular value, as returned from
   *   IslandoraAuthorityAbstractController::search().
   */
  public function buildMatch($result) {
    return array(
      $this->elementHash => $result,
      'full-display' => $result,
    );
  }

  /**
   * {@inheritdoc}
   */
  public function validate($string) {
    return NULL;
  }

  /**
   * {@inheritdoc}
   */
  public function fieldHasValidation() {
    return FALSE;
  }

}
