var mongoose = require('mongoose');
var mongoosePaginate = require('mongoose-paginate-v2');

var gameRecordSchema = mongoose.Schema({
    username: String,
    date: { type: Date, default: Date.now },
    seed: Number,
    score: Number,
    level: Number,
    easyMode: Boolean,
    result: Number,
    description: String,
    recording: String,
    variant: String
});

gameRecordSchema.index({ date: 1, username: -1 }, { unique: true });

gameRecordSchema.plugin(mongoosePaginate);

module.exports = mongoose.model('GameRecord', gameRecordSchema);
