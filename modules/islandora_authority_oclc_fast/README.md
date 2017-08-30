# Islandora Authority OCLC FAST Provider

## Introduction

This module provides the ability to autocomplete against [OCLC's assignFAST endpoint](http://www.oclc.org/developer/develop/web-services/fast-api/assign-fast.en.html).

## Requirements

This module requires the following modules/libraries:

* [Islandora Authority](https://github.com/discoverygarden/islandora_authority)

## Installation

Install as usual, see [this](https://drupal.org/documentation/install/modules-themes/modules-7) for further information.

## Controller Configuration

Machine name: `oclc_fast`

This controller uses an adapted subset of Solr controllers configuration.

The parent element accepts:
* `islandora_authority_t_pattern`:  A pattern as used by Drupal's `format_string()`/`t()` function.
* `islandora_authority_fq`: A list of tilde-separated Lucene statements, e.g.: `PID:ir*`, to ensure all results come from the IR namespace. (Note: separating is done via a naive explode; tildes in values break it)
* `islandora_authority_oclc_fast_index`: The name of the query index against which to query. See the [endpoint's "Query Indicies" table](http://www.oclc.org/developer/develop/web-services/fast-api/assign-fast.en.html) for a list of those available.

Each contained authority element specifies:
* `islandora_authority_oclc_fast_field`: The field referenced by the given element. See the [endpoint's "FAST Heading Response Fields" table](http://www.oclc.org/developer/develop/web-services/fast-api/assign-fast.en.html) for a more authoritative list. They presently define:
    * `idroot`
    * `auth`
    * `type`
    * `tag`
    * `raw`
    * `breaker`
    * `indicator`

    For convenience, we additionally generate:
    * `idroot_uri`, returning a URL to the given result.
* `islandora_authority_mapping`: An optional placeholder, which will make values for this field available for substitution into the `t_pattern` above.

## Maintainers/Sponsors

Current maintainers:

* [discoverygarden Inc.](https://github.com/discoverygarden)

Sponsors:

* [Discovery Virginia](http://virginiahumanities.org/discovery-virginia/)

## Development

If you would like to contribute to this module, please check out our helpful [Documentation for Developers](https://github.com/Islandora/islandora/wiki#wiki-documentation-for-developers) info, as well as our [Developers](http://islandora.ca/developers) section on the Islandora.ca site.

Also include any Travis gotcha's here.

## License

[GPLv3](http://www.gnu.org/licenses/gpl-3.0.txt)
