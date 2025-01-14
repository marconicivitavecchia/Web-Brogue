# Personalizzazione del gioco

Per creare una versione custom del gioco, bisogna ricompilare gli eseguibili. Io ho usato la versione Brogue 1.7.5.

Per prima cosa aggiungere la variante a `client/variantLookup.js`, alla fine della lista:

```js
           "BROGUEMARCONI": {
                code: "BROGUEMARCONI",
                display: "Brogue Marconi (test)",
                consoleColumns: 100,
                consoleRows: 34
            },
```

Aggiungere il corrispettivo su `server/config.js`:

```js
       "BROGUEMARCONI": {
            binaryPath: "binaries/brogue-marconi",
            version: "1.0.0",
            versionGroup: "1.0.x",
            modernCmdLine: false,
            supportsDownloads: false,
            maxSeed: Integer(4294967295)
        },
```

Per creare l'esegubile, bisogna correggere un bug negli include.

Navigare in `brogue-1.7.5/` e modificare il file `src/brogue/IncludeGlobals.h` nelle righe 50-51 come segue:

```h
extern short messageArchivePosition; // Aggiungere extern
extern char messageArchive[MESSAGE_ARCHIVE_LINES][COLS*2]; // Aggiungere extern
```

A questo punto, possiamo compilare. Sempre dalla cartella `brogue-1.7.5/` e mettere l'eseguibile nella cartella dei binari:

```sh
make web
cp bin/brogue ../binaries/brogue-marconi
```

Per finire, riavviamo il web server, tutto dovrebbe funzionare correttamente!
