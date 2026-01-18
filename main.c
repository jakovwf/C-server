/*
 * HTTP Echo Server - Čist konzolni output
 * Kompajliranje: gcc main.c -o main.exe -lws2_32
 * Pokretanje: .\main.exe
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <winsock2.h>

#pragma comment(lib, "ws2_32.lib")

#define PORT 8080
#define BUFFER_SIZE 8192

// URL dekodiranje
void urlDecode(char* dst, const char* src) {
    char a, b;
    while (*src) {
        if (*src == '+') {
            *dst++ = ' ';
            src++;
        } else if ((*src == '%') && ((a = src[1]) && (b = src[2])) && 
                   (isxdigit(a) && isxdigit(b))) {
            if (a >= 'a') a -= 'a' - 'A';
            if (a >= 'A') a -= ('A' - 10);
            else a -= '0';
            if (b >= 'a') b -= 'a' - 'A';
            if (b >= 'A') b -= ('A' - 10);
            else b -= '0';
            *dst++ = 16 * a + b;
            src += 3;
        } else {
            *dst++ = *src++;
        }
    }
    *dst = '\0';
}

// Izvlačenje parametra iz query stringa
char* getParam(char* query, const char* param) {
    static char value[1024];
    char* start = strstr(query, param);
    if (!start) return NULL;
    
    start += strlen(param) + 1;
    char* end = strchr(start, '&');
    
    if (end) {
        strncpy(value, start, end - start);
        value[end - start] = '\0';
    } else {
        strcpy(value, start);
    }
    
    char decoded[1024];
    urlDecode(decoded, value);
    strcpy(value, decoded);
    
    return value;
}

// HTML forma
void generateHTML(char* output, const char* message) {
    if (message == NULL || strlen(message) == 0) {
        sprintf(output,
            "HTTP/1.1 200 OK\r\n"
            "Content-Type: text/html; charset=UTF-8\r\n"
            "Connection: close\r\n"
            "\r\n"
            "<!DOCTYPE html>\n"
            "<html>\n"
            "<head>\n"
            "    <meta charset='UTF-8'>\n"
            "    <title>Echo Server</title>\n"
            "    <style>\n"
            "        body {\n"
            "            font-family: 'Segoe UI', Arial, sans-serif;\n"
            "            background: linear-gradient(135deg, #667eea 0%%, #764ba2 100%%);\n"
            "            display: flex;\n"
            "            justify-content: center;\n"
            "            align-items: center;\n"
            "            height: 100vh;\n"
            "            margin: 0;\n"
            "        }\n"
            "        .container {\n"
            "            background: white;\n"
            "            padding: 40px;\n"
            "            border-radius: 15px;\n"
            "            box-shadow: 0 10px 40px rgba(0,0,0,0.2);\n"
            "            max-width: 500px;\n"
            "            width: 90%%;\n"
            "        }\n"
            "        h1 {\n"
            "            color: #333;\n"
            "            text-align: center;\n"
            "            margin-bottom: 30px;\n"
            "        }\n"
            "        .form-group {\n"
            "            margin-bottom: 20px;\n"
            "        }\n"
            "        label {\n"
            "            display: block;\n"
            "            color: #555;\n"
            "            margin-bottom: 8px;\n"
            "            font-weight: 500;\n"
            "        }\n"
            "        input[type='text'] {\n"
            "            width: 100%%;\n"
            "            padding: 12px;\n"
            "            border: 2px solid #ddd;\n"
            "            border-radius: 8px;\n"
            "            font-size: 16px;\n"
            "            box-sizing: border-box;\n"
            "            transition: border 0.3s;\n"
            "        }\n"
            "        input[type='text']:focus {\n"
            "            outline: none;\n"
            "            border-color: #667eea;\n"
            "        }\n"
            "        button {\n"
            "            width: 100%%;\n"
            "            padding: 14px;\n"
            "            background: linear-gradient(135deg, #667eea 0%%, #764ba2 100%%);\n"
            "            color: white;\n"
            "            border: none;\n"
            "            border-radius: 8px;\n"
            "            font-size: 16px;\n"
            "            font-weight: 600;\n"
            "            cursor: pointer;\n"
            "            transition: transform 0.2s;\n"
            "        }\n"
            "        button:hover {\n"
            "            transform: translateY(-2px);\n"
            "        }\n"
            "        .info {\n"
            "            background: #f0f4ff;\n"
            "            padding: 15px;\n"
            "            border-radius: 8px;\n"
            "            margin-top: 20px;\n"
            "            font-size: 14px;\n"
            "            color: #555;\n"
            "        }\n"
            "    </style>\n"
            "</head>\n"
            "<body>\n"
            "    <div class='container'>\n"
            "        <h1>C Echo Server</h1>\n"
            "        <form action='/' method='GET'>\n"
            "            <div class='form-group'>\n"
            "                <label for='message'>Tvoja poruka:</label>\n"
            "                <input type='text' id='message' name='message' \n"
            "                       placeholder='Unesi poruku...' required>\n"
            "            </div>\n"
            "            <button type='submit'>Posalji serveru</button>\n"
            "        </form>\n"
            "        <div class='info'>\n"
            "            <strong>Kako radi:</strong><br>\n"
            "            Server je napisan u C jeziku i slusa na portu %d.<br>\n"
            "            Unesi poruku i server ce ti je vratiti nazad (echo)!\n"
            "        </div>\n"
            "    </div>\n"
            "</body>\n"
            "</html>", PORT);
    } else {
        sprintf(output,
            "HTTP/1.1 200 OK\r\n"
            "Content-Type: text/html; charset=UTF-8\r\n"
            "Connection: close\r\n"
            "\r\n"
            "<!DOCTYPE html>\n"
            "<html>\n"
            "<head>\n"
            "    <meta charset='UTF-8'>\n"
            "    <title>Odgovor servera</title>\n"
            "    <style>\n"
            "        body {\n"
            "            font-family: 'Segoe UI', Arial, sans-serif;\n"
            "            background: linear-gradient(135deg, #667eea 0%%, #764ba2 100%%);\n"
            "            display: flex;\n"
            "            justify-content: center;\n"
            "            align-items: center;\n"
            "            height: 100vh;\n"
            "            margin: 0;\n"
            "        }\n"
            "        .container {\n"
            "            background: white;\n"
            "            padding: 40px;\n"
            "            border-radius: 15px;\n"
            "            box-shadow: 0 10px 40px rgba(0,0,0,0.2);\n"
            "            max-width: 500px;\n"
            "            width: 90%%;\n"
            "            text-align: center;\n"
            "        }\n"
            "        h1 { color: #333; margin-bottom: 20px; }\n"
            "        .response {\n"
            "            background: #e8f5e9;\n"
            "            border-left: 4px solid #4caf50;\n"
            "            padding: 20px;\n"
            "            margin: 20px 0;\n"
            "            border-radius: 8px;\n"
            "            font-size: 18px;\n"
            "            color: #2e7d32;\n"
            "            word-wrap: break-word;\n"
            "        }\n"
            "        a {\n"
            "            display: inline-block;\n"
            "            margin-top: 20px;\n"
            "            padding: 12px 30px;\n"
            "            background: linear-gradient(135deg, #667eea 0%%, #764ba2 100%%);\n"
            "            color: white;\n"
            "            text-decoration: none;\n"
            "            border-radius: 8px;\n"
            "            font-weight: 600;\n"
            "            transition: transform 0.2s;\n"
            "        }\n"
            "        a:hover { transform: translateY(-2px); }\n"
            "    </style>\n"
            "</head>\n"
            "<body>\n"
            "    <div class='container'>\n"
            "        <h1>Server je odgovorio!</h1>\n"
            "        <div class='response'>\n"
            "            <strong>Echo odgovor:</strong><br>\n"
            "            \"%s\"\n"
            "        </div>\n"
            "        <a href='/'>Posalji novu poruku</a>\n"
            "    </div>\n"
            "</body>\n"
            "</html>", message);
    }
}

int main() {
    WSADATA wsaData;
    SOCKET serverSocket, clientSocket;
    struct sockaddr_in serverAddr, clientAddr;
    int clientAddrLen = sizeof(clientAddr);
    char buffer[BUFFER_SIZE];
    char response[BUFFER_SIZE];
    int requestCount = 0;

    // Inicijalizacija
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        printf("GRESKA: WSAStartup nije uspeo.\n");
        return 1;
    }

    serverSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (serverSocket == INVALID_SOCKET) {
        printf("GRESKA: Kreiranje socketa nije uspelo.\n");
        WSACleanup();
        return 1;
    }

    // Bind
    memset(&serverAddr, 0, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    serverAddr.sin_port = htons(PORT);

    if (bind(serverSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
        printf("GRESKA: bind() nije uspeo. Port %d je mozda zauzet.\n", PORT);
        closesocket(serverSocket);
        WSACleanup();
        return 1;
    }

    // Listen
    if (listen(serverSocket, 10) == SOCKET_ERROR) {
        printf("GRESKA: listen() nije uspeo.\n");
        closesocket(serverSocket);
        WSACleanup();
        return 1;
    }

    // Čist, čitljiv banner
    printf("========================================================\n");
    printf("  HTTP Echo Server - Pokrenut na portu %d\n", PORT);
    printf("========================================================\n\n");
    printf("Otvori browser: http://localhost:%d\n\n", PORT);
    printf("Pritisnite Ctrl+C za izlaz.\n");
    printf("--------------------------------------------------------\n\n");

    while (1) {
        clientSocket = accept(serverSocket, (struct sockaddr*)&clientAddr, &clientAddrLen);
        if (clientSocket == INVALID_SOCKET) continue;

        requestCount++;

        // Primi HTTP zahtev
        memset(buffer, 0, BUFFER_SIZE);
        int recvSize = recv(clientSocket, buffer, BUFFER_SIZE - 1, 0);
        
        if (recvSize > 0) {
            buffer[recvSize] = '\0';

            // Parsiranje GET zahteva
            char* requestLine = strtok(buffer, "\r\n");
            if (requestLine) {
                // Pronađi query string
                char* queryStart = strchr(requestLine, '?');
                char* message = NULL;

                if (queryStart) {
                    char* httpVersion = strstr(queryStart, " HTTP");
                    if (httpVersion) *httpVersion = '\0';
                    message = getParam(queryStart, "message");
                }

                // Konzolni output
                printf("[Zahtev #%d]\n", requestCount);
                printf("  Od:      %s:%d\n", 
                       inet_ntoa(clientAddr.sin_addr), 
                       ntohs(clientAddr.sin_port));
                printf("  Metoda:  %s\n", requestLine);
                
                if (message) {
                    printf("  Poruka:  \"%s\"\n", message);
                    printf("  Status:  Echo odgovor poslat\n");
                } else {
                    printf("  Status:  HTML forma poslata\n");
                }
                printf("\n");

                // Generiši i pošalji odgovor
                memset(response, 0, BUFFER_SIZE);
                generateHTML(response, message);
                send(clientSocket, response, strlen(response), 0);
            }
        }

        closesocket(clientSocket);
    }

    closesocket(serverSocket);
    WSACleanup();
    return 0;
}

/*
 * ========================================================
 *  KOMPAJLIRANJE I POKRETANJE
 * ========================================================
 * 
 * 1. Kompajliraj:
 *    gcc main.c -o main.exe -lws2_32
 * 
 * 2. Pokreni:
 *    .\main.exe
 * 
 * 3. Otvori browser:
 *    http://localhost:8080
 * 
 * ========================================================
 *  OČEKIVANI OUTPUT
 * ========================================================
 * 
 * ========================================================
 *   HTTP Echo Server - Pokrenut na portu 8080
 * ========================================================
 * 
 * Otvori browser: http://localhost:8080
 * 
 * Pritisnite Ctrl+C za izlaz.
 * --------------------------------------------------------
 * 
 * [Zahtev #1]
 *   Od:      127.0.0.1:52341
 *   Metoda:  GET / HTTP/1.1
 *   Status:  HTML forma poslata
 * 
 * [Zahtev #2]
 *   Od:      127.0.0.1:52342
 *   Metoda:  GET /?message=Hello HTTP/1.1
 *   Poruka:  "Hello"
 *   Status:  Echo odgovor poslat
 * 
 * ========================================================
 */