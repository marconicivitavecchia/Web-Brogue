define([
    "jquery",
    "underscore",
    "backbone",
    "backboneAutocomplete"
], function ($, _, Backbone, AutocompleteView) {

    var UsersPageSelectView = AutocompleteView.extend({
        onSelect: function(model){
            console.log("onselect: " + model.get("username")); 
        },
        searchMethod: function(model) { // method passed to filter(..) on the collection
            var label = model.get('username').toLowerCase();
            // the method is bound to the view, with current value of the user input available as `this.searchValue`
            var searchValue = this.searchValue.toLowerCase().trim(); 
    
            console.log("searchValue: " + searchValue);

            if (label.indexOf(searchValue) !== -1) {
                return true;
            } else {
                return false;
            }
        },
        initialise: function() {
            this.collection.fetch();
            console.log("fetching collection")
        }
    })

    return UsersPageSelectView;

});

