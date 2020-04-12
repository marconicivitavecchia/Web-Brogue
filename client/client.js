// Main entry point in the client side application.  Initializes all main views.

require.config({
    paths: {
        jquery : "libs/jquery",
        underscore : "libs/underscore",
        backbone : "libs/backbone",
        moment: "libs/moment",
        backbonePaginator: "libs/backbone.paginator",
        backgrid: "libs/backgrid",
        backgridPaginator: "libs/backgrid-paginator",
        io: "socket.io/socket.io.js",
        chart: "libs/chart"
    },
    shim: {
        'backbone': {
            deps: ['underscore', 'jquery'],
            exports: "Backbone"
        },

        'backgrid': {
            deps: ['backbone'],
            exports: "Backgrid"
        },

        'backgridPaginator': {
            deps: ['backbone', 'backgrid'],
            exports: 'Backgrid.Paginator'
        }
    }
});

require([
    "jquery",
    "underscore",
    "backbone",
    "backbonePaginator",
    "backgrid",
    "backgridPaginator",
    "dispatcher",
    "tests/debug-mode",
    "dataIO/socket",
    "dataIO/router",
    "models/high-scores",
    "models/chat",
    "models/site-news",
    "models/cause-stats-model",
    "models/level-stats-model",
    "models/general-stats-model",
    "models/level-probability-model",
    "models/dpad-button",
    "views/view-activation-helpers",
    "views/auth-view",
    "views/chat-view",
    "views/console-chat-view",
    "views/play-view",
    "views/header-view",
    "views/current-games-view",
    "views/mini-scores-view",
    "views/all-scores-view",
    "views/site-news-view",
    "views/console-view",
    "views/console-keystroke-processing-view",
    "views/popups/seed-popup-view",
    "views/statistics-view",
    "views/level-stats-view",
    "views/general-stats-view",
    "views/cause-stats-view",
    "views/level-probability-view",
    "views/dpad-button-view"
], function( $, _, Backbone, BackbonePaginator, Backgrid, BackgridPaginator, dispatcher, debugMode, socket, router, HighScoresModel, ChatModel, SiteNewsModel, CauseStatsModel, LevelStatsModel, GeneralStatsModel, LevelProbabilityModel, DPadButtonModel, activate, AuthView, ChatView, ConsoleChatView, PlayView, HeaderView, CurrentGamesView, HighScoresView, AllScoresView, SiteNewsView, ConsoleView, ConsoleKeyProcessingView, SeedPopupView, StatisticsView, LevelStatsView, GeneralStatsView, CauseStatsView, LevelProbabilityView, DPadButtonView){
    
    // If you want to enable debug mode, uncomment this function
    debugMode();
    
    // initialize each view
    var authView = new AuthView();
    var playView = new PlayView();
    var headerView = new HeaderView();
    var currentGamesView = new CurrentGamesView();
    var consoleView = new ConsoleView();
    var chatView = new ChatView({model: new ChatModel()});
    var consoleChatView = new ConsoleChatView({model: new ChatModel()});
    var statisticsView = new StatisticsView();
    var levelStatsView = new LevelStatsView({model: new LevelStatsModel()});
    var causeStatsView = new CauseStatsView({model: new CauseStatsModel()});
    var generalStatsView = new GeneralStatsView({model: new GeneralStatsModel()});
    var levelProbabilityView = new LevelProbabilityView({model: new LevelProbabilityModel()});
    var siteNewsView = new SiteNewsView({model: new SiteNewsModel() });
    var consoleKeyboardView = new ConsoleKeyProcessingView();
    var popups = {
        seedView : new SeedPopupView(),
    };

    //DPad
    var upArrowView = new DPadButtonView({el: "#console-up", model: new DPadButtonModel({ keyToSend: 63232 })});
    var upRightArrowView = new DPadButtonView({el: "#console-up-right", model: new DPadButtonModel({ keyToSend: 117 })});
    var rightArrowView = new DPadButtonView({el: "#console-right", model: new DPadButtonModel({ keyToSend: 63235 })});
    var downRightArrowView = new DPadButtonView({el: "#console-down-right", model: new DPadButtonModel({ keyToSend: 110 })});
    var downArrowView = new DPadButtonView({el: "#console-down", model: new DPadButtonModel({ keyToSend: 63233 })});
    var downLeftArrowView = new DPadButtonView({el: "#console-down-left", model: new DPadButtonModel({ keyToSend: 98 })});
    var leftArrowView = new DPadButtonView({el: "#console-left", model: new DPadButtonModel({ keyToSend: 63234 })});
    var upLeftArrowView = new DPadButtonView({el: "#console-up-left", model: new DPadButtonModel({ keyToSend: 121 })});
    var centreArrowView = new DPadButtonView({el: "#console-centre", model: new DPadButtonModel({ keyToSend: 53 })});
    var upRightRightIView = new DPadButtonView({el: "#console-up-right-right", model: new DPadButtonModel({ keyToSend: "i".charCodeAt(0) })});
    var rightRightXView = new DPadButtonView({el: "#console-right-right", model: new DPadButtonModel({ keyToSend: "x".charCodeAt(0) })});
    var downRightRightZView = new DPadButtonView({el: "#console-right-right", model: new DPadButtonModel({ keyToSend: "z".charCodeAt(0) })});

    var highScoresModel = new HighScoresModel();
    highScoresModel.fetch();
    setInterval(function() { highScoresModel.fetch(); }, 5 * 60 * 1000);
    var highScoresView = new HighScoresView({model: highScoresModel});

    var allScoresModel = new HighScoresModel();
    allScoresModel.fetch();
    setInterval(function() { allScoresModel.fetch(); }, 5 * 60 * 1000);
    var allScoresView = new AllScoresView({model: allScoresModel});

    // use dispatcher to co-ordinate multi-view actions on routed commands
    dispatcher.on("quit", highScoresView.quit, highScoresView);
    dispatcher.on("quit", consoleView.exitToLobby, consoleView);

    dispatcher.on("fail", highScoresView.quit, highScoresView);
    dispatcher.on("fail", consoleView.exitToLobby, consoleView);

    dispatcher.on("login", headerView.login, headerView);
    dispatcher.on("login", highScoresView.login, highScoresView);
    dispatcher.on("login", allScoresView.login, allScoresView);
    dispatcher.on("login", chatView.login, chatView);
    dispatcher.on("login", consoleChatView.login, consoleChatView);
    dispatcher.on("login", currentGamesView.login, currentGamesView);

    dispatcher.on("anon-login", headerView.anonymousLogin, headerView);
    dispatcher.on("anon-login", chatView.login, chatView);
    dispatcher.on("anon-login", consoleChatView.login, consoleChatView);

    dispatcher.on("logout", highScoresView.logout, highScoresView);
    dispatcher.on("logout", allScoresView.logout, allScoresView);
    dispatcher.on("logout", consoleChatView.logout, consoleChatView);
    dispatcher.on("logout", chatView.logout, chatView);
    dispatcher.on("logout", currentGamesView.logout, currentGamesView);
    dispatcher.on("logout", authView.logout, authView);

    dispatcher.on("all-scores", allScoresView.activate, allScoresView);

    dispatcher.on("chat", chatView.chatMessage, chatView);
    dispatcher.on("chat", consoleChatView.chatMessage, consoleChatView);

    dispatcher.on("showConsole", consoleView.resize, consoleView);

    dispatcher.on("startGame", headerView.startGame, headerView);
    dispatcher.on("startGame", consoleView.initialiseForNewGame, consoleView);

    dispatcher.on("observeGame", headerView.observeGame, headerView);
    dispatcher.on("observeGame", consoleView.initialiseForNewGame, consoleView);

    dispatcher.on("recordingGame", headerView.recordingGame, headerView);
    dispatcher.on("recordingGame", consoleView.initialiseForNewGame, consoleView);

    dispatcher.on("leaveGame", headerView.leaveGame, headerView);

    dispatcher.on("reconnect", authView.requestLogin, authView);
    dispatcher.on("reconnect", consoleView.exitToLobby, consoleView);

    dispatcher.on("focusConsole", consoleView.giveKeyboardFocus, consoleView);

    dispatcher.on("showSeedPopup", popups.seedView.showSeedPopup, popups.seedView);
    // set up routes for the messages from the websocket connection (only)
    router.registerHandlers({
        //Must bind 'this' to the scope of the view so we can use the internal view functions
        "error" : console.error.bind(console),
        "brogue" : consoleView.queueUpdateCellModelData.bind(consoleView),
        "quit" : function(data) { dispatcher.trigger("quit", data) },
        "lobby" : currentGamesView.updateRowModelData.bind(currentGamesView),
        "chat": function(data) { dispatcher.trigger("chat", data) },
        "auth" : authView.handleMessage.bind(authView),
        "seed" : popups.seedView.handleMessage.bind(popups.seedView),
        "fail" : function(data) { dispatcher.trigger("fail", data) },
    });

    // clean up application
    $(window).on("unload", function(){
        socket.close();
    });
    
    // responsive resizing
    var throttledResize = _.debounce(function(){
            consoleView.resize();
        }, 100);
    $(window).resize(throttledResize);

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

  function viewportHandler() {
    
    function translateDPadButton(dPadButtonElement, dPadLeftCentre, dPadTopCentre, buttonLeftOffset, buttonTopOffset) {
        
        let translateX = visualViewport.offsetLeft + dPadLeftCentre + buttonLeftOffset;
        let translateY = visualViewport.offsetTop + dPadTopCentre + buttonTopOffset;

        dPadButtonElement.style.transform = 'translate(' +  translateX + 'px,' + translateY + 'px) scale(' + 1 / visualViewport.scale + ')';
    };
    
    let visualViewport = window.visualViewport;

    let buttonRect = consoleUpDPadButton.getBoundingClientRect();

    let dPadWidth = buttonRect.width * 4
    let dPadHeight = buttonRect.height * 3;
    let dPadOffsetBotRatio = 0.2;
    let dPadOffsetLeftRatio = 0.1;

    let dPadButtonOffsetX = buttonRect.width;
    let dPadButtonOffsetY = buttonRect.height;

    let dPadBotOffset = (visualViewport.height - dPadHeight) * dPadOffsetBotRatio;
    let dPadTopOffset = visualViewport.height - dPadBotOffset - dPadHeight;

    let dPadLeftOffset = (visualViewport.height - dPadWidth) * dPadOffsetLeftRatio;

    let dPadLeftCentre = dPadLeftOffset + dPadWidth / 2;
    let dPadTopCentre = dPadTopOffset + dPadHeight / 2;

    translateDPadButton(consoleUpDPadButton, dPadLeftCentre, dPadTopCentre, 0, -dPadButtonOffsetY);
    translateDPadButton(consoleUpRightDPadButton, dPadLeftCentre, dPadTopCentre, dPadButtonOffsetX, -dPadButtonOffsetY);
    translateDPadButton(consoleRightDPadButton, dPadLeftCentre, dPadTopCentre, dPadButtonOffsetX, 0);
    translateDPadButton(consoleDownRightDPadButton, dPadLeftCentre, dPadTopCentre, dPadButtonOffsetX, dPadButtonOffsetY);
    translateDPadButton(consoleDownDPadButton, dPadLeftCentre, dPadTopCentre, 0, dPadButtonOffsetY);
    translateDPadButton(consoleDownLeftDPadButton, dPadLeftCentre, dPadTopCentre, -dPadButtonOffsetX, dPadButtonOffsetY);
    translateDPadButton(consoleLeftDPadButton, dPadLeftCentre, dPadTopCentre, -dPadButtonOffsetX, 0);
    translateDPadButton(consoleUpLeftDPadButton, dPadLeftCentre, dPadTopCentre, -dPadButtonOffsetX, -dPadButtonOffsetY);

    translateDPadButton(consoleCentreDPadButton, dPadLeftCentre, dPadTopCentre, 0, 0);

    translateDPadButton(consoleUpRightRightDPadButton, dPadLeftCentre, dPadTopCentre, 2 * dPadButtonOffsetX, -dPadButtonOffsetY);
    translateDPadButton(consoleRightRightDPadButton, dPadLeftCentre, dPadTopCentre, 2 * dPadButtonOffsetX, 0);
    translateDPadButton(consoleDownRightRightDPadButton, dPadLeftCentre, dPadTopCentre, 2 * dPadButtonOffsetX, dPadButtonOffsetY);

    //console.log("visualviewport offsetLeft " + visualViewport.offsetLeft + " offsetTop " + visualViewport.offsetTop + " height: " + visualViewport.height + " width: " + visualViewport.width + " scale: " + visualViewport.scale);
    //console.log("buttonUpLeftOffset: " + buttonUpLeftOffset + " buttonUpTopOffset:" + buttonUpTopOffset);
    //console.log("buttonUpLeftOffsetDelta: " + (buttonUpLeftOffset - visualViewport.offsetLeft) + " buttonUpTopOffsetDelta:" + (buttonUpTopOffset - visualViewport.offsetTop));

  }

  if(window.visualViewport) {
    window.visualViewport.addEventListener('scroll', viewportHandler);
    window.visualViewport.addEventListener('resize', viewportHandler);
    window.addEventListener('scroll', viewportHandler);
  } 

    activate.endLoading();
});
