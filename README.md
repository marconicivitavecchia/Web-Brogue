Web Brogue
==========

Per il README originale, cliccare [qui](README-orig.md).

I passi che abbiamo seguito per arrivare a questa versione customizzata dall'originale sono [qui](README-marconi.md).

Di seguito le istruzioni per eseguire questo repo di Web Brogue.

## Creazione macchina EC2
Andare su AWS Academy Login (https://www.awsacademy.com/SiteLogin), creare una macchina EC2 con le seguenti caratteristiche (per tutto il resto lasciare il default):

- Ubuntu 64 bit
- usare la chiave vockey.pem
- aprire le porte 80 (HTTP), 443 (HTTPS), 8080

Una volta fatta partire la macchina EC2, collegatevi via SSH come abbiamo visto in classe.

Da qui, i comandi sono più o meno quelli della guida ufficiale (https://github.com/flend/web-brogue). Di seguito il dettaglio

## Configurazione macchina EC2

Collegarsi via terminale ed eseguire i seguenti comandi:

```sh
# Installare mongodb
sudo apt-get install gnupg curl

curl -fsSL https://www.mongodb.org/static/pgp/server-8.0.asc | \
   sudo gpg -o /usr/share/keyrings/mongodb-server-8.0.gpg \
   --dearmor

echo "deb [ arch=amd64,arm64 signed-by=/usr/share/keyrings/mongodb-server-8.0.gpg ] https://repo.mongodb.org/apt/ubuntu noble/mongodb-org/8.0 multiverse" | sudo tee /etc/apt/sources.list.d/mongodb-org-8.0.list

sudo apt-get update
sudo apt-get upgrade
sudo apt-get install -y mongodb-org
```
A questo punto riavviare la macchina, quindi:

```sh
# Avviamo MongoDB
sudo systemctl start mongod
sudo systemctl enable mongod

# installs nvm (Node Version Manager)
curl -o- https://raw.githubusercontent.com/nvm-sh/nvm/v0.40.1/install.sh | bash

# download and install Node.js (you may need to restart the terminal)
nvm install 18

# verifies the right Node.js version is in the environment
node -v # should print `v18.20.5`

# verifies the right npm version is in the environment
npm -v # should print `10.8.2`

# Installare il compilatore C++
sudo apt-get install build-essential
gcc -v # l'output dovrebbe finire con la riga "gcc version 13.3.0 (Ubuntu 13.3.0-6ubuntu2~24.04)" o qualcosa di simile

# Scaricare web-brogue
git clone https://github.com/flend/web-brogue.git

# Installare le dipendenze
cd web-brogue/server
npm install

# Far partire il web server
npm start
```

Se tutto va bene, il server è up and running!

## Lanciare il gioco dopo un riavvio
### Riavvio manuale
Dopo il riavvio, se la macchina già non parte automaticamente, collegarsi nuovamente in ssh ed eseguire i seguenti comandi:

```sh
cd ~/web-brogue/server
nvm install 18 && npm start
```

### Avvio automatico
Per avviare automaticamente l'applicazione all'avvio della macchina, per prima cosa creare un file di servizio systemd:

```sh
sudo nano /etc/systemd/system/nodeapp.service
```

Inserisci questo contenuto nel file:

```config
[Unit]
Description=Node.js Web Server Brogue
After=network.target

[Service]
Type=simple
User=ubuntu
WorkingDirectory=/home/ubuntu/web-brogue/server
ExecStart=npm start
Restart=on-failure

[Install]
WantedBy=multi-user.target
```

Dopo aver salvato il file, esegui questi comandi:

```sh
# Ricarica la configurazione di systemd
sudo systemctl daemon-reload

# Abilita il servizio all'avvio
sudo systemctl enable nodeapp

# Avvia il servizio
sudo systemctl start nodeapp
```

Per vericare lo stato dell'app:

```sh
sudo systemctl status nodeapp
```

Per riavviare l'app:

```sh
sudo systemctl restart nodeapp
```
