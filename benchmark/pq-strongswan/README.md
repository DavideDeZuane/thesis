# Strongswan Tester

Il file `strongswan.conf` definisce quelle che sono le configurazioni del deamon, viene letto da `libstrongswan` al momento di inizilizzazione o di reloading.
Il file `swanctl.conf` viene utilizzato per definire i parametri del peer.

Nel nostro esempio abbiamo:

- **Carol**: Initiator
- **Moon**: Responder

Ognuno dei quali è un container in esecuzione, la cui immagine è quella specificata all'interno del `Dockerfile`.

## Authentication


### PSK

Per eseguire l'autenticazione tramite psk per ike occorre specificare nella sezione secrets il secrets nel seguente modo 
```
secrets {
    ike-<suffix> {
        id-<suffix>
        secrets
    }

}
```

## Initiator

Il file di configurazione dell'initiator è `carol/swanctl.conf`.
Questo è strutturato come include di diversi singoli file ognuno dei quali contiene le singole tipologie di connessioni da realizzare, questi si trovano nella directory `carol/conn`.

| Name     | File  | Chiper-Suite  |
|---|---|---|
| alpha    |   |   | 
| bravo    |   |   |
| charlie  |   |   |
| delta    |   |   |
| echo     |   |   |

Per l'autenticazione abbiamo testato principalmente l'autenticazione mediante certificati tramite algoritmi di firma post-quantum (tra cui falcon e dilithium nelle loro varie versioni).
Abbiamo le seguenti directory:

- `x509`: al cui interno vanno messi i certificati che il peer utilizzerà per autenticarsi
- `x509ca`: al cui interno si trovano i certificati delle CA da utilizzare per verficare i certificati
- `pkcs8`: al cui interno troviamo le chiavi private, il cui certificato di chiave pubblica si trova nella directory `x509`

> La directory in cui posizionare la chiave privata dipende dall'algoritmo utilizzato per generarla.

### Initiate

Per iniziare la SA di tipo IKE, va utilizzato il seguente comando:

```
swanctl --initiate --ike <connection>
```
> Si può specificare anche quelle che è la CHILD\_SA da stabilire, ma a noi questa fase non interessa.

## Responder

File di configurazione: `moon/swanctl.conf`
