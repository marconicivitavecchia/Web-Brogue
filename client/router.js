define([
    'jquery',
    'underscore',
    'backbone',
    'dispatcher'
], function($, _, Backbone, dispatcher) {

    var BrogueRouter = Backbone.Router.extend({

        routes: {
            "highScores":        "highScores",
            "currentGames":      "currentGames",
            "gameStatistics":    "gameStatistics",
            "viewRecording/:variant-:id(/:turn)": "viewRecording"
        },

        started: false,

        highScores: function() {
            dispatcher.trigger("all-scores");
        },
        currentGames: function() {
            dispatcher.trigger("currentGames");
        },
        gameStatistics: function() {
            dispatcher.trigger("gameStatistics");
        },
        viewRecording: function(variant, id, turn) {

            var recordingId = 'recording-' + id;
            if (turn) {
                recordingId = recordingId.concat("-" + turn);
            }
            dispatcher.trigger("recordingGame", {variant: variant, id: recordingId});
            dispatcher.trigger("showConsole");
        },
        //Only activate the router on login, to avoid races when viewing recordings etc.
        login: function() {

            if(!this.started) { 
                Backbone.history.start();
                this.started = true;
            }
        }
    });
    
    return new BrogueRouter();
  });