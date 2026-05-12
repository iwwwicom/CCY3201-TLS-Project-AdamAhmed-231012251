# CCY3201 - Networks Security - Project 12
**Arab Academy for Science, Technology and Maritime Transport**  
**College of Computing and Information Technology**  
**Course:** Networks Security (CCY3201)  
**TA:** Marwa Alazab‏

---

## Part 1 - TLS (HTTPS using OpenSSL)

### Overview
A simple HTTPS web application built using OpenSSL in C. The server hosts a basic HTML page served over TLS. The client connects to the server, performs a TLS handshake, and receives the HTML response. Traffic is captured and decrypted using Wireshark.

---

### Environment
| Machine | OS | IP |
|---|---|---|
| Server | Linux Mint | 192.168.1.6 |
| Client | Linux Mint | 192.168.1.5 |

---

### Step 1 - Generate Certificates

All certificates are signed by a Root CA generated using OpenSSL.

**Generate Root CA:**
```bash
openssl genrsa -out ~/tls-project/ca/ca.key 4096
openssl req -new -x509 -days 3650 -key ~/tls-project/ca/ca.key \
  -out ~/tls-project/ca/ca.crt \
  -subj "/C=EG/ST=Alexandria/O=AAST/CN=MyRootCA"
```

**Generate Server Certificate:**
```bash
openssl genrsa -out ~/tls-project/server/server.key 2048
openssl req -new -key ~/tls-project/server/server.key \
  -out ~/tls-project/server/server.csr \
  -subj "/C=EG/ST=Alexandria/O=AAST/CN=server.local"
openssl x509 -req -days 365 \
  -in ~/tls-project/server/server.csr \
  -CA ~/tls-project/ca/ca.crt \
  -CAkey ~/tls-project/ca/ca.key \
  -CAcreateserial \
  -out ~/tls-project/server/server.crt
```

**Generate Client Certificate:**
```bash
openssl genrsa -out ~/tls-project/client/client.key 2048
openssl req -new -key ~/tls-project/client/client.key \
  -out ~/tls-project/client/client.csr \
  -subj "/C=EG/ST=Alexandria/O=AAST/CN=client.local"
openssl x509 -req -days 365 \
  -in ~/tls-project/client/client.csr \
  -CA ~/tls-project/ca/ca.crt \
  -CAkey ~/tls-project/ca/ca.key \
  -CAcreateserial \
  -out ~/tls-project/client/client.crt
```

**Verify both certificates:**
```bash
openssl verify -CAfile ~/tls-project/ca/ca.crt ~/tls-project/server/server.crt
openssl verify -CAfile ~/tls-project/ca/ca.crt ~/tls-project/client/client.crt
```

**How certificates and private keys are stored securely:**
- Private keys are stored with `chmod 600` so only the owner can read them
- CA private key is kept only on the server and never shared
- Certificates (.crt) are public and can be shared freely
- Private keys are never committed to GitHub (see .gitignore)

---

### Step 2 - TLS Client/Server Application

The server and client are written in C using the OpenSSL library.

**Compile server:**
```bash
cd ~/tls-project/server
gcc server.c -o server -lssl -lcrypto
```

**Compile client:**
```bash
cd ~/tls-project/client
gcc client.c -o client -lssl -lcrypto
```

**Run server:**
```bash
./server
```

**Run client:**
```bash
./client 192.168.1.6
```

**Result:**
```
Connected! TLS: TLSv1.2 | Cipher: ECDHE-RSA-AES256-GCM-SHA384
HTTP/1.1 200 OK
Content-Type: text/html
Content-Length: 90
Connection: close
<html><body><h1>Hello from TLS Server</h1><p>CCY3201 - Networks Security</p></body></html>
```

---

### Step 3 - Capture TLS Traffic with Wireshark

Wireshark was run on the client VM during the TLS connection.

- Interface: `ens33`
- Filter: `tcp.port == 4433`
- Saved as: `tls_client_server_231012251.pcap`

The capture shows the full TLS handshake: TCP handshake → ClientHello → ServerHello → Certificate → Finished → encrypted Application Data.

---

### Step 4 - Decrypt TLS Traffic

The client code uses `SSL_CTX_set_keylog_callback` to write session keys to `tls_keys.log`. This file is loaded into Wireshark to decrypt the traffic.

**In Wireshark:**
- Edit → Preferences → Protocols → TLS
- Set log file to: `/home/adam/tls-project/client/tls_keys.log`

After loading the key log, Wireshark shows the plain HTTP GET request and HTTP 200 response.

- Saved as: `tls_decrypted_client_server_231012251.pcap`

---

### Step 5 - TLS Handshake Explanation

| Field | Value |
|---|---|
| TLS Version | TLSv1.2 |
| Cipher Suite | ECDHE-RSA-AES256-GCM-SHA384 |
| Key Exchange | ECDHE (Elliptic Curve Diffie-Hellman Ephemeral) |
| Authentication | RSA |
| Encryption | AES-256-GCM |
| MAC | SHA-384 |

**Handshake steps:**
1. **ClientHello** - Client sends supported TLS versions and cipher suites
2. **ServerHello** - Server picks TLS version and cipher suite
3. **Certificate** - Server sends its certificate for the client to verify
4. **ServerHelloDone** - Server signals end of hello phase
5. **ClientKeyExchange** - Client sends key exchange data
6. **ChangeCipherSpec** - Both sides switch to encrypted communication
7. **Finished** - Handshake complete, application data flows encrypted

---

### Project Structure
```
tls-project/
├── ca/
│   └── ca.crt
├── server/
│   ├── server.c
│   ├── server.crt
│   └── tls_keys.log
├── client/
│   ├── client.c
│   ├── client.crt
│   └── tls_keys.log
└── captures/
    ├── tls_client_server_231012251.pcap
    └── tls_decrypted_client_server_231012251.pcap
```

---

### Notes
- Private keys are not uploaded to GitHub
- All certificates are self-signed by our own Root CA
- TLS 1.2 was used for compatibility with Wireshark decryption
