// $Id: autocomplete.js,v 1.23 2008/01/04 11:53:21 goba Exp $

/**
 * Attaches the autocomplete behavior to all required fields
 */

if (typeof Drupal.settings.islandora_authority == 'undefined') {
  Drupal.settings.islandora_authority = new Object();
}
Drupal.behaviors.islandora_authority_autocomplete = function (context) {
  var acdb = [];
  $('input.islandora_authority_autocomplete:not(.islandora_authority_autocomplete-processed)', context).each(function () {
    var uri = this.value;
    if (!acdb[uri]) {
      acdb[uri] = new Drupal.settings.islandora_authority.ACDB(uri);
    }
    var input = $('#' + this.id.substr(0, this.id.length - 13))
      .attr('autocomplete', 'OFF')[0];
    $(input.form).submit(Drupal.settings.islandora_authority.autocompleteSubmit);
    new Drupal.settings.islandora_authority.jsAC(input, acdb[uri]);
    $(this).addClass('islandora_authority_autocomplete-processed');
  });
};

/**
 * Prevents the form from submitting if the suggestions popup is open
 * and closes the suggestions popup when doing so.
 */
Drupal.settings.islandora_authority.autocompleteSubmit = function () {
  return $('#islandora_authority_autocomplete').each(function () {
    this.owner.hidePopup();
  }).size() == 0;
};

/**
 * An AutoComplete object
 */
Drupal.settings.islandora_authority.jsAC = function (input, db) {
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
Drupal.settings.islandora_authority.jsAC.prototype.onkeydown = function (input, e) {
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
Drupal.settings.islandora_authority.jsAC.prototype.onkeyup = function (input, e) {
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
Drupal.settings.islandora_authority.jsAC.prototype.select = function (node) {
  //TODO:  Test this...
  var obj = node.autocompleteSet;
  var parents = this.input.id.split('--');
  for(var prop in obj) {
    if(obj.hasOwnProperty(prop) && typeof obj[prop] !== 'function' && 
      prop != 'full-display' && prop != 'alts') {
      //slice to get rid of the name of the current field
      var id_parts = parents.slice(0, parents.length - 1);
      
      //Add the part which the current property represents, while making the property
      //  name match what should have been put into the id, so we can select the
      //  relevant field below.
      id_parts.push(prop.replace(/(\]\[|_| )/g, '-'));
      
      //Update the contents of the required field.
      $('#'+id_parts.join('--')).val(obj[prop]);
    }
  }
};

/**
 * Highlights the next suggestion
 */
Drupal.settings.islandora_authority.jsAC.prototype.selectDown = function () {
  if (this.selected2){
    var next = $(this.selected2).nextAll('li:first')[0];
    if (next) {
      this.highlightSub(next);
    }
    else {
      var lis = $('li', this.popup2);
      if (lis.size() > 0) {
        this.highlightSub(lis.get(0));
      }
    }
  }
  else if (this.selected){
    var next = $(this.selected).nextAll('li:first')[0];
    if (next) {
      this.highlight(this.selected.nextSibling);
    }
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
Drupal.settings.islandora_authority.jsAC.prototype.selectUp = function () {
  var prev = false;
  if (this.selected2) { 
    prev = $(this.selected2).prevAll('li:first')[0];
    if (prev) {
      this.highlightSub(prev);
    }
  }
  else if (this.selected) {
    prev = $(this.selected).prevAll('li:first')[0];
    if (prev) {
      this.highlight(prev);
    }
  }
};

Drupal.settings.islandora_authority.jsAC.prototype.selectLeft = function () {
  if (this.selected2) {
    this.unhighlightSub(this.selected2);
    this.highlight(this.selected);
  }
};

Drupal.settings.islandora_authority.jsAC.prototype.selectRight = function () {
  if (this.selected2) {
    //Insert the given object...
    this.hidePopup(false);
  }
  else if (this.selected) {
    this.showSubmenu(this.selected);
    if (typeof this.popup2 != 'undefined') {
      this.highlightSub(this.popup2.firstChild.firstChild);
    }
  }
  else {
    //Dunno...
    //this.select(this.selected);
  }
};

/**
 * Highlights a suggestion
 */
Drupal.settings.islandora_authority.jsAC.prototype.highlight = function (node) {
  if (this.selected2) {
    this.unhighlightSub(this.selected2);
  }
  if (this.selected) {
    $(this.selected).removeClass('selected');
  }
  $(node).addClass('selected');
  this.selected = node;
  
  //Try to show a submenu (additional decisions made there)
  this.showSubmenu(this.selected);
};

Drupal.settings.islandora_authority.jsAC.prototype.highlightSub = function (node) {
  if (this.selected2) {
    $(this.selected2).removeClass('selected');
  }
  $(node).addClass('selected');
  this.selected2 = node;
};

/**
 * Unhighlights a suggestion
 */
Drupal.settings.islandora_authority.jsAC.prototype.unhighlight = function (node) {
  if (!this.selected2) {
    $(node).removeClass('selected');
    this.selected = false;
  }
};

Drupal.settings.islandora_authority.jsAC.prototype.unhighlightSub = function (node) {
  $(node).removeClass('selected');
  this.selected2 = false;
};

/**
 * Hides the autocomplete suggestions
 */
Drupal.settings.islandora_authority.jsAC.prototype.hidePopup = function (keycode) {
  // Select item if the right key or mousebutton was pressed...  seems kinda redundant?
  if ((keycode && keycode != 46 && keycode != 8 && keycode != 27) || !keycode) {
    if (this.selected2) {
      this.select(this.selected2);
    }
    else if (this.selected) {
      this.select(this.selected);
    }
    //this.input.value = this.selected.autocompleteValue;
  }
  
  // Hide popups
  var popup = this.popup2;
  if (popup) {
    delete this.popup2;
    $(popup).fadeOut('fast', function() {$(popup).remove();});
  }
  
  popup = this.popup;
  if (popup) {
    delete this.popup;
    $(popup).fadeOut('fast', function() {$(popup).remove();});
  }
  this.selected = false;
  this.selected2 = false;
};

/**
 * Positions the suggestions popup and starts a search
 */
Drupal.settings.islandora_authority.jsAC.prototype.populatePopup = function () {
  // Show popup
  if (this.popup) {
    $(this.popup).remove();
  }
  this.selected = false;
  this.popup = document.createElement('div');
  this.popup.id = 'islandora_authority_autocomplete';
  this.popup.owner = this;
  $(this.popup).css({
    marginTop: this.input.offsetHeight +'px',
    width: (this.input.offsetWidth - 4) +'px',
    overflow: 'visible'
  });
  $(this.input).before(this.popup);

  // Do search
  this.db.owner = this;
  this.db.search(this.input.value);
};

Drupal.settings.islandora_authority.jsAC.prototype.showSubmenu = function (node) {
  // Show popup
  if (this.popup2) {
    $(this.popup2).hide();
  }
  this.selected2 = false;
  if (typeof node.alt_popup != 'undefined') { 
    /*this.popup2 = document.createElement('div');
    this.popup2.id = 'islandora_authority_submenu';
    this.popup2.owner = this;
    
    */

    this.popup2 = node.alt_popup;
    $(this.popup2)
      .css({
        marginLeft: (node.offsetWidth - 4) / 2 +'px',
        width: (node.offsetWidth) +'px',
        top: (-node.offsetTop + 4) + 'px',
        align: 'right',
        float: 'right'
      })
      .show();
      
    //$(this.selected).before(this.popup2);
  }
};

/**
 * Fills the suggestion popup with any matches received
 */
Drupal.settings.islandora_authority.jsAC.prototype.found = function (matches) {
  // If no value in the textfield, do not show the popup.
  if (!this.input.value.length) {
    return false;
  }

  // Prepare matches
  var ul = document.createElement('ul');
  var ac = this;
  for (var key in matches) {
    var obj = matches[key];
    var li = document.createElement('li');
    $(li)
      .html('<div>'+ obj['full-display'] +'</div>')
      .css({
        float: 'left'
      })
      .click(function () {
        ac.select(this);
      })
      .mouseenter(function () {
        ac.highlight(this);
      })
      .mouseleave(function () {
        ac.unhighlight(this);
      }); //Gonna require some shenanigans to make it stay selected when using the mouse...
       
    var alt_ul = document.createElement('ul');
    var alts = obj['alts']
    for (var prop in alts) {
      if(typeof obj[prop] !== 'function') {
        var alt_li = document.createElement('li');
        //FIXME:  Something is broken with the mouse handling...  Never selects in the submenu?
        $(alt_li)
          .html('<div>'+ alts[prop]['full-display'] +'</div>')
          .mouseenter(function () {
            ac.highlightSub(this);
          })
          .mouseleave(function () {
            ac.unhighlightSub(this);
          })
          .click(function () {
            ac.select(this);
          });
        alt_li.autocompleteSet = alts[prop];
        $(alt_ul).append(alt_li).hide();
      }
    }
    
    if (alt_ul.childNodes.length > 0) {
      li.alt_popup = alt_ul;
      $(li).append(alt_ul);
    }
  
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

Drupal.settings.islandora_authority.jsAC.prototype.setStatus = function (status) {
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
Drupal.settings.islandora_authority.ACDB = function (uri) {
  this.uri = uri;
  this.delay = 300;
  this.cache = {};
};

/**
 * Performs a cached and delayed search
 */
Drupal.settings.islandora_authority.ACDB.prototype.search = function (searchString) {
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
Drupal.settings.islandora_authority.ACDB.prototype.cancel = function() {
  if (this.owner) this.owner.setStatus('cancel');
  if (this.timer) clearTimeout(this.timer);
  this.searchString = '';
};

