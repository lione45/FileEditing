#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <netdb.h>
#include <pthread.h>
#include <signal.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>


#define MSG_COMMAND_ERROR "Errore: Nel Comando ./client Mancano <IP_SERVER> E <PORT_SERVER>. Esecuzione Corretta: ./client <IP_SERVER> E <PORT_SERVER>.\n"

#define MSG_CLIENT_WELCOME "\tBenvenuto Nel Remote File Editing Di:\n\t\tMattia Golino N86001982;\n\t\tDavide Belardi N86001864.\n"

#define MSG_ERROR_THCREATE "Errore: Non Sono Riuscito A Creare Un Nuovo Thread.\n"

#define MSG_ERROR_PORT "Erorre La Porta Inserita Non è Valida.\n"

#define MAXLEN 6000

int Client_Socket, Port_n, Connection;
off_t dim;

void Check_Condition_Client(int, char **);
void Errore(const char *);
void *Gestione_Client(void *);
char Pulisci_buffer(char []);
char menu_iniziale(char []);
char Decidi_Da_Dove_Leggere(char []);
char Decidi_Da_Dove_Scrivere(char []);
char Da_Quale_Lunghezza_Leggere(char []);

int main(int argc, char **argv){
	if(argc < 3){
	}else{
		system("clear");
		int ret;
		pthread_t th_id;
		struct sockaddr_in Server_Addr;
		struct hostent *Server;
		Server = gethostbyname(argv[1]);
		Port_n = atoi(argv[2]);
		if((Client_Socket = socket(AF_INET, SOCK_STREAM, 0)) < 0){
			Errore("[-]Errorre Nell'Apertura Del Socket.\n");
		}else{
			write(1, "[+]Il Socket Del Client è Stato Creato.\n", strlen("[+]Il Socket Del Client è Stato Creato.\n"));
		}
		memset(&Server_Addr, '\0', sizeof(Server_Addr));
		Server_Addr.sin_family = AF_INET;
		Server_Addr.sin_port = htons(Port_n);
		memcpy(&Server_Addr.sin_addr, Server->h_addr, Server->h_length);
		if((Connection = connect(Client_Socket, (struct sockaddr *)&Server_Addr, sizeof(Server_Addr))) < 0){
			Errore("[-]Errore Di Connessione Al Server.\n");
		}else{
			write(1, "[+]Connesso Al Server.\n\n", strlen("[+]Connesso Al Server.\n\n"));
		}
		while(1){
			char buffer[MAXLEN];
			bzero(buffer, sizeof(buffer));
			write(1, MSG_CLIENT_WELCOME, sizeof(MSG_CLIENT_WELCOME));
			*buffer = menu_iniziale(buffer);
			*buffer = Pulisci_buffer(buffer);
			write(Client_Socket, buffer, sizeof(buffer));
			if(strcmp(buffer, "Scrivi") == 0 || strcmp(buffer, "scrivi") == 0){
				bzero(buffer, sizeof(buffer));
				*buffer = Decidi_Da_Dove_Scrivere(buffer);
				*buffer = Pulisci_buffer(buffer);
				write(Client_Socket, buffer, sizeof(buffer));
				if(strcmp(buffer, "Append") == 0 || strcmp(buffer, "append") == 0){
					if(read(Client_Socket, buffer, sizeof(buffer)) <= sizeof(buffer)){
						write(1, buffer, sizeof(buffer));
						bzero(buffer, sizeof(buffer));
						fgets(buffer, MAXLEN, stdin);
						write(Client_Socket, buffer, sizeof(buffer));
						bzero(buffer, sizeof(buffer));
						if(read(Client_Socket, buffer, sizeof(buffer)) <= sizeof(buffer)){
							write(1, buffer, sizeof(buffer));
							bzero(buffer, sizeof(buffer));
						}
					}
				}else if(strcmp(buffer, "OffSet") == 0 || strcmp(buffer, "offset") == 0){
					if(read(Client_Socket, buffer, sizeof(buffer)) <= sizeof(buffer)){
						write(1, buffer, sizeof(buffer));
						bzero(buffer, sizeof(buffer));
						fgets(buffer, MAXLEN, stdin);
						*buffer = Pulisci_buffer(buffer);
						write(Client_Socket, buffer, sizeof(buffer));
						bzero(buffer, sizeof(buffer));
						if(read(Client_Socket, buffer, sizeof(buffer)) <= sizeof(buffer)){
							write(1, buffer, sizeof(buffer));
							bzero(buffer, sizeof(buffer));
							fgets(buffer, MAXLEN, stdin);
							write(Client_Socket, buffer, sizeof(buffer));
							bzero(buffer, sizeof(buffer));
							if(read(Client_Socket, buffer, sizeof(buffer)) <= sizeof(buffer)){
								write(1, buffer, sizeof(buffer));
								bzero(buffer, sizeof(buffer));
							}
						}
					}
				}else{
					bzero(buffer, sizeof(buffer));
					if(read(Client_Socket, buffer, sizeof(buffer)) <= sizeof(buffer)){
						write(1, buffer, sizeof(buffer));
						bzero(buffer, sizeof(buffer));
					}
				}
			}else if(strcmp(buffer, "Leggi") == 0 || strcmp(buffer, "leggi") == 0){
				bzero(buffer, sizeof(buffer));
				*buffer = Decidi_Da_Dove_Leggere(buffer);
				*buffer = Pulisci_buffer(buffer);
				write(Client_Socket, buffer, sizeof(buffer));
				if(strcmp(buffer, "Inizio") == 0 || strcmp(buffer, "inizio") == 0){
					bzero(buffer, sizeof(buffer));
					if(read(Client_Socket, buffer, sizeof(buffer)) <= sizeof(buffer)){
						write(1,"\n", sizeof("\n"));
						write(1, buffer, strlen(buffer));
						bzero(buffer, sizeof(buffer));
					}
				}else if(strcmp(buffer, "OffSet") == 0 || strcmp(buffer, "offset") == 0){
						*buffer = Da_Quale_Lunghezza_Leggere(buffer);
						*buffer = Pulisci_buffer(buffer);
						write(Client_Socket, buffer, sizeof(buffer));
						if(strcmp(buffer, "Byte") == 0 || strcmp(buffer, "byte") == 0){
							bzero(buffer, sizeof(buffer));
							read(Client_Socket, buffer, sizeof(buffer));
							write(1, buffer, sizeof(buffer));
							bzero(buffer, sizeof(buffer));
							fgets(buffer, MAXLEN, stdin);
							*buffer = Pulisci_buffer(buffer);
							write(Client_Socket, buffer, sizeof(buffer));
							bzero(buffer, sizeof(buffer));
							if(read(Client_Socket, buffer, sizeof(buffer)) <= sizeof(buffer)){
								write(1,"\n", sizeof("\n"));
								write(1, buffer, strlen(buffer));
								bzero(buffer, sizeof(buffer));
							}
						}else if(strcmp(buffer, "Stringa") == 0 || strcmp(buffer, "stringa") == 0){
							bzero(buffer, sizeof(buffer));
							read(Client_Socket, buffer, sizeof(buffer));
							write(1, buffer, sizeof(buffer));
							bzero(buffer, sizeof(buffer));
							fgets(buffer, MAXLEN, stdin);
							*buffer = Pulisci_buffer(buffer);
							write(Client_Socket, buffer, sizeof(buffer));
							read(Client_Socket, buffer, sizeof(buffer));
							write(1, buffer, sizeof(buffer));
							bzero(buffer, sizeof(buffer));
							fgets(buffer, MAXLEN, stdin);
							*buffer = Pulisci_buffer(buffer);
							write(Client_Socket, buffer, sizeof(buffer));
							memset(buffer, '\0', MAXLEN);
							if(read(Client_Socket, buffer, sizeof(buffer)) <= sizeof(buffer)){
								write(1,"\n", sizeof("\n"));
								write(1, buffer, sizeof(buffer));
								bzero(buffer, sizeof(buffer));
							}
						}else{
							bzero(buffer, sizeof(buffer));
							if(read(Client_Socket, buffer, sizeof(buffer)) <= sizeof(buffer)){
								write(1, buffer, sizeof(buffer));
								bzero(buffer, sizeof(buffer));
							}
						}
				}else{
					bzero(buffer, sizeof(buffer));
					if(read(Client_Socket, buffer, sizeof(buffer)) <= sizeof(buffer)){
						write(1, buffer, sizeof(buffer));
						bzero(buffer, sizeof(buffer));
					}
				}
			}else if(strcmp(buffer, "Dimensione") == 0 || strcmp(buffer, "dimensione") == 0){
				bzero(buffer, sizeof(buffer));
				if(read(Client_Socket, buffer, sizeof(buffer)) <= sizeof(buffer)){
					write(1, buffer, sizeof(buffer));
					bzero(buffer, sizeof(buffer));
				}
			}else if(strcmp(buffer, "Esci") == 0 || strcmp(buffer, "esci") == 0){
					bzero(buffer, sizeof(buffer));
					if(read(Client_Socket, buffer, sizeof(buffer)) <= sizeof(buffer)){
						write(1, buffer, sizeof(buffer));
						bzero(buffer, sizeof(buffer));
						break;
					}
			}else{
				bzero(buffer, sizeof(buffer));
				if(read(Client_Socket, buffer, sizeof(buffer)) <= sizeof(buffer)){
					write(1, buffer, sizeof(buffer));
					bzero(buffer, sizeof(buffer));
				}
			}
		}
	}
	close(Client_Socket);
	return 0;
}

void Errore(const char *msg){
	perror(msg);
	exit(1);
}

char Pulisci_buffer(char p[]){
	int dim;
	if(strrchr(p, '\n')){
		dim = strlen(p) - 1;
		p[dim]  = '\0';
	}
	return *p;
}

char menu_iniziale(char a[]){
	printf(" ____________________________________________________________\n|Scegli Cosa Fare:\t\t\t\t\t     |\n|____________________________________________________________|\n|\t\t\t\t\t\t\t     |\n");
	printf("|Scrivi)Scrivi Nel File;\t\t\t\t     |\n|Leggi)Leggi Nel File;\t\t\t\t\t     |\n|Dimensione)Visualizza La Dimensione Del File;\t\t     |\n|Esci)Esci Dal Programma;\t\t\t\t     |");
	printf("\n|____________________________________________________________|\n");
	write(1,"\nQual e' La Tua Risposta:", strlen("\nQual e' La Tua Risposta:"));
	fgets(a, MAXLEN, stdin);
	return *a;
}

char Decidi_Da_Dove_Leggere(char a[]){
	printf(" ____________________________________________________________\n|Scegli Cosa Fare:\t\t\t\t\t     |\n|____________________________________________________________|\n|\t\t\t\t\t\t\t     |\n");
	printf("|Inizio)Leggi Il File Dall'Inizio;\t\t\t     |\n|OffSet)Decidi Da Dove Far Iniziare E Finire La Lettura;     |");
	printf("\n|____________________________________________________________|\n");
	write(1,"\nQual e' La Tua Risposta:", strlen("\nQual e' La Tua Risposta:"));
	fgets(a, MAXLEN, stdin);
	return *a;
}

