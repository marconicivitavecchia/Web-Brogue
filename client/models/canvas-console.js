// Model for a single console cell.  Keeps CSS data about where the cell is placed, the foreground, background, and character that is shown.

define([
    'jquery',
    'underscore',
    'backbone'
], function($, _, Backbone) {

    var CanvasConsoleModel = Backbone.Model.extend({
        defaults: {
            consoleCells: []
        },

        initialize: function() {
           
        }    
    });

    return CanvasConsoleModel;

});
