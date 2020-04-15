// Button which switches on and off dpad

define([
    "jquery",
    "underscore",
    "backbone",
], function($, _, Backbone) {

    var DpadVisibilityButtonView = Backbone.View.extend({
        
        el: "#console-dpad",

        events : {
            "click" : "handleClick"
        },
        
        handleClick : function(event){
        
            event.preventDefault();

            $('#console-dpad-holder').toggleClass("inactive");

            this.positionDPad();
            
        },
        positionDPad : function () {

            let consoleActiveDPadButton = document.getElementById('console-dpad');
            let consoleCentreDPadButton = document.getElementById('console-centre');
            let consoleUpDPadButton = document.getElementById('console-up');
            let consoleDownDPadButton = document.getElementById('console-down');
            let consoleLeftDPadButton = document.getElementById('console-left');
            let consoleRightDPadButton = document.getElementById('console-right');
            let consoleDownRightDPadButton = document.getElementById('console-down-right');
            let consoleUpRightDPadButton = document.getElementById('console-up-right');
            let consoleUpLeftDPadButton = document.getElementById('console-up-left');
            let consoleDownLeftDPadButton = document.getElementById('console-down-left');
            let consoleUpRightRightDPadButton = document.getElementById('console-up-right-right');
            let consoleRightRightDPadButton = document.getElementById('console-right-right');
            let consoleDownRightRightDPadButton = document.getElementById('console-down-right-right');
            
            function translateDPadButton(dPadButtonElement, dPadLeftOffset, dPadTopOffset, buttonLeftOffset, buttonTopOffset, buttonRect) {
                
                let thisButtonRect = dPadButtonElement.getBoundingClientRect();
        
                let translateX = visualViewport.offsetLeft + dPadLeftOffset + buttonLeftOffset + (buttonRect.width - thisButtonRect.width) / 2;
                let translateY = visualViewport.offsetTop + dPadTopOffset + buttonTopOffset + (buttonRect.height - thisButtonRect.height) / 2;;
        
                applyScaledTranslation(dPadButtonElement, translateX, translateY);
            };
        
            function applyScaledTranslation(dPadButtonElement, x, y) {
                dPadButtonElement.style.transform = 'translate(' +  x + 'px,' + y + 'px) scale(' + 1 / visualViewport.scale + ')';
            };
            
            let visualViewport = window.visualViewport;
        
            let buttonRect = consoleUpDPadButton.getBoundingClientRect();
            let largestButtonRect = buttonRect;
        
            let dPadWidth = buttonRect.width * 5;
            let dPadHeight = buttonRect.height * 3;
            let dPadOffsetBotRatio = 0.2;
            let dPadOffsetLeftRatio = 0.05;
        
            let dPadButtonOffsetX = buttonRect.width;
            let dPadButtonOffsetY = buttonRect.height;
        
            let dPadBotOffset = Math.max(0, (visualViewport.height - dPadHeight) * dPadOffsetBotRatio);
            let dPadTopOffset = Math.max(0, visualViewport.height - dPadBotOffset - dPadHeight);
        
            let dPadLeftOffset = Math.max(0, (visualViewport.width - dPadWidth) * dPadOffsetLeftRatio);
        
            let dpadRect = consoleActiveDPadButton.getBoundingClientRect();
            applyScaledTranslation(consoleActiveDPadButton, visualViewport.offsetLeft + dPadLeftOffset, visualViewport.offsetTop + visualViewport.height - dpadRect.height);
        
            let dpadButtonsTopOffset = dPadTopOffset;
            let dpadButtonsLeftOffset = dPadLeftOffset;
        
            translateDPadButton(consoleUpDPadButton, dpadButtonsLeftOffset, dpadButtonsTopOffset, 1 * dPadButtonOffsetX, 0, largestButtonRect);
            translateDPadButton(consoleUpRightDPadButton, dpadButtonsLeftOffset, dpadButtonsTopOffset, 2 * dPadButtonOffsetX, 0, largestButtonRect);
            translateDPadButton(consoleRightDPadButton, dpadButtonsLeftOffset, dpadButtonsTopOffset, 2 * dPadButtonOffsetX, dPadButtonOffsetY, largestButtonRect);
            translateDPadButton(consoleDownRightDPadButton, dpadButtonsLeftOffset, dpadButtonsTopOffset, 2 * dPadButtonOffsetX, 2 * dPadButtonOffsetY, largestButtonRect);
            translateDPadButton(consoleDownDPadButton, dpadButtonsLeftOffset, dpadButtonsTopOffset, 1 * dPadButtonOffsetX, 2 * dPadButtonOffsetY, largestButtonRect);
            translateDPadButton(consoleDownLeftDPadButton, dpadButtonsLeftOffset, dpadButtonsTopOffset, 0 * dPadButtonOffsetX, 2 * dPadButtonOffsetY, largestButtonRect);
            translateDPadButton(consoleLeftDPadButton, dpadButtonsLeftOffset, dpadButtonsTopOffset, 0 * dPadButtonOffsetX, dPadButtonOffsetY, largestButtonRect);
            translateDPadButton(consoleUpLeftDPadButton, dpadButtonsLeftOffset, dpadButtonsTopOffset, 0 * dPadButtonOffsetX, 0, largestButtonRect);
        
            translateDPadButton(consoleCentreDPadButton, dpadButtonsLeftOffset, dpadButtonsTopOffset, 1 * dPadButtonOffsetX, dPadButtonOffsetY, largestButtonRect);
        
            translateDPadButton(consoleUpRightRightDPadButton, dpadButtonsLeftOffset, dpadButtonsTopOffset, 3 * dPadButtonOffsetX, 0, largestButtonRect);
            translateDPadButton(consoleRightRightDPadButton, dpadButtonsLeftOffset, dpadButtonsTopOffset, 3 * dPadButtonOffsetX, dPadButtonOffsetY, largestButtonRect);
            translateDPadButton(consoleDownRightRightDPadButton, dpadButtonsLeftOffset, dpadButtonsTopOffset, 3 * dPadButtonOffsetX, 2 * dPadButtonOffsetY, largestButtonRect);
        
            //console.log("visualviewport offsetLeft " + visualViewport.offsetLeft + " offsetTop " + visualViewport.offsetTop + " height: " + visualViewport.height + " width: " + visualViewport.width + " scale: " + visualViewport.scale);
            //console.log("buttonUpLeftOffset: " + buttonUpLeftOffset + " buttonUpTopOffset:" + buttonUpTopOffset);
            //console.log("buttonUpLeftOffsetDelta: " + (buttonUpLeftOffset - visualViewport.offsetLeft) + " buttonUpTopOffsetDelta:" + (buttonUpTopOffset - visualViewport.offsetTop));
        
          }
    });

    return DpadVisibilityButtonView;
});