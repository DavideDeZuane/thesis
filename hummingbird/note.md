
## Idee

    // il parsing della proposal per il momento non mi interessa poichè se mi ha risposto vuol dire che gli vabene
    // andiamo a prendere direttamente il materiale per il key exchange
    // ad ogni payload che parso ridimensiono il buffer 
    //invece che allocare ogni volta un buffer di dimensione minore fare un buffer pool
    // oppure andare ad incrementare il buffer sulla base del campo lunghezza contenuto nel next payload
Low-latency



## C

Nel definire le costanti conviene utilizzare il tipo enum rispetto al define in quanto è presente un controllo sul tipo mentre sull'altro no. A livello di prestazioni sono uguali dato che vengono sostiuti i valori in fase di compilazione

### Documentazione

Per generare la documentazione utilizzare `doxygen`.

Nel file header il commento deve essere fatto su come è definita l'interfaccia pubblica, si concnetra su cosa fa la funzione.
Nel file sorgente (.c) dovresti fornire commenti che spiegano i dettagli dell'implementazione, quindi si concentra su come lo fa

### Struct 

Nel definire una struct in casi in cui rappresentano un header di un pacchetto è molto importante fare attenzione alla dimensione di tali struttura. Questo perchè il compilatore per tali strutture utilizza il padding a dimensioni di multipli dei dati per garantire l'allineamento della struttura e quindi avere un accesso alla memoria più veloce.

Esempio nel mio caso la struct ritornava come dimensione 32 byte, quando in verità la sua dimensione era di 28 causando errore nel parsing di questo, per evitare che ciò accadda occore aggiungere l'attributo specificato nel codice che segue.

> L'allineamento tramite padding è utilizzato per garantire un accesso in memoria ottimizzato

```
typedef struct {
    uint64_t initiator_spi;   
    uint64_t responder_spi;  
    uint8_t next_payload; 
    uint8_t version;        
    uint8_t exchange_type; 
    uint8_t flags;        
    uint32_t message_id;  
    uint32_t length;     
} __attribute__((packed)) ike_header_t
```

#### Bit fields
// Campo di dimensione 1 bit questi sono i bit field permettono di gestire campi di una struttura con una dimensioni specifica in bit


### Puntatori

    //printf("%s\n", cfg->initiator.ip); //non possiamo accedere direttamente ai membri della struttura ma dobbiamo dereferenziare il puntatore e accedere (cosa che fa l'operatore ->)
    //il punto va utilizzato quando abbiamo l'istanza direttamente e non un puntatore
    //mentre la creazione del fd stabilisce quale è il punto di comunicazione cioè la sua esistenza 
    //il binding permette di configurare dove deve ascoltare il punto di comunicazion

    //definendo const in questo modo stiamo dicendo che il puntatore che ritorna e i dati che referenzia sono immutabili
const Config* get_config(void) {
    return &config;
}

### Socket 

`sockaddr_in` è la struct utilizzata per rappresentare un indirizzo e una porta per le comunicazioni tramite socket in IPv4. Nasce dalla combinazione di:

- `sockaddr`: è la struttura base e rappresenta un indirizzo IP generico. In cui andiamo a definire la famiglia dell'indirizzo (AF_INET, dove AF sta per Address Family) e i dati veri e propri
- `in`: sta per internet e indica che la struttura è specifica per il protocollo internet, quindi contiene campi specifici per IPv4.


- Per linkare la libreria cjson utilizzare il flag -lcjson durante la compilazione
- Per linkare la libreria openssl utilizzare i flag `-lssl -lcrypto` in fase di compilazion

## IKE


            //nel caso dell'ike_sa_init il next payload è SA ovvero quello della Security Association per settare i parametri
            //dopodichè come next payload di quello abbiamo quello del KE (ovvero Key exchang)
            //dopo di quello i nonce