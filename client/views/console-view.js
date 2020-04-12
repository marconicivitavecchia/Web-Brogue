// View for the entire console.  It is responsible for setting up all of the console-cell views

define([
    "jquery",
    "underscore",
    "backbone",
    "dispatcher",
    "variantLookup",
    'dataIO/send-keypress',
    "views/console-cell-view",
    "models/console-cell",
    "views/view-activation-helpers"
], function($, _, Backbone, dispatcher, variantLookup, sendKeypressEvent, ConsoleCellView, CellModel, activate) {

    var _MESSAGE_UPDATE_SIZE = 10;

    var _consoleCells = [];
    var _consoleWidth;
    var _consoleHeight;
    var _consoleCellTopOffsetPercent;
    var _consoleCellLeftOffsetPercent;
    var _consoleCellWidthPercent;
    var _consoleCellHeightPercent;
    var _consoleCellCharSizePx;
    var _consoleCellCharPaddingPx;
    var _consoleCellAspectRatio = 0.53;  //TODO: we may eventually want this to be adjustable

    // See BrogueCode/rogue.h for all brogue event definitions
    var KEYPRESS_EVENT_CHAR = 0;

    var Console = Backbone.View.extend({
        el: "#console",
        events: {
            'keydown' : 'keydownHandler',
            'keyup' : 'keyupHandler',
            //'focus' : 'giveKeyboardFocus'
        },

        giveKeyboardFocus : function(){
            $('#console-keyboard').focus();
        },  

        keydownHandler: function(event) {

            event.preventDefault();

            //Acknowledge direction keys on keydown which includes key repeat
            var eventKey = event.key;
            var ctrlKey = event.ctrlKey;
            var shiftKey = event.shiftKey;

            var returnCode;

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

        initialize: function() {
            this.$el.addClass("full-height");
        },

        initialiseForNewGame: function(data) {

            var variantCode = _.findWhere(_.values(variantLookup.variants), {default: true}).code;
            if('variant' in data) {
                variantCode = data.variant;
            }
            
            this.consoleColumns = variantLookup.variants[variantCode].consoleColumns;
            this.consoleRows = variantLookup.variants[variantCode].consoleRows;
            this.initializeConsoleCells();
            this.resize();
        },

        initializeConsoleCells: function() {
            var consoleCellsFragment = document.createDocumentFragment();

            _consoleCells = [];
            this.$el.children("div.console-cell").remove();

            for (var i = 0; i < this.consoleColumns; i++) {
                var column = [];
                for (var j = 0; j < this.consoleRows; j++) {
                    var cellModel = new CellModel({
                        x: i,
                        y: j,
                        widthPercent: _consoleCellWidthPercent,
                        heightPercent: _consoleCellHeightPercent,
                        charSizePx: _consoleCellCharSizePx,
                        charPaddingPx: _consoleCellCharPaddingPx,
                        topOffsetPercent: _consoleCellTopOffsetPercent,
                        leftOffsetPercent: _consoleCellLeftOffsetPercent
                    });

                    var cellView = new ConsoleCellView({
                        model: cellModel,
                        id: "console-cell-" + i + "-" + j
                    });

                    consoleCellsFragment.appendChild(cellView.render().el);
                    column.push(cellView);
                }
                _consoleCells.push(column);
            }
            
            this.$el.append(consoleCellsFragment);
        },
        calculateConsoleSize: function() {
            _consoleWidth = this.$el.width();
            _consoleHeight = this.$el.height();
        },
        calculateConsoleCellSize: function() {

            _consoleCellWidthPercent = 100 / this.consoleColumns;

            // Cell Aspect Ratio
            var cellPixelWidth = _consoleWidth * (_consoleCellWidthPercent / 100);
            var cellPixelHeight = cellPixelWidth / _consoleCellAspectRatio;

            //If this height will make the console go off screen, recalculate size and horizontally center instead
            if (cellPixelHeight * this.consoleRows > _consoleHeight) {
                cellPixelHeight = _consoleHeight / this.consoleRows;
                cellPixelWidth = cellPixelHeight * _consoleCellAspectRatio;

                _consoleCellHeightPercent = 100 / this.consoleRows;
                _consoleCellWidthPercent = 100 * cellPixelWidth / _consoleWidth;
                _consoleCellTopOffsetPercent = 0;

                var leftOffSetPx = (_consoleWidth - cellPixelWidth * this.consoleColumns) / 2;
                _consoleCellLeftOffsetPercent = leftOffSetPx / _consoleWidth * 100;
            }
            else {
                // Vertically center the console
                _consoleCellHeightPercent = 100 * cellPixelHeight / _consoleHeight;
                _consoleCellLeftOffsetPercent = 0;
                var topOffSetPx = (_consoleHeight - cellPixelHeight * this.consoleRows) / 2;
                _consoleCellTopOffsetPercent = topOffSetPx / _consoleHeight * 100;
            }

            // Cell Character Positioning
            _consoleCellCharSizePx = cellPixelHeight * 3 / 5;
            _consoleCellCharPaddingPx = cellPixelHeight / 10;
        },
        render: function() {

            for (var i = 0; i < this.consoleColumns; i++) {
                for (var j = 0; j < this.consoleRows; j++) {
                    _consoleCells[i][j].render();
                }
            }
        },

        resize: function() {
            this.calculateConsoleSize();
            this.calculateConsoleCellSize();
            this.setNewConsoleCellSize();
        },
        setNewConsoleCellSize : function() {

            for (var i = 0; i < this.consoleColumns; i++) {
                for (var j = 0; j < this.consoleRows; j++) {
                    _consoleCells[i][j].model.set({
                        widthPercent: _consoleCellWidthPercent,
                        heightPercent: _consoleCellHeightPercent,
                        charSizePx: _consoleCellCharSizePx,
                        charPaddingPx: _consoleCellCharPaddingPx,
                        topOffsetPercent: _consoleCellTopOffsetPercent,
                        leftOffsetPercent: _consoleCellLeftOffsetPercent
                    });
                    _consoleCells[i][j].model.calculatePositionAttributes();
                    _consoleCells[i][j].applySize();
                }
            }
        },
        
        queueUpdateCellModelData : function(data){
            // todo -- comment
            var self = this;
            setTimeout(function(){
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

                _consoleCells[dataXCoord][dataYCoord].model.set({
                    char: combinedUTF16Char,
                    foregroundRed: dataArray[dIndex++],
                    foregroundGreen: dataArray[dIndex++],
                    foregroundBlue: dataArray[dIndex++],
                    backgroundRed: dataArray[dIndex++],
                    backgroundGreen: dataArray[dIndex++],
                    backgroundBlue: dataArray[dIndex++]
                });

                _consoleCells[dataXCoord][dataYCoord].render();
            }
        },
        
        clearConsole : function(){
            for (var i = 0; i < this.consoleColumns; i++) {
                for (var j = 0; j < this.consoleRows; j++) {
                    _consoleCells[i][j].model.clear();
                    _consoleCells[i][j].render();
                }
            }
        },
        
        exitToLobby : function(message){
            activate.lobby();
            activate.currentGames();
            dispatcher.trigger("leaveGame");
            this.clearConsole();
        }
    });

    return Console;

});
