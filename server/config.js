// Object containing global constants for server program

var path = require('path');

var config = {
    port : {
        HTTP : 8080
    },
    brogueVariants : {
        "BROGUECEV19": {
            binaryPath: "binaries/brogue-ce19-c813284",
            version: "1.9.2",
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
        }
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
