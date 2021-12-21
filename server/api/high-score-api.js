var mongoose = require('mongoose');
var GameRecord = require("../database/game-record-model");
var paginate = require("express-paginate");
var sanitize = require('mongo-sanitize');
var _ = require("underscore");
var stats = require('../stats/stats.js');

module.exports = function(app, config) {

    var sortFromQueryParams = function(req, defaultSort) {
        
        if (req.query.sort) {
            if (req.query.order && req.query.order === "desc") {
                return { [sanitize(req.query.sort)] : -1 }
            }
            else {
                return { [sanitize(req.query.sort)] : 1 };
            }
        }
        else {
            return defaultSort;
        }
    };

    var filterGameRecords = function(gameRecords) {

        var filteredGameRecords = [];

        _.each(gameRecords, function(gameRecord) {

            var filteredRecord =
                _.pick(gameRecord,
                    '_id', 'username', 'score', 'seed', 'level', 'result', 'easyMode', 'description', 'date', 'variant');

            if('recording' in gameRecord && gameRecord.recording != undefined) {
                filteredRecord.recording = 'recording-' + gameRecord._id;
                filteredRecord.link = 'viewRecording/' + gameRecord.variant + "-" + gameRecord._id;
            }

            if('recording' in gameRecord && stats.doesVariantSupportRecordingDownloads(config, gameRecord.variant)) {
                filteredRecord.download = 'recordings/' + gameRecord._id;
            }

            filteredGameRecords.push(filteredRecord);
        });

        return filteredGameRecords;
    };

    app.use(paginate.middleware(10, 50));

    app.get("/api/games", function (req, res) {

        var generateHighScoreStatsQuery = function(variant, username) {
            if (username && variant) {
                return { variant: variant, username: username};
            }
            else if (username) {
                return { username: username };
            }
            else if (variant) {
                return { variant: variant };
            }
            else 
                return { };
        }

        var query = generateHighScoreStatsQuery(sanitize(req.query.variant), sanitize(req.query.username));

        if(req.query.previousdays) {
            var previousDays = new Date();
            var dateOffset = (24*60*60*1000) * sanitize(req.query.previousdays);
            previousDays.setTime(new Date().getTime() - dateOffset);

            var startTime = previousDays;

            _.extend(query, {date: {
                $gte: startTime,
            }});
        }

        GameRecord.paginate(query, {
            page: sanitize(req.query.page),
            limit: sanitize(req.query.limit),
            sort: sortFromQueryParams(req, { date: -1 })
        }, function (err, gameRecords, pageCount, itemCount) {

            if (err) return next(err);
            
            res.format({
                json: function () {

                    var gameRecordsFiltered = filterGameRecords(gameRecords.docs);

                    res.json({
                        object: 'list',
                        data: gameRecordsFiltered,
                        pageCount: pageCount,
                        itemCount: itemCount
                    });
                }
            });
        });
    });

    app.get("/api/dailygames", function (req, res) {

        var now = new Date();
        var startTime = now.setUTCHours(0,0,0,0);
        var endTime = now.setUTCHours(24,0,0,0);

        var query = {
            date: {
                $gte: startTime,
                $lt: endTime
            },
            easyMode: false,
            score: {
                $gt: 0
            }
        };

        if(req.query.variant) {
            query['variant'] = sanitize(req.query.variant);
        }

        GameRecord.paginate(
            query,
            {   page: sanitize(req.query.page),
                limit: sanitize(req.query.limit),
                sort: sortFromQueryParams(req, { score: -1 })
        }, function (err, gameRecords, pageCount, itemCount) {

            if (err) return next(err);

            var gameRecordsFiltered = filterGameRecords(gameRecords.docs);

            res.format({
                json: function () {
                    res.json({
                        object: 'list',
                        data: gameRecordsFiltered,
                        pageCount: pageCount,
                        itemCount: itemCount
                    });
                }
            });
        });
    });

    app.get("/api/games/:username", function (req, res) {

        var query = {
            username: sanitize(req.params.username)
        };

        if(req.query.variant) {
            query['variant'] = sanitize(req.query.variant);
        }

        if(req.query.previousdays) {
            var previousDays = new Date();
            var dateOffset = (24*60*60*1000) * sanitize(req.query.previousdays);
            previousDays.setTime(new Date().getTime() - dateOffset);

            var startTime = previousDays;

            _.extend(query, {date: {
                $gte: startTime,
            }});
        }

        GameRecord.paginate(query, {
            page: sanitize(req.query.page),
            limit: sanitize(req.query.limit),
            sort: sortFromQueryParams(req, { date: -1 })
        }, function (err, gameRecords, pageCount, itemCount) {

            if (err) return next(err);

            var gameRecordsFiltered = filterGameRecords(gameRecords.docs);

            res.format({
                json: function () {
                    res.json({
                        object: 'list',
                        data: gameRecordsFiltered,
                        pageCount: pageCount,
                        itemCount: itemCount
                    });
                }
            });
        });
    });

    app.get("/api/games/id/:id", function (req, res) {

        GameRecord.paginate({_id: sanitize(req.params.id)}, {
            page: sanitize(req.query.page),
            limit: sanitize(req.query.limit),
            sort: sortFromQueryParams(req, { date: -1 })
        }, function (err, gameRecords, pageCount, itemCount) {

            if (err) return next(err);

            res.format({
                json: function () {
                    res.json({
                        object: 'list',
                        data: filterGameRecords(gameRecords.docs),
                        pageCount: pageCount,
                        itemCount: itemCount
                    });
                }
            });
        });
    });
};