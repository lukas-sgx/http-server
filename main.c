#include <stdio.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

int init(){
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(8080);

    int server_fd = socket(AF_INET, SOCK_STREAM, 0);

    if (server_fd < 0) {
        perror("socket");
        exit(EXIT_FAILURE);
    }

    if (bind(server_fd, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        perror("bind");
        exit(EXIT_FAILURE);
    }

    if (listen(server_fd, 10) < 0) {
        perror("listen");
        exit(EXIT_FAILURE);
    }

    return server_fd;
}

void openFile(const char *path, char *html, size_t buffer_size) {
    FILE* file = fopen(path, "r");
    if (!file) {
        strcpy(html, "<h1>Erreur: fichier non trouvé</h1>");
        return;
    }

    size_t read_len = fread(html, 1, buffer_size - 1, file);
    html[read_len] = '\0';

    fclose(file);
}

void route(char *path, char *response){
    const char *status;
    char html[2048];

    if (strcmp(path, "/") == 0) {
        openFile("src/index.html", html, sizeof(html));
        status = "200";
    } else if (strcmp(path, "/hello") == 0) {
        openFile("src/hello.html", html, sizeof(html));
        status = "200";
    } else {
        openFile("src/errors/404.html", html, sizeof(html));
        status = "404";
    }


    sprintf(response,
        "HTTP/1.1 %s\r\n"
        "Content-Type: text/html\r\n"
        "Content-Length: %lu\r\n"
        "\r\n"
        "%s",
        status, strlen(html), html);
}

int main(int argc, char const *argv[])
{
    int server_fd = init();
    
    char buffer[1024], method[8], path[256], response[2048];
    int req = 0;

    while (1)
    {
        int client_fd = accept(server_fd, NULL, NULL);
        if (client_fd < 0) {
            perror("accept");
            continue;
        }

        req = recv(client_fd, buffer, sizeof(buffer)-1, 0);
        buffer[req] = '\0';

        sscanf(buffer, "%s %s", method, path);

        route(path, response);

        send(client_fd, response, strlen(response), 0);

        close(client_fd);
    }
    

    close(server_fd);
    return 0;
}


