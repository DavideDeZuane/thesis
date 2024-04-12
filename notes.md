# Notes

Le princiali implementazioni di IKEv2 che consideriamo sono:

- Strongswan
- iked (proposto da OpenBSD)

Descrivere tramite grafico quale è il flusso del traffico strongswan fino al
kernel per poi uscire 


## Minimal IKE (FastEagle, windEagle)

Fare una bozza di implementazione, quindi definendo quelle che sono le funzionalità base andare ad implementarle in C.
Questo perchè andando a vedere il codice di strongswan risulta molto complesso e articolato, quindi farlo da zero così da capirlo meglio
e riuscire a schedularlo real time

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

