var config = require('../config');
var _ = require('underscore');
var Controller = require('./controller-base');

var allUsers = require('../user/all-users');

// Controller for broadcasting lobby updates to all users who are currently in the lobby

function LobbyController(socket) {
    this.controllerName = "lobby";
    this.socket = socket;
    this.controllers = null;    
    this.broadcastInterval = null;
    
    // App starts in the lobby - start listening for updates
    this.userDataListen();
}

LobbyController.prototype = new Controller();
_.extend(LobbyController.prototype, {
    controllerName: "lobby",
    handlerCollection : {
        requestAllUserData : function(includeEveryone){
            this.sendAllUserData(includeEveryone);
        }
    },
    
    userDataListen : function(){
        var self = this;
        this.broadcastInterval = setInterval(function(){
            self.sendAllUserData();
        }, config.lobby.UPDATE_INTERVAL);
    },
    stopUserDataListen: function(){
        clearInterval(this.broadcastInterval);
    },
    sendAllUserData: function(){
        
        var self = this;
        var rawLobbyData = [];

        for (var gameName in allUsers.users) {

            // Send status of all tracked games to the lobby

            var userEntry = allUsers.users[gameName];
            var thisLobbyData = userEntry.lobbyData
            thisLobbyData["gameName"] = gameName;
            
            // update idle time
            var timeDiff = process.hrtime(userEntry.lastUpdateTime)[0];
            thisLobbyData["idle"] = timeDiff;

            rawLobbyData.push(thisLobbyData);
        }

        rawLobbyData.sort( function( a, b ) { return b.timeDiff - a.timeDiff; } )
        
        // In the event our periodic calling tries to send data to a closed socket
        this.sendMessage("lobby", rawLobbyData, function(err){
            if (!err){
                return;
            }
            
            self.stopUserDataListen();
            self.defaultSendCallback(err);
        });

    }
});

module.exports = LobbyController;
