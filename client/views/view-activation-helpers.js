
// Define our repeatable jquery activation and deactivation functions for our common view actions.

define([
    'jquery',
    'router'
], function($, router) {
    
    var activate = {
        endLoading : function(){
            $('#about').removeClass('inactive');
            $('#loading').addClass('inactive');
        },

        currentGames : function(){
            router.navigate("currentGames");
            $('#all-scores, #server-statistics').addClass('inactive');
            $('#current-games, #mini-scores, #chat, #site-news').removeClass('inactive');
        },

        statistics : function(){
            router.navigate("gameStatistics");
            $('#current-games, #mini-scores, #all-scores, #site-news').addClass('inactive');
            $('#server-statistics').removeClass('inactive');
        },

        highScores : function() {
            router.navigate("highScores");
            $('#current-games, #mini-scores, #server-statistics, #site-news').addClass('inactive');
            $('#all-scores, #chat').removeClass('inactive');
        },
        
        selectTilesConsole : function(tilesConsole) {
            this.tilesConsole = tilesConsole;
        },

        console : function(){
            $('#lobby').addClass("inactive");

            if(this.tilesConsole) {
                $("#canvas-console-holder").removeClass("inactive");
                $("#console-holder").addClass("inactive");
                $("#canvas-console-canvas").focus();
            }
            else {
                $("#console-holder").removeClass("inactive");
                $("#canvas-console-holder").addClass("inactive");
                $("#console").focus();
            }
        },
        
        lobby: function(){
            $('#lobby').removeClass("inactive");
            $("#console-holder").addClass("inactive");
            $("#canvas-console-holder").addClass("inactive");

        },
        
        loggedIn: function(){
            $('#auth').addClass("inactive");
        },
        
        resetAll: function(){
            $('#all-scores, #console-holder, #canvas-console-holder, #server-statistics').addClass("inactive");
            $('#lobby, #auth, #current-games, #mini-scores').removeClass("inactive");
        }
    };
    
    return activate;
    
});