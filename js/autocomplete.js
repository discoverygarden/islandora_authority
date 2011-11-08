// $Id: autocomplete.js,v 1.23 2008/01/04 11:53:21 goba Exp $

/**
 * Attaches the autocomplete behavior to all required fields
 */

/*if (typeof Drupal.behaviors.islandora_authority == 'undefined') {
  Drupal.behaviors.islandora_authority = new Object();
}*/
Drupal.behaviors.islandora_authority_autocomplete = function (context) {
  var acdb = [];
  $('input.islandora_authority_autocomplete:not(.islandora_authority_autocomplete-processed)', context).each(function () {
    var uri = this.value;
    if (!acdb[uri]) {
      acdb[uri] = new Drupal.behaviors.islandora_authority_ACDB(uri);
    }
    var input = $('#' + this.id.substr(0, this.id.length - 13))
      .attr('autocomplete', 'OFF')[0];
    $(input.form).submit(Drupal.behaviors.islandora_authority_autocompleteSubmit);
    new Drupal.behaviors.islandora_authority_jsAC(input, acdb[uri]);
    $(this).addClass('islandora_authority_autocomplete-processed');
  });
};

/**
 * Prevents the form from submitting if the suggestions popup is open
 * and closes the suggestions popup when doing so.
 */
Drupal.behaviors.islandora_authority_autocompleteSubmit = function () {
  return $('#autocomplete').each(function () {
    this.owner.hidePopup();
  }).size() == 0;
};

/**
 * An AutoComplete object
 */
Drupal.behaviors.islandora_authority_jsAC = function (input, db) {
  var ac = this;
  this.input = input;
  this.db = db;

  $(this.input)
    .keydown(function (event) {return ac.onkeydown(this, event);})
    .keyup(function (event) {ac.onkeyup(this, event);})
    .blur(function () {ac.hidePopup();ac.db.cancel();});
};

/**
 * Handler for the "keydown" event
 */
Drupal.behaviors.islandora_authority_jsAC.prototype.onkeydown = function (input, e) {
  if (!e) {
    e = window.event;
  }
  switch (e.keyCode) {
    case 40: // down arrow
      this.selectDown();
      return false;
    case 38: // up arrow
      this.selectUp();
      return false;
    case 37:
      this.selectLeft();
      return false;
    case 39:
      this.selectRight();
      return false;
    default: // all other keys
      return true;
  }
};

/**
 * Handler for the "keyup" event
 */
Drupal.behaviors.islandora_authority_jsAC.prototype.onkeyup = function (input, e) {
  if (!e) {
    e = window.event;
  }
  switch (e.keyCode) {
    case 16: // shift
    case 17: // ctrl
    case 18: // alt
    case 20: // caps lock
    case 33: // page up
    case 34: // page down
    case 35: // end
    case 36: // home
    case 37: // left arrow
    case 38: // up arrow
    case 39: // right arrow
    case 40: // down arrow
      return true;

    case 9:  // tab
    case 13: // enter
    case 27: // esc
      this.hidePopup(e.keyCode);
      return true;

    default: // all other keys
      if (input.value.length > 0)
        this.populatePopup();
      else
        this.hidePopup(e.keyCode);
      return true;
  }
};

/**
 * Puts the currently highlighted suggestion into the autocomplete field
 */
Drupal.behaviors.islandora_authority_jsAC.prototype.select = function (node) {
  //TODO:  Test this...
  var obj = node.autocompleteSet;
  var parents = this.input.id.split('--');
  for(var prop in obj) {
    if(obj.hasOwnProperty(prop) && typeof obj[prop] !== 'function' && 
      prop != 'full-display' && prop != 'alts') {
      //slice to get rid of the name of the current field
      var id_parts = parents.slice(0, parents.length - 1);
      
      //Add the which the current property represents
      id_parts.push(prop.replace(/_/g, '-'));
      
      //Update the contents of the required property.
      $('#'+id_parts.join('--')).val(obj[prop]);
    }
  }
};

/**
 * Highlights the next suggestion
 */
Drupal.behaviors.islandora_authority_jsAC.prototype.selectDown = function () {
  if (this.selected && this.selected.nextSibling) {
    this.highlight(this.selected.nextSibling);
  }
  else {
    var lis = $('li', this.popup);
    if (lis.size() > 0) {
      this.highlight(lis.get(0));
    }
  }
};

/**
 * Highlights the previous suggestion
 */
Drupal.behaviors.islandora_authority_jsAC.prototype.selectUp = function () {
  if (this.selected && this.selected.previousSibling) {
    this.highlight(this.selected.previousSibling);
  }
};

Drupal.behaviors.islandora_authority_jsAC.prototype.selectLeft = function () {
  if (this.selected && this.selected2) {
    this.highlight(this.selected);
  }
};

Drupal.behaviors.islandora_authority_jsAC.prototype.selectRight = function () {
  if (this.selected && this.selected2) {
    //Insert the given object...
  }
  else if (this.selected) {
    this.showSubmenu(this.selected);
  }
};

/**
 * Highlights a suggestion
 */
Drupal.behaviors.islandora_authority_jsAC.prototype.highlight = function (node) {
  if (this.selected) {
    $(this.selected).removeClass('selected');
  }
  $(node).addClass('selected');
  this.selected = node;
  
  //Try to show a submenu (additional decisions made there)
  this.showSubmenu(this.selected);
};

/**
 * Unhighlights a suggestion
 */
Drupal.behaviors.islandora_authority_jsAC.prototype.unhighlight = function (node) {
  $(node).removeClass('selected');
  this.selected = false;
};

/**
 * Hides the autocomplete suggestions
 */
