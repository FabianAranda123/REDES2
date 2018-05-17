#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <pthread.h>
#include <error.h>
#include <fcntl.h>

#define REQUEST_FILE '2'
#define SENDING '1'
#define COMPLETE '0'

#define END_OF_FILE 1
#define TAM_FILE 4
#define NAME_FILE 30
#define TAM_DATA 100
#define TAM_HEADER END_OF_FILE + TAM_FILE + NAME_FILE
#define TAM_TRAMA TAM_HEADER + TAM_DATA
#define TAM_PATH 255

#define IP_MASTER "127.0.0.1" //"192.168.1.115"
#define PORT 5000
#define filename "incoming"
#define NAME_DIR "files_recv"

void* thread_proc(void *arg);



/**
* Almacena el fd_socket el descriptor del socket. En caso de fallo imprime error.
*/
void getSocket( int *fd_socket ) {
	*fd_socket = socket( AF_INET, SOCK_STREAM, 0 );
	
	if( *fd_socket == - 1 ) {
		perror("Error al abrir el socket");
		exit( -1 );
	}
	printf("SUCCESS GETSOCKET\n");
}

/**
*Conecta el socket a la dirección IP y puerto especificados.
*Retorna 0 si la conexión se realizo correctamente y -1 en caso de error o en el caso
*que no se haya conectado con el servidor.
*/
int connect_to_server( int *fd_socket, char *IP, int port ) {
	struct sockaddr_in serv;
	int status_connect;

	memset( &serv, 0, sizeof( serv ) );
	
	serv.sin_family = AF_INET;
	serv.sin_addr.s_addr = inet_addr( IP ); //localhost INADDR_ANY
	serv.sin_port = htons( port );

	status_connect = connect( *fd_socket, ( struct sockaddr *) &serv, sizeof( serv ) );

	if( status_connect < 0 ) {
		perror("CONNECT: ");
		exit( -1 );
	}

	printf("SUCCESS CONNECT\n");
	return status_connect;
}

int main(int argc, char *argv[])
{
	struct sockaddr_in sAddr;
	int sockfd,connfd;
	int status;
	pthread_t thread_id;
	int val;

	sockfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

	val = 1;
	status = setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &val, sizeof(val));
	if (status < 0) {
		perror("Error - port");
		return 0;
	}

	sAddr.sin_family = AF_INET;
	sAddr.sin_port = htons( PORT );
	sAddr.sin_addr.s_addr = INADDR_ANY;

	status = bind(sockfd, (struct sockaddr *) &sAddr, sizeof(sAddr));
	if( status < 0 ) {
		perror("Error - Bind");
		return 0;
	}

	status = listen(sockfd, 5);
	if (status < 0) {
		perror("Error - Listen");
		return 0;
	}

   while( 1 ) {
	   connfd = accept( sockfd, NULL, NULL );
	   if( connfd < 0 ) { 
		   printf("Accept error on server\n");
		   //error("ERROR on accept"); 
		   return 0;
	   }
	   
	   printf("client connected to child thread %lu with pid %d.\n", pthread_self(), getpid());
	   status = pthread_create(&thread_id, NULL, thread_proc, (void *) &connfd);
	   
	   if( status != 0 ) {
		   printf("Could not create thread.\n");
		   return 0;
	   }
	   
	   sched_yield();
	}
	pthread_join (thread_id, NULL);
}

void recv_flag( int fd_sock ) {
	char flag;
	int n = 1;

	if( recv( fd_sock, &flag, 1, 0 ) < 0 ) {
		perror("recv_flag");
		n = 0;
	}

	printf("%c %d", flag, n );
}

void send_flag( int fd_sock ) {
	char flag = 'X';

	if( send( fd_sock, &flag, 1, 0 ) < 0 ) {
		perror("send_flag");
	}
}

