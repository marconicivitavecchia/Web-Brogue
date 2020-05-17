// View for the entire console.  It is responsible for setting up all of the console-cell views

define([
    "jquery",
    "underscore",
    "backbone",
    "rot",
    "dispatcher",
    "variantLookup",
    'dataIO/send-keypress',
    'dataIO/send-mouse',
    "models/console-canvas-cell",
    "views/view-activation-helpers",
    "views/remap-brogue-glyphs"
], function($, _, Backbone, ROT, dispatcher, variantLookup, sendKeypressEvent, sendMouseEvent, ConsoleCanvasCellModel, activate, remapBrogueGlyphs) {

    var _MESSAGE_UPDATE_SIZE = 10;
    var MOUSE_UP_EVENT_CHAR = 1;
    var MOUSE_DOWN_EVENT_CHAR = 2;
    var MOUSE_HOVER_EVENT_CHAR = 5;
    var MOUSEOVER_SIDEBAR_RATE_LIMIT_MS = 200;

    var KEYPRESS_EVENT_CHAR = 0;
    var REFRESH_EVENT_CHAR = 50;

    var Console = Backbone.View.extend({
        el: "#canvas-console-canvas",
        events: {
            'keydown' : 'keydownHandler',
            'keyup' : 'keyupHandler',
            "click" : "handleClick",
            "mousemove" : "handleMousemove"
        },

        keydownHandler: function(event) {

            event.preventDefault();

            //Acknowledge direction keys on keydown which includes key repeat
            var eventKey = event.key;
            var ctrlKey = event.ctrlKey;
            var shiftKey = event.shiftKey;

            var returnCode;

            //Ignore keydown of modifiers
            if(eventKey === "Shift" || eventKey === "Control" || eventKey === "Alt") {
                return;
            }

            //Special keys

            switch (eventKey) {
                case "Clear": //centre (5)
                    returnCode = 53;
                    break;
                case "PageUp": //page up (9)
                    returnCode = 117; // map to u
                    break;
                case "PageDown": //page_down (3)
                    returnCode = 110; // map to n
                    break;
                case "End": //end (1)
                    returnCode = 98; // map to b
                    break;
                case "Home": //home (7)
                    returnCode = 121; // map to y
                    break;
                case "ArrowLeft": //left-arrow (4)
                    returnCode = 63234;
                    break;
                case "ArrowUp": //up-arrow(8)
                    returnCode = 63232;
                    break;
                case "ArrowRight": //right-arrow(6)
                    returnCode = 63235;
                    break;
                case "ArrowDown": //down-arrow(2)
                    returnCode = 63233;
                    break;
            }

            if (returnCode) {
                sendKeypressEvent(KEYPRESS_EVENT_CHAR, returnCode, ctrlKey, shiftKey);
            }
        },

        keyupHandler : function(event){            
            
            event.preventDefault();

            //Acknowledge non-direction keys on keyup (no repeat)
            var eventKey = event.key;
            var ctrlKey = event.ctrlKey;
            var shiftKey = event.shiftKey;
            
            var returnCode;

            //Ignore keyup of modifiers
            if(eventKey === "Shift" || eventKey === "Control" || eventKey === "Alt") {
                return;
            }

            //Client only keys
            if(eventKey == "h") {
                this.toggleTiles();
                return;
            }

            //Special keys - return early if handled by keydown

            switch (eventKey) {
                case "Enter": //enter
                    returnCode = 13;
                    break;
                case "Escape": //esc
                    returnCode = 27;
                    break;
                case "Backspace": // backspace
                    returnCode = 127; // map to DELETE_KEY
                    break;
                case "Tab": // tab
                    returnCode = 9;
                    break;
                case "Delete": // delete
                    returnCode = 127;
                    break;
                case "Clear": //centre (5)
                    return;
                case "PageUp": //page up (9)
                    return;
                case "PageDown": //page_down (3)
                    return;
                case "End": //end (1)
                    return;
                case "Home": //home (7)
                    return;
                case "ArrowLeft": //left-arrow (4)
                    return;
                case "ArrowUp": //up-arrow(8)
                    return;
                case "ArrowRight": //right-arrow(6)
                    return;
                case "ArrowDown": //down-arrow(2)
                    return;
            }

            //Alphanumerics
            if(!returnCode) {
               returnCode = eventKey.charCodeAt(0);
            }

            if (returnCode) {
                sendKeypressEvent(KEYPRESS_EVENT_CHAR, returnCode, ctrlKey, shiftKey);
            }
        },

        handleClick : function(event){
            
            event.preventDefault();

            var clickCoords = this.d.eventToPosition(event);
           
            sendMouseEvent(
                MOUSE_DOWN_EVENT_CHAR, 
                clickCoords[0], 
                clickCoords[1], 
                event.ctrlKey, 
                event.shiftKey
            );
            sendMouseEvent(
                MOUSE_UP_EVENT_CHAR, 
                clickCoords[0], 
                clickCoords[1], 
                event.ctrlKey, 
                event.shiftKey
            );
        },
        
        handleMousemove : function(event){

            event.preventDefault();

            var sendMouseOverEvent = function(x, y, ctrlKey, shiftKey) {

                sendMouseEvent(
                    MOUSE_HOVER_EVENT_CHAR,
                    x,
                    y,
                    ctrlKey,
                    shiftKey
                );
            };

            var d = new Date();
            var timeNow = d.getTime();

            var mouseDestCoords = this.d.eventToPosition(event);

            //Rate limit mouseMoves in the sidebar since the game will always re-render fully.
            //Allow all other mouseMoves at full rate, providing they enter a new cell
            if(mouseDestCoords[0] >= 20 ||
                timeNow > this.lastMouseOver + MOUSEOVER_SIDEBAR_RATE_LIMIT_MS) {

                if(!_.isEqual(mouseDestCoords, this.lastMouseDestCoords)) {
                    clearTimeout(this.mouseOverDelayedSend);
                    sendMouseOverEvent(mouseDestCoords[0], mouseDestCoords[1], event.ctrlKey, event.shiftKey);
                    this.lastMouseOver = d.getTime();
                    this.lastMouseDestCoords = mouseDestCoords;
                }
            }
            else {
                //Otherwise set a timeOut for this event to fire at the rate limit
                clearTimeout(this.mouseOverDelayedSend);
                var delay = this.lastMouseOver + MOUSEOVER_SIDEBAR_RATE_LIMIT_MS - timeNow;
                this.mouseOverDelayedSend = setTimeout(sendMouseOverEvent, delay, mouseDestCoords[0], mouseDestCoords[1], event.ctrlKey, event.shiftKey);
            }
        },

        toggleTiles: function() {
            if(!this.useTiles) {
                this.useTiles = true;
                this.setupCanvas(true);
            }
            else {
                this.useTiles = false;
                this.setupCanvas(true);
            }
        },

        setupTileCanvas: function(width, height) {
            if(this.canvas) {
                this.canvas.remove();
            }

            this.d = new ROT.Display({ 
                layout: "tile",
                bg: "transparent",
                tileWidth: this.tileWidth,
                tileHeight: this.tileHeight,
                tileMap: this.tileMap,
                tileSet: this.tileSet,
                tileColorize: true,
                width: width,
                height: height
            });
            var canvas = this.d.getContainer();
            this.canvas = canvas;
            this.$el.append(canvas);
        },

        setupCharCanvas: function(width, height) {
            if(this.canvas) {
                this.canvas.remove();
            }

            this.d = new ROT.Display({ width: width, height: height, fontFamily: "Source Code Pro", spacing: 1.3, fontSize: 25});
            var canvas = this.d.getContainer();
            this.canvas = canvas;
            this.$el.append(canvas);
        },

        initialize: function() {
            
            this.tileSet = document.createElement("img");
            this.tileSet.src = "tiles.png";

            this.setupTileMapping();

            this.useTiles = true;
        
        },

        initialiseForNewGame: function(data) {
            
            var variantCode = _.findWhere(_.values(variantLookup.variants), {default: true}).code;
            if('variant' in data) {
                variantCode = data.variant;
            }

            this.variantCode = data.variant;
            this.variant = variantLookup.variants[variantCode];

            this.consoleColumns = this.variant.consoleColumns;
            this.consoleRows = this.variant.consoleRows;
            this.remapGlyphs = this.variant.remapGlyphs;

            this.setupCanvas(false);
        },

        setupCanvas: function(sendRefresh) {
            
            //Setup display
            //Currently slightly expensive since nukes the canvas each time
            
            if(this.useTiles) {  
                this.setupTileCanvas(this.consoleColumns, this.consoleRows);
            }
            else {
                this.setupCharCanvas(this.consoleColumns, this.consoleRows);
            }

            //Send refresh, only on switch since it is sent the first time by the server (brogueInterface)
            if(sendRefresh) {
                sendKeypressEvent(REFRESH_EVENT_CHAR, 0, 0, 0);
            }

            this.resize();
        },

        renderCell: function(cellData) {

            var cellCharacter = cellData["char"];
            var rgbForegroundString = "rgb(" +
                cellData["foregroundRed"] + "," +
                cellData["foregroundGreen"] + "," +
                cellData["foregroundBlue"] + ")";
            var rgbBackgroundString = "rgb(" +
                cellData["backgroundRed"] + "," +
                cellData["backgroundGreen"] + "," +
                cellData["backgroundBlue"] + ")";

            this.d.draw(cellData["x"], cellData["y"], cellCharacter, rgbForegroundString, rgbBackgroundString);
        },

        resize: function() {
            
            if(!this.useTiles) {
                var wrapperWidth = document.getElementById("canvas-console-wrapper").offsetWidth;
                var wrapperHeight = document.getElementById("canvas-console-wrapper").offsetHeight;
                var maxFontSize = this.d.computeFontSize(wrapperWidth, wrapperHeight);

                console.log("wrapperWidth: " + wrapperWidth + " wrapperHeight: " + wrapperHeight + " maxFontSize: " + maxFontSize);

                this.d.setOptions({
                    fontSize: maxFontSize, 
                });  
            }
            else {
                var wrapperWidth = document.getElementById("canvas-console-wrapper").offsetWidth;
                var wrapperHeight = document.getElementById("canvas-console-wrapper").offsetHeight;

                //Abused to set the convas style width
                this.d.computeFontSize(wrapperWidth, wrapperHeight);
            }
        },
        
        queueUpdateCellModelData : function(data){
            
            if ($("#canvas-console-holder").hasClass("inactive")) {
                return;
            }
            
            var self = this;
            setTimeout(function() {
                self.updateCellModelData(data);
            }, 0);
        },

        updateCellModelData: function (data) {
            var dataArray = new Uint8Array(data);
            var dataLength = dataArray.length;
            var dIndex = 0;

            while (dIndex < dataLength) {
                var dataXCoord = dataArray[dIndex++];
                var dataYCoord = dataArray[dIndex++];

                var combinedUTF16Char = dataArray[dIndex++] << 8 | dataArray[dIndex++];

                //console.log("[(" + dataXCoord + "," + dataYCoord + ") " + combinedUTF16Char + "]");

                // Status updates have coords (255,255). For now ignore these, eventually we may find a UI use for them
                if (dataXCoord === 255 && dataYCoord === 255) {
                    dIndex += _MESSAGE_UPDATE_SIZE - 2;
                    continue;
                }

                //Other out-of-range data
                if (dataXCoord >= this.consoleColumns || dataXCoord < 0 ||
                    dataYCoord < 0 || dataYCoord >= this.consoleRows) {
                    console.error("Out of range cell update: [(" + dataXCoord + "," + dataYCoord + ") " + combinedUTF16Char + "]");
                    dIndex += 6;
                    continue;
                }

                var charToDraw = String.fromCharCode(combinedUTF16Char);
                if(this.useTiles) {
                    charToDraw = combinedUTF16Char;
                }
                else if(this.remapGlyphs) {
                    charToDraw = remapBrogueGlyphs(combinedUTF16Char);
                }               
                
                //Just selectively render this cell
                this.renderCell({
                    x: dataXCoord,
                    y: dataYCoord,
                    char: charToDraw,
                    foregroundRed: dataArray[dIndex++],
                    foregroundGreen: dataArray[dIndex++],
                    foregroundBlue: dataArray[dIndex++],
                    backgroundRed: dataArray[dIndex++],
                    backgroundGreen: dataArray[dIndex++],
                    backgroundBlue: dataArray[dIndex++]
                });
            }
        },

        setupTileMapping: function() {

            this.tileWidth = 19;
            this.tileHeight = 33;

            this.tileMap = {};

            //32 -> 127 are 7-bit ASCII
            //128 -> 255 are the tiles
            //Locations in G_*, offset by 2
            //G_POTION = 130, but in 128 position on tilemap

            for(i = 32; i < 128; i++) {
                //var mapChar = String.fromCharCode(i);
                var tileLocation = [i % 16 * this.tileWidth, Math.floor(i / 16) * this.tileHeight];
                this.tileMap[i] = tileLocation;
            }

            for (i = 130; i < 255; i++) {
                var tileLocation = [(i - 2) % 16 * this.tileWidth, Math.floor((i - 2) / 16) * this.tileHeight];
                this.tileMap[i] = tileLocation;
            }
        },
        
        exitToLobby : function(message){
            activate.lobby();
            activate.currentGames();
            dispatcher.trigger("leaveGame");
        }
    });

    return Console;

});
