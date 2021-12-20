var mongoose = require('mongoose');
var mongoosePaginate = require('mongoose-paginate-v2');

var newsRecordSchema = mongoose.Schema({
    seq: Number,
    date: { type: Date, default: Date.now },
    story: String
});

newsRecordSchema.plugin(mongoosePaginate);

module.exports = mongoose.model('NewsRecord', newsRecordSchema);