/**
* Envia el archivo al master
*/
void send_file( char *file_name ) {
	int fd_sock, nread;
	char trama[TAM_TRAMA];
	char buffer[100], flag = 'X';
	FILE *fp;

	getSocket( &fd_sock );
	connect_to_server( &fd_sock, IP_MASTER, 1111 ); //192.168.43.69
	printf("CONECTADO");

	fp = fopen( file_name, "r" );

	while( !feof( fp ) ) {
		//Leemos los datos del archivo.
		nread = fread( buffer, sizeof(char), TAM_DATA, fp );

		//Llenamos la trama.
		//Flag para determinar que se esta enviado el archivo.
		trama[0] = ( nread == TAM_DATA ) ? SENDING : COMPLETE; //'1'
		//Copia el tamaño del archivo.
		memcpy( trama + END_OF_FILE, &nread, 4 ); //
		printf("N = %d -> %c\n", nread, trama[0] );
		//Copiamos el nombre del archivo.
		memcpy( trama + END_OF_FILE + TAM_FILE, file_name, NAME_FILE );
		//Añade los bytes.
		memcpy( trama + TAM_HEADER, buffer, nread );

		//Envia los datos al master,
		if( send( fd_sock, trama, nread + TAM_HEADER, 0 ) < 0 ) {
			perror("ERROR");
		}

		//
		recv_flag( fd_sock );

		memset( trama, '\0', TAM_TRAMA );
	}

	close( fd_sock );
}

void recv_file( int fd_sock ) {
	char buf[TAM_TRAMA], ruta[TAM_PATH];
	int isFirstTrama = 1, fd_file, fd_sock_m;
	long rd = 0;
	
	printf("Conexion establecida con el cliente en el canal %d\n", fd_sock );
	printf("RECEIVED FILE\n");
	
	if( recv( fd_sock, buf + END_OF_FILE + TAM_FILE, NAME_FILE, 0 ) < 0 ) {
		perror("RECV_FILE NAME");
	}

	buf[0] = REQUEST_FILE;
	int m = 100;
	memcpy( buf + END_OF_FILE, &m, TAM_FILE );
	printf("GET FILE: %s\n", buf );

	getSocket( &fd_sock_m );
	connect_to_server( &fd_sock_m, IP_MASTER, 1111 ); //192.168.43.69
	printf("CONECTADO\n");

	send( fd_sock_m, buf, NAME_FILE, 0 );
	printf("A la espera de bytes\n");
	//Ciclo para recibir los archivos del cliente.
	recv_flag( fd_sock_m );

	memset( buf, '\0', TAM_TRAMA );
	while( 1 ) {
		send_flag( fd_sock_m );

		//Recibe la trama.
		if( ( rd = recv( fd_sock_m, buf, TAM_TRAMA, 0 ) ) < 0 ) {
			fprintf( stderr, "%s", "Error al recibir datos del cliente\n" );
			fprintf( stderr, "Termino de la conexion con canal %d\n", fd_sock_m );
			close( fd_sock_m );
			return;
		
		} else {
			//No hubo problema al recibir la trama.
			//Verificamos si el archivo se esta enviando o se llego al ultimo paquete
			char status = buf[0];
			printf("RECIBIDO status = %c buf = %s\n", status, buf );

			if( status == SENDING ) {
				//Se reciben los datos del archivos.
			
				//Si es la primer trama se crea el archivo.
				if( isFirstTrama ) {
					//Ruta donde se almacenarán los archivos
					getcwd( ruta, sizeof( ruta ) - 1 );
					strcat( ruta, NAME_DIR );
					strncat( ruta, buf + END_OF_FILE + TAM_FILE, NAME_FILE );
					printf("PATH %s\n", ruta);
					//Creamos el archivo
					fd_file = open( ruta, O_CREAT | O_RDWR, 0666 );
				
					if( fd_file < 0 ) {
						printf("No se pudo crear el archivo %s\n\n", buf );
						fprintf( stderr, "Termino de la conexion con canal %d\n", fd_sock_m );
						close( fd_sock_m );
						return;
					}
				
					isFirstTrama = 0;
				}
			
				//Se almacenan los ultimos datos en el archivo.
				//printf("TAM LAST TRAMA %ld - %d--> %ld\n", rd, TAM_HEADER, rd - TAM_HEADER );
				int count = ( rd > 35 ) ? rd - 35 : rd;
				if( write( fd_file, buf + TAM_HEADER, count ) < 0 ) {
					perror("Error al escribir en el fichero: ");
					printf("BUF %s %ld\n", buf, rd );
					fprintf( stderr, "Termino de la conexion con canal %d\n", fd_sock_m );
					
					close( fd_file );
					close( fd_sock_m );
					break;	
				}
			
			} else if( status == COMPLETE ) {
				//Se termino de enviar el archivo.
				//Se almacenan los ultimos datos en el archivo.
				//En la ultima trama en lugar de ir el tamaño del archivo
				//viene el tamaño de la trama que es menos que TAM_TRAMA.
				int pr;
				memcpy( &pr, buf + END_OF_FILE, 4 );
				pr = pr -35;
				printf("COMPLETE\n");
				
				if( write( fd_file, buf + TAM_HEADER, pr ) < 0 ) {
					perror("Error al escribir en el fichero: ");
					close( fd_file );
					fprintf( stderr, "Termino de la conexion con canal %d\n", fd_sock_m );
					close( fd_sock_m );
					break;
				}
				
				isFirstTrama = 1;
				close( fd_file );
				memset( buf, 0, TAM_TRAMA );
			
			} else {
				printf("BUF %s; BUF[0] %d; STAUS %u READ %ld\n", buf, ntohl(buf[0]), status, rd );
				fprintf( stderr, "Termino de la conexion con canal %d\n", fd_sock_m );
				close( fd_sock_m );
				close( fd_file );
				break;
			}
			
			//Limpiamos el buffer de datos.
			memset( buf + TAM_HEADER, 0, TAM_DATA );
		}
	}
}

