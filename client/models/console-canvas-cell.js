// Model for a single console cell on the canvas. Keeps the char and colour information.

define([
    'jquery',
    'underscore',
    'backbone'
], function($, _, Backbone) {

    var ConsoleCanvasCellModel = Backbone.Model.extend({
        defaults: {
            char: 0,
            foregroundRed: 255,
            foregroundGreen: 255,
            foregroundBlue: 255,
            backgroundRed: 0,
            backgroundGreen: 0,
            backgroundBlue: 0,
        },

        clear : function(){
            this.set({
                char: 0,
                foregroundRed: 255,
                foregroundGreen: 255,
                foregroundBlue: 255,
                backgroundRed: 0,
                backgroundGreen: 0,
                backgroundBlue: 0
            });
        }
      
    });

    return ConsoleCanvasCellModel;

});
