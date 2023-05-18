#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <netinet/in.h>
#include <unistd.h>
#include <pthread.h>
#include <arpa/inet.h>
#include <signal.h>
#include <fcntl.h>

#define MSG_ERROR_COMMAND "Errore: Nel Comando ./server Mancano <PORT_SERVER>, <FILE_NAME> E <MAX_LEN>.\nEsecuzione Corretta Del Comando: ./server <PORT_SERVER> <FILE_NAME> <MAX_LEN>.\n"

#define MSG_ERROR_PORT "Errore: La Porta Inserita Non è Valida.\n"

#define MSG_ERROR_PERMISSION "Errore: Non Siete Autorizzati a Modificare Questo File.\n"

#define MSG_ERROR_FILE "Errore: Il File Inserito Non Esiste.\n"

#define MSG_ERROR_LENGTH "Errore: La Dimensione Inserita è Minore Della Dimensione Attuale Del File.\n"

#define MSG_ERROR_THCREATE "Errore: Non Sono Riuscito A Creare Un Nuovo Thread.\n"

#define MSG_CLOSE_SERVER_NO_USER "[-]Tutti I Client Connessi Si Sono Disconnessi. Il Server Verrà Chiuso.\n"

#define MAXLEN 6000

struct stat file;
int Sock_fd, New_Sock_fd, Port_n, ret, n_connessioni = 0;
off_t dim;
pthread_mutex_t mutex_n_connessioni = PTHREAD_MUTEX_INITIALIZER;

typedef struct nodo{
	pthread_t th_id;
	int socket;
	int porta;
	char *indirizzo_client;
	char buffer[MAXLEN];
	char titolo[15];
	off_t dimMax;
}Connessione;

typedef struct lista{
	Connessione info;
	struct lista *next;
}Processo;

Processo *nuovo_nodo();
Processo *inserisci_nodo();
Processo *Chiusura_Connessione(Processo *, int);
void Check_Condition(int, char **);
void Errore(const char *);
void *Gestione_Server(void *);
void Check_Condition(int, char **);
int File_Editing(Processo *);
char Funzione_Dimensione(char [], char []);
char Funzione_Scrivi_Append(char [], char [], off_t);
char Funzione_Scrivi_Da_Offset(char [], char [], off_t, long);
char Funzione_Leggi_Da_Un_Certo_Byte(char [], char [], off_t);
char Funzione_Leggi_Da_Inizio(char [], char []);
char Funzione_Leggi_Una_Stringa(char [], char [], off_t);
off_t Calcola_Dimensione(int);
int Dimensione_buffer(char []);

