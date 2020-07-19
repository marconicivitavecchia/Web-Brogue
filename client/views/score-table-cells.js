define([
    "jquery",
    "underscore",
    "backbone",
    "config",
    "dispatcher",
    "dataIO/send-generic",
    "backgrid",
    "moment",
    "views/view-activation-helpers"
], function ($, _, Backbone, config, dispatcher, send, Backgrid, Moment) {

    var LevelFormatter = _.extend({}, Backgrid.CellFormatter.prototype, {
        fromRaw: function (rawValue, model) {

            var easyModeQualifier = "";

            if(model.get("easyMode")) {
                easyModeQualifier = "&";
            }

            if(rawValue == 0) {
                if(model.get("easyMode")) {
                    return "&Win!";
                }
                else {
                    return "Win!";
                }
            }
            else if(rawValue) {
                return easyModeQualifier + "L" + rawValue.toString();
            }
        }
    });

    var LevelCell = Backgrid.StringCell.extend({
        formatter: LevelFormatter
    });

    var WatchGameUriCell = Backgrid.UriCell.extend({

        events : {
            "click #watch-game" : "watchGame",
            "click #link-game": "copyLink"
        },

        watchGame: function(event) {
            event.preventDefault();

            var gameId = $(event.target).data("gameid");
            var gameDescription = $(event.target).data("gamedescription");
            var gameVariant = $(event.target).data("variant");

            dispatcher.trigger("recordingGame", {id: gameId, recording: gameDescription, variant: gameVariant});
            dispatcher.trigger("showConsole");
        },

        copyLink: function(event) {
            event.preventDefault();
            var gameLink = window.location.host + "/" + $(event.target).attr("href");
            var $temp = $("<input>");
            $("body").append($temp);
            $temp.val(gameLink).select();
            document.execCommand("copy");
            $temp.remove();
        },

        render: function () {

            var formatDate = function(date) {
                return Moment(date).format('MMMM Do YYYY, h:mm:ss a');
            };

            this.$el.empty();
            var rawNameValue = this.model.get(this.column.get("name"));
            var formattedNameValue = this.formatter.fromRaw(rawNameValue, this.model);
            if(formattedNameValue) {
                this.$el.append($("<a>", {
                    href: '#brogue',
                    title: this.model.title,
                    id: 'watch-game',
                    "data-gameid": formattedNameValue,
                    "data-variant": this.model.get("variant"),
                    "data-gamedescription": this.model.get("username") + "-" + this.model.get("seed") + "-" + formatDate(this.model.get("date"))
                }).text("Watch"));
            }

            var downloadValue = this.model.get("download");
            if(downloadValue) {
                this.$el.append(" ");
                this.$el.append($("<a>", {
                    href: 'api/' + downloadValue,
                    title: this.model.title,
                    id: 'download-game',
                }).text("Download"));
            }

            var downloadValue = this.model.get("link");
            if(downloadValue) {
                this.$el.append(" ");
                this.$el.append($("<a>", {
                    href: '#' + downloadValue,
                    title: this.model.title,
                    id: 'link-game',
                }).text("Copy Link"));
            }

            this.delegateEvents();
            return this;
        },

    });

    var tableCells = {
        levelCell: LevelCell,
        watchGameUriCell: WatchGameUriCell
    };

    return tableCells;
});
