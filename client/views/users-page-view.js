define([
    "jquery",
    "underscore",
    "backbone"
], function ($, _, Backbone) {

    var UsersPageView = Backbone.View.extend({

        el: '#users-page',
        headingTemplate: _.template($('#users-page-template').html()),

        initialize: function() {

            this.refresh();
        },

        render: function() {

            this.$el.html(this.headingTemplate({ }));
            return this;
        },

        refresh: function() {
            this.render();
        }
    });

    return UsersPageView;

});

