#include "networking.h"

void handle_response(struct response * res, int server_socket){
    //displays buffer
    if(res->type == RES_DISP){
        printf("%s\n", res->body);
    }
    //edits file locally
    else if(res->type == RES_EDIT){
        //creates dummy file for editing
        int fd = open(".temp_editing", O_WRONLY | O_CREAT, 0664);
        write(fd, res->body, strlen(res->body));
        close(fd);

        //opens emacs
        if( !fork() ){
            char * args[4] = {"emacs", "-nw", ".temp_editing", NULL};
            execvp("emacs", args);
        } else {
            int status;
            wait( &status );

            //writes edited file to socket
            char * edited = calloc(1, BUFFER_SIZE);
            int edited_fd = open(".temp_editing", O_RDONLY);
            read(edited_fd, edited, BUFFER_SIZE);
            write(server_socket, edited, BUFFER_SIZE);
            remove(".temp_editing");
            remove(".temp_editing~");
            close(edited_fd);
            free(edited);

            //displays server confirmation of succesfully edited file
            struct response * response_buffer = calloc(1, sizeof(struct response));
            read(server_socket, response_buffer, BUFFER_SIZE);
            printf("%s\n", response_buffer->body);
        }
    }
}

int main(int argc, char **argv) {
    int server_socket;

    if (argc == 2){
      server_socket = client_setup( argv[1]);
    }
    else{
      server_socket = client_setup( "127.0.0.1" );
    }

    while(1){
        char * buffer = calloc(BUFFER_SIZE, sizeof(char *));
        printf("slicky-wiki$ ");
        fgets(buffer, BUFFER_SIZE, stdin);
        buffer[strlen(buffer) - 1] = 0;

        write(server_socket, buffer, BUFFER_SIZE);

        struct response * res = calloc(1, sizeof(struct response));
        read(server_socket, res, BUFFER_SIZE);

        handle_response(res, server_socket);
    }
}
