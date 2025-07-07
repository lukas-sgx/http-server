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

char *openFile(const char *path) {
    FILE *file = fopen(path, "r");
    if (!file) {
        return strdup("<h1>Erreur : fichier introuvable</h1>");
    }

    size_t size = 0;
    size_t capacity = 1024;
    char *html = malloc(capacity);
    if (!html) {
        fclose(file);
        return NULL;
    }

    size_t bytes;
    while ((bytes = fread(html + size, 1, capacity - size - 1, file)) > 0) {
        size += bytes;
        if (capacity - size <= 1) {
            capacity *= 2;
            char *tmp = realloc(html, capacity);
            if (!tmp) {
                free(html);
                fclose(file);
                return NULL;
            }
            html = tmp;
        }
    }

    html[size] = '\0';
    fclose(file);
    return html;
}

void route(char *path, char *response){
    const char *status;
    char *html = NULL;

    if (strcmp(path, "/") == 0) {
        html = openFile("src/index.html");
        status = "200";
    } else if (strcmp(path, "/hello") == 0) {
        html = openFile("src/hello.html");
        status = "200";
    } else {
        html = openFile("src/errors/404.html");
        status = "404";
    }


    sprintf(response,
        "HTTP/1.1 %s\r\n"
        "Content-Type: text/html\r\n"
        "Content-Length: %lu\r\n"
        "\r\n"
        "%s",
        status, strlen(html), html);

    free(html);
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


