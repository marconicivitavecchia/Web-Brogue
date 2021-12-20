var mongoose = require('mongoose');

var newsRecordSchema = mongoose.Schema({
    seq: Number,
    date: { type: Date, default: Date.now },
    story: String
});

module.exports = mongoose.model('NewsRecord', newsRecordSchema);