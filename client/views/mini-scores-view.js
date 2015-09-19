// Embedded high scores view

define([
    "jquery",
    "underscore",
    "backbone",
    "views/high-scores-item-view.js"
], function ($, _, Backbone, HighScoreItemView) {

    var HighScoresView = Backbone.View.extend({
        el: '#mini-scores',
        headingTemplate: _.template($('#mini-scores-heading').html()),

        events: {
            "click #mini-scores-user" : "selectUserScores",
            "click #mini-scores-all" : "selectAllScores"
        },

        initialize: function() {
            this.listenTo(this.model, "add", this.render);
            this.listenTo(this.model, "change", this.render);
        },

        render: function() {

            this.$el.html(this.headingTemplate({ username: this.model.username }));
            var table = $('#mini-scores-table');
            $('mini-scores-table-heading').siblings().empty();

            this.model.each(function(score) {
                var highScoreView = new HighScoreItemView({ model: score });
                var $tr = highScoreView.render().$el;

                table.append($tr);
            }, this);

            return this;
        },

        refresh: function() {
            this.model.fetch();
        },

        login: function(userName) {
            this.model.setUserName(userName);

            this.render();
        },

        logout: function() {
            this.model.clearUserName();

            this.render();
        },

        quit: function() {
            this.refresh();
        },

        selectUserScores: function(event) {

            event.preventDefault();
            console.log('selectUserScores');

            this.model.setUserScores();
            this.refresh();
        },

        selectAllScores: function(event) {

            event.preventDefault();
            console.log('selectAllScores');

            this.model.setAllScores();
            this.refresh();
        }
    });

    return HighScoresView;

});
