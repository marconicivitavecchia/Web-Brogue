var _ = require("underscore");
var GameRecord = require("../database/game-record-model");
var stats = require('../stats/stats.js');
var brogueConstants = require('../brogue/brogue-constants.js');

module.exports = {

    generateCalculateGeneralStatsQuery: function(variant, user) {
        if(user) {
            return { variant: variant, user: user};
        }
        else 
            return { variant: variant };
    },

    calculateGeneralStats: function(res, variant, user) {
    
        if (!variant) {
            res.json({});
            return;
        }
        
        const query = this.generateCalculateGeneralStatsQuery(variant, user);
        
        GameRecord.find(query).lean().exec(function (err, games) {

            var filteredGames = stats.filterForValidGames(games, variant);

            var allEasyModeGames = _.where(filteredGames, {easyMode: true});
            var allNormalModeGames = _.filter(filteredGames, function(game) { return game.easyMode != true; });

            var allEasyModeVictories = _.where(allEasyModeGames, {result: brogueConstants.notifyEvents.GAMEOVER_VICTORY});
            var allEasyModeQuits = _.where(allEasyModeGames, {result: brogueConstants.notifyEvents.GAMEOVER_QUIT});
            var allEasyModeDeaths = _.where(allEasyModeGames, {result: brogueConstants.notifyEvents.GAMEOVER_DEATH});
            var allEasyModeSuperVictories = _.where(allEasyModeGames, {result: brogueConstants.notifyEvents.GAMEOVER_SUPERVICTORY});

            var allNormalModeVictories = _.where(allNormalModeGames, {result: brogueConstants.notifyEvents.GAMEOVER_VICTORY});
            var allNormalModeQuits = _.where(allNormalModeGames, {result: brogueConstants.notifyEvents.GAMEOVER_QUIT});
            var allNormalModeDeaths = _.where(allNormalModeGames, {result: brogueConstants.notifyEvents.GAMEOVER_DEATH});
            var allNormalModeSuperVictories = _.where(allNormalModeGames, {result: brogueConstants.notifyEvents.GAMEOVER_SUPERVICTORY});

            var totalLumenstonesPerGame = _.map(allNormalModeGames, function (game) {

                var lumenRe = new RegExp("with\\s+(\\d+)\\s+lumenstones");
                var descriptionMatch = lumenRe.exec(game.description);
                if (descriptionMatch) {
                    return parseInt(descriptionMatch[1]) || 0;
                }
                else {
                    return 0;
                }

                return game;
            });

            var totalLumenstones = _.reduce(totalLumenstonesPerGame, function(memo, num){ return memo + num; }, 0);

            var totalLevelsPerGame = _.map(filteredGames, function (game) { return parseInt(game.level) || 0 });
            var totalLevels = _.reduce(totalLevelsPerGame, function(memo, num){ return memo + num; }, 0);

            var allVictories = allNormalModeVictories.concat(allNormalModeSuperVictories);
            var allVictoriesSortedByDate = _.sortBy(allVictories, 'date');
            
            var lastVictory = _.last(allVictoriesSortedByDate);
            var lastVictoryData;
            if(!lastVictory) {
                lastVictoryData = { date: "Never", username: "No-one" };
            }
            else {
                lastVictoryData = { date: lastVictory.date, username: lastVictory.username };
            }

            var normalModeGamesByUser = _.groupBy(allNormalModeGames, 'username');
            var victoryStreaksByUser = _.map(normalModeGamesByUser, function (games, username) {
                var longestStreakLastVictory;
                var streakCounter = 0;
                var longestStreakCounter = 0;

                var usersGames = _.sortBy(games, 'date');
                _.each(usersGames, function(v, index) {
                    if(v.result === brogueConstants.notifyEvents.GAMEOVER_SUPERVICTORY || v.result === brogueConstants.notifyEvents.GAMEOVER_VICTORY) {
                        streakCounter++;
                        if(streakCounter >= longestStreakCounter) {
                            longestStreakCounter = streakCounter;
                            longestStreakLastVictory = v;
                        }
                    }
                    else {
                        streakCounter = 0;
                    }
                });
                
                return { username: username, longestStreak: longestStreakCounter, lastVictory: longestStreakLastVictory }
            });

            var lastStreakData;

            //Sort by longestStreak then date
            var streakData = _.filter(victoryStreaksByUser, function(v) { return v.longestStreak > 0 });
            if(streakData.length > 0) {
                var longestStreaks = _.sortBy( _.sortBy(streakData, function (v) { v.lastVictory.date } ), 'longestStreak');
                var longestStreak = _.last(longestStreaks);

                lastStreakData = { date: longestStreak.lastVictory.date, username: longestStreak.lastVictory.username, length: longestStreak.longestStreak };
            }
            else {
                lastStreakData = { date: "Never", username: "No-one", length: 0 };
            }
            
            var statsSummary = {};

            statsSummary.totalGames = filteredGames.length;

            statsSummary.totalEasyModeGames = allEasyModeGames.length;
            statsSummary.totalNormalModeGames = allNormalModeGames.length;

            statsSummary.totalEasyModeVictories = allEasyModeVictories.length;
            statsSummary.totalEasyModeQuits = allEasyModeQuits.length;
            statsSummary.totalEasyModeDeaths = allEasyModeDeaths.length;
            statsSummary.totalEasyModeSuperVictories = allEasyModeSuperVictories.length;

            statsSummary.totalNormalModeVictories = allNormalModeVictories.length;
            statsSummary.totalNormalModeQuits = allNormalModeQuits.length;
            statsSummary.totalNormalModeDeaths = allNormalModeDeaths.length;
            statsSummary.totalNormalModeSuperVictories = allNormalModeSuperVictories.length;

            statsSummary.totalLumenstones = totalLumenstones;
            statsSummary.totalLevels = totalLevels;

            statsSummary.lastVictory = lastVictoryData;
            statsSummary.lastStreak = lastStreakData;

            res.json(statsSummary);
        });
    }
};
