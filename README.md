# Islandora Authority Module [![Build Status](https://travis-ci.org/discoverygarden/islandora_authority.png?branch=7.x)](https://travis-ci.org/discoverygarden/islandora_authority)

## Introduction

This module adds two Drupal form API elements and a menu path used for autocompletion used for autocompletion on one of them. These two new elements are:
* `islandora_authority_textfield`: A textfield which can autocomplete on a Solr field.
* `islandora_authority_hidden`: A hidden element which can be autocompleted, as a result of a search.

One caveat: Due to how we perform lookups, forms in which these elements _must_ pass through Objective Forms to recieve `#hash` values.

## Requirements

This module requires the following modules/libraries:

* [Islandora Solr](https://github.com/Islandora/islandora_solr)
* [Objective Forms](https://github.com/Islandora/objective_forms)

## Installation

Install as usual, see [this](https://drupal.org/documentation/install/modules-themes/modules-7) for further information.

## Configuration

Our configuration is stored in `#user_data` on our `islandora_authority_textfield` and `islandora_authority_hidden` elements.

Generally, multiple `islandora_authority` elements are direct children of a common parent. The framework defines a single parameter on the parent:
* `islandora_authority_controller`: A machine name indicating the controller to use. Defaults to `solr` if otherwise unspecified.

### Solr Controller

Machine name: `solr`

This controller is attempted if no other is specified.

The Solr controller defines two parameters on the parent element:
* `islandora_authority_t_pattern`:  A pattern as used by Drupal's `format_string()`/`t()` function.
* `islandora_authority_fq`: A list of tilde-separated Lucene statements, e.g.: `PID:ir*`, to ensure all results come from the IR namespace. (Note: separating is done via a naive explode; tildes in values break it)

Each contained authority element specifies:
* `islandora_authority_solr_search_field`: Should be an EdgeNGram'd Solr field.
* `islandora_authority_solr_validate_field`: An optional Solr field to validate the contents of the element against. This does not prevent the form from being saved; it simply provides visual feedback on form load and element change for whether the field contains an authorized value. This should be an untokenized Solr field.
* `islandora_authority_solr_display_field`: An optional Solr field, used for display purposes (defaults to the search field).
* `islandora_authority_mapping`: An optional placeholder, which will make values for this field available for substitution into the `t_pattern` above.
* `islandora_authority_search_without_dismax`: A flag which should be set (to "true") when `islandora_authority_solr_search_field` is not an EdgeNGram'd Solr field, so queries will be made as (non-dismax) prefix queries instead of depending on the behaviour of EdgeNGrams. Note that EdgeNGrams are more predicatable in the results they return; prefix/wildcard queries [skip analysis](https://wiki.apache.org/solr/AnalyzersTokenizersTokenFilters#Analyzers), so indexing analysis can result in false-negatives.

## Troubleshooting/Issues

Having problems or solved a problem? Contact
[discoverygarden](http://support.discoverygarden.ca).

## Maintainers/Sponsors

Current maintainers:

* [discoverygarden Inc.](https://github.com/discoverygarden)

## Development

If you would like to contribute to this module, please check out our helpful [Documentation for Developers](https://github.com/Islandora/islandora/wiki#wiki-documentation-for-developers) info, as well as our [Developers](http://islandora.ca/developers) section on the Islandora.ca site.

## License

[GPLv3](http://www.gnu.org/licenses/gpl-3.0.txt)
