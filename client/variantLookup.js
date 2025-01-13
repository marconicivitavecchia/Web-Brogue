define([
    'jquery',
    'underscore',
    'backbone'
], function($, _, Backbone) {

    //Includes all historical variants, so the API response can be prettified
    var VariantLookup = {
        variants: {
            "BROGUECEV140": {
                code: "BROGUECEV140",
                display: "BrogueCE 1.14",
                consoleColumns: 100,
                consoleRows: 34,
                remapGlyphs: true,
                tiles: true,
                default: true
            },
            "BROGUECEV130": {
                code: "BROGUECEV130",
                display: "BrogueCE 1.13",
                consoleColumns: 100,
                consoleRows: 34,
                remapGlyphs: true,
                tiles: true,
                disabled: true
            },
            "BROGUECEV120": {
                code: "BROGUECEV120",
                display: "BrogueCE 1.12",
                consoleColumns: 100,
                consoleRows: 34,
                remapGlyphs: true,
                tiles: true,
                disabled: true
            },
            "BROGUECEV111": {
                code: "BROGUECEV111",
                display: "BrogueCE 1.11.1",
                consoleColumns: 100,
                consoleRows: 34,
                remapGlyphs: true,
                tiles: true,
                disabled: true
            },
            "BROGUECEV110": {
                code: "BROGUECEV110",
                display: "BrogueCE 1.10",
                consoleColumns: 100,
                consoleRows: 34,
                remapGlyphs: true,
                tiles: true,
                disabled: true
            },
            "BROGUECEV19": {
                code: "BROGUECEV19",
                display: "BrogueCE 1.9.3",
                consoleColumns: 100,
                consoleRows: 34,
                remapGlyphs: true,
                tiles: true,
                disabled: true
            },
            "BROGUECEV18": {
                code: "BROGUECEV18",
                display: "BrogueCE 1.8.3",
                consoleColumns: 100,
                consoleRows: 34,
                remapGlyphs: true,
                tiles: true,
                disabled: true
            },
            "BROGUEV175": {
                code: "BROGUEV175",
                display: "Brogue 1.7.5",
                consoleColumns: 100,
                consoleRows: 34
            },
            "BROGUEV174": {
                code: "BROGUEV174",
                display: "Brogue 1.7.4 [early 2019]",
                consoleColumns: 100,
                consoleRows: 34,
                disabled: true
            },
            "BROGUEV174DISCORD": {
                code: "BROGUEV174DISCORD",
                display: "Brogue 1.7.4",
                consoleColumns: 100,
                consoleRows: 34,
                disabled: true
            },
            "BULLETBROGUEV110": {
                code: "BULLETBROGUEV110",
                display: "Bullet Brogue v1.1",
                consoleColumns: 100,
                consoleRows: 34,
                remapGlyphs: true,
                tiles: true,
                highlight: true
            },
            "BULLETBROGUEV100": {
                code: "BULLETBROGUEV100",
                display: "Bullet Brogue v1.0",
                consoleColumns: 100,
                consoleRows: 34,
                remapGlyphs: true,
                tiles: true,
                disabled: true
            },
            "RAPIDBROGUEV160": {
                code: "RAPIDBROGUEV160",
                display: "Rapid Brogue v1.6.0",
                consoleColumns: 100,
                consoleRows: 34,
                remapGlyphs: true,
                tiles: true,
                highlight: true
            },
            "RAPIDBROGUEV150": {
                code: "RAPIDBROGUEV150",
                display: "Rapid Brogue v1.5.0",
                consoleColumns: 100,
                consoleRows: 34,
                remapGlyphs: true,
                tiles: true,
                disabled: true
            },
            "RAPIDBROGUEV140": {
                code: "RAPIDBROGUEV140",
                display: "Rapid Brogue v1.4.0",
                consoleColumns: 100,
                consoleRows: 34,
                remapGlyphs: true,
                tiles: true,
                highlight: false,
                disabled: true
            },
            "RAPIDBROGUEV130": {
                code: "RAPIDBROGUEV130",
                display: "Rapid Brogue v1.3.0",
                consoleColumns: 100,
                consoleRows: 34,
                remapGlyphs: true,
                tiles: true,
                highlight: false,
                disabled: true
            },
            "RAPIDBROGUEV120": {
                code: "RAPIDBROGUEV120",
                display: "Rapid Brogue v1.2.0",
                consoleColumns: 100,
                consoleRows: 34,
                remapGlyphs: true,
                tiles: true,
                highlight: false,
                disabled: true
            },
            "RAPIDBROGUEV110": {
                code: "RAPIDBROGUEV110",
                display: "Rapid Brogue v1.1.0",
                consoleColumns: 100,
                consoleRows: 34,
                remapGlyphs: true,
                tiles: true,
                highlight: false,
                disabled: true
            },
            "RAPIDBROGUEV100": {
                code: "RAPIDBROGUEV100",
                display: "Rapid Brogue v1.0.0",
                consoleColumns: 100,
                consoleRows: 34,
                remapGlyphs: true,
                tiles: true,
                disabled: true
            },
            "GBROGUEV1180211": {
                code: "GBROGUEV1180211",
                display: "gBrogue v1.18.02.11",
                consoleColumns: 100,
                consoleRows: 36
            },
            "UNBROGUEV113": {
                code: "UNBROGUEV113",
                display: "unBrogue v1.1.6",
                consoleColumns: 100,
                consoleRows: 34
            },
            "BROGUEPLUSV1741": {
                code: "BROGUEPLUSV1741",
                display: "Brogue+ v1.7.4.1",
                consoleColumns: 100,
                consoleRows: 34
            },
            "BROGUEMARCONI": {
                code: "BROGUEMARCONI",
                display: "Brogue Marconi (test)",
                consoleColumns: 100,
                consoleRows: 34
            },
        }
    };
    return VariantLookup;
});
