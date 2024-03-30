# Note

## Implemteazioni

Per quanto riguarda le implementazioni di IPSec più utilizzate abbiamo:

- **StrongSwan**: fornisce supporto completo a IPSec e IKE, ha una comunità molto attiva che segue la sua implementazione
- **LibreSwan**: si adatta fortemente agli standard, fa della stabilità il suo punto forte, quindi è utilizzato per connessioni IPSec standard
- **OpenSwan**: si è spostata l'impolementazione sui due elencati, viene utilizzato nei sistemi legacy

### Strongswan 

Nella versione 6.0 di StrongSwan sono presenti metodi post quantum, è possibile eseguire test di strongswan tramite la sua build [docker](https://github.com/strongX509/docker/tree/master/pq-strongswan#readme).
Strongswan mette a disposizione un ambiente di integrazione e test di regressione virtualizzato per garantire la qualità di ciascuna versione.
Alcuni link utili:

- [Sorgente](https://docs.strongswan.org/docs/5.9/devs/devs.html)

## Crypto Linux

La Crypto API di Linux è un componente chiave utilizzato da StrongSwan per eseguire le operazioni crittografiche necessarie per implementare IPsec in un ambiente Linux. Ecco come la Crypto API di Linux è correlata a StrongSwan:

- **Gestione delle operazioni crittografiche**: StrongSwan utilizza la Crypto API di Linux per eseguire le operazioni crittografiche richieste per la negoziazione delle associazioni di sicurezza IPsec. Questo include l'uso di algoritmi di crittografia simmetrica per cifrare il traffico, algoritmi di firma digitale per l'autenticazione e la generazione di numeri casuali per la creazione delle chiavi.
- **Interfaccia con la Crypto API**: StrongSwan interagisce con la Crypto API di Linux attraverso un'interfaccia di programmazione (API) standard fornita dal kernel. Questa API consente a StrongSwan di eseguire operazioni crittografiche in modo efficiente e sicuro, senza dover implementare la crittografia direttamente all'interno dell'applicazione.
- **Supporto per hardware criptografico**: StrongSwan può beneficiare del supporto hardware criptografico disponibile su alcuni sistemi Linux. La Crypto API fornisce un'interfaccia per il driver hardware criptografico, consentendo a StrongSwan di sfruttare accelerazione hardware per operazioni crittografiche, migliorando le prestazioni.
- **Gestione delle chiavi e della sicurezza**: StrongSwan utilizza la Crypto API di Linux per la gestione delle chiavi di crittografia e della sicurezza delle comunicazioni IPsec. La Crypto API fornisce strumenti per proteggere le chiavi sensibili e gestirle in modo sicuro.

Link utili:

- [Architettura](https://www.kernel.org/doc/html/v4.18/crypto/architecture.html)

### Introduction

In queste API si fa riferimento agli algoritmi crittografici come **trasformazioni** per questo motivo vengono indicati con *tfm*. Oltre alle funzinoni crittografiche offre anche operazioni di compressione.