Drupal.behaviors.islandora_authority_jsAC.prototype.hidePopup = function (keycode) {
  // Select item if the right key or mousebutton was pressed
  if (this.selected && ((keycode && keycode != 46 && keycode != 8 && keycode != 27) || !keycode)) {
    this.select(this.selected);
    //this.input.value = this.selected.autocompleteValue;
  }
  
  // Hide popups
  var popup = this.popup2;
  if (popup) {
    this.popup2 = null;
    $(popup).fadeOut('fast', function() {$(popup).remove();});
  }
  
  popup = this.popup;
  if (popup) {
    this.popup = null;
    $(popup).fadeOut('fast', function() {$(popup).remove();});
  }
  this.selected = false;
};

/**
 * Positions the suggestions popup and starts a search
 */
Drupal.behaviors.islandora_authority_jsAC.prototype.populatePopup = function () {
  // Show popup
  if (this.popup) {
    $(this.popup).remove();
  }
  this.selected = false;
  this.popup = document.createElement('div');
  this.popup.id = 'autocomplete';
  this.popup.owner = this;
  $(this.popup).css({
    marginTop: this.input.offsetHeight +'px',
    width: (this.input.offsetWidth - 4) +'px',
    display: 'none'
  });
  $(this.input).before(this.popup);

  // Do search
  this.db.owner = this;
  this.db.search(this.input.value);
};

Drupal.behaviors.islandora_authority_jsAC.prototype.showSubmenu = function (node) {
  // Show popup
  if (this.popup2) {
    $(this.popup2).remove();
  }
  
  if (typeof node.alt_menu != 'undefined') { 
    this.popup2 = document.createElement('div');
    this.popup2.id = 'autocomplete2';
    this.popup2.owner = this;
    
    $(this.popup2)
      .css({
        marginLeft: (node.offsetWidth - 4) +'px',
        width: (node.offsetWidth - 4) +'px',
        display: 'none'
      })
      .append(node.alt_menu)
      .show(); //Seems kinda silly with the display: none...  anyway.
      
    $(this.popup).before(this.popup2);
  }
};

/**
 * Fills the suggestion popup with any matches received
 */
Drupal.behaviors.islandora_authority_jsAC.prototype.found = function (matches) {
  // If no value in the textfield, do not show the popup.
  if (!this.input.value.length) {
    return false;
  }

  // Prepare matches
  var ul = document.createElement('ul');
  var ac = this;
  for (key in matches) {
    //TODO:  Make it save/build the entire list...
    var obj = matches[key];
    var li = document.createElement('li');
    $(li)
      .html('<div>'+ obj['full-display'] +'</div>')
      .mousedown(function () {ac.select(this);})
      .mouseover(function () {ac.highlight(this);})
      .mouseout(function () {ac.unhighlight(this);});
    
    if (obj['alts'].length > 0) {
      var alt_ul = document.createElement('ul');
      var alts = obj['alts']
      for (prop in alts) {
        if(obj.hasOwnProperty(prop) && typeof obj[prop] !== 'function') {
          var alt_li = document.createElement('li');
          $(alt_li)
            .html('<div>'+ alts[prop]['full-display'] +'</div>')
            .mousedown(function () {ac.select(this);})
            .mouseover(function () {ac.highlight(this);})
            .mouseout(function () {ac.unhighlight(this);});
          alt_li.autocompleteSet = alts[prop];
          $(alt_ul).append(alt_li);
        }
      }
      li.alt_popup = alt_ul;
    }
    
    //TODO:  Make it save the entire list somewhere.
    li.autocompleteSet = obj;
    $(ul).append(li);
  }

  // Show popup with matches, if any
  if (this.popup) {
    if (ul.childNodes.length > 0) {
      $(this.popup).empty().append(ul).show();
    }
    else {
      $(this.popup).css({visibility: 'hidden'});
      this.hidePopup();
    }
  }
};

Drupal.behaviors.islandora_authority_jsAC.prototype.setStatus = function (status) {
  switch (status) {
    case 'begin':
      $(this.input).addClass('throbbing');
      break;
    case 'cancel':
    case 'error':
    case 'found':
      $(this.input).removeClass('throbbing');
      break;
  }
};

/**
 * An AutoComplete DataBase object
 */
Drupal.behaviors.islandora_authority_ACDB = function (uri) {
  this.uri = uri;
  this.delay = 300;
  this.cache = {};
};

/**
 * Performs a cached and delayed search
 */
Drupal.behaviors.islandora_authority_ACDB.prototype.search = function (searchString) {
  var db = this;
  this.searchString = searchString;

  // See if this key has been searched for before
  if (this.cache[searchString]) {
    return this.owner.found(this.cache[searchString]);
  }

  // Initiate delayed search
  if (this.timer) {
    clearTimeout(this.timer);
  }
  this.timer = setTimeout(function() {
    db.owner.setStatus('begin');

    // Ajax GET request for autocompletion
    $.ajax({
      type: "GET",
      url: db.uri +'/'+ Drupal.encodeURIComponent(searchString),
      dataType: 'json',
      success: function (matches) {
        if (typeof matches['status'] == 'undefined' || matches['status'] != 0) {
          db.cache[searchString] = matches;
          // Verify if these are still the matches the user wants to see
          if (db.searchString == searchString) {
            db.owner.found(matches);
          }
          db.owner.setStatus('found');
        }
      },
      error: function (xmlhttp) {
        alert(Drupal.ahahError(xmlhttp, db.uri));
      }
    });
  }, this.delay);
};

/**
 * Cancels the current autocomplete request
 */
Drupal.behaviors.islandora_authority_ACDB.prototype.cancel = function() {
  if (this.owner) this.owner.setStatus('cancel');
  if (this.timer) clearTimeout(this.timer);
  this.searchString = '';
};

