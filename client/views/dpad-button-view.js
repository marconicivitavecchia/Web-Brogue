// View for a d-pad button

define([
    "jquery",
    "underscore",
    "backbone",
    'dataIO/send-keypress',
    "models/dpad-button",
], function($, _, Backbone, sendKeypressEvent, DPadButtonModel) {

    // See BrogueCode/rogue.h for all brogue event definitions
    var KEYPRESS_EVENT_CHAR = 0;

    //Pass in el and model
    var DpadButtonView = Backbone.View.extend({
        
        events : {
            "click" : "handleClick"
        },
        
        handleClick : function(event){
        
            event.preventDefault();
            sendKeypressEvent(KEYPRESS_EVENT_CHAR, this.model.get("keyToSend"), false, false);
        }
    });

    return DpadButtonView;
});
