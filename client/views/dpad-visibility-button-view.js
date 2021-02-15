// Button which switches on and off dpad

define([
    "jquery",
    "underscore",
    "backbone",
], function($, _, Backbone) {

    var DpadVisibilityButtonView = Backbone.View.extend({
        
        events : {
            "click" : "handleClick"
        },
        
        handleClick : function(event){
        
            event.preventDefault();

            var dPadHolder = '#' + this.prefix + 'dpad-holder';
            $(dPadHolder).toggleClass("inactive");

            this.positionDPad();
            
        },

        setDPadPrefix : function (prefix) {
            this.prefix = prefix;
        },

        positionDPad : function () {
            // obsolete - we'll use CSS instead
        }
    });

    return DpadVisibilityButtonView;
});