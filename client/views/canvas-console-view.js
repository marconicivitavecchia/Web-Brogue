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
    "views/view-activation-helpers"
], function($, _, Backbone, ROT, dispatcher, variantLookup, sendKeypressEvent, sendMouseEvent, ConsoleCanvasCellModel, activate) {

    var _MESSAGE_UPDATE_SIZE = 10;
    var MOUSE_UP_EVENT_CHAR = 1;
    var MOUSE_DOWN_EVENT_CHAR = 2;
    var MOUSE_HOVER_EVENT_CHAR = 5;
    var MOUSEOVER_SIDEBAR_RATE_LIMIT_MS = 200;

    const G_UP_ARROW = 128;
    const G_DOWN_ARROW = 129;
    const G_POTION = 130;
    const G_GRASS = 131;
    const G_WALL = 132;
    const G_DEMON = 133;
    const G_OPEN_DOOR = 134;
    const G_GOLD = 135;
    const G_CLOSED_DOOR = 136;
    const G_RUBBLE = 137;
    const G_KEY = 138;
    const G_BOG = 139;
    const G_CHAIN_TOP_LEFT = 140;
    const G_CHAIN_BOTTOM_RIGHT = 141;
    const G_CHAIN_TOP_RIGHT = 142;
    const G_CHAIN_BOTTOM_LEFT = 143;
    const G_CHAIN_TOP = 144;
    const G_CHAIN_BOTTOM = 145;
    const G_CHAIN_LEFT = 146;
    const G_CHAIN_RIGHT = 147;
    const G_FOOD = 148;
    const G_UP_STAIRS = 149;
    const G_VENT = 150;
    const G_DOWN_STAIRS = 151;
    const G_PLAYER = 152;
    const G_BOG_MONSTER = 153;
    const G_CENTAUR = 154;
    const G_DRAGON = 155;
    const G_FLAMEDANCER = 156;
    const G_GOLEM = 157;
    const G_TENTACLE_HORROR = 158;
    const G_IFRIT = 159;
    const G_JELLY = 160;
    const G_KRAKEN = 161;
    const G_LICH = 162;
    const G_NAGA = 163;
    const G_OGRE = 164;
    const G_PHANTOM = 165;
    const G_REVENANT = 166;
    const G_SALAMANDER = 167;
    const G_TROLL = 168;
    const G_UNDERWORM = 169;
    const G_VAMPIRE = 170;
    const G_WRAITH = 171;
    const G_ZOMBIE = 172;
    const G_ARMOR = 173;
    const G_STAFF = 174;
    const G_WEB = 175;
    const G_MOUND = 176;
    const G_BLOAT = 177;
    const G_CENTIPEDE = 178;
    const G_DAR_BLADEMASTER = 179;
    const G_EEL = 180;
    const G_FURY = 181;
    const G_GOBLIN = 182;
    const G_IMP = 183;
    const G_JACKAL = 184;
    const G_KOBOLD = 185;
    const G_MONKEY = 186;
    const G_PIXIE = 187;
    const G_RAT = 188;
    const G_SPIDER = 189;
    const G_TOAD = 190;
    const G_BAT = 191;
    const G_WISP = 192;
    const G_PHOENIX = 193;
    const G_ALTAR = 194;
    const G_LIQUID = 195;
    const G_FLOOR = 196;
    const G_CHASM = 197;
    const G_TRAP = 198;
    const G_FIRE = 199;
    const G_FOLIAGE = 200;
    const G_AMULET = 201;
    const G_SCROLL = 202;
    const G_RING = 203;
    const G_WEAPON = 204;
    const G_GEM = 205;
    const G_TOTEM = 206;
    const G_GOOD_MAGIC = 207;
    const G_BAD_MAGIC = 208;
    const G_DOORWAY = 209;
    const G_CHARM = 210;
    const G_WALL_TOP = 211;
    const G_DAR_PRIESTESS = 212;
    const G_DAR_BATTLEMAGE = 213;
    const G_GOBLIN_MAGIC = 214;
    const G_GOBLIN_CHIEFTAN = 215;
    const G_OGRE_MAGIC = 216;
    const G_GUARDIAN = 217;
    const G_WINGED_GUARDIAN = 218;
    const G_EGG = 219;
    const G_WARDEN = 220;
    const G_DEWAR = 221;
    const G_ANCIENT_SPIRIT = 222;
    const G_LEVER = 223;
    const G_LEVER_PULLED = 224;
    const G_BLOODWORT_STALK = 225;
    const G_FLOOR_ALT = 226;
    const G_UNICORN = 227;
    const G_TURRET = 228;
    const G_WAND = 229;
    const G_GRANITE = 230;
    const G_CARPET = 231;
    const G_CLOSED_IRON_DOOR = 232;
    const G_OPEN_IRON_DOOR = 233;
    const G_TORCH = 234;
    const G_CRYSTAL = 235;
    const G_PORTCULLIS = 236;
    const G_BARRICADE = 237;
    const G_STATUE = 238;
    const G_CRACKED_STATUE = 239;
    const G_CLOSED_CAGE = 240;
    const G_OPEN_CAGE = 241;
    const G_PEDESTAL = 242;
    const G_CLOSED_COFFIN = 243;
    const G_OPEN_COFFIN = 244;
    const G_MAGIC_GLYPH = 245;
    const G_BRIDGE = 246;
    const G_BONES = 247;
    const G_ELECTRIC_CRYSTAL = 248;
    const G_ASHES = 249;
    const G_BEDROLL = 250;
    const G_BLOODWORT_POD = 251;

    const U_MIDDLE_DOT = '\u00b7';
    const U_FOUR_DOTS = '\u2237';
    const U_DIAMOND = '\u25c7';
    const U_FLIPPED_V = '\u22CF';
    const U_ESZETT = '\u00df';
    const U_ANKH = '\u2640';
    const U_MUSIC_NOTE = '\u266A';
    const U_LIGHTNING_BOLT = '\u03DF';
    const U_FILLED_CIRCLE = '\u25cf';
    const U_NEUTER = '\u26b2';
    const U_U_ACUTE = '\u00da';
    const U_CURRENCY = '\u00A4';
    const U_UP_ARROW = '\u2191';
    const U_DOWN_ARROW = '\u2193';
    const U_LEFT_ARROW = '\u2190';
    const U_RIGHT_ARROW = '\u2192';
    const U_OMEGA = '\u03A9';

    //Web-brogue specific mappings to avoid emojis and glyphs not in some fonts
    const U_ARIES = '\u03C8';
    const U_CIRCLE = 'o';
    const U_CIRCLE_BARS = '\u25C6';
    const U_FILLED_CIRCLE_BARS = '\u25C7';

    // See BrogueCode/rogue.h for all brogue event definitions
    var KEYPRESS_EVENT_CHAR = 0;

    var Console = Backbone.View.extend({
        el: "#console-canvas",
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

        initialize: function() {
            
            this.useTiles = false;
            if(this.useTiles) {
                this.tileSet = document.createElement("img");
                this.tileSet.src = "tiles.png";

                this.setupTileMapping();

                this.d = new ROT.Display({ 
                        layout: "tile",
                        bg: "transparent",
                        tileWidth: this.tileWidth,
                        tileHeight: this.tileHeight,
                        tileMap: this.tileMap,
                        tileSet: this.tileSet,
                        tileColorize: true,
                        width: 10,
                        height: 10
                    });
                var canvas = this.d.getContainer();
                this.$el.append(canvas);
            }
            else {
                this.d = new ROT.Display({ width: 10, height: 10, fontFamily: "Source Code Pro", spacing: 1.5, fontSize: 25});
                var canvas = this.d.getContainer();
                this.$el.append(canvas);
            }
        },

        initialiseForNewGame: function(data) {

            var variantCode = _.findWhere(_.values(variantLookup.variants), {default: true}).code;
            if('variant' in data) {
                variantCode = data.variant;
            }

            this.variant = variantLookup.variants[variantCode];
            
            this.consoleColumns = this.variant.consoleColumns;
            this.consoleRows = this.variant.consoleRows;
            this.remapGlyphs = this.variant.remapGlyphs;
            
            //Create consoleCells models
            var consoleCells = [];
            for (var i=0; i<this.consoleColumns; i++) {
                consoleCells[i] = [];
                for (var j=0; j<this.consoleRows; j++) {
                    consoleCells[i][j] = new ConsoleCanvasCellModel();
                }
            }

            this._consoleCells = consoleCells;

            //Setup display
            this.d.setOptions({
                width: this.consoleColumns, 
                height: this.consoleRows
            });

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
                //Be slightly more conservative than rot.js, based on experience of different browsers
                var wrapperWidth = document.getElementById("console-wrapper").offsetWidth;
                var wrapperHeight = document.getElementById("console-wrapper").offsetHeight;
                var maxFontSize = this.d.computeFontSize(wrapperWidth, wrapperHeight) - 1;

                console.log("wrapperWidth: " + wrapperWidth + " wrapperHeight: " + wrapperHeight + " maxFontSize: " + maxFontSize);

                this.d.setOptions({
                    fontSize: maxFontSize, 
                });  
            }
            else {
                var wrapperWidth = document.getElementById("console-wrapper").offsetWidth;
                var wrapperHeight = document.getElementById("console-wrapper").offsetHeight;

                //Abused to set the convas style width
                this.d.computeFontSize(wrapperWidth, wrapperHeight);
            }
        },
        
        queueUpdateCellModelData : function(data){
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
                    charToDraw = this.remapBrogueGlyphs(combinedUTF16Char);
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

        remapBrogueGlyphs: function(glyphCode) {

            //Map between brogue glyphs and unicode if we are in font-rendering
            //or use directly (mapping in display) in tile-rendering mode
            if (glyphCode < 128) return String.fromCharCode(glyphCode);

            switch (glyphCode) {
                case G_UP_ARROW: return U_UP_ARROW;
                case G_DOWN_ARROW: return U_DOWN_ARROW;
                case G_POTION: return '!';
                case G_GRASS: return '"';
                case G_WALL: return '#';
                case G_DEMON: return '&';
                case G_OPEN_DOOR: return '\'';
                case G_GOLD: return '*';
                case G_CLOSED_DOOR: return '+';
                case G_RUBBLE: return ',';
                case G_KEY: return '-';
                case G_BOG: return '~';
                case G_CHAIN_TOP_LEFT:
                case G_CHAIN_BOTTOM_RIGHT:
                    return '\\';
                case G_CHAIN_TOP_RIGHT:
                case G_CHAIN_BOTTOM_LEFT:
                    return '/';
                case G_CHAIN_TOP:
                case G_CHAIN_BOTTOM:
                    return '|';
                case G_CHAIN_LEFT:
                case G_CHAIN_RIGHT:
                    return '-';
                case G_FOOD: return ';';
                case G_UP_STAIRS: return '<';
                case G_VENT: return '=';
                case G_DOWN_STAIRS: return '>';
                case G_PLAYER: return '@';
                case G_BOG_MONSTER: return 'B';
                case G_CENTAUR: return 'C';
                case G_DRAGON: return 'D';
                case G_FLAMEDANCER: return 'F';
                case G_GOLEM: return 'G';
                case G_TENTACLE_HORROR: return 'H';
                case G_IFRIT: return 'I';
                case G_JELLY: return 'J';
                case G_KRAKEN: return 'K';
                case G_LICH: return 'L';
                case G_NAGA: return 'N';
                case G_OGRE: return 'O';
                case G_PHANTOM: return 'P';
                case G_REVENANT: return 'R';
                case G_SALAMANDER: return 'S';
                case G_TROLL: return 'T';
                case G_UNDERWORM: return 'U';
                case G_VAMPIRE: return 'V';
                case G_WRAITH: return 'W';
                case G_ZOMBIE: return 'Z';
                case G_ARMOR: return '[';
                case G_STAFF: return '/';
                case G_WEB: return ':';
                case G_MOUND: return 'a';
                case G_BLOAT: return 'b';
                case G_CENTIPEDE: return 'c';
                case G_DAR_BLADEMASTER: return 'd';
                case G_EEL: return 'e';
                case G_FURY: return 'f';
                case G_GOBLIN: return 'g';
                case G_IMP: return 'i';
                case G_JACKAL: return 'j';
                case G_KOBOLD: return 'k';
                case G_MONKEY: return 'm';
                case G_PIXIE: return 'p';
                case G_RAT: return 'r';
                case G_SPIDER: return 's';
                case G_TOAD: return 't';
                case G_BAT: return 'v';
                case G_WISP: return 'w';
                case G_PHOENIX: return 'P';
                case G_ALTAR: return '|';
                case G_LIQUID: return '~';
                case G_FLOOR: return U_MIDDLE_DOT;
                case G_CHASM: return U_FOUR_DOTS;
                case G_TRAP: return U_DIAMOND;
                case G_FIRE: return U_FLIPPED_V;
                case G_FOLIAGE: return U_ARIES;
                case G_AMULET: return U_ANKH;
                case G_SCROLL: return U_MUSIC_NOTE;
                case G_RING: return U_CIRCLE;
                case G_WEAPON: return U_UP_ARROW;
                case G_GEM: return U_FILLED_CIRCLE;
                case G_TOTEM: return U_NEUTER;
                case G_GOOD_MAGIC: return U_FILLED_CIRCLE_BARS;
                case G_BAD_MAGIC: return U_CIRCLE_BARS;
                case G_DOORWAY: return U_OMEGA;
                case G_CHARM: return U_LIGHTNING_BOLT;
                case G_WALL_TOP: return '#';
                case G_DAR_PRIESTESS: return 'd';
                case G_DAR_BATTLEMAGE: return 'd';
                case G_GOBLIN_MAGIC: return 'g';
                case G_GOBLIN_CHIEFTAN: return 'g';
                case G_OGRE_MAGIC: return 'O';
                case G_GUARDIAN: return U_ESZETT;
                case G_WINGED_GUARDIAN: return U_ESZETT;
                case G_EGG: return U_FILLED_CIRCLE;
                case G_WARDEN: return 'Y';
                case G_DEWAR: return '&';
                case G_ANCIENT_SPIRIT: return 'M';
                case G_LEVER: return '/';
                case G_LEVER_PULLED: return '\\';
                case G_BLOODWORT_STALK: return U_ARIES;
                case G_FLOOR_ALT: return U_MIDDLE_DOT;
                case G_UNICORN: return U_U_ACUTE;
                case G_TURRET: return U_FILLED_CIRCLE;
                case G_WAND: return '~';
                case G_GRANITE: return '#';
                case G_CARPET: return U_MIDDLE_DOT;
                case G_CLOSED_IRON_DOOR: return '+';
                case G_OPEN_IRON_DOOR: return '\'';
                case G_TORCH: return '#';
                case G_CRYSTAL: return '#';
                case G_PORTCULLIS: return '#';
                case G_BARRICADE: return '#';
                case G_STATUE: return U_ESZETT;
                case G_CRACKED_STATUE: return U_ESZETT;
                case G_CLOSED_CAGE: return '#';
                case G_OPEN_CAGE: return '|';
                case G_PEDESTAL: return '|';
                case G_CLOSED_COFFIN: return '-';
                case G_OPEN_COFFIN: return '-';
                case G_MAGIC_GLYPH: return U_FOUR_DOTS;
                case G_BRIDGE: return '=';
                case G_BONES: return ',';
                case G_ELECTRIC_CRYSTAL: return U_CURRENCY;
                case G_ASHES: return '\'';
                case G_BEDROLL: return '=';
                case G_BLOODWORT_POD: return '*';
            } 
            
            return String.fromCharCode(glyphCode);
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
