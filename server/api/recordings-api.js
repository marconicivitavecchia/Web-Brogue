var mongoose = require('mongoose');
var GameRecord = require("../database/game-record-model");
var paginate = require("express-paginate");
var _ = require("underscore");
const fs = require('fs');

module.exports = function(app, config) {

    var filterGameRecords = function(gameRecords) {

        var filteredGameRecords = [];

        _.each(gameRecords, function(gameRecord) {

            var filteredRecord =
                _.pick(gameRecord,
                    'recording', 'variant');

            if('recording' in gameRecord && gameRecord.recording != undefined) {
                filteredRecord.recording = 'recording-' + gameRecord._id;
            }

            filteredGameRecords.push(filteredRecord);
        });

        return filteredGameRecords;
    };

    app.get("/api/recordings/:id", function (req, res) {

        var recordingId = req.params.id;

        GameRecord.findById(recordingId, function (err, gameRecord) {

            if (err) return next(err);

            if(!('recording' in gameRecord)) {
                res.status(404).send('Not found');
                return;
            }

            var file = gameRecord.recording;

            var filename = "webbrogue-recording-" + recordingId + ".broguerec";
            var mimetype = "application/octet-stream";

            res.setHeader('Content-disposition', 'attachment; filename=' + filename);
            res.setHeader('Content-type', mimetype);

            var filestream = fs.createReadStream(file);
            filestream.pipe(res);

            filestream.on('error', (err) => {
                console.log('Error reading recordings file: ' + recordingId);
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
            query['variant'] = req.query.variant;
        }

        GameRecord.paginate(
            query,
            {   page: req.query.page,
                limit: req.query.limit,
                sortBy: sortFromQueryParams(req, '-score')
        }, function (err, gameRecords, pageCount, itemCount) {

            if (err) return next(err);

            var gameRecordsFiltered = filterGameRecords(gameRecords);

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

        var query = {username: req.params.username};

        if(req.query.variant) {
            query['variant'] = req.query.variant;
        }

        if(req.query.variant) {
            query = { 'variant': req.query.variant };
        }

        if(req.query.previousdays) {
            var previousDays = new Date();
            var dateOffset = (24*60*60*1000) * req.query.previousdays;
            previousDays.setTime(new Date().getTime() - dateOffset);

            var startTime = previousDays;

            _.extend(query, {date: {
                $gte: startTime,
            }});
        }

        GameRecord.paginate(query, {
            page: req.query.page,
            limit: req.query.limit,
            sortBy: sortFromQueryParams(req, '-date')
        }, function (err, gameRecords, pageCount, itemCount) {

            if (err) return next(err);

            var gameRecordsFiltered = filterGameRecords(gameRecords);

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

        GameRecord.paginate({_id: req.params.id}, {
            page: req.query.page,
            limit: req.query.limit,
            sortBy: sortFromQueryParams(req, '-date')
        }, function (err, gameRecords, pageCount, itemCount) {

            if (err) return next(err);

            res.format({
                json: function () {
                    res.json({
                        object: 'list',
                        data: filterGameRecords(gameRecords),
                        pageCount: pageCount,
                        itemCount: itemCount
                    });
                }
            });
        });
    });
};