// Model for a list of high scores

define([
    'jquery',
    'underscore',
    'backbone'
], function($, _, Backbone) {

    var HighScores = Backbone.Collection.extend({
        url: '/api/games',
        parse: function(data) {
            return data.data;
        },
        setAllScores: function() {
            this.url = 'api/games';
        },
        setUserScores: function() {
            this.url = 'api/games/' + this.username;
        },
        setDailyScores: function() {
            this.url = 'api/dailygames';
        },
        setMonthlyScores: function() {
            this.url = 'api/monthlygames';
        },
        setUserName: function(username) {
            this.username = username;
        },
        clearUserName: function() {
            delete this.username;
        }
    });

    return HighScores;

});