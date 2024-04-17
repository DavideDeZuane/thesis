# Ideas

Fare crafting di pacchetti con errori in modo da similuare un canale più rumoroso 

# Strongswan

Charon è il daemon ed ha la seguente architettura
```
      +---------------------------------+       +----------------------------+
      |          Credentials            |       |          Backends          |
      +---------------------------------+       +----------------------------+

       +------------+    +-----------+        +------+            +----------+
       |  receiver  |    |           |        |      |  +------+  | CHILD_SA |
       +----+-------+    | Scheduler |        | IKE- |  | IKE- |--+----------+
            |            |           |        | SA   |--| SA   |  | CHILD_SA |
    +-------+--+         +-----------+        |      |  +------+  +----------+
 <->|  socket  |               |              | Man- |
    +-------+--+         +-----------+        | ager |  +------+  +----------+
            |            |           |        |      |  | IKE- |--| CHILD_SA |
       +----+-------+    | Processor |--------|      |--| SA   |  +----------+
       |   sender   |    |           |        |      |  +------+
       +------------+    +-----------+        +------+

      +---------------------------------+       +----------------------------+
      |               Bus               |       |      Kernel Interface      |
      +---------------------------------+       +----------------------------+
             |                    |                           |
      +-------------+     +-------------+                     V
      | File-Logger |     |  Sys-Logger |                  //////
      +-------------+     +-------------+
```

## Command

E' possibile specificare molteplici connessioni da utilizzare per per strongswan, per specificare quale utilizzare per stabilire la SA occorre utilizzare il seguente comando:

```
swanctl --initiate --ike <connection_name> --child <child_name>
```

Dove child si specifica la Child\_SA che si vuole realizzre tra i due peer. E' possibile omettere l'opzione `child` e quindi stabilire solo la IKE\_SA.


Per verificare che i certificati, ma in generale tutta la configurazione, vengano correttamente caricati possiamo utilizzare il comando:

```
swanctl --load-all
```

## Certificate

All'interno del container di strongswan è presente pki con alcune primitive post-quantum in particolare dilithium e falcon. Se si utilizzano quelle generate da openssl tramite oqs-provider non funzionano

### Root 

Generare la chiave privata

Sign Certificate
```
 pki --self --ca --lifetime 1825 --in CA.falcon512.private.pem --dn "CN=Root Falcon" --outform pem > CA.falcon512.cert.pem
```

I certificati ecdsa generati con openssl a volte non sono riconosciuti da strongswan per questo motivo conviene utilizzare pki per generarli. I comandi sono i seguenti

```
pki --gen --type ecdsa --size 256 --outform pem > file.pem
# Per firmare la chiave pubblica
pki --pub --in /etc/ipsec.d/private/server-key.pem --type rsa \
  | pki --issue --lifetime 1825 \
      --cacert /etc/ipsec.d/cacerts/ca-cert.pem \
      --cakey /etc/ipsec.d/private/ca-key.pem \
      --dn "CN=<Server_IP>" --san @<Server_IP> --san <Server_IP> \
      --flag serverAuth --flag ikeIntermediate --outform pem \
  >  /etc/ipsec.d//certs/server-cert.pem
```

## Options

Alcune opzioni interessanti per quanto riguarda la ike sa sono:

- `fragmentation = no` per disabilitare la frammentazione ike, abilitata di default
- `send_reqcert = no` specifico che il peer non deve mandare all'altro la richiesta di fornire il certificato
- `send_cert = always` di default il peer manda il certificato solo quando richiesto, in questo modo invece lo manda sempre

## Configuration

Il file di confiugurazione di strongswan sono due

- `/etc/ipsec.conf`: questo è ormai deprecato
- `/etc/strongswan.conf`: si utilizza questo per avere maggiore flessibilità

Il file `strongswan.conf` è il file di configurazione letto da libstrongswan al
momento di inizializzazione o di reloading. (Il file legacy di ipsec.conf è
troppo oneroos da leggere)

### Dumping Traffic 

Per eseguire il dumping del traffico fare riferimento a questo [link](https://docs.strongswan.org/docs/5.9/install/trafficDumps.html)


## Key Exchange

I vari metodi per fare key-exchange si trovano all'interno del sorgente di strongswan:
`src/libstrongswan/crypto/key_exchange.h`. E andando a vedere li abbiamo che kyber1 ha come numero 1080 che è proprio quello presente nel pacchetto.

E' presente il fa key\_exchange\_is\_kem che verifica se il key exchange è kem


## OUTPUT

Un elenco completo è disponibile al seguente [link](https://docs.strongswan.org/docs/6.0/config/logging.html).
Ogni output è taggato, il tag indica da quale sottosistema del daemon il log proviene:

- [IKE]: Questo tag indica messaggi relativi al protocollo IKE stesso, come inizializzazione della sessione, autenticazione, negoziazione di parametri di sicurezza, ecc. 
- [CFG]: provengono dal plugin di configurazione e management (`libcharon/config`)
- [NET]: proviene dall'IKE network communication (`libcharon/network`)
- [ENC]: encoding


## Exchange

Possiamo osservare all'interno dell'IKE SA INIT che è presente il payload Key Exchange al cui interno troviamo il materiale che i due si scambiano 

Nella dimensoine del pacchetto riportata dai log di ike quello è solamente il payload generato da lui. Quando andiamo a fare la cattura abbiamo 42byte aggiuntivi, questi sono dovunti a:

- 8byte: header TCP
- 20byte: header IP 
- 14byte: header MAC

Inoltre nel caso dell'auth, abbiamo che la dimensione degli header è di 34 byte per i vari frammenti IP mentre nel caso in cui si va a considerare la dimensione del packetto IKE\_AUTH abbiamo che l'header a questo punto è 42 + 4 byte dovuti all' Non-ESP Marker definito nell'RFC 3948, che serve per IKE Header Format for Port 4500


# Openssl

## Output

Tramite l'opzione `-certopt` è possibile personalizzare la stampa dei campi del certificato

- `no_header`: Esclude l'header dal dump del certificato.
- `no_version`: Esclude la versione dal dump del certificato.
- `no_serial`: Esclude il numero di serie dal dump del certificato.
- `no_validity`: Esclude la validità dal dump del certificato.
- `no_subject`: Esclude il soggetto dal dump del certificato.
- `no_issuer`: Esclude l'emittente dal dump del certificato.
- `no_pubkey`: Esclude la chiave pubblica dal dump del certificato.
- `no_signame`: Esclude il nome della firma dal dump del certificato.
- `no_extensions`: Esclude le estensioni dal dump del certificato.
- `no_sigdump`: Esclude la firma dal dump del certificato.
- `no_aux`: Esclude i dati ausiliari dal dump del certificato.
- `no_header`: Esclude l'header dal dump del certificato.

## Command

### List

Fare la lista di tutti gli algoritmi di un provider:
```console
openssl list -signature-algorithms -provider oqsprovider
```

### Genkey

Per generare una chiave:

```console
openssl genpkey -algorithm <algorithm> -out <out_file>
```

### Root Certificate

Per generare una chiave e un certificato associato a quella chiave insieme:

```console
openssl req -x509 -new -newkey <algorithm> -keyout <key_file> -out <cert_file> -nodes -subj "/CN=oqstest" -days 365
```

### Certificate Signing Request

Si va a generare una chiave e la richiesta di firma del certificato insieme.
Una volta creati si utilizzano le chiavi e i certificati delle CA per firmare la richiesta di certificato.
Infine si rimuove la richiesta di certificato.

```console
openssl req -new -newkey <algorithm> -keyout <key_file> \
            -out <csr_file> -nodes -subj "/CN=test server" 
openssl x509 -req -in <csr_file> -out <cert_filr> \ 
             -CA <ca_cert> -CAkey <ca_key>        \
             -CAcreateserial -days 365
rm *.csr
```

## PEM

Le chiavi sono in formato `pem` che codifica le informazioni in base64, quindi ogni carattere, quindi ogni carattere racchiude l'informazione di 6 bit


## OCSP

Online Certificate Status Protocol.
Un modo intelligente per gestire agevolmente i certificati senza mettere i root certificate su ogni peer è specificare l'ocsp uri all'interno della configurazione di strongswan. In questo modo sarà lui a dire se il certificato è valido o meno



