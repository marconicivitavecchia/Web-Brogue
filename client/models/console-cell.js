// Model for a single console cell.  Keeps CSS data about where the cell is placed, the foreground, background, and character that is shown.

define([
    'jquery',
    'underscore',
    'backbone'
], function($, _, Backbone) {

    var ConsoleCellModel = Backbone.Model.extend({
        defaults: {
            char: "",
            foregroundRed: 255,
            foregroundGreen: 255,
            foregroundBlue: 255,
            backgroundRed: 0,
            backgroundGreen: 0,
            backgroundBlue: 0,
            x: 0,
            y: 0,
            consoleWidth: 800,
            consoleHeight: 600,
            consoleColumns: 100,
            consoleRows: 34
        },

        clear : function(){
            this.set({
                char: "",
                foregroundRed: 255,
                foregroundGreen: 255,
                foregroundBlue: 255,
                backgroundRed: 0,
                backgroundGreen: 0,
                backgroundBlue: 0
            });
        }

    });

    return ConsoleCellModel;

});
