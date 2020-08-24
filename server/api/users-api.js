var mongoose = require('mongoose');
var UsersRecord = require("../user/user-model");
var sanitize = require('mongo-sanitize');
var _ = require("underscore");

module.exports = function(app, config) {

    app.get("/api/users", function (req, res, next) {

        UsersRecord.find({}).lean().exec(function (err, users) {

            res.format({
                json: function () {
                    const allUsers = _.map(users, function(user) { return user.username; });
                    res.json(allUsers);
                }
            });
        });
    });

    app.get("/api/users/:partial", function (req, res, next) {

        var query = "{ $text: { $search: \"" + sanitize(req.params.partial) + "\" } }";
    
        console.log(query);

        UsersRecord.find(query).lean().exec(function (err, users) {

            res.format({
                json: function () {

                    console.log(JSON.stringify(users));

                    const allUsers = _.map(users, function(user) { return user.username; });
                    res.json(allUsers);
                }
            });
        });
    });
};