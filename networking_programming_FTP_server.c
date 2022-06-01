#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <stdio.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#define IPADDR          "IP address"          // IP address for connection ftp server
#define FTPPORT         21                      // port for connection ftp server

#define MAXBUF          1024                    // max size of buffer(1k)

#define COMUSER         "USER id\r\n"         // USER command with connection ID
#define COMPASS         "PASS pass\r\n"         // PASS command with connection password
#define COMPASV         "PASV\r\n"              // PASV command
#define COMRETR         "RETR textfile.txt\r\n"  // RETR command with fixed file name

void dec_to_hex(int dec, char* ftp_port_hex);

int main(int argc, char* argv[]) {
        int socket_fd;                          // variable for creat socket
        int file_count;                         // variable for count file size for buf size

        struct sockaddr_in ftpServer;           // struct for connect ftp server
        char buf[MAXBUF];                       // buffer for read file
        char *temp;                             // temporary variable for using strcat

        int file_count_num = 0;                 // variable for count number of file read
        int strtok_count_num = 0;               // variable for count number of strtok result
        char ftp_port_hex[10] = "";             // ftp port using hexa-decimal
        unsigned int ftp_port_dec;              // ftp port using decimal

        /* create socket using TCP */
        socket_fd = socket(AF_INET, SOCK_STREAM, 0);

        /* if creating socket is error, finish program */
        if ((socket_fd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
                fprintf(stderr, "socket create error.\n");
                return 1;
        } else {
                fprintf(stderr, "socket create success!\n");
        }

        /* setting struct */
        ftpServer.sin_family = AF_INET;                 // internet adress family
        ftpServer.sin_addr.s_addr = inet_addr(IPADDR);  // input fixed IP address
        ftpServer.sin_port = htons(FTPPORT);            // input fixed port using byte order function

        /* connect to server and if error occurs, finish program */
        if (connect(socket_fd, (struct sockaddr *)&ftpServer, sizeof(ftpServer)) == -1) {
                fprintf(stderr, "connect error\n");
                return 1;
        } else {
                fprintf(stderr, "connect success!\n");
        }

        /* send command USER, PASS, PASV */
        send(socket_fd, COMUSER, strlen(COMUSER), 0);
        send(socket_fd, COMPASS, strlen(COMPASS), 0);
        send(socket_fd, COMPASV, strlen(COMPASV), 0);

        while (file_count = read(socket_fd, buf, MAXBUF) > 0) {
                /* count_num = 0 -> 331 Please specify the password.
                 * count_num = 1 -> 230 Login successful.
                 * count_num = 2 -> 227 Entering Passive Mode (IP address, ?, ?). */

                if (file_count_num++ == 2) {
                        printf("%s\n", buf);
                        temp = strtok(buf, ",");        // cut string with ','
                        while (temp != NULL) {
                                strtok_count_num++;
                                /* strtok_count_num = 1 -> IP address
                                 * strtok_count_num = 2 -> IP address
                                 * strtok_count_num = 3 -> IP address
                                 * strtok_count_num = 4 -> IP address
                                 * strtok_count_num = 5 -> 1st ftp port
                                 * strtok_count_num = 6 -> 2nd ftp port
                                 * */
                                if (strtok_count_num == 5) {
                                        printf("1st ftp port(dec -> hex): %s -> ", temp);
                                        dec_to_hex(atoi(temp), ftp_port_hex);
                                } else if (strtok_count_num == 6) {
                                        temp = strtok(temp, ")");
                                        printf("2nd ftp port(dec -> hex): %s -> ", temp);
                                        dec_to_hex(atoi(temp), ftp_port_hex);
                                }
                                temp = strtok(NULL, ",");
                        }
                }
                /* finish to get port, end loop */
                if (strtok_count_num >= 6) break;
        }
        
        /* create 2nd socket using TCP */
        int socket_fd2 = socket(AF_INET, SOCK_STREAM, 0);
        struct sockaddr_in ftpServer2;          // 2nd struct for connect ftp server

        if ((socket_fd2 = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
                fprintf(stderr, "\nsocket2 create error.\n");
                return 1;
        } else {
                fprintf(stderr, "\nsocket2 create success!\n");
        }

        /* setting 2nd struct */
        ftpServer2.sin_family = AF_INET;
        ftpServer2.sin_addr.s_addr = inet_addr(IPADDR);

        /* change hexa-decimal port to decimal port that can get "welcome.txt" file */
        ftp_port_dec = strtol(ftp_port_hex, NULL, 16);
        printf("final ftp port(hex -> dec): %s -> %d\n", ftp_port_hex, ftp_port_dec);
        ftpServer2.sin_port = htons(ftp_port_dec);      // input new port

        /* 2nd connection to server with new port */
        if (connect(socket_fd2, (struct sockaddr *)&ftpServer2, sizeof(ftpServer2)) == -1) {
                fprintf(stderr, "connect2 error\n");
                return 1;
        } else {
                fprintf(stderr, "connect2 success!\n");
        }

        /* send RETR command */
        send(socket_fd, COMRETR, strlen(COMRETR), 0);

        /* reset buffer and read "welcome.txt" */
        memset(buf, 0x00, MAXBUF);
        while (file_count = read(socket_fd2, buf, MAXBUF) > 0) {
                printf("%s\n", buf);
        }

        close(socket_fd);
        close(socket_fd2);
        return 0;
}

/* change integer decimal to char array hexa-decimal
 * use call by reference for change char array value
 *
 * INPUT: decimal(integer), hexa-decimal(char array address)
 * Change decimal to hexa-decimal and concatenate result */

void dec_to_hex(int dec, char* ftp_port_hex) {
        int store_hex = 0;                      // array index
        int quo, rem;                           // quotient, remainder
        char sym_hex[10] = "";                  // symmetry hexa result
        char hex[10] = "";                      // normal hexa result
        char hex_char[] = "0123456789ABCDEF";   // element array of hexa

        while (1) {
                quo = dec/16;                   // calculate quotient
                rem = dec - (quo * 16);         // calculate remainder
                sym_hex[store_hex++] = hex_char[rem];   // store hexa element
                if (quo <= 0) break;            // if quotient <= 0, end loop
                dec = quo;
        }

        for(int i = --store_hex; i >= 0; i--) { // make symmetry hexa to normal
                hex[store_hex - i] = sym_hex[i];
        }

        printf("%s\n", hex);
        strcat(ftp_port_hex, hex);              // concatenate hexa result for get port
}