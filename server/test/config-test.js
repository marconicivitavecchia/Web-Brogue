// Object containing global constants for server program

var path = require('path');

var config = {
    port : {
        HTTP : 8080
    },
    brogueVariants : {
        "BROGUE": {
          binaryPath: "brogue/bin/brogue",
          version: "1.7.4",
          versionGroup: "1.7.4",
          modernCmdLine: true,
          supportsDownloads: true
        },
        "GBROGUE": {
            binaryPath: "gbrogue/bin/brogue",
            version: "1.18.02.11",
            versionGroup: "1.18.02.11",
            modernCmdLine: false,
            supportsDownloads: false
        }
    },
    defaultBrogueVariant: "BROGUE",
    path : {
        CLIENT_DIR : path.normalize(__dirname + "/../client/"),
        GAME_DATA_DIR : path.normalize(__dirname + "/../game-data/"),
    },
    db : {
        url : "mongodb://localhost/server_test"
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
