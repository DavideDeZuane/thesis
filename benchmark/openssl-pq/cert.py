import os
import base64
import Crypto
from Crypto.PublicKey import RSA

def pem_file_size_in_bits(file_path):
    with open(file_path, 'r') as file:
        pem_data = file.read()
        # Rimuovi l'header e il footer PEM
        pem_data = pem_data.replace('-----BEGIN PUBLIC KEY-----', '')
        pem_data = pem_data.replace('-----END PUBLIC KEY-----', '')
        # Decodifica il contenuto PEM da base64
        decoded_data = base64.b64decode(pem_data)
        # Calcola la dimensione del contenuto decodificato in bit
        decoded_size_bits = len(decoded_data) * 8
        return decoded_size_bits

cert_path='public.pem'

with open(cert_path, 'r') as f:
    cert_data = f.read()

cert = RSA.import_key(cert_data)
key_size = cert.size_in_bits()
        
        # Ottieni il modulo (n) e l'esponente pubblico (e)
n = cert.n.bit_length()
e = cert.e.bit_length()
size = pem_file_size_in_bits(cert_path)
        
        # Stampa le informazioni
print(f"Dimensione della chiave pubblica: {size} bit")
print(f"Modulo (n): {n}")
print(f"Esponente pubblico (e): {e}")
