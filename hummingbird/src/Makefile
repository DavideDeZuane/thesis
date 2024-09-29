# Nome del target eseguibile
TARGET = hummingbird

# File sorgenti
SRCS = main.c \
       config/config.c \
       initiator/initiator.c \
       ike/header.c \
       initiator/responder.c \
       ike/payload.c \
       ike/network.c \
       ike/crypto.c

# Librerie da collegare
LIBS = -lcjson -lssl -lcrypto

# Opzioni di compilazione
# -Wall sta per warning all attiva tutti gli warning del compilatore
# gli altri flag specificano le directory all'interno delle quali cercare i file header
CFLAGS = -Wall -I./ike -I./config -I./initiator
LDFLAGS = $(LIBS)

# Regola principale per costruire il target, diciamo quali sono tutti i file sorgenti di cui ha bisogno
# CC è il comando del compilatore
# o indica il nome dell'output $@ è una variabile automatica in Makefile che rappresenta il target della regola corrente, in questo caso, $(TARGET), quindi il nome dell'eseguibile.
# $^ Rappresenta tutti i prerequisiti della regola. In questo caso, tutti i file sorgenti $(SRCS) 
$(TARGET): $(SRCS)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

# Regola per pulire i file generati
clean:
	rm -f $(TARGET)

# Regola predefinita
all: $(TARGET)
