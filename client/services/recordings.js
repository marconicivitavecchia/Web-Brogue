define([
    'jquery',
    'underscore',
    'backbone',
    "dataIO/send-generic"
], function($, _, Backbone, send) {

    var Recordings = {
        
        startRecording: function(data) {

            send("brogue", "recording", {recording: data.id});
        }
            
    };
    return Recordings;
});