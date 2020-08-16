var request = require('supertest');
var mongoose = require("mongoose");
var gameRecord = require("../database/game-record-model");
var brogueConstants = require("../brogue/brogue-constants.js");
var expect = require("chai").expect;
var assert = require("chai").assert;
var server = require("./server-test");
var config = require("./config-test");

var db = mongoose.createConnection(config.db.url);

describe("api/recordings", function(){

    beforeEach(function(done) {

        var gameRecord1 = {
            username: "flend",
            date: new Date("2012-05-26T07:56:00.123Z"),
            score: 100,
            seed: 200,
            level: 3,
            result: brogueConstants.notifyEvents.GAMEOVER_DEATH,
            easyMode: false,
            description: "Killed by a pink jelly on depth 3.",
            recording: "test/testfile.broguerec",
            variant: "BROGUE"
        };

        var gameRecord2 = {
            username: "flend",
            date: new Date("2012-05-27T07:56:00.123Z"),
            score: 100,
            seed: 200,
            level: 4,
            result: brogueConstants.notifyEvents.GAMEOVER_DEATH,
            easyMode: false,
            description: "Killed by a pink jelly on depth 3.",
            recording: "test/testfile.broguerec",
            variant: "GBROGUE"
        };

        var self = this;

        gameRecord.create([gameRecord1, gameRecord2], function(err, doc) {
            self.currentTest.gameId1 = doc[0]._id;
            self.currentTest.gameId2 = doc[1]._id;
            done();
        });
    });

    afterEach(function(done) {

        gameRecord.remove({}, function() {
            done();
        });
    });

    it("returns status 404 for non-existent recordings", function(done) {
        request(server)
            .get("/api/recordings/doesnotexist")
            .expect(404, done)
    });

    it("downloads a file with the right headers for existing recordings", function(done) {
        
        var self = this;

        request(server)
            .get("/api/recordings/" + this.test.gameId1)
            .end(function(err, res) {
                expect(res.status).to.equal(200);
                expect(res.header['content-disposition']).to.equal('attachment; filename=webbrogue-recording-' + self.test.gameId1 + '.broguerec');
                done();
            });
    });

    it("returns for 404 for valid recordings of old versions", function(done) {
        
        request(server)
            .get("/api/recordings/" + this.test.gameId2)
            .expect(404, done);
    });

});