define([
    "jquery",
    "underscore",
    "backbone",
    "variantLookup"
], function ($, _, Backbone, variantLookup) {

    var GeneralStatsView = Backbone.View.extend({

        el: '#general-statistics',
        headingTemplate: _.template($('#general-statistics-template').html()),

        events: {
            "click #general-stats-by-level-list" : "selectAllStatsOptions"
        },

        initialize: function() {
            this.listenTo(this.model, "add", this.render);
            this.listenTo(this.model, "change", this.render);

            this.setDefaultGeneralStats();
            this.refresh();
        },

        render: function() {

            var variantData = _.values(variantLookup.variants);

            this.$el.html(this.headingTemplate(
                {   stats : this.model.toJSON(),
                    variants: variantData,
                }));

            return this;
        },

        refresh: function() {
            this.model.fetch();
            this.render();
        },

        setDefaultGeneralStats: function() {
            this.model.setVariantGeneralStats(_.values(variantLookup.variants)[0].code);
            this.model.fetch();
        },

        selectAllStatsOptions: function(event) {
            
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
                this.model.setVariantGeneralStats(code);
                this.model.fetch();
            }
        }
    });

    return GeneralStatsView;

});

