define([
    "jquery",
    "underscore",
    "backbone",
    "chart",
    "config",
    "variantLookup"
], function ($, _, Backbone, Chart, config, variantLookup) {

    var LevelProbabilityView = Backbone.View.extend({

        el: '#level-probability',
        headingTemplate: _.template($('#level-probability-template').html()),

        events: {
            "click #probability-deaths-by-level-list" : "selectAllLevelsOptions"
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
                        name: "probability",
                        label: "Probability",
                        cell: "number",
                        sortable: false,
                        editable: false
                    }],

                collection: this.model
            });

            this.renderOptions();
            this.setDefaultDeathsProbabilityStats();
            this.refresh();
        },

        renderOptions: function() {

            var variantData = _.values(variantLookup.variants);

            this.$el.html(this.headingTemplate(
                {   username: this.model.username,
                    variants: variantData}));
        },

        render: function() {

            $("#level-probability-grid").append(this.grid.render().$el);

            //Level probability chart

            var ctx = document.getElementById("level-probability-chart");
            var levelData = this.model.pluck("level");
            var probabilityData = this.model.pluck("probability");

            new Chart(ctx, {
                type: 'bar',
                data: {
                    labels: levelData,
                    datasets: [{
                        label: 'probability',
                        data: probabilityData,
                        backgroundColor: 'rgba(255, 99, 132, 0.2)',
                        borderColor: 'rgba(255,99,132,1)',
                        borderWidth: 1
                    }]
                },
                options: {
                    title: {
                        display: true,
                        text: 'Death probability by level'
                    },
                    scales: {
                        xAxes: [{
                            scaleLabel: {
                                display: true,
                                labelString: 'Level'
                            }
                        }],
                        yAxes: [{
                            scaleLabel: {
                                display: true,
                                labelString: 'Frequency'
                            },
                            ticks: {
                                beginAtZero:true
                            }
                        }]
                    }
                }
            });

            return this;
        },

        refresh: function() {
            this.model.fetch();
            this.render();
        },

        setDefaultDeathsProbabilityStats: function() {
            this.model.setVariantForLevelProbabilityStats(config.variants[0].code);
            this.model.fetch();
        },
        
        selectAllLevelsOptions: function(event) {
            
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
                this.model.setVariantForLevelProbabilityStats(code);
                this.model.fetch();
            }
        }
    });

    return LevelProbabilityView;
});

