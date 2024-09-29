L'architettura x86 lavora in little endian, tuttavia tutto lo stack TCP/IP delle reti lavora in big endian è quindi necessaria tutta la conversione.
Va utilizzata la libreria `endian.h`

Mentre quando inviamo dobbiamo fare la conversione da little endian a big endian
In ricezione dobbiamo fare il contrario attenzione a questo



Codici di Colore ANSI

    Codice di Colore per il Testo:
        Nero: \033[0;30m
        Rosso: \033[0;31m
        Verde: \033[0;32m
        Giallo: \033[0;33m
        Blu: \033[0;34m
        Magenta: \033[0;35m
        Ciano: \033[0;36m
        Bianco: \033[0;37m

    Codice di Colore per il Testo in Grassetto:
        Nero: \033[1;30m
        Rosso: \033[1;31m
        Verde: \033[1;32m
        Giallo: \033[1;33m
        Blu: \033[1;34m
        Magenta: \033[1;35m
        Ciano: \033[1;36m
        Bianco: \033[1;37m

    Codici di Colore per lo Sfondo:
        Nero: \033[0;40m
        Rosso: \033[0;41m
        Verde: \033[0;42m
        Giallo: \033[0;43m
        Blu: \033[0;44m
        Magenta: \033[0;45m
        Ciano: \033[0;46m
        Bianco: \033[0;47m

    Reset del Colore:
        Reset: \033[0m