void* thread_proc( void *arg ) {
	int connfd;
	int nread, nsend;
	char buffer[TAM_DATA], file_name[NAME_FILE];
	char *flag = "GET_FILE_XD\0"; //Si esta bandera se modifica se debe modificar tambien en el programa Java.
	FILE *fp;

	connfd = *( (int*) arg );

	/* Recibe la petición del cliente Java. Si buffer es igual a SENDING_FILE_XD
	** Quiere decir que se solicitó un archivo, en caso contrario se espera recibir
	** un fichero cuyo nombre se almacenó en buffer.
	*/
	if( recv( connfd, file_name, NAME_FILE, 0 ) < 0 ) {
		perror("RECV THREAD_PROC");
	}

	if( memcmp( file_name, flag, strlen( flag ) ) == 0 ) {
		//Se solicito un archivo.
		printf("get file %s %lu\n", file_name, strlen( file_name ) );
		recv_file( connfd );

		//Cierra el canal de comunicación.
		close( connfd );
		printf("client disconnected from child thread %lu with pid %d.\n", pthread_self(), getpid());

		return NULL;
	}

	//Se espera un archivo del cliente Java.
	printf("NAME FILE %s\n", file_name );

	//Se crea el archivo o se trunca a 0 bytes.
	fp = fopen( file_name, "wb" );

	//Verifica que el apuntador al archivo no sea nulo.
	if( fp == NULL ) {
		printf("File not found!\n");
		return NULL;
	}

	//Comienza a recibir los datos del archivo.
	while( ( nsend = recv( connfd, buffer, sizeof buffer, 0 ) ) > 0 ) {
		fwrite( buffer, sizeof( char ), nsend, fp );
		fprintf( stdout, "Received %d bytess\n", nsend );
	}

	//Termino de escribir en el fichero. Lo cerramos
	fclose( fp );

	//Una vez almacenado el fichero se envia al master.
	send_file( file_name );

	//Cierra el canal de comunicación.
	close( connfd );

	printf("client disconnected from child thread %lu with pid %d.\n", pthread_self(), getpid());
	return NULL;
}





















