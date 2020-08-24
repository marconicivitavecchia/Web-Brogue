var request = require('supertest');
var mongoose = require("mongoose");
var userRecord = require("../user/user-model");
var expect = require("chai").expect;
var server = require("./server-test");
var config = require("./config-test");

var db = mongoose.createConnection(config.db.url);

describe("users", function(){

    beforeEach(function(done) {

        var userRecord1 = {
            username: "flend",
            password: "flend1"
        };

        var userRecord2 = {
            username: "whitechapel",
            password: "apassword"
        };

        var userRecord3 = {
            username: "dave",
            password: "fluffy"
        };

        userRecord.create([userRecord1, userRecord2, userRecord3], function() {
            done();
        });
    });

    afterEach(function(done) {

        userRecord.remove({}, function() {
            done();
        });
    });

    it("returns status 200", function(done) {
      request(server)
          .get("/api/users")
          .set('Accept', 'application/json')
          .expect('Content-Type', /json/)
          .expect(200, done)
    });

    it("returns all user names", function(done) {
        request(server)
            .get("/api/users")
            .set('Accept', 'application/json')
            .end(function(err, res) {
                var bodyObj = JSON.parse(res.text);
                expect(bodyObj).to.deep.equal(["flend", "whitechapel", "dave"]);
                done();
            });
    });
});