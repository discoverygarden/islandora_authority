(function ($) {

/**
 * Attaches the autocomplete behavior to all required fields.
 */
Drupal.behaviors.islandora_authority_autocomplete = {
  attach: function (context, settings) {
    var acdb = [];
    $('input.islandora-authority-autocomplete', context).once('islandora-authority-autocomplete', function () {
      var uri = this.value;
      if (!acdb[uri]) {
        acdb[uri] = new Drupal.ACDB(uri);
      }
      var $input = $('#' + this.id.substr(0, this.id.length - 34))
        .attr('autocomplete', 'OFF')
        .attr('aria-autocomplete', 'list');
      $($input[0].form).submit(Drupal.autocompleteSubmit);
      $input.parent()
        .attr('role', 'application')
        .append($('<span class="element-invisible" aria-live="assertive"></span>')
          .attr('id', $input.attr('id') + '-autocomplete-aria-live')
        );
      new Drupal.islandora_authority_jsAC($input, acdb[uri]);
    });
  }
};

/**
 * An AutoComplete object.
 */
Drupal.islandora_authority_jsAC = Drupal.jsAC;

var oldonkeydown = Drupal.islandora_authority_jsAC.prototype.onkeydown;

/**
 * Handler for the "keydown" event.
 */
Drupal.islandora_authority_jsAC.prototype.onkeydown = function (input, e) {
  if (!e) {
    e = window.event;
  }

  switch (e.keyCode) {
    case 37:
      this.selectLeft();
      return false;
    case 39:
      this.selectRight();
      return false;
    default: // All other keys.
      return oldonkeydown.call(this, input, e);
  }
};

/**
 * Puts the currently highlighted suggestion into the autocomplete field.
 */
Drupal.islandora_authority_jsAC.prototype.select = function (node) {
  var obj = $(node).data('autocompleteSet');
  var parents = this.input.id.split('--');
  for(var prop in obj) {
    if(obj.hasOwnProperty(prop) && typeof obj[prop] !== 'function' &&
      prop != 'full-display' && prop != 'alts') {
      //slice to get rid of the name of the current field
      var id_parts = parents.slice(0, parents.length - 1);

      //Add the part which the current property represents, while making the property
      // name match what should have been put into the id, so we can select the
      // relevant field below.
      id_parts.push(prop.replace(/(\]\[|_| )/g, '-'));

      //Update the contents of the required field.
      $('#'+id_parts.join('--')).val(obj[prop]);
    }
  }
  this.sub_unhighlight(this.sub_selected);
  this.unhighlight(this.selected);
};

var oldselectdown = Drupal.islandora_authority_jsAC.prototype.selectDown;

/**
 * Highlights the next suggestion.
 */
Drupal.islandora_authority_jsAC.prototype.selectDown = function () {
  if (this.sub_selected && this.sub_selected.nextSibling) {
    this.sub_highlight(this.sub_selected.nextSibling);
  }
  else if (this.sub_selected && this.sub_popup) {
    var lis = $('li', this.sub_popup);
    if (lis.length > 0) {
      this.sub_highlight(lis.get(0));
    }
  }
  else {
    oldselectdown.call(this)
  }
};

var oldselectup = Drupal.islandora_authority_jsAC.prototype.selectUp;

/**
 * Highlights the previous suggestion.
 */
Drupal.islandora_authority_jsAC.prototype.selectUp = function () {
  if (this.sub_selected && this.sub_selected.previousSibling) {
    this.sub_highlight(this.sub_selected.previousSibling);
  }
  else if (this.sub_selected && this.sub_popup) {
    // No-op.
  }
  else {
    oldselectup.call(this)
  }
};

Drupal.islandora_authority_jsAC.prototype.selectRight = function () {
  if (this.sub_selected) {
    this.hidePopup(false);
  }
  else if (this.selected) {
    var first_subitem = $(this.sub_popup).find('li').first().get().pop();
    if (typeof first_subitem != 'undefined') {
      this.sub_highlight(first_subitem);
    }
    else {
      this.hidePopup(false);
    }
  }
};

Drupal.islandora_authority_jsAC.prototype.selectLeft = function () {
  if (this.sub_selected) {
    this.sub_unhighlight(this.sub_selected);
  }
};

var oldhighlight = Drupal.islandora_authority_jsAC.prototype.highlight;

/**
 * Highlights a suggestion.
 */
Drupal.islandora_authority_jsAC.prototype.highlight = function (node) {
  oldhighlight.call(this, node);
  this.showSubmenu(this.selected);
};
Drupal.islandora_authority_jsAC.prototype.sub_highlight = function (node) {
  if (this.sub_selected) {
    $(this.sub_selected).removeClass('selected');
  }
  $(node).addClass('selected');
  this.sub_selected = node;
  $(this.ariaLive).html($(this.sub_selected).html());
};

/**
 * Unhighlights a suggestion.
 */
Drupal.islandora_authority_jsAC.prototype.sub_unhighlight = function (node) {
  $(node).removeClass('selected');
  this.sub_selected = false;
  $(this.ariaLive).empty();
};

/**
 * Hides the autocomplete suggestions.
 */
Drupal.islandora_authority_jsAC.prototype.hidePopup = function (keycode) {
  // Select item if the right key or mousebutton was pressed.
  if (((keycode && keycode != 46 && keycode != 8 && keycode != 27) || !keycode)) {
    if (this.sub_selected) {
      this.select(this.sub_selected);
    }
    else if (this.selected) {
      this.select(this.selected);
    }
  }

  // Hide popup.
  var sub_popup = this.sub_popup;
  if (sub_popup) {
    this.sub_popup = null;
    $(sub_popup).fadeOut('fast', function () { $(sub_popup).remove(); });
  }
  var popup = this.popup;
  if (popup) {
    this.popup = null;
    $(popup).fadeOut('fast', function () { $(popup).remove(); });
  }

  this.selected = false;
  this.sub_selected = false;
  $(this.ariaLive).empty();
};

Drupal.islandora_authority_jsAC.prototype.showSubmenu = function (node) {
  // Show popup
  var ul = $(node).find('ul');
  var same = this.sub_popup && this.sub_popup.get().pop() == ul.get().pop();
  if (!same) {
    if (this.sub_popup) {
      $(this.sub_popup).hide();
    }
    if (this.sub_selected) {
      this.sub_unhighlight(this.sub_selected);
    }
    this.sub_popup = ul
      .css({
        marginLeft: (node.offsetWidth - 4) +'px',
        width: (node.offsetWidth) +'px',
        top: (node.offsetTop) + 'px'
      })
      .show();
  }
};

/**
 * Fills the suggestion popup with any matches received.
 */
Drupal.islandora_authority_jsAC.prototype.found = function (matches) {
  // If no value in the textfield, do not show the popup.
  if (!this.input.value.length) {
    return false;
  }

  // Prepare matches.
  var ul = $('<ul></ul>');
  var ac = this;
  for (var key in matches) {
    var li = $('<li></li>');
    li
      .html($('<div></div>').html(matches[key]['full-display']))
      .mousedown(function () { ac.select(this); })
      .mouseover(function () { ac.highlight(this); })
      .mouseout(function () { ac.unhighlight(this); })
      .data('autocompleteSet', matches[key])
      .appendTo(ul);
    var alt_ul = $('<ul></ul>');
    alt_ul.hide();
    for (var prop in matches[key]['alts']) {
      $('<li></li>')
        .html($('<div></div>').html(matches[key]['alts'][prop]['full-display']))
        .mousedown(function (evt) {
          if (evt.which == 1) {
            ac.select(this);
          }
          evt.stopPropagation();
        })
        .mouseover(function () { ac.sub_highlight(this); })
        .mouseout(function () { ac.sub_unhighlight(this); })
        .data('autocompleteSet', matches[key]['alts'][prop])
        .appendTo(alt_ul);
    }
    if (alt_ul.children().length > 0) {
      $(li).append(alt_ul);
    }
  }

  // Show popup with matches, if any.
  if (this.popup) {
    if (ul.children().length) {
      $(this.popup).empty().append(ul).show();
      $(this.ariaLive).html(Drupal.t('Autocomplete popup'));
    }
    else {
      $(this.popup).css({ visibility: 'hidden' });
      this.hidePopup();
    }
  }
};

})(jQuery);
