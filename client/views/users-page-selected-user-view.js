define([
    "jquery",
    "underscore",
    "backbone",
    "dispatcher"
], function ($, _, Backbone, dispatcher) {

    var UsersPageSelectedUserView = Backbone.View.extend({

        el: '#user-stats-selected-user',
        template: _.template($('#user-stats-selected-template').html()),
        userName: '',

        initialize: function() {
            this.refresh();
        },

        render: function() {

            this.$el.html(this.template({ userName: this.userName }));
            return this;
        },

        refresh: function() {
            this.render();
        },

        setSelectedUser: function(userName) {
            this.userName = userName;
            this.refresh();
        },

        //Event handler
        userSelected: function(userName) {
            this.setSelectedUser(userName);
        }
    });

    return UsersPageSelectedUserView;

});