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
        chart: "libs/chart",
        rot: "libs/rot"
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
    "views/canvas-console-view",
    "views/popups/seed-popup-view",
    "views/statistics-view",
    "views/level-stats-view",
    "views/general-stats-view",
    "views/cause-stats-view",
    "views/level-probability-view",
    "views/dpad-button-view",
    "views/dpad-visibility-button-view"
], function( $, _, Backbone, BackbonePaginator, Backgrid, BackgridPaginator, dispatcher, debugMode, socket, router,
     HighScoresModel, ChatModel, SiteNewsModel, CauseStatsModel, LevelStatsModel, GeneralStatsModel, LevelProbabilityModel, DPadButtonModel,
     activate, AuthView, ChatView, ConsoleChatView, PlayView, HeaderView, CurrentGamesView, HighScoresView, AllScoresView, SiteNewsView,
     ConsoleView, CanvasConsoleView, SeedPopupView, StatisticsView, LevelStatsView, GeneralStatsView, CauseStatsView, LevelProbabilityView,
     DPadButtonView, DPadButtonVisibilityView){
    
    // If you want to enable debug mode, uncomment this function
    debugMode();
    
    // initialize each view
    var authView = new AuthView();
    var playView = new PlayView();
    var headerView = new HeaderView();
    var currentGamesView = new CurrentGamesView();
    var chatView = new ChatView({model: new ChatModel()});
    var statisticsView = new StatisticsView();
    var levelStatsView = new LevelStatsView({model: new LevelStatsModel()});
    var causeStatsView = new CauseStatsView({model: new CauseStatsModel()});
    var generalStatsView = new GeneralStatsView({model: new GeneralStatsModel()});
    var levelProbabilityView = new LevelProbabilityView({model: new LevelProbabilityModel()});
    var siteNewsView = new SiteNewsView({model: new SiteNewsModel() });
    var popups = {
        seedView : new SeedPopupView(),
    };

    //Console
    var consoleView = new ConsoleView();
    var consoleChatView = new ConsoleChatView({el: "#console-chat", model: new ChatModel()});

    //Canvas console
    var consoleCanvasView = new CanvasConsoleView();
    var consoleCanvasChatView = new ConsoleChatView({el: "#canvas-console-chat", model: new ChatModel()});

    //DPad - console
    var dPadVisibilityButton = new DPadButtonVisibilityView({el: "#console-dpad"});
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
    var downRightRightZView = new DPadButtonView({el: "#console-down-right-right", model: new DPadButtonModel({ keyToSend: "Z".charCodeAt(0) })});

    //DPad - canvas console
    var dPadVisibilityButton = new DPadButtonVisibilityView({el: "#canvas-console-dpad"});
    new DPadButtonView({el: "#canvas-console-up", model: new DPadButtonModel({ keyToSend: 63232 })});
    new DPadButtonView({el: "#canvas-console-up-right", model: new DPadButtonModel({ keyToSend: 117 })});
    new DPadButtonView({el: "#canvas-console-right", model: new DPadButtonModel({ keyToSend: 63235 })});
    new DPadButtonView({el: "#canvas-console-down-right", model: new DPadButtonModel({ keyToSend: 110 })});
    new DPadButtonView({el: "#canvas-console-down", model: new DPadButtonModel({ keyToSend: 63233 })});
    new DPadButtonView({el: "#canvas-console-down-left", model: new DPadButtonModel({ keyToSend: 98 })});
    new DPadButtonView({el: "#canvas-console-left", model: new DPadButtonModel({ keyToSend: 63234 })});
    new DPadButtonView({el: "#canvas-console-up-left", model: new DPadButtonModel({ keyToSend: 121 })});
    new DPadButtonView({el: "#canvas-console-centre", model: new DPadButtonModel({ keyToSend: 53 })});
    new DPadButtonView({el: "#canvas-console-up-right-right", model: new DPadButtonModel({ keyToSend: "i".charCodeAt(0) })});
    new DPadButtonView({el: "#canvas-console-right-right", model: new DPadButtonModel({ keyToSend: "x".charCodeAt(0) })});
    new DPadButtonView({el: "#canvas-console-down-right-right", model: new DPadButtonModel({ keyToSend: "Z".charCodeAt(0) })});

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
    dispatcher.on("quit", consoleCanvasView.exitToLobby, consoleCanvasView);
    dispatcher.on("quit", consoleView.exitToLobby, consoleView);


    dispatcher.on("fail", highScoresView.quit, highScoresView);
    dispatcher.on("fail", consoleCanvasView.exitToLobby, consoleCanvasView);
    dispatcher.on("fail", consoleView.exitToLobby, consoleView);

    dispatcher.on("login", headerView.login, headerView);
    dispatcher.on("login", highScoresView.login, highScoresView);
    dispatcher.on("login", allScoresView.login, allScoresView);
    dispatcher.on("login", chatView.login, chatView);
    dispatcher.on("login", consoleChatView.login, consoleChatView);
    dispatcher.on("login", consoleCanvasChatView.login, consoleCanvasChatView);

    dispatcher.on("login", currentGamesView.login, currentGamesView);

    dispatcher.on("anon-login", headerView.anonymousLogin, headerView);
    dispatcher.on("anon-login", chatView.login, chatView);
    dispatcher.on("anon-login", consoleChatView.login, consoleChatView);
    dispatcher.on("anon-login", consoleCanvasChatView.login, consoleCanvasChatView);


    dispatcher.on("logout", highScoresView.logout, highScoresView);
    dispatcher.on("logout", allScoresView.logout, allScoresView);
    dispatcher.on("logout", consoleChatView.logout, consoleChatView);
    dispatcher.on("logout", consoleCanvasChatView.logout, consoleCanvasChatView);

    dispatcher.on("logout", chatView.logout, chatView);
    dispatcher.on("logout", currentGamesView.logout, currentGamesView);
    dispatcher.on("logout", authView.logout, authView);

    dispatcher.on("all-scores", allScoresView.activate, allScoresView);

    dispatcher.on("chat", chatView.chatMessage, chatView);
    dispatcher.on("chat", consoleChatView.chatMessage, consoleChatView);
    dispatcher.on("chat", consoleCanvasChatView.chatMessage, consoleCanvasChatView);


    dispatcher.on("showConsole", consoleCanvasView.resize, consoleCanvasView);
    dispatcher.on("showConsole", consoleView.resize, consoleView);

    dispatcher.on("showChat", consoleCanvasView.resize, consoleCanvasView);
    dispatcher.on("showChat", consoleView.resize, consoleView);

    dispatcher.on("hideChat", consoleCanvasView.resize, consoleCanvasView);
    dispatcher.on("hideChat", consoleView.resize, consoleView);

    dispatcher.on("startGame", headerView.startGame, headerView);
    dispatcher.on("startGame", consoleCanvasView.initialiseForNewGame, consoleCanvasView);
    dispatcher.on("startGame", consoleView.initialiseForNewGame, consoleView);

    dispatcher.on("observeGame", headerView.observeGame, headerView);
    dispatcher.on("observeGame", consoleCanvasView.initialiseForNewGame, consoleCanvasView);
    dispatcher.on("observeGame", consoleView.initialiseForNewGame, consoleView);

    dispatcher.on("recordingGame", headerView.recordingGame, headerView);
    dispatcher.on("recordingGame", consoleCanvasView.initialiseForNewGame, consoleCanvasView);
    dispatcher.on("recordingGame", consoleView.initialiseForNewGame, consoleView);

    dispatcher.on("leaveGame", headerView.leaveGame, headerView);

    dispatcher.on("reconnect", authView.requestLogin, authView);
    dispatcher.on("reconnect", consoleCanvasView.exitToLobby, consoleCanvasView);
    dispatcher.on("reconnect", consoleView.exitToLobby, consoleView);

    dispatcher.on("focusConsole", consoleCanvasView.giveKeyboardFocus, consoleCanvasView);
    dispatcher.on("focusConsole", consoleView.giveKeyboardFocus, consoleView);

    dispatcher.on("showSeedPopup", popups.seedView.showSeedPopup, popups.seedView);

    dispatcher.on("brogue", consoleCanvasView.queueUpdateCellModelData, consoleCanvasView);
    dispatcher.on("brogue", consoleView.queueUpdateCellModelData, consoleView);
    
    // set up routes for the messages from the websocket connection (only)
    router.registerHandlers({
        //Must bind 'this' to the scope of the view so we can use the internal view functions
        "error" : console.error.bind(console),
        "brogue" : function(data) { dispatcher.trigger("brogue", data) },
        "quit" : function(data) { dispatcher.trigger("quit", data) },
        "lobby" : currentGamesView.updateRowModelData.bind(currentGamesView),
        "chat": function(data) { dispatcher.trigger("chat", data) },
        "auth" : authView.handleMessage.bind(authView),
        "seed" : popups.seedView.handleMessage.bind(popups.seedView),
        "fail" : function(data) { dispatcher.trigger("fail", data) },
    });
    
    //debugging
    setInterval(socket.outputPerformanceTracking, 5000);

    // clean up application
    $(window).on("unload", function(){
        socket.close();
    });
    
    // responsive resizing
    var throttledResize = _.debounce(function(){
            consoleCanvasView.resize();
        }, 100);
    $(window).resize(throttledResize);

    // dpad translation and scaling
    if(window.visualViewport) {
        window.visualViewport.addEventListener('scroll', dPadVisibilityButton.positionDPad);
        window.visualViewport.addEventListener('resize', dPadVisibilityButton.positionDPad);
        window.addEventListener('scroll', dPadVisibilityButton.positionDPad);
    } 

    activate.endLoading();
});
