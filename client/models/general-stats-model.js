define([
    'jquery',
    'underscore',
    'backbone',
    'moment'
], function($, _, Backbone, Moment) {

    var GeneralStatsModel = Backbone.Model.extend({
        url: '/api/stats/general',
        defaults: function() {
            return {
                totalGames: 0,
                totalEasyModeGames: 0,
                totalNormalModeVictories: 0,
                totalNormalModeSuperVictories: 0,
                totalLumenstones: 0,
                totalLevels: 0,
                lastVictoryDate: '',
                lastVictoryUser: '',
                lastStreakDate: '',
                lastStreakLength: '',
                lastStreakUser: ''
            }
        },
        parse: function (data) {

            var parsedData = _.pick(data, 'totalGames', 'totalEasyModeGames', 'totalNormalModeVictories', 'totalNormalModeSuperVictories', 'totalLumenstones', 'totalLevels');
            if(data.lastVictory.date !== 'Never') {
                parsedData.lastVictoryDate = Moment(data.lastVictory.date).format('MMMM Do YYYY, h:mm:ss a');
            }
            else {
                parsedData.lastVictoryDate = data.lastVictory.date;
            }

            parsedData.lastVictoryUser = data.lastVictory.username;

            if(data.lastStreak.date !== 'Never') {
                parsedData.lastStreakDate = Moment(data.lastStreak.date).format('MMMM Do YYYY, h:mm:ss a');
                parsedData.lastStreakUser = data.lastStreak.username;
                parsedData.lastStreakLength = data.lastStreak.length;
            }
            else {
                parsedData.lastStreakDate = data.lastStreak.date;
                parsedData.lastStreakUser = data.lastStreak.username;
                parsedData.lastStreakLength = "None";
            }

            return parsedData;
        },
        setUserGeneralStats: function(username) {
            this.useUser = true;
            this.userCode = username;
            this.setUrl();
        },
        setAllUserGeneralStats: function() {
            this.useUser = false;
            this.setUrl();
        },
        setVariantGeneralStats: function(variantCode) {
            this.useVariant = true;
            this.variantCode = variantCode;
            this.setUrl();
        },
        setAllVariantGeneralStats: function(variantCode) {
            this.useVariant = false;
            this.setUrl();
        },
        setUrl: function() {
            if(this.useUser && this.useVariant) {
                this.url = 'api/stats/general?variant=' + this.variantCode + '&username=' + this.userCode;
            }
            else if(this.useUser) {
                this.url = 'api/stats/general?username=' + this.userCode;
            }
            else if(this.useVariant) {
                this.url = 'api/stats/general?variant=' + this.variantCode;
            }
            else {
                this.url = 'api/stats/general';
            }
        }
    });

    return GeneralStatsModel;

});