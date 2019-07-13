define([
    "jquery",
    "underscore",
    "backbone",
    "config",
    "variantLookup"
], function ($, _, Backbone, config, variantLookup) {

    var CauseStatisticsView = Backbone.View.extend({

        el: '#cause-statistics',
        headingTemplate: _.template($('#cause-statistics-template').html()),

        events: {
            "click #deaths-by-cause-list" : "selectAllCausesOptions"
        },

        initialize: function() {
            this.listenTo(this.model, "add", this.render);
            this.listenTo(this.model, "change", this.render);

            this.grid = new Backgrid.Grid({
                columns: [
                    {
                        name: "level",
                        label: "Level",
                        cell: "integer",
                        sortable: false,
                        editable: false
                    }, {
                        name: "rank",
                        label: "Rank",
                        cell: "integer",
                        sortable: false,
                        editable: false
                    }, {
                        name: "cause",
                        label: "Cause",
                        cell: "string",
                        sortable: false,
                        editable: false
                    }, {
                        name: "frequency",
                        label: "Frequency",
                        cell: "integer",
                        sortable: false,
                        editable: false
                    }],

                collection: this.model
            });

            this.renderOptions();
            this.setDefaultVariantCauses();
        },

        renderOptions: function() {

            var variantData = _.values(variantLookup.variants);

            this.$el.html(this.headingTemplate(
                {   username: this.model.username,
                    variants: variantData}));
        },


        render: function() {

            $("#cause-stats-grid").append(this.grid.render().$el);
            return this;
        },

        refresh: function() {
            this.model.fetch();
        },

        setDefaultVariantCauses: function() {
            this.model.setVariantForCauseStats(config.variants[0].code);
            this.model.fetch();
        },

        selectAllCausesOptions: function(event) {
            
            event.preventDefault();

            if(!event.target.id) {
                return;
            }

            var codeAfterHyphenIndex = event.target.id.lastIndexOf("-")
            
            if(codeAfterHyphenIndex == -1) {
                return;
            }

            var code = event.target.id.substring(codeAfterHyphenIndex + 1);

            if(code in variantLookup.variants) {
                this.model.setVariantForCauseStats(code);
                this.model.fetch();
            }
        }
    });

    return CauseStatisticsView;
});