char Da_Quale_Lunghezza_Leggere(char a[]){
	printf(" ____________________________________________________________\n|Scegli Cosa Fare:\t\t\t\t\t     |\n|____________________________________________________________|\n|\t\t\t\t\t\t\t     |\n");
	printf("|Byte)Leggi Da Un Determinato Byte Fino Alla Fine;\t     |\n|Stringa)Decidi Da Quale E Fino A Quale Byte Leggere;\t     |");
	printf("\n|____________________________________________________________|\n");
	write(1,"\nQual e' La Tua Risposta:", strlen("\nQual e' La Tua Risposta:"));
	fgets(a, MAXLEN, stdin);
	return *a;
}

char Decidi_Da_Dove_Scrivere(char a[]){
	printf(" ____________________________________________________________\n|Scegli Cosa Fare:\t\t\t\t\t     |\n|____________________________________________________________|\n|\t\t\t\t\t\t\t     |\n");
	printf("|Append)Srivi Nel File Da Dove è Stato Interrotto;\t     |\n|OffSet)Decidi Da Dove Far Iniziare La Scrittura;\t     |");
	printf("\n|____________________________________________________________|\n");
	write(1,"\nQual e' La Tua Risposta:", strlen("\nQual e' La Tua Risposta:"));
	fgets(a, MAXLEN, stdin);
	return *a;
}

void Check_Condition_Client(int argc, char **argv){
        if(argc != 3){
                Errore(MSG_COMMAND_ERROR);
        }
        int port = atoi(argv[2]);
        if(port < 1024 || port > 65525){
                Errore(MSG_ERROR_PORT);
        }
}