int main(int argc, char **argv){
	struct sockaddr_in Server_Addr, Client_Addr;
	system("clear");
	Check_Condition(argc, argv);
	socklen_t Client_Len;
	Processo *Lista = NULL;
	if((Sock_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0){
		Errore("Errorre Nell'Apertura Del Socket.\n");
	}else{
		write(1, "[+]Il Socket Del Server è Stato Creato.\n", strlen("[+]Il Socket Del Server è Stato Creato.\n"));
	}
	memset(&Server_Addr, '\0', sizeof(Server_Addr));
	Port_n = atoi(argv[1]);
	Server_Addr.sin_family = AF_INET;
	Server_Addr.sin_port = htons(Port_n);
	Server_Addr.sin_addr.s_addr = INADDR_ANY;
	if(bind(Sock_fd, (struct sockaddr *) &Server_Addr, sizeof(Server_Addr)) < 0){
		Errore("Bindin Fallito.\n");
	}else{
		write(1, "[+]Binding Con La Porta:", strlen("[+]Binding Con La Porta:"));
		write(1, argv[1], strlen(argv[1]));
	}
	if(listen(Sock_fd, SOMAXCONN) < 0){
		Errore("Errore Nella Chiamata Listen.\n");
	}else{
		write(1, "\n[+]Listening....\n", strlen("\n[+]Listening....\n"));
	}
	while(1){
		if((New_Sock_fd = accept(Sock_fd, (struct sockaddr *)&Client_Addr, &Client_Len)) < 0){
			Errore("[-]Impossibile Accettare La Connessione.\n");
		}
		printf("[+]Connessione Accettata Da %s: %d.\n", inet_ntoa(Client_Addr.sin_addr), ntohs(Client_Addr.sin_port));
		Lista = inserisci_nodo(Lista);
		pthread_mutex_init(&mutex_n_connessioni, NULL);
		pthread_mutex_lock(&mutex_n_connessioni);
		n_connessioni++;
		pthread_mutex_unlock(&mutex_n_connessioni);
		if(Lista != NULL){
			Lista->info.socket = New_Sock_fd;
			Lista->info.porta = ntohs(Client_Addr.sin_port);
			Lista->info.indirizzo_client = inet_ntoa(Client_Addr.sin_addr);
			Lista->info.dimMax = atoi(argv[3]);
			strcpy(Lista->info.titolo,argv[2]);
			if((ret = pthread_create((&(Lista->info.th_id)), NULL, Gestione_Server, (void *) (Lista))) != 0){
				Errore(MSG_ERROR_THCREATE);
			}
			Lista = Lista->next;
		}
	}
}


void Errore(const char *msg){
	perror(msg);
	exit(1);
}

Processo *nuovo_nodo(){
	Processo *nuovo_processo = (Processo *)malloc(sizeof(Processo));
	nuovo_processo->info.th_id = -1;
	nuovo_processo->info.socket = -1;
	nuovo_processo->info.porta = -1;
	nuovo_processo->info.dimMax = 0;
	bzero(nuovo_processo->info.buffer, sizeof(nuovo_processo->info.buffer));
	bzero(nuovo_processo->info.titolo, sizeof(nuovo_processo->info.titolo));
	nuovo_processo->next = NULL;
	return nuovo_processo;	
}

Processo *inserisci_nodo(Processo *Inserisci){
	if(Inserisci == NULL){
		Inserisci = nuovo_nodo();
	}else{
		Inserisci->next = inserisci_nodo(Inserisci->next);
	}
	return Inserisci;
}

void *Gestione_Server(void *arg){
	Processo *th_utente = (Processo *)arg;
	bzero(th_utente->info.buffer, sizeof(th_utente->info.buffer));
	while(1){
		ret = File_Editing(th_utente);
		if(ret == 0){
			printf("[-]Disconnesso Da %s: %d.\n", th_utente->info.indirizzo_client, th_utente->info.porta);
			th_utente = Chiusura_Connessione(th_utente, th_utente->info.socket);
			if(n_connessioni == 0){
				write(1, MSG_CLOSE_SERVER_NO_USER, strlen(MSG_CLOSE_SERVER_NO_USER));
				exit(0);
			}
		}
	}
}

Processo *Chiusura_Connessione(Processo *Lista, int fd){
	if(Lista != NULL){
		Lista->next = Chiusura_Connessione(Lista->next, fd);
		if(Lista->info.socket == fd){
			pthread_mutex_lock(&mutex_n_connessioni);
			n_connessioni--;
			pthread_mutex_unlock(&mutex_n_connessioni);
			free(Lista);
		}
	}
	return Lista;
}

void Check_Condition(int argc, char **argv){
	if(argc != 4){
		Errore(MSG_ERROR_COMMAND);
	}
	int port = atoi(argv[1]);
	int dim_finale = atoi(argv[3]);
	if(port < 1024 || port > 65525){
		Errore(MSG_ERROR_PORT);
	}
	if(strcmp(argv[2], "Server.c") == 0 || strcmp(argv[2], "Client.c") == 0){
		Errore(MSG_ERROR_PERMISSION);
	}
	if(stat(argv[2], &file) == -1){
		Errore(MSG_ERROR_FILE);
	}
	if(dim_finale < file.st_size){
		Errore(MSG_ERROR_LENGTH);
	}
}

int File_Editing(Processo *utente){
	int val = 1;
	do{
		read(utente->info.socket, utente->info.buffer, sizeof(utente->info.buffer));
		if(strcmp(utente->info.buffer, "Scrivi") == 0 || strcmp(utente->info.buffer, "scrivi") == 0){
			bzero(utente->info.buffer, sizeof(utente->info.buffer));
			read(utente->info.socket, utente->info.buffer, sizeof(utente->info.buffer));
				if(strcmp(utente->info.buffer, "Append") == 0 || strcmp(utente->info.buffer, "append") == 0){
					bzero(utente->info.buffer, sizeof(utente->info.buffer));
					strcpy(utente->info.buffer, "Cosa Vuoi Scrivere?");
					write(utente->info.socket, utente->info.buffer, sizeof(utente->info.buffer));
					bzero(utente->info.buffer, sizeof(utente->info.buffer));
					read(utente->info.socket, utente->info.buffer, sizeof(utente->info.buffer));
					*utente->info.buffer = Funzione_Scrivi_Append(utente->info.buffer, utente->info.titolo, utente->info.dimMax);
					write(utente->info.socket, utente->info.buffer, sizeof(utente->info.buffer));
					bzero(utente->info.buffer, sizeof(utente->info.buffer));
				}else if(strcmp(utente->info.buffer, "OffSet") == 0 || strcmp(utente->info.buffer, "offset") == 0){
					strcpy(utente->info.buffer, "Da Quale Byte Vuoi Far Partire La Scrittura?");
					write(utente->info.socket, utente->info.buffer, sizeof(utente->info.buffer));
					bzero(utente->info.buffer, sizeof(utente->info.buffer));
					read(utente->info.socket, utente->info.buffer, sizeof(utente->info.buffer));
					long leng_write = atoi(utente->info.buffer);
					bzero(utente->info.buffer, sizeof(utente->info.buffer));
					strcpy(utente->info.buffer, "Cosa Vuoi Scrivere?");
					write(utente->info.socket, utente->info.buffer, sizeof(utente->info.buffer));
					bzero(utente->info.buffer, sizeof(utente->info.buffer));
					read(utente->info.socket, utente->info.buffer, sizeof(utente->info.buffer));
					*utente->info.buffer = Funzione_Scrivi_Da_Offset(utente->info.buffer, utente->info.titolo, utente->info.dimMax, leng_write);
					write(utente->info.socket, utente->info.buffer, sizeof(utente->info.buffer));
					bzero(utente->info.buffer, sizeof(utente->info.buffer));
				}else{
					bzero(utente->info.buffer, sizeof(utente->info.buffer));
					strcpy(utente->info.buffer,"\nComando Inserito Errato Operazione Annullata.\n\n");
					write(utente->info.socket, utente->info.buffer, sizeof(utente->info.buffer));
					bzero(utente->info.buffer, sizeof(utente->info.buffer));
				}
		}else if(strcmp(utente->info.buffer, "Leggi") == 0 || strcmp(utente->info.buffer, "leggi") == 0){
			bzero(utente->info.buffer, sizeof(utente->info.buffer));
			read(utente->info.socket, utente->info.buffer, sizeof(utente->info.buffer));
			if(strcmp(utente->info.buffer, "Inizio") == 0 || strcmp(utente->info.buffer, "inizio") == 0){
				bzero(utente->info.buffer, sizeof(utente->info.buffer));
				*utente->info.buffer = Funzione_Leggi_Da_Inizio(utente->info.buffer, utente->info.titolo);
				write(utente->info.socket, utente->info.buffer, sizeof(utente->info.buffer));
				bzero(utente->info.buffer, sizeof(utente->info.buffer));
			}else if(strcmp(utente->info.buffer, "OffSet") == 0 || strcmp(utente->info.buffer, "offset") == 0){
				bzero(utente->info.buffer, sizeof(utente->info.buffer));
				read(utente->info.socket, utente->info.buffer, sizeof(utente->info.buffer));
				if(strcmp(utente->info.buffer, "Byte") == 0 || strcmp(utente->info.buffer, "byte") == 0){
					bzero(utente->info.buffer, sizeof(utente->info.buffer));
					strcpy(utente->info.buffer, "Da Quale Byte Vuoi Far Partire La Lettura?");
					write(utente->info.socket, utente->info.buffer, sizeof(utente->info.buffer));
					bzero(utente->info.buffer, sizeof(utente->info.buffer));
					read(utente->info.socket, utente->info.buffer, sizeof(utente->info.buffer));
					long leng_read = atoi(utente->info.buffer);
					*utente->info.buffer = Funzione_Leggi_Da_Un_Certo_Byte(utente->info.buffer, utente->info.titolo, leng_read);
					write(utente->info.socket, utente->info.buffer, sizeof(utente->info.buffer));
					bzero(utente->info.buffer, sizeof(utente->info.buffer));
				}else if(strcmp(utente->info.buffer, "Stringa") == 0 || strcmp(utente->info.buffer, "stringa") == 0){
					bzero(utente->info.buffer, sizeof(utente->info.buffer));
					strcpy(utente->info.buffer, "Da Quale Byte Vuoi Far Partire La Lettura?");
					write(utente->info.socket, utente->info.buffer, sizeof(utente->info.buffer));
					read(utente->info.socket, utente->info.buffer, sizeof(utente->info.buffer));
					long start_read = atoi(utente->info.buffer);
					bzero(utente->info.buffer, sizeof(utente->info.buffer));
					strcpy(utente->info.buffer, "A Quale Byte Vuoi Far Finire La Lettura?");
					write(utente->info.socket, utente->info.buffer, sizeof(utente->info.buffer));
					bzero(utente->info.buffer, sizeof(utente->info.buffer));
					read(utente->info.socket, utente->info.buffer, sizeof(utente->info.buffer));
					*utente->info.buffer = Funzione_Leggi_Una_Stringa(utente->info.buffer, utente->info.titolo, start_read);
					write(utente->info.socket, utente->info.buffer, sizeof(utente->info.buffer));
					bzero(utente->info.buffer, sizeof(utente->info.buffer));
				}else{
					bzero(utente->info.buffer, sizeof(utente->info.buffer));
					strcpy(utente->info.buffer,"\nComando Inserito Errato Operazione Annullata.\n\n");
					write(utente->info.socket, utente->info.buffer, sizeof(utente->info.buffer));
					bzero(utente->info.buffer, sizeof(utente->info.buffer));
				}
			}else{
				bzero(utente->info.buffer, sizeof(utente->info.buffer));
				strcpy(utente->info.buffer,"\nComando Inserito Errato Operazione Annullata.\n\n");
				write(utente->info.socket, utente->info.buffer, sizeof(utente->info.buffer));
				bzero(utente->info.buffer, sizeof(utente->info.buffer));
			}
		}else if(strcmp(utente->info.buffer, "Dimensione") == 0 || strcmp(utente->info.buffer, "dimensione") == 0){
			bzero(utente->info.buffer, sizeof(utente->info.buffer));
			*utente->info.buffer = Funzione_Dimensione(utente->info.buffer, utente->info.titolo);
			write(utente->info.socket, utente->info.buffer, sizeof(utente->info.buffer));
			bzero(utente->info.buffer, sizeof(utente->info.buffer));
		}else if(strcmp(utente->info.buffer, "Esci") == 0 || strcmp(utente->info.buffer, "esci") == 0){
			strcpy(utente->info.buffer,"\n[-]Disconnesso Dal Server.\n\n");
			write(utente->info.socket, utente->info.buffer, sizeof(utente->info.buffer));
			val = 0;
			bzero(utente->info.buffer, sizeof(utente->info.buffer));
		}else{
			bzero(utente->info.buffer, sizeof(utente->info.buffer));
			strcpy(utente->info.buffer,"\nScelta Errata Riprova.\n\n");
			write(utente->info.socket, utente->info.buffer, sizeof(utente->info.buffer));
			bzero(utente->info.buffer, sizeof(utente->info.buffer));
		}
		bzero(utente->info.buffer, sizeof(utente->info.buffer));
	}while(val != 0);
	return val;
}

off_t Calcola_Dimensione(int fp){
	off_t Dim = lseek(fp, 0L, SEEK_END);;
	lseek(fp, 0L, SEEK_SET);
	return Dim;
}

char Funzione_Dimensione(char Output[],char titolo[]){
	int file_descriptor = open(titolo, O_RDONLY);
	char Dimensione_In_Stringa[10];
	bzero(Dimensione_In_Stringa, sizeof(Dimensione_In_Stringa));
	dim = Calcola_Dimensione(file_descriptor);
	strcpy(Output,"\nLa Dimensione Del File \"");
	strcat(Output,"test.txt");
	strcat(Output, "\" è ");
	sprintf(Dimensione_In_Stringa, "%ld", dim);
	strcat(Output, Dimensione_In_Stringa);
	strcat(Output, " byte.");
	strcat(Output, "\n\n");
	close(file_descriptor);
	return *Output;
}

char Funzione_Scrivi_Append(char buffer[], char NomeFile[], off_t dimMax){
	if(strlen(buffer) <= dimMax){
		int file_descriptor = open(NomeFile, O_WRONLY, O_APPEND);
		int len = 0;
		lseek(file_descriptor, 0L, SEEK_END);
		len = Dimensione_buffer(buffer);
		char supporto[len];
		bzero(supporto, sizeof(supporto));
		strcpy(supporto, buffer);
		write(file_descriptor, supporto, sizeof(supporto));
		write(file_descriptor, "-", strlen("-"));
		bzero(buffer, sizeof(*buffer));
		bzero(supporto, sizeof(supporto));
		strcpy(buffer,"\nStringa Inserita Correttamente Nel File.\n\n");
		close(file_descriptor);
	}else{
		bzero(buffer, sizeof(*buffer));
		strcpy(buffer,"\nStringa Non Inserita Correttamente Nel File Dimensione Massima Raggiunta.\n\n");
	}
	return *buffer;
}

int Dimensione_buffer(char p[]){
	int dim;
	if(strrchr(p, '\n')){
		dim = strlen(p) - 1;
	}
	return dim;
}

char Funzione_Scrivi_Da_Offset(char buffer[], char NomeFile[], off_t dimMax, long Inizio_Scrittura){
	if(strlen(buffer) < dimMax && Inizio_Scrittura < dimMax){
		int file_descriptor = open(NomeFile, O_WRONLY, O_APPEND);
		lseek(file_descriptor, Inizio_Scrittura, SEEK_SET);
		write(file_descriptor, "-", strlen("-"));
		int Len = 0;
		Len = Dimensione_buffer(buffer);
		char supporto[Len];
		bzero(supporto, sizeof(supporto));
		strcpy(supporto, buffer);
		write(file_descriptor, supporto, sizeof(supporto));
		write(file_descriptor, "-", strlen("-"));
		bzero(buffer, sizeof(*buffer));
		bzero(supporto, sizeof(supporto));
		strcpy(buffer,"\nStringa Inserita Correttamente Nel File.\n\n");
		close(file_descriptor);
	}else{
		if(Inizio_Scrittura >= dimMax){
			bzero(buffer, sizeof(*buffer));
			strcpy(buffer,"\nStringa Non Inserita Nel File Lunghezza Offset Maggiore Della Dimensone Del File.\n\n");
		}else{
			bzero(buffer, sizeof(*buffer));
			strcpy(buffer,"\nStringa Non Inserita Nel File Lunghezza Massima Superata.\n\n");
		}
	}
	return *buffer;
}

char Funzione_Leggi_Da_Inizio(char buffer[], char NomeFile[]){
	int file_descriptor = open(NomeFile, O_RDONLY);
	off_t dim = Calcola_Dimensione(file_descriptor);
	off_t pointer = lseek(file_descriptor, 0L, SEEK_SET);
	while(pointer < dim){
		read(file_descriptor, buffer, MAXLEN);
		pointer++;
	}
	strcat(buffer, "\n\n");
	close(file_descriptor);
	return *buffer;
}

char Funzione_Leggi_Da_Un_Certo_Byte(char buffer[], char NomeFile[], off_t leng_read){
	int file_descriptor = open(NomeFile, O_RDONLY);
	dim = Calcola_Dimensione(file_descriptor);
	off_t pointer = lseek(file_descriptor, leng_read, SEEK_SET);
	bzero(buffer, sizeof(*buffer));
	if(pointer > dim){
			strcpy(buffer, "Errore: Il Punto Di Partenza Scelto è Maggiore Dell'Ultimo byte Scritto Nel File.");
	}else{
		while(pointer < dim){
			read(file_descriptor, buffer, MAXLEN);
			pointer++;
		}
	}
	strcat(buffer, "\n\n");
	close(file_descriptor);
	return *buffer;
}

char Funzione_Leggi_Una_Stringa(char buffer[], char NomeFile[], off_t start_read){
	int file_descriptor = open(NomeFile, O_RDONLY);
	long end_read = atoi(buffer);
	dim = Calcola_Dimensione(file_descriptor);
	bzero(buffer, sizeof(*buffer));
	if(start_read < dim && start_read < end_read && end_read <= dim){
		off_t Diff = end_read - start_read;
		off_t pointer = lseek(file_descriptor, start_read, SEEK_SET);
		int scarto = Diff;
		char stringa[scarto]; 
		memset(stringa, '\0', scarto);
		while(pointer < end_read){
			if(pointer < end_read){
				read(file_descriptor, stringa, sizeof(stringa));
				break;
			}
		}
		strcat(stringa, "\n\n");
		strcpy(buffer, stringa);
	}else{
		if(start_read  > dim){
			strcpy(buffer, "Errore: Il Punto Di Partenza Scelto è Maggiore Dell'Ultimo byte Scritto Nel File.");
		}else if(start_read  > end_read){
			strcpy(buffer, "Errore: Il Punto Di Partenza Scelto è Maggiore Del Punto Finale Scelto.");
		}else if(end_read > dim){
			strcpy(buffer, "Errore: Il Punto Di Fine Scelto è Maggiore Dell'Ultimo byte Scritto Nel File.");
		}
		strcat(buffer, "\n\n");
	}
	close(file_descriptor);
	return *buffer;
}

