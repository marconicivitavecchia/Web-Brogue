// View for a single console cell

define([
    "jquery",
    "underscore",
    "dispatcher",
    "backbone",
    "dataIO/send-mouse",
    "models/console-cell",
    "models/console-cell-shared"
], function($, _, dispatcher, Backbone, sendMouseEvent, CellModel, ConsoleCellShared) {

    // See BrogueCode/rogue.h for all brogue event definitions
    var MOUSE_UP_EVENT_CHAR = 1;
    var MOUSE_DOWN_EVENT_CHAR = 2;
    var RIGHT_MOUSE_DOWN_EVENT_CHAR = 3;
    var RIGHT_MOUSE_UP_EVENT_CHAR = 4;
    var MOUSE_HOVER_EVENT_CHAR = 5;
    var MOUSEOVER_SIDEBAR_RATE_LIMIT_MS = 200;

    var ConsoleCellView = Backbone.View.extend({
        tagName: "div",
        className: "console-cell",
        events : {
            "click" : "handleClick",
            "mouseover" : "handleMouseover"
        },

        initialize: function() {
            this.applySize();
        },

        render: function() {
            var cellCharacter = this.model.get("char");
            var rgbForegroundString = "rgb(" +
                    this.model.get("foregroundRed") + "," +
                    this.model.get("foregroundGreen") + "," +
                    this.model.get("foregroundBlue") + ")";
            var rgbBackgroundString = "rgb(" +
                    this.model.get("backgroundRed") + "," +
                    this.model.get("backgroundGreen") + "," +
                    this.model.get("backgroundBlue") + ")";

            this.el.textContent = cellCharacter;
            this.el.style.color = rgbForegroundString;
            this.el.style.backgroundColor = rgbBackgroundString;
            return this;
        },

        applySize : function(){
            var w = this.model.get("consoleWidth");
            var h = this.model.get("consoleHeight");
            var c = this.model.get("consoleColumns");
            var r = this.model.get("consoleRows");
            var x = this.model.get("x");
            var y = this.model.get("y");
            var ar = 16/29; // cell aspect ratio

            // Characters line up better when height is an integer
            var cellHeight = Math.floor(Math.min(h / r, w / (c * ar)));
            var cellWidth = Math.round(cellHeight * ar * 4) / 4;
            var leftPadding = Math.floor(0.5 * (w - cellWidth * c));
            var topPadding = Math.floor(0.5 * (h - cellHeight * r));

            this.el.style.top = (y * cellHeight + topPadding) + "px";
            this.el.style.left = (x * cellWidth + leftPadding) + "px";
            this.el.style.width = cellWidth + "px";
            this.el.style.height = cellHeight + "px";
            this.el.style.fontSize = cellHeight + "px";
            this.el.style.lineHeight = cellHeight + "px";
        },

        handleClick : function(event){

            event.preventDefault();

            sendMouseEvent(
                MOUSE_DOWN_EVENT_CHAR,
                this.model.get("x"),
                this.model.get("y"),
                event.ctrlKey,
                event.shiftKey
            );
            sendMouseEvent(
                MOUSE_UP_EVENT_CHAR,
                this.model.get("x"),
                this.model.get("y"),
                event.ctrlKey,
                event.shiftKey
            );
        },

        handleMouseover : function(event){

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

            var x = this.model.get("x");
            var y = this.model.get("y");

            //Rate limit mouseOvers in the sidebar since the game will always re-render fully.
            //Allow all other mouseOvers at full rate
            if(x >= 20 ||
                timeNow > ConsoleCellShared.lastMouseOver + MOUSEOVER_SIDEBAR_RATE_LIMIT_MS) {

                clearTimeout(ConsoleCellShared.mouseOverDelayedSend);
                sendMouseOverEvent(x, y, event.ctrlKey, event.shiftKey);
                ConsoleCellShared.lastMouseOver = d.getTime();
            }
            else {
                //Otherwise set a timeOut for this event to fire at the rate limit
                clearTimeout(ConsoleCellShared.mouseOverDelayedSend);
                var delay = ConsoleCellShared.lastMouseOver + MOUSEOVER_SIDEBAR_RATE_LIMIT_MS - timeNow;
                ConsoleCellShared.mouseOverDelayedSend = setTimeout(sendMouseOverEvent, delay, x, y, event.ctrlKey, event.shiftKey);
            }
        }
    });

    return ConsoleCellView;

});
