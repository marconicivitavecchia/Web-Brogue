define([
    'jquery',
    'underscore',
    'backbone',
    'moment',
    'variantLookup'
], function($, _, Backbone, Moment, VariantLookup) {

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
                lastStreakUser: '',
                lastMasteryStreakDate: '',
                lastMasteryStreakLength: '',
                lastMasteryStreakUser: '',
                variantName: ''
            }
        },
        parse: function (data) {

            var parsedData = _.pick(data, 'totalGames', 'totalEasyModeGames', 'totalNormalModeVictories', 'totalNormalModeSuperVictories', 'totalLumenstones', 'totalLevels');
            if(data.lastVictory.date !== 'Never') {
                var date = Moment(data.lastVictory.date);
                parsedData.lastVictoryDate = (date.format('YYYY') == Moment().format('YYYY') ? 
                    date.format('MMM DD, h a') :
                    date.format('MMM DD, YYYY, h a'));
            }
            else {
                parsedData.lastVictoryDate = data.lastVictory.date;
            }

            parsedData.lastVictoryUser = data.lastVictory.username;

            if(data.lastStreak.date !== 'Never') {
                var date = Moment(data.lastStreak.date);
                parsedData.lastStreakDate = (date.format('YYYY') == Moment().format('YYYY') ? 
                    date.format('MMM DD, h a') :
                    date.format('MMM DD, YYYY, h a'));
                parsedData.lastStreakUser = data.lastStreak.username;
                parsedData.lastStreakLength = data.lastStreak.length;
            }
            else {
                parsedData.lastStreakDate = data.lastStreak.date;
                parsedData.lastStreakUser = data.lastStreak.username;
                parsedData.lastStreakLength = "None";
            }

            if(data.lastMasteryStreak.date !== 'Never') {
                var date = Moment(data.lastMasteryStreak.date);
                parsedData.lastMasteryStreakDate = (date.format('YYYY') == Moment().format('YYYY') ? 
                    date.format('MMM DD, h a') :
                    date.format('MMM DD, YYYY, h a'));
                parsedData.lastMasteryStreakUser = data.lastMasteryStreak.username;
                parsedData.lastMasteryStreakLength = data.lastMasteryStreak.length;
            }
            else {
                parsedData.lastMasteryStreakDate = data.lastMasteryStreak.date;
                parsedData.lastMasteryStreakUser = data.lastMasteryStreak.username;
                parsedData.lastMasteryStreakLength = "None";
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
            this.set("variantName", VariantLookup.variants[this.variantCode].display);
            this.setUrl();
        },
        setAllVariantGeneralStats: function(variantCode) {
            this.useVariant = false;
            this.set("variantName", "All Variants");
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