
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <sys/socket.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include"Funciones.h"

#define B_INI 1        //Byte inicial de la trama
#define TAM_DATA 4     //Tamano del contenido de datos
#define NAME_FILE 30   //Bytes que contienen el nombre del archivo que se desea guardar o recuperar de los servidores
#define DATA 100       //Bytes que contienen los datos del archivo que se desea guardar o recuperar de los servidores


int main(int argc, char* argv[]){
	
	int id_mirror, tamFile;
	char mirror_addr[15];   //Direccion de algun worker para realizar la conexion
	int worker_port;	    //Puerto de algun worker para realizar la conexion

	int canal, s;             //Canal de comunicacion
	char server_addr[15];  //Direccion del servidor worker
	int server_port;       //Puerto del servidor master
	char trama[B_INI + TAM_DATA + NAME_FILE + DATA];  //Cadena donde se almacena la trama recibida
	char byte_ini;
	struct sockaddr_in client_info;   //Informacion del cliente 
	int clien_info_len; 
	
	
	if(argc < 3){
		printf( "                      servidor     cliente  " );
		printf( "Faltan argumentos: [ip] [puerto] [ip] [puerto]" );
		exit(0);
	}
	/*CONEXIÓN CON EL ESPEJO*/ 

	
	/////////////Conexion con el mirror correspondiente/////////
	strcpy(mirror_addr, argv[1]);    //Direccion del mirror
	worker_port = atoi(argv[2]);                      //Puerto del mirror
	
	
	if((id_mirror = mirrorConection(mirror_addr, worker_port)) < 0) // conexion con el mirror
	{
		printf("Error al conectar con el espejo\n");
	}
	//send(id_mirror, &datarecv, sizeof(datarecv), 0);


	/////CREACION DEL SERVIDOR PARA ESPERAR CONEXIÓN CON EL MASTER //////////
	strcpy(server_addr, argv[3]);  //Direccion del servidor master
	server_port = atoi(argv[4]);                //Puerto del servidor master

	if((canal = createServer(server_addr, server_port)) < 0 ) //Creando un servidor y devolviendo un canal de comunicacion con el cliente
	{
		printf("Error al creal el worker Server\n");
		return -1;
	}
	

	/////////////////////////////////// ETAPA DE  COMUNICACIONES ///////////////////////////////////////////////////////////////////////

	
	while(1)  //Ciclo que permite estar siempre esperando peticiones del cliente
	{
		
		recv(canal, trama, sizeof(trama), 0); //Recibimos la trama
		memcpy(&byte_ini, &trama[0], 1);      //Obtenemos el primer byte

		if((byte_ini == '1') || (byte_ini == '0')) //Cliente(Master) nos envia un archivo
		{
		
			
			saveFile(canal,trama, id_mirror); //Guardando archivo 
			send(id_mirror, trama, sizeof(trama), 0);
			
			
		}
		else if(byte_ini == '2') //Master solicita un archivo
		{
			getFile(canal,trama); //Recuperando parte del archivo del mirror
		}
		else
		{
			printf("Solicitud no valida\n");
		}
		close(canal);

	}


	return 0;
}