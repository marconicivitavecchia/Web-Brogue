// Full high scores view

define([
    "jquery",
    "underscore",
    "backbone",
    "dispatcher",
    "config",
    "views/score-table-cells",
    "variantLookup"
], function ($, _, Backbone, dispatcher, config, TableCells, variantLookup) {

    var UserStatsScoresView = Backbone.View.extend({
        el: '#user-stats-scores',
        headingTemplate: _.template($('#user-stats-scores-heading').html()),

        events: {
            "click #user-stats-scores-options-list" : "selectAllScoresForUserOptions"
        },

        initialize: function() {
            this.listenTo(this.model, "add", this.render);
            this.listenTo(this.model, "change", this.render);

            this.grid = new Backgrid.Grid({
                columns: [
                    {
                        name: "username",
                        label: "User name",
                        cell: "string",
                        sortable: false,
                        editable: false
                    }, {
                        name: "prettyDate",
                        label: "Date",
                        cell: "string",
                        sortable: true,
                        editable: false
                    }, {
                        name: "prettyVariant",
                        label: "Version",
                        cell: "string",
                        sortable: true,
                        editable: false
                    }, {
                        name: "score",
                        label: "Score",
                        cell: "integer",
                        sortable: true,
                        editable: false
                    }, {
                        name: "level",
                        label: "Level",
                        cell: TableCells.levelCell,
                        sortable: true,
                        editable: false
                    }, {
                        name: "seed",
                        label: "Seed",
                        cell: "string",
                        sortable: true,
                        editable: false
                    }, {
                        name: "description",
                        label: "Message",
                        cell: "string",
                        sortable: false,
                        editable: false
                    }, {
                        name: "recording",
                        label: "Recording",
                        cell: TableCells.watchGameUriCell,
                        sortable: false,
                        editable: false
                    }],

                collection: this.model
            });

            this.paginator = new Backgrid.Extension.Paginator({
                collection: this.model
            });
         
            this.setDefaultVariantScores();
        },

        renderOptions: function() {

            var variantData = _.values(variantLookup.variants);
            var scoresTypeSelected = this.model.getScoresTypeSelected();

            this.$el.html(this.headingTemplate(
                {   username: this.model.username,
                    variants: variantData,
                    scoresTypeSelected: scoresTypeSelected }));
        },

        render: function() {

            this.renderOptions();

            $("#user-stats-scores-grid").append(this.grid.render().$el);
            $("#user-stats-scores-paginator").append(this.paginator.render().$el);

            return this;
        },

        refresh: function() {
            this.model.fetch();
        },

        //Event handler
        userSelected: function(userName) {
            this.model.setUserName(userName);
            this.setDefaultVariantScores();
        },

        setDefaultVariantScores: function() {
            var defaultVariantCode = _.findWhere(_.values(variantLookup.variants), {default: true}).code;
            this.model.setVariantTopScores(defaultVariantCode, true);
            this.model.fetch();
            this.render();
        },

        selectAllScoresForUserOptions: function(event) {
            
            event.preventDefault();

            if(!event.target.id) {
                return;
            }

            var codeAfterHyphenIndex = event.target.id.lastIndexOf("-")
            
            if(codeAfterHyphenIndex == -1) {
                return;
            }

            var code = event.target.id.substring(codeAfterHyphenIndex + 1);

            if(code in variantLookup.variants) {
                this.model.setVariantTopScores(code, true);
                this.model.fetch();
                this.render();
            }
        }
    });

    return UserStatsScoresView;

});
