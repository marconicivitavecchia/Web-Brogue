// Object containing global constants for server program

var path = require('path');

var config = {
    port : {
        HTTP : 8080
    },
    brogueVariants : {
        "BROGUEPLUSV1741": {
            binaryPath: "brogueplus/bin/brogue",
            version: "1.7.4.1",
            versionGroup: "1.7.4.1",
            modernCmdLine: false,    //Uses v1.8.x+ standard command line
            supportsDownloads: true //Replays should work with desktop version
        },
        "BROGUECEV111": {
            binaryPath: "binaries/brogue-ce111",
            version: "1.11.1",
            versionGroup: "1.10.x",
            modernCmdLine: true,    //Uses v1.8.x+ standard command line
            supportsDownloads: true //Replays should work with desktop version
        },
        "BROGUECEV110": {
            binaryPath: "binaries/brogue-ce110",
            version: "1.10",
            versionGroup: "1.10.x",
            modernCmdLine: true,    //Uses v1.8.x+ standard command line
            supportsDownloads: true //Replays should work with desktop version
        },
        "BROGUECEV19": {
            binaryPath: "binaries/brogue-ce193-d22c4a1",
            version: "1.9.3",
            versionGroup: "1.9.x",
            modernCmdLine: true,    //Uses v1.8.x+ standard command line
            supportsDownloads: true //Replays should work with desktop version
        },
        "BROGUECEV18": {
          binaryPath: "binaries/brogue-ce18",
          version: "1.8.3",
          versionGroup: "1.8.x",
          modernCmdLine: true,    //Uses v1.8.x+ standard command line
          supportsDownloads: true //Replays should work with desktop version
        },
        "BROGUEV174": {
            binaryPath: "binaries/brogue-fd99bbe",
            version: "1.7.4",
            versionGroup: "1.7.4",
            modernCmdLine: false,
            supportsDownloads: false
        },
        "GBROGUEV1180211": {
            binaryPath: "gbrogue/bin/brogue",
            version: "1.18.02.11",
            versionGroup: "1.18.02.11",
            modernCmdLine: false,
            supportsDownloads: false
        },
        "BROGUEV175": {
            binaryPath: "brogue-1.7.5/bin/brogue",
            version: "1.7.5",
            versionGroup: "1.7.5",
            modernCmdLine: false,
            supportsDownloads: false
        },
        "UNBROGUEV113": {
            binaryPath: "unBrogue/bin/brogue",
            version: "1.1.3",
            versionGroup: "1.1.3",
            modernCmdLine: false,
            supportsDownloads: false
        },
        "BROGUEV174DISCORD": {
            binaryPath: "brogue/bin/brogue",
            version: "1.7.4b",
            versionGroup: "1.7.4b",
            modernCmdLine: false,
            supportsDownloads: false
        },
        "RAPIDBROGUEV100": {
            binaryPath: "binaries/rapid-brogue-v100",
            version: "1.0.0",
            versionGroup: "1.0.x",
            modernCmdLine: true,    //Uses v1.8.x+ standard command line
            supportsDownloads: true //Replays should work with desktop version
        },
        "RAPIDBROGUEV110": {
            binaryPath: "binaries/rapid-brogue-v110",
            version: "1.1.0",
            versionGroup: "1.1.x",
            modernCmdLine: true,    //Uses v1.8.x+ standard command line
            supportsDownloads: true //Replays should work with desktop version
        },
        "RAPIDBROGUEV120": {
            binaryPath: "binaries/rapid-brogue-v120",
            version: "1.2.0",
            versionGroup: "1.2.x",
            modernCmdLine: true,    //Uses v1.8.x+ standard command line
            supportsDownloads: true //Replays should work with desktop version
        },
    },
    defaultBrogueVariant: "BROGUECEV18",
    path : {
        CLIENT_DIR : path.normalize(__dirname + "/../client/"),
        GAME_DATA_DIR : path.normalize(__dirname + "/../game-data/")
    },
    db : {
        url : "mongodb://localhost/brogue"
    },
    lobby : {
        UPDATE_INTERVAL : 1000,
        TIMEOUT_INTERVAL : 300000
    },
    auth : {
        secret: 'asecret',
        tokenExpiryTime: 90 * 24 * 60 * 60 * 1000
    }
};

module.exports = config;
