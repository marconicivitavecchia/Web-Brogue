// Javascript handles keyboard input in an inconvienient way for the purposes of our console.  This view is to help process data using an input field in HTML.

define([
    "jquery",
    "underscore",
    "backbone"
], function($, _, Backbone){
    
    var ConsoleKeyProcessor = Backbone.View.extend({
        el : '#console-keyboard',
        events : {
            'input' : 'inputHandler'
        },
        
        // input event fires after keydown is fired
        inputHandler : function(event){
            
            this.el.value = "";    
        }
        
    });
    
    return ConsoleKeyProcessor;
});