// View for login and registration forms

define([
    "jquery",
    "underscore",
    "backbone",
    "dispatcher",
    "util",
    "dataIO/send-generic",
    "dataIO/router",
    "views/view-activation-helpers"
], function ($, _, Backbone, dispatcher, util, send, router, activate) {

    var AuthenticationView = Backbone.View.extend({
        el: "#auth",

        events: {
            "click #login-button": "loginSubmit",
            "click #register-button": "registerSubmit",
            "click #to-register": "changeToRegister",
            "click #to-login": "changeToLogin"
        },
        templates: {
            login: _.template($('#login').html()),
            register: _.template($('#register').html())
        },
        initialize: function () {
            this.model.set({
                username: "",
                type: "",
                loggedIn: false
            });
            
            this.requestLogin();
        },
        render: function (templateName) {
            this.$el.html(this.templates[templateName]);
        },
        requestLogin: function () {

            var storedToken = util.getItem('sessionId');

            if(storedToken) {
                send("auth", "login", { token: storedToken });
            }
            else {
                send("auth", "anon-login");
            }

            this.render("login");
        },
        logout: function() {

            this.model.set({
                username: "",
                type: "",
                loggedIn: false
            });

            //Request an anon login
            send("auth", "anon-login");
        },
        loginSubmit: function (event) {
            event.preventDefault();
            
            var loginData = {
                username: $('#username').val(),
                password: $('#password').val()
            };
            send("auth", "login", loginData);
        },
        registerSubmit: function (event) {
            event.preventDefault();

            if($('#username').val().trim() === "" ||
                $('#password').val().trim() === "") {
                $('#auth-message')
                    .removeClass()
                    .addClass("error")
                    .html("Please fill in all the boxes.");
                return;
            }

            var registerData = {
                username: $('#username').val(),
                password: $('#password').val(),
                repeat: $('#password-repeat').val()
            };
            send("auth", "register", registerData);
        },
        changeToRegister: function (event) {
            event.preventDefault();
            this.render("register");
        },
        changeToLogin: function (event) {
            event.preventDefault();
            this.render("login");
        },
        handleMessage: function (message) {

            if (message.result === "fail") {
                $('#auth-message')
                        .removeClass()
                        .addClass("error")
                        .html(message.data);
                return;
            }
            
            if (message.result === "logout"){
                this.render("login");
                dispatcher.trigger("logout");
                activate.resetAll();
                return;
            }

            if(message.result === "success") {

                switch (message.data.message) {
                    case "anon-logged-in":

                        this.model.set({
                            username: message.data.username,
                            type: "anon",
                            loggedIn: true
                        });

                        dispatcher.trigger("anon-login", message.data.username);
                        break;


                    case "logged-in" :
                        activate.loggedIn();

                        this.model.set({
                            username: message.data.username,
                            type: "registered",
                            loggedIn: true
                        });

                        dispatcher.trigger("login", message.data.username);

                        //var headerMessage = '{"type" : "header", "data" : "'+ this.model.get("username") +'"}'
                        //router.route(headerMessage);

                        util.setItem('sessionId', message.data.token);

                        break;
                    case "registered" :
                        this.render("login");
                        $('#auth-message')
                            .removeClass()
                            .addClass("success")
                            .html("Successfully Registered - Please log in");
                        break;
                }
            }

        }
    });

    return AuthenticationView;

});
