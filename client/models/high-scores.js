// Model for a list of high scores

define([
    'jquery',
    'underscore',
    'backbone',
    'backbonePaginator',
    'moment',
    'variantLookup'
], function($, _, Backbone, BackbonePaginator, Moment, VariantLookup) {

    var HighScores = Backbone.PageableCollection.extend({
        url: '/api/games',

        // Initial pagination states
        state: {
            pageSize: 10,
            sortKey: "date",
            order: 1
        },

        // You can remap the query parameters from `state` keys from
        // the default to those your server supports
        queryParams: {
            sortKey: "sort",
            order: "order",
            pageSize: "limit"
        },

        formatDate: function(date) {
            return Moment(date).format('MMMM Do YYYY, h:mm:ss a');
        },

        lookupVariant: function(variant) {
            if(variant in VariantLookup.variants) {
                return VariantLookup.variants[variant].display;
            }
            else {
                return "Not found";
            }
        },

        parseState: function (resp, queryParams, state, options) {
           return {totalRecords: resp.itemCount };
        },

        // get the actual records
        parseRecords: function (resp, options) {

            var records = resp.data;

            _.each(records, function(element, index, list) {
                element.prettyDate = this.formatDate(element.date);
                element.prettyVariant = this.lookupVariant(element.variant);
            }, this);

            return resp.data;
        },

        setAllScores: function() {
            this.url = 'api/games';
            this.state.sortKey = "date";
            this.scoresTypeSelected = "All scores";
        },
        setAllScoresForPreviousDays: function(days) {
            this.url = 'api/games?previousdays=' + days;
            this.state.sortKey = "date";
            this.scoresTypeSelected = "Scores for " + days + "previous days.";
        },
        setUserScoresForPreviousDays: function(days) {
            this.url = 'api/games/' + this.username + '?previousdays=' + days;
            this.state.sortKey = "date";
            this.scoresTypeSelected = "User " + this.username + " scores for " + days + "previous days.";
        },
        setAllTopScores: function() {
            if(this.username) {
                this.url = 'api/games?username=' + this.username;
                this.scoresTypeSelected = "User " + this.username + " all scores";
            }
            else {
                this.url = 'api/games';
                this.scoresTypeSelected = "All scores";
            }
            this.state.sortKey = "score";
        },
        setUserTopScores: function() {
            this.url = 'api/games/' + this.username;
            this.state.sortKey = "score";
            this.scoresTypeSelected = "User " + this.username + " all scores";
        },
        setDailyTopScores: function() {
            this.url = 'api/dailygames';
            this.state.sortKey = "score";
            this.scoresTypeSelected = "Daily scores";
        },
        setMonthlyTopScores: function() {
            this.url = 'api/monthlygames';
            this.state.sortKey = "score";
            this.scoresTypeSelected = "Monthly scores";
        },
        setVariantTopScores: function(variantCode) {
            if(this.username) {
                this.url = 'api/games?variant=' + variantCode + '&username=' + this.username;
                this.scoresTypeSelected = "User " + this.username + " scores for " + this.lookupVariant(variantCode);
            }
            else {
                this.url = 'api/games?variant=' + variantCode;
                this.scoresTypeSelected = "Scores for " + this.lookupVariant(variantCode);
            }
            this.state.sortKey = "score";
        },
        setUserName: function(username) {
            this.username = username;
        },
        clearUserName: function() {
            delete this.username;
        },
        getScoresTypeSelected: function() {
            return this.scoresTypeSelected;
        }
    });

    return HighScores;

});