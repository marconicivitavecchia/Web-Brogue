define([
    'jquery',
    'underscore',
    'backbone'
], function($, _, Backbone) {

    var DPadButtonModel = Backbone.Model.extend({
        defaults: {
            keyToSend: 0
        }
    });

    return DPadButtonModel;

});