// Button which switches on and off dpad

define([
    "jquery",
    "underscore",
    "backbone",
], function($, _, Backbone) {

    var DpadVisibilityButtonView = Backbone.View.extend({
        
        events : {
            "click" : "handleClick"
        },
        
        handleClick : function(event){
        
            event.preventDefault();

            var dPadHolder = '#' + this.prefix + 'dpad-holder';
            $(dPadHolder).toggleClass("inactive");

            this.positionDPad();
            
        },

        setDPadPrefix : function (prefix) {
            this.prefix = prefix;
        },

        positionDPad : function () {
            
            //Don't do a resize etc. if the prefix is not set yet
            if (!this.prefix) {
                return;
            }

            let consoleActiveDPadButton = document.getElementById(this.prefix + 'dpad');
            let consoleCentreDPadButton = document.getElementById(this.prefix + 'centre');
            let consoleUpDPadButton = document.getElementById(this.prefix + 'up');
            let consoleDownDPadButton = document.getElementById(this.prefix + 'down');
            let consoleLeftDPadButton = document.getElementById(this.prefix + 'left');
            let consoleRightDPadButton = document.getElementById(this.prefix + 'right');
            let consoleDownRightDPadButton = document.getElementById(this.prefix + 'down-right');
            let consoleUpRightDPadButton = document.getElementById(this.prefix + 'up-right');
            let consoleUpLeftDPadButton = document.getElementById(this.prefix + 'up-left');
            let consoleDownLeftDPadButton = document.getElementById(this.prefix + 'down-left');
            let consoleDownRightRightDPadButton = document.getElementById(this.prefix + 'down-right-right');
            let consoleRightRightDPadButton = document.getElementById(this.prefix + 'right-right');
            let consoleUpRightRightDPadButton = document.getElementById(this.prefix + 'up-right-right');
            let consoleUpUpRightRightDPadButton = document.getElementById(this.prefix + 'up-up-right-right');
            let consoleUpUpRightDPadButton = document.getElementById(this.prefix + 'up-up-right');
            let consoleUpUpDPadButton = document.getElementById(this.prefix + 'up-up');
            let consoleUpUpLeftDPadButton = document.getElementById(this.prefix + 'up-up-left');
            let consoleUpUpUpLeftDPadButton = document.getElementById(this.prefix + 'up-up-up-left');


            function translateDPadButton(dPadButtonElement, dPadLeftOffset, dPadTopOffset, buttonLeftOffset, buttonTopOffset, buttonRect) {

                let thisButtonRect = dPadButtonElement.getBoundingClientRect();

                let translateX = visualViewport.offsetLeft + dPadLeftOffset + buttonLeftOffset + (buttonRect.width - thisButtonRect.width) / 2;
                let translateY = visualViewport.offsetTop + dPadTopOffset + buttonTopOffset + (buttonRect.height - thisButtonRect.height) / 2;

                applyScaledTranslation(dPadButtonElement, translateX, translateY);
            };

            function applyScaledTranslation(dPadButtonElement, x, y) {
                dPadButtonElement.style.transform = 'translate(' +  x + 'px,' + y + 'px) scale(' + 1 / visualViewport.scale + ')';
            };

            let visualViewport = window.visualViewport;

            let buttonRect = consoleUpDPadButton.getBoundingClientRect();
            let largestButtonRect = buttonRect;

            let dPadWidth = buttonRect.width * 5;
            let dPadHeight = buttonRect.height * 5;
            let dPadOffsetBotRatio = 0.1;
            let dPadOffsetLeftRatio = 0.02;

            let dPadButtonOffsetX = buttonRect.width;
            let dPadButtonOffsetY = buttonRect.height;

            let dPadBotOffset = Math.max(0, (visualViewport.height - dPadHeight) * dPadOffsetBotRatio);
            let dPadTopOffset = Math.max(0, visualViewport.height - dPadBotOffset - dPadHeight);

            let dPadLeftOffset = Math.max(0, (visualViewport.width - dPadWidth) * dPadOffsetLeftRatio);

            let dpadActivateRect = consoleActiveDPadButton.getBoundingClientRect();
            applyScaledTranslation(consoleActiveDPadButton, visualViewport.offsetLeft + visualViewport.width - dpadActivateRect.width, visualViewport.offsetTop + visualViewport.height - dpadActivateRect.height);

            let dpadButtonsTopOffset = dPadTopOffset;
            let dpadButtonsLeftOffset = dPadLeftOffset;

            translateDPadButton(consoleUpDPadButton, dpadButtonsLeftOffset, dpadButtonsTopOffset, 1 * dPadButtonOffsetX, 2 * dPadButtonOffsetY, largestButtonRect);
            translateDPadButton(consoleUpRightDPadButton, dpadButtonsLeftOffset, dpadButtonsTopOffset, 2 * dPadButtonOffsetX, 2 * dPadButtonOffsetY, largestButtonRect);
            translateDPadButton(consoleRightDPadButton, dpadButtonsLeftOffset, dpadButtonsTopOffset, 2 * dPadButtonOffsetX, 3 * dPadButtonOffsetY, largestButtonRect);
            translateDPadButton(consoleDownRightDPadButton, dpadButtonsLeftOffset, dpadButtonsTopOffset, 2 * dPadButtonOffsetX, 4 * dPadButtonOffsetY, largestButtonRect);
            translateDPadButton(consoleDownDPadButton, dpadButtonsLeftOffset, dpadButtonsTopOffset, 1 * dPadButtonOffsetX, 4 * dPadButtonOffsetY, largestButtonRect);
            translateDPadButton(consoleDownLeftDPadButton, dpadButtonsLeftOffset, dpadButtonsTopOffset, 0 * dPadButtonOffsetX, 4 * dPadButtonOffsetY, largestButtonRect);
            translateDPadButton(consoleLeftDPadButton, dpadButtonsLeftOffset, dpadButtonsTopOffset, 0 * dPadButtonOffsetX, 3 * dPadButtonOffsetY, largestButtonRect);
            translateDPadButton(consoleUpLeftDPadButton, dpadButtonsLeftOffset, dpadButtonsTopOffset, 0 * dPadButtonOffsetX, 2 * dPadButtonOffsetY, largestButtonRect);

            translateDPadButton(consoleCentreDPadButton, dpadButtonsLeftOffset, dpadButtonsTopOffset, 1 * dPadButtonOffsetX, 3 * dPadButtonOffsetY, largestButtonRect);

            translateDPadButton(consoleUpRightRightDPadButton, dpadButtonsLeftOffset, dpadButtonsTopOffset, 3 * dPadButtonOffsetX, 2 * dPadButtonOffsetY, largestButtonRect);
            translateDPadButton(consoleRightRightDPadButton, dpadButtonsLeftOffset, dpadButtonsTopOffset, 3 * dPadButtonOffsetX, 3 * dPadButtonOffsetY, largestButtonRect);
            translateDPadButton(consoleDownRightRightDPadButton, dpadButtonsLeftOffset, dpadButtonsTopOffset, 3 * dPadButtonOffsetX, 4 * dPadButtonOffsetY, largestButtonRect);

            translateDPadButton(consoleUpUpLeftDPadButton, dpadButtonsLeftOffset, dpadButtonsTopOffset, 0 * dPadButtonOffsetX, 1 * dPadButtonOffsetY, largestButtonRect);
            translateDPadButton(consoleUpUpRightRightDPadButton, dpadButtonsLeftOffset, dpadButtonsTopOffset, 3 * dPadButtonOffsetX, 1 * dPadButtonOffsetY, largestButtonRect);
            translateDPadButton(consoleUpUpRightDPadButton, dpadButtonsLeftOffset, dpadButtonsTopOffset, 2 * dPadButtonOffsetX, 1 * dPadButtonOffsetY, largestButtonRect);
            translateDPadButton(consoleUpUpDPadButton, dpadButtonsLeftOffset, dpadButtonsTopOffset, 1 * dPadButtonOffsetX, 1 * dPadButtonOffsetY, largestButtonRect);
            translateDPadButton(consoleUpUpUpLeftDPadButton, dpadButtonsLeftOffset, dpadButtonsTopOffset, 0 * dPadButtonOffsetX, 0 * dPadButtonOffsetY, largestButtonRect);

            //console.log("visualviewport offsetLeft " + visualViewport.offsetLeft + " offsetTop " + visualViewport.offsetTop + " height: " + visualViewport.height + " width: " + visualViewport.width + " scale: " + visualViewport.scale);
            //console.log("buttonUpLeftOffset: " + buttonUpLeftOffset + " buttonUpTopOffset:" + buttonUpTopOffset);
            //console.log("buttonUpLeftOffsetDelta: " + (buttonUpLeftOffset - visualViewport.offsetLeft) + " buttonUpTopOffsetDelta:" + (buttonUpTopOffset - visualViewport.offsetTop));

        }
    });

    return DpadVisibilityButtonView;
});