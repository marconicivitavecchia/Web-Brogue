var mongoose = require('mongoose');
var UsersRecord = require("../user/user-model");
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
};