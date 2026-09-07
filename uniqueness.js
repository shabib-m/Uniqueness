/**
 * @file
 * uniqueness.js - JavaScript for the uniqueness width
 */

(function($) {
  var uniqueness;
  function uniquenessEscapeHtml(value) {
    return $('<div>').text(value == null ? '' : value).html();
  }

  Backdrop.behaviors.uniqueness = {
    attach: function (context) {
      var settings = Backdrop.settings.uniqueness;
      var $widget = $('.uniqueness-dyn', context);
      if (!uniqueness && settings && $widget.length) {
        uniqueness = new Backdrop.uniqueness(settings.URL, $widget);
      }
      if (!uniqueness) {
        return;
      }
      // Search off title.
      $('#edit-title', context).once('uniqueness', function() {
        $(this).keyup(function() {
          var input = $.trim(this.value);
          if (input.length >= uniqueness.minCharacters) {
            uniqueness.search('title', input);
          }
          else if(input.length == 0 && !uniqueness.prependResults) {
            uniqueness.clear();
          }
        });
      });
      // Search off tags.
      $('#edit-taxonomy-tags-1', context).once('uniqueness', function() {
        $(this).blur(function() {
          var input = $.trim(this.value);
          // Some tags set.
          if (input.length > 0) {
            uniqueness.search('tags', input);
          }
        });
      });
    }
  };

  Backdrop.uniqueness = function (uri, widget) {
    this.uri = uri;
    this.delay = 500;
    this.widget = widget;
    this.list = $('.item-list ul', widget);
    this.notifier = $('.uniqueness-search-notifier', widget);
    this.widgetCSS = {
      // Use the notifier text as the loading indicator.
      'background-image' : 'none'
    };
    this.searchCache = {};
    this.listCache = {};
    this.prependResults = Backdrop.settings.uniqueness['prependResults'];
    this.nid = Backdrop.settings.uniqueness['nid'];
    this.type = Backdrop.settings.uniqueness['type'];
    this.minCharacters = Backdrop.settings.uniqueness['minCharacters'];
    this.autoOpen = $(widget).closest('fieldset');
  }

  Backdrop.uniqueness.prototype.update = function (data) {
    var expand = false;
    uniqueness.notifier.removeClass('uniqueness-dyn-searching').empty();
    uniqueness.widget.css('background-image', '');
    uniqueness = this;
    if (uniqueness.prependResults) {
      if (data == undefined && uniqueness.listCache != {}) {
        data = uniqueness.listCache;
      }
      var items = '';
      $.each(data, function(i, item) {
        // Only use what we haven't seen before.
        if (uniqueness.listCache[item.nid] == undefined) {
          items += '<li><a href="' + uniquenessEscapeHtml(item.href) + '" target="_blank" rel="noopener noreferrer">' + uniquenessEscapeHtml(item.title) + '</a> ' + (item.status == 0 ? '(' + Backdrop.t('not published') + ')' : '') + '</li>';
          // Store the new item.
          uniqueness.listCache[item.nid] = item;
          expand = true;
        }
      });
      // Show list.
      this.list.prepend(items);
    }
    else { // Replace content. //@todo still use caching?
      $(".uniqueness-description", uniqueness.widget.parent()).toggle(data != undefined);
      if (data == undefined) {
        uniqueness.clear();
        if ($('#edit-title')[0].value.length) {
          uniqueness.notifier.html(Backdrop.settings.uniqueness['noResultsString']);
        }
        return;
      }
      var items = '';
      $.each(data, function(i, item) {
        if (item.more) {
          items += '<li>' + Backdrop.t("... and others.") + '</li>';
        }
        else {
          items += '<li><a href="' + uniquenessEscapeHtml(item.href) + '" target="_blank" rel="noopener noreferrer">' + uniquenessEscapeHtml(item.title) + '</a> ' + (item.status == 0 ? '(' + Backdrop.t('not published') + ')' : '') + '</li>';
        }
      });
      this.list.html(items);
      expand = items.length;
    }
    if (expand && uniqueness.autoOpen) {
      uniqueness.autoOpen.removeClass('collapsed');
      // Only auto open the fieldset once per page load.
      uniqueness.autoOpen = null;
    }
  }

  Backdrop.uniqueness.prototype.search = function (element, searchString) {
    uniqueness = this;

    // If this string has been searched for before we do nothing.
    if (uniqueness.prependResults && uniqueness.searchCache[searchString]) {
      return;
    }

    if (this.timer) {
      clearTimeout(this.timer);
    }
    if (this.request) {
      this.request.abort();
      this.request = null;
    }
    searchString = $.trim(searchString);
    this.timer = setTimeout(function () {
      // Inform user we're searching.
      if (uniqueness.notifier.hasClass('uniqueness-dyn-searching') == false) {
        uniqueness.notifier.addClass('uniqueness-dyn-searching').html(Backdrop.settings.uniqueness['searchingString']);
        uniqueness.widget.css(uniqueness.widgetCSS);
      }
      var params = {};
      if (uniqueness.nid != undefined) {
        params.nid = uniqueness.nid;
      }
      if (uniqueness.type != undefined) {
        params.type = uniqueness.type;
      }
      params[element] = searchString;
      uniqueness.request = $.getJSON(uniqueness.uri, params, function (data) {
        if (data != undefined && data != 'false') {
          // Found results.
          uniqueness.update(data);
          // Save this string, it found results.
          uniqueness.searchCache[searchString] = searchString;
          var blockSet = true;
        }
        // Nothing new found so show existing results.
        if (blockSet == undefined) {
          uniqueness.update();
        }
      }).fail(function () {
        uniqueness.notifier.removeClass('uniqueness-dyn-searching').text(Backdrop.t('Unable to search for related content.'));
        uniqueness.widget.css('background-image', '');
      }).always(function () {
        uniqueness.request = null;
      });
    }, uniqueness.delay);
  }

  Backdrop.uniqueness.prototype.clear = function () {
    this.list.empty();
  }
})(jQuery);
