# Notes

Le princiali implementazioni di IKEv2 che consideriamo sono:

- Strongswan
- iked (proposto da OpenBSD)

Descrivere tramite grafico quale è il flusso del traffico strongswan fino al
kernel per poi uscire 



Le comunicazione tra le reti non sono più sicure a causa della presenza di
computer quantistici che sono in grado di rompere la crittografia asimmetrica
utilizzata in questa tipologia di applicazioni.

Le prime tecnologie che consentono alle comunicazioni di essere resistenti alle
ad attacchi da parte di quantum computer sono:

- Quantum Key Distribution (QKD): si riferisce a un metodo di scambio di chiavi quantistiche per garantire la sicurezza delle comunicazion
- Post Quantum Cryptography (PQC):  si riferisce allo sviluppo di algoritmi crittografici che rimangono sicuri anche in un mondo in cui i computer quantistici possono risolvere algoritmi crittografici tradizionali in modo efficiente

Per ottenere comunicazioni quantum-resistant, entrambe le tecnologie citate devono essere 
incorporate in un protocollo di sicurezza di rete. Per esempio TLS



[Open Quantum Safe](https://openquantumsafe.org) è una libreria che fornisce i principali algoritmi di
crittografia quantum-resistant. E' presente anche un tool che esegue un
banchmarking delle prestazioni [oqsprofile](https://github.com/open-quantum-safe/profiling?tab=readme-ov-file)




## Realtime

Guardare al seguenti [link](https://access.redhat.com/documentation/it-it/red_hat_enterprise_linux_for_real_time/7/html/tuning_guide/chap-realtime-specific_tuning)

E' possibile specificare a strongswan di operare tramite lo scheduler realtime
utilizzando la seguente impostazione di systemd 
```
[Service]
...
CPUQuota=25%
CPUSchedulingPolicy=deadline
CPUSchedulingPriority=99
```

## IKEv2

Per quanto riguarda l'approfondimento di sistemi operativi dedicati ho trovato
un approfondimento riguardo spin, in particolare è stato effetutato un model
checking di ikev2 traimte spin (/resources/appro/Modelcheck-IKEv2.pdf).
Un idea potrebbe essere farlo con ikev2 post-quantum e quindi vedere cosa cambia
quando si aggiungo lo scambio intermedio

## Strongswan

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

### Configuration

Il file di confiugurazione di strongswan sono due

- `/etc/ipsec.conf`: questo è ormai deprecato
- `/etc/strongswan.conf`: si utilizza questo per avere maggiore flessibilità


### Dumping Traffic 

Per eseguire il dumping del traffico fare riferimento a questo [link](https://docs.strongswan.org/docs/5.9/install/trafficDumps.html)


## RFC 92

Di base IKEv2 utilizza ECDH per stabilire il segreto condiviso tra initiator
and responder. La complessità del problema di DH è equivalente al problema del
logaritmo discreto (rispettivamente nel gruppo moltiplicativo, per quello
glassico e nel gruppo elliptic curve).

Il problema alla base è che per i computer classici questo problema risulta di
complessità esponenziale, mentre per i computer quantistici questo potrebbe
diventare un problema di complessità esponenziale.

> La sicuretta di IKEv2 in questo modo viene compromessa.

Si cerca di fare in modo che questo sia resistente ad attacchi quantum-computer,
questa famiglia di crittosistemi è conosciuta come post-quantum cryptography.

E' essenziale realizzare un o più scambi di chiave post-quantum in congiunzione
con lo scabmio di chiae tramite ECDH per fare in modo che la risultante chiave
condivisa sia resistente ad attacchi quantum. 

Allo stato attuale non esistono scambi di chiave post-quantum che sono studiati
bene tanto quanto ECDH, realizzare multipli scambi di chiave 



### Extension

Si vuole mantenere la retrocompatibilità realizzando molteplici 
