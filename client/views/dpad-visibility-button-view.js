// Button which switches on and off dpad

define([
    "jquery",
    "underscore",
    "backbone",
], function($, _, Backbone) {

    var DpadVisibilityButtonView = Backbone.View.extend({
        
        el: "#console-dpad",

        events : {
            "click" : "handleClick"
        },
        
        handleClick : function(event){
        
            event.preventDefault();

            $('#console-dpad-holder').toggleClass("inactive");
        }
    });

    return DpadVisibilityButtonView;
});