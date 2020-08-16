var mongoose = require('mongoose');
var GameRecord = require("../database/game-record-model");
var paginate = require("express-paginate");
var _ = require("underscore");
const fs = require('fs');
var stats = require('../stats/stats.js');

module.exports = function(app, config) {

    app.get("/api/recordings/:id", function (req, res, next) {

        var recordingId = req.params.id;

        GameRecord.findById(recordingId, function (err, gameRecord) {

            if (err) return next(err);

            if(!gameRecord || 
                gameRecord && !('recording' in gameRecord)) {
                res.status(404).send('Not found');
                return;
            }

            //Blacklist old versions that don't have desktop versions
            
            if(!stats.doesVariantSupportRecordingDownloads(config, gameRecord.variant)) {
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
        });
    });
};