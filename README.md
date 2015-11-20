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

Generally, multiple `islandora_authority` elements are direct children of a common parent. The parent currently has two parameters:
* `islandora_authority_t_pattern`:  A pattern as used by Drupal's `format_string()`/`t()` function.
* `islandora_authority_fq`: A list of tilde-separated Lucene statements, e.g.: `PID:ir*`, to ensure all results come from the IR namespace. (Note: separating is done via a naive explode; tildes in values break it)

Each contained authority element specifies:
* `islandora_authority_solr_search_field`: A EdgeNGram'd Solr field.
* `islandora_authority_solr_validate_field`: An optional facetable Solr field to validate the contents of the element against. This does not prevent the form from being saved; it simply provides visual feedback on form load and element change for whether the field contains an authorized value.
* `islandora_authority_solr_display_field`: An optional Solr field, used for display purposes (defaults to the search field).
* `islandora_authority_mapping`: An optional placeholder, which will make values for this field available for substitution into the `t_pattern` above.

## Troubleshooting/Issues

Having problems or solved a problem? Check out the Islandora google groups for a solution.

* [Islandora Group](https://groups.google.com/forum/?hl=en&fromgroups#!forum/islandora)
* [Islandora Dev Group](https://groups.google.com/forum/?hl=en&fromgroups#!forum/islandora-dev)

## Maintainers/Sponsors

Current maintainers:

* [discoverygarden Inc.](https://github.com/discoverygarden)

## Development

If you would like to contribute to this module, please check out our helpful [Documentation for Developers](https://github.com/Islandora/islandora/wiki#wiki-documentation-for-developers) info, as well as our [Developers](http://islandora.ca/developers) section on the Islandora.ca site.

Also include any Travis gotcha's here.

## License

[GPLv3](http://www.gnu.org/licenses/gpl-3.0.txt)
