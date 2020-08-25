define([
    'jquery',
    'underscore',
    'backbone',
], function($, _, Backbone) {

    var UserDetails = Backbone.Collection.extend({
        url: '/api/users',
    });

    return UserDetails;

});