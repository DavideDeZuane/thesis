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

Dove child si specifica la Child\_SA che si vuole realizzre tra i due peer

### Certificate

Per verificare che i certificati vengano correttamente caricati possiamo utilizzare il comando:

```
swanctl --load-all
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

Ho messo due opzioni che hanno il seguente certificato:

- `send_reqcert = no` specifico che il peer non deve mandare all'altro la richiesta di fornire il certificato
- `send_cert = always` di default il peer manda il certificato solo quando richiesto, in questo modo invece lo manda sempre

## Configuration

Il file di confiugurazione di strongswan sono due

- `/etc/ipsec.conf`: questo è ormai deprecato
- `/etc/strongswan.conf`: si utilizza questo per avere maggiore flessibilità


### Dumping Traffic 

Per eseguire il dumping del traffico fare riferimento a questo [link](https://docs.strongswan.org/docs/5.9/install/trafficDumps.html)



## OUTPUT

Un elenco completo è disponibile al seguente [link](https://docs.strongswan.org/docs/6.0/config/logging.html).
Ogni output è taggato, il tag indica da quale sottosistema del daemon il log proviene:

- [IKE]: Questo tag indica messaggi relativi al protocollo IKE stesso, come inizializzazione della sessione, autenticazione, negoziazione di parametri di sicurezza, ecc. 
- [CFG]: provengono dal plugin di configurazione e management (`libcharon/config`)
- [NET]: proviene dall'IKE network communication (`libcharon/network`)
- [ENC]: encoding



## Result 

Per eseguire l'output delle catture tramite tcpdump sul terminale utilizzare il seguente comando, il flag -e server per mostrare informazioni aggiuntive:
```console
tcpdump -r <file> -e 
```
E' possibile filtrare il traffico in base a porta e protocollo, per esempio 
```
tcpdump -r ikev2-bravo.pcapng -e udp port 500
```


Considerando le tre differenti connessioni ike abbiamo che:

- alpha: la dimensione dei pacchetti IKE\_SA\_INIT è dell'ordine di 200-300 byte, si utilizza x22519
- bravo: la dimensione dei pacchetti e dell'ordine dei 1000 byte, si utilizza kyber1
