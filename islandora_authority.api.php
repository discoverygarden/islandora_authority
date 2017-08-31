<?php

/**
 * @file
 * API documentation.
 */

/**
 * Get the mapping of usable controller names.
 *
 * @return array
 *   An associative array mapping the (unique) names by which a controller may
 *   be reference in a form to its corresponding PHP classname.
 */
function hook_islandora_authority_controllers() {
  return array(
    'solr' => 'IslandoraAuthoritySolrController',
  );
}
