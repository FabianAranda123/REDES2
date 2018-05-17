
#define WORKER_ADDR "127.0.0.1"
#define MIRROR_ADDR "127.0.0.1"




///////////////Funcion que permite hacer conexion con workrs o mirrors
int workerConnection(char worker_addr[], char workermirror_addr[], int worker_port, int workermirror_port)
{
	int s;                            //ID del socket
	struct sockaddr_in worker_info;   //Estructura con informacion del worker

	if((s = socket(AF_INET, SOCK_STREAM, 0))<0)  //Creacion del socket
	{
		printf("Error en socket() workers\n");
	}

	//Llenando estructura con datos del worker
	bzero((char*)&worker_info, sizeof(worker_info));  //Llenando estructura de ceros
	worker_info.sin_family = AF_INET;                 //Diciendo que el tipo de conexion es de internet
	worker_info.sin_port = htons(worker_port);               //Puerto del worker al cual nos queremos conectar
	worker_info.sin_addr.s_addr = inet_addr(worker_addr);     //Direccion del worker al cual nos queremos conectar

	if((connect(s, (struct sockaddr*)&worker_info, sizeof(worker_info)))<0) //Haciendo conexion con worker, en caso de no encontrarlo se intenta con mirror
	{
		printf("Error al conectar con worker, conectando con mirror....\n");

		//Llenando estructura con datos del mirror
		bzero((char*)&worker_info, sizeof(worker_info));  //Llenando estructura de ceros
		worker_info.sin_family = AF_INET;                 //Diciendo que el tipo de conexion es de internet
		worker_info.sin_port = htons(workermirror_port);               //Puerto del worker al cual nos queremos conectar
		worker_info.sin_addr.s_addr = inet_addr(workermirror_addr);     //Direccion del worker al cual nos queremos conectar

		if((connect(s, (struct sockaddr*)&worker_info, sizeof(worker_info)))<0) //Haciendo conexion con mirror
		{
			printf("Error al conectar con mirror\n");
			return -1;
		}

	}

	return s; //Conexion con worker o mirror exitosa
}








/////////////////////////// FUNCION QUE GUARDA ARCHIVO EN WORKERS //////////////////////////////
void saveFile(int canal, char trama[])
{
	char b_ini;           //byte inicial que indica si es la ultima trama o no
	int fileSize;         //Tamano de los datos que se esta recibiendo
	char fileName[30];    //Nombre del archivo que se esta recibindo
	char fileData[100];   //Datos del archivo que se recibe

	FILE *fp;			  //Apuntador al archivo para guardar los dtaos y leerlos
	int contador = 0;     //Contador de numero de bytes recibidos del archivo
	char datarecv[135];   //Nueva trama para guardar los datos recibidos

	int distrib;          //Bytes recibidos divididos entre 3

	int w;                //Identificador de conexion con los worker 
	int worker_port;      //Puerto de los worker
	int workermirror_port; //Puerto del worker mirror
	char worker_addr[15]; //Direccion de los worker
	char workermirror_addr[15]; //Direccion del worker mirror

	int bytes_read;       //Numero de bytes que se leen en un archivo
	char datasend[135];   //Trama que sera enviada a los workers

	char command[40];     //COmando para borarr el archivo local

	char flag = 'f';


	printf("Se solicito guardar un archivo\n");

	//OBTENIENDO DATOS DE LA PRIMER TRAMA ENVIADA
	memcpy(&b_ini, &trama[0], 1);       //Byte inicial 0 si es la ultima trama o 1 si aun faltan tramas por recibir
	memcpy(&fileSize, &trama[1], 4);    //Tamano de los datos que se enviaron
	memcpy(&fileName, &trama[5], 30);   //Nombre del archivo a guardar
	memcpy(&fileData, &trama[35], 100); //Datos del archivo




	//////////////////// RECIBIENDO ARCHIVO Y ALMACENANDOLO DE MANERA LOCAL///////////////
	fp = fopen(fileName, "w"); //Abrimos archivo temporal

	while (b_ini == '1')  //Mientras no sea la trama final
	{
		fwrite(&fileData, 1, fileSize, fp);          //Escribimos los datos en el archivo
		memset(datarecv, '\0', 135);
		printf("fileSize: %d\n", fileSize );
		contador = contador + fileSize;
		printf("b_ini %c\n", b_ini);				 //aumenta contador para saber bytes en total
		if((recv(canal, datarecv, 135, 0))<0)
		{
			perror("error recv\n");
		}
		if((send(canal, &flag, sizeof(char), 0))<0)
		{
			perror("error  en send\n");
		}
		memcpy(&b_ini, &datarecv[0], 1);             //Byte inicial 0 si es la ultima trama o 1 si aun faltan tramas por recibir
		memcpy(&fileSize, &datarecv[1], 4);	         //Tamano de los datos que se enviaron
		memcpy(&fileData, &datarecv[35], 100);       //Datos del archivo
	}
	fwrite(&fileData, 1, fileSize, fp);  //Escribimos los datos de la ultima trama
	memset(datarecv, '\0', 135);
	printf("fileSize Final: %d\n", fileSize );
	printf("b_ini %c\n", b_ini);	
	contador = contador + fileSize;      //Aumenta contador para saber bytes en total


	fclose(fp); //Cerramos archivo temporal
	printf("Archivo recibido exitosamente  +..+  Bytes recibidos: %d\n",contador );


	/////////////////// DIVIDIENDO ARCHIVO ENTRE 3 WORKERS///////////////////
	
	////Coonexion con Workers/////

	strcpy(worker_addr, WORKER_ADDR);       //Direccion de los worker
	strcpy(workermirror_addr, MIRROR_ADDR); //Direccion de mirrors

	printf("Conectando con workers o mirrors\n");

	fp = fopen(fileName, "r");  //Abriendo archivo para leerlo y mandarlo a alos 3 workers


	////////Conexion con worker o mirror 1////////
	worker_port = 6666;       //Puerto del worker
	workermirror_port = 6667; //Puerto del mirror
	if((w=workerConnection(worker_addr, workermirror_addr, worker_port, workermirror_port)) < 0)
	{
		printf("Error al conectar con worker y mirror 1 \n");
	}
	//Enviando datos a worker o mirror 1
	printf("Conexion con worker o mirror exitosa, enviando parte 1 a worker o mirror 1.....\n");
	distrib = contador/3; //NUmero de bytes que le toca a cada worker excepto el ultimo
	while (distrib > 100)  //Mientrasfd existan mas de 100 bytes por leer en el archivo
	{
		bytes_read = fread(fileData, 1, 100, fp);  //Se leen 100 bytes del archivo y se guardan para enviarlos por la trama
		distrib = distrib - 100;                   //Se restan los bytes leidos

		//Llenando la trama para enviar al worker 1
		b_ini = '1';                                 //Indicamos al worker que no es la ultima trama
		memcpy(&datasend, &b_ini, 1);				 //Copiando byte inicial a la trama
		memcpy(&datasend[1], &bytes_read, 4);        //Copiando numero de bytes leidos a la trama
		memcpy(&datasend[5], &fileName, 30);		 //Copiando nombre del archivo a la trama
		memcpy(&datasend[35], &fileData, 100);		 //Copiando bytes con el contenido del archivo a la trama
		send(w, &datasend, sizeof(datasend), 0);     //Enviando trama al worker
		recv(w, &flag, sizeof(char), 0);
	} 

	//Llenamos la ultima trama
	bytes_read = fread(fileData, 1, distrib, fp);  //Se leen 100 bytes del archivo y se guardan para enviarlos por la trama
	b_ini = '0';                                 //Indicamos al worker que es la ultima trama
	memcpy(&datasend, &b_ini, 1);				 //Copiando byte inicial a la trama
	memcpy(&datasend[1], &bytes_read, 4);        //Copiando numero de bytes leidos a la trama
	memcpy(&datasend[5], &fileName, 30);		 //Copiando nombre del archivo a la trama
	memcpy(&datasend[35], &fileData, 100);		 //Copiando bytes con el contenido del archivo a la trama
	send(w, &datasend, sizeof(datasend), 0);     //Enviando trama al worker
	recv(w, &flag, sizeof(char), 0);
	printf("Envio a worker o mirror 1 exitoso\n");

	////////Conexion con worker o mirror 2////////
	worker_port = 7777;       //Puerto del worker
	workermirror_port = 7778; //Puerto del mirror
	if((w=workerConnection(worker_addr, workermirror_addr, worker_port, workermirror_port)) < 0)
	{
		printf("Error al conectar con worker y mirror 2 \n");
	}
	//Enviando datos a worker o mirror 2
	printf("Conexion con worker o mirror exitosa, enviando parte 2 a worker o mirror 2.....\n");
	distrib = contador/3; //NUmero de bytes que le toca a cada worker excepto el ultimo
	while (distrib > 100)  //Mientras existan mas de 100 bytes por leer en el archivo
	{
		bytes_read = fread(fileData, 1, 100, fp);  //Se leen 100 bytes del archivo y se guardan para enviarlos por la trama
		distrib = distrib - 100;                   //Se restan los bytes leidos

		//Llenando la trama para enviar al worker 2
		b_ini = '1';                                 //Indicamos al worker que no es la ultima trama
		memcpy(&datasend, &b_ini, 1);				 //Copiando byte inicial a la trama
		memcpy(&datasend[1], &bytes_read, 4);        //Copiando numero de bytes leidos a la trama
		memcpy(&datasend[5], &fileName, 30);		 //Copiando nombre del archivo a la trama
		memcpy(&datasend[35], &fileData, 100);		 //Copiando bytes con el contenido del archivo a la trama
		send(w, &datasend, sizeof(datasend), 0);     //Enviando trama al worker
		recv(w, &flag, sizeof(char), 0);
	} 

	//Llenamos la ultima trama
	bytes_read = fread(fileData, 1, distrib, fp);  //Se leen 100 bytes del archivo y se guardan para enviarlos por la trama
	b_ini = '0';                                 //Indicamos al worker que es la ultima trama
	memcpy(&datasend, &b_ini, 1);				 //Copiando byte inicial a la trama
	memcpy(&datasend[1], &bytes_read, 4);        //Copiando numero de bytes leidos a la trama
	memcpy(&datasend[5], &fileName, 30);		 //Copiando nombre del archivo a la trama
	memcpy(&datasend[35], &fileData, 100);		 //Copiando bytes con el contenido del archivo a la trama
	send(w, &datasend, sizeof(datasend), 0);     //Enviando trama al worker
	recv(w, &flag, sizeof(char), 0);

	printf("Envio a worker o mirror 2 exitoso\n");

	printf("Conexion con worker o mirror exitosa, enviando parte 3 a worker o mirror 3.....\n");
	////////Conexion con worker o mirror 3////////
	worker_port = 8888;       //Puerto del worker
	workermirror_port = 8889; //Puerto del mirror
	if((w=workerConnection(worker_addr, workermirror_addr, worker_port, workermirror_port)) < 0)
	{
		printf("Error al conectar con worker y mirror 3 \n");
	}
	while((bytes_read = fread(fileData, 1, 100, fp)) == 100) //Leemos el archivo hasta que encontremos el final
	{
		b_ini = '1';                                 //Indicamos al worker que no es la ultima trama
		memcpy(&datasend, &b_ini, 1);				 //Copiando byte inicial a la trama
		memcpy(&datasend[1], &bytes_read, 4);        //Copiando numero de bytes leidos a la trama
		memcpy(&datasend[5], &fileName, 30);		 //Copiando nombre del archivo a la trama
		memcpy(&datasend[35], &fileData, 100);		 //Copiando bytes con el contenido del archivo a la trama
		send(w, &datasend, sizeof(datasend), 0);     //Enviando trama al worker
		recv(w, &flag, sizeof(char), 0);
	}
	b_ini = '0';                                 //Indicamos al worker que es la ultima trama
	memcpy(&datasend, &b_ini, 1);				 //Copiando byte inicial a la trama
	memcpy(&datasend[1], &bytes_read, 4);        //Copiando numero de bytes leidos a la trama
	memcpy(&datasend[5], &fileName, 30);		 //Copiando nombre del archivo a la trama
	memcpy(&datasend[35], &fileData, 100);		 //Copiando bytes con el contenido del archivo a la trama
	send(w, &datasend, sizeof(datasend), 0);     //Enviando trama al worker
	recv(w, &flag, sizeof(char), 0);

	printf("Envio a worker o mirror 3 exitoso\n");

	fclose (fp); //Cerrando archivo 

	//Eliminamos el archivo creado ya que ha sido enviado
	strcpy(command, "rm \0");
	strcat(command, fileName);
	system(command);




}









void getFile(int canal, char trama[])
{
	printf("Se solicito obtener un archivo\n");

	char b_ini;           //byte inicial que indica si es la ultima trama o no
	int fileSize;         //Tamano de los datos que se esta recibiendo
	char fileName[30];    //Nombre del archivo que se esta recibindo
	char fileData[100];   //Datos del archivo que se recibe

	int w;                //Identificador de conexion con los worker 
	int worker_port;      //Puerto de los worker
	int workermirror_port; //Puerto del worker mirror
	char worker_addr[15]; //Direccion de los worker
	char workermirror_addr[15]; //Direccion del worker mirror

	char datasend[135];   //Trama que sera enviada a los workers
	char datarecv[135];  //TRama donde se reciben los datos

	FILE *fp;         //Archivo en el cual guarda el archivo temporalmente
	int bytes_read;   //Bytes que se leen del archivo

	char command[50]; //Comando para eliminar archivo local

	char flag='f';

	memcpy(&fileName, &trama[5], 30);

	fp = fopen(fileName, "w");     //ABRIENDO ARCHIVO DONDE SE JUNTARAN LAS PARTES QUE ENVIEN LOS WORKERS

	//////////DIRECCIONES DE WORKERS Y MIRRORS
	strcpy(worker_addr, WORKER_ADDR);       //Direccion de los worker
	strcpy(workermirror_addr, MIRROR_ADDR); //Direccion de mirrors





	///////PIDIENDO ARCHIVO A WORKER O MIRROR 1/////////////////
	worker_port = 6666;       //Puerto del worker
	workermirror_port = 6667; //Puerto del mirror
	if((w=workerConnection(worker_addr, workermirror_addr, worker_port, workermirror_port)) < 0)
	{
		printf("Error al conectar con worker y mirror 1 \n");
	}

	//Pidiendo datos a worker o mirror 1
	printf("Conexion con worker o mirror exitosa, solicitando parte 1 a worker o mirror 1.....\n");
	b_ini = '2';                                 //Indicamos al worker que queremos un archivo
	memcpy(&datasend, &b_ini, 1);				 //Copiando byte inicial a la trama
	memcpy(&datasend[5], &fileName, 30);		 //Copiando nombre del archivo a la trama
	send(w, &datasend, sizeof(datasend), 0);     //Enviando trama solicitando archivo al worker
	recv(w, flag, sizeof(flag), 0);

	recv(w, datarecv, sizeof(datarecv), 0);      //Recibiendo primera trama del worker
	send(w, &flag, sizeof(flag), 0);

	while(datarecv[0] == '1')
	{
		fwrite(&datarecv[35], 1, 100, fp);   //Escribiendo en el archivo 
		recv(w, datarecv, sizeof(datarecv), 0); //Esperando otra trama
		send(w, &flag, sizeof(flag), 0);
	}
	//Copiando los datos de la trama final
	memcpy(&fileSize, &datarecv[1], 4); 
	fwrite(&datarecv[35], 1, fileSize, fp);






	///////PIDIENDO ARCHIVO A WORKER O MIRROR 2/////////////////
	worker_port = 7777;       //Puerto del worker
	workermirror_port = 7778; //Puerto del mirror
	if((w=workerConnection(worker_addr, workermirror_addr, worker_port, workermirror_port)) < 0)
	{
		printf("Error al conectar con worker y mirror 2 \n");
	}

	//Pidiendo datos a worker o mirror 1
	printf("Conexion con worker o mirror exitosa, solicitando parte 2 a worker o mirror 2.....\n");
	b_ini = '2';                                 //Indicamos al worker que queremos un archivo
	memcpy(&datasend, &b_ini, 1);				 //Copiando byte inicial a la trama
	memcpy(&datasend[5], &fileName, 30);		 //Copiando nombre del archivo a la trama
	send(w, &datasend, sizeof(datasend), 0);     //Enviando trama solicitando archivo al worker
	recv(w, flag, sizeof(flag), 0);

	recv(w, datarecv, sizeof(datarecv), 0);      //Recibiendo primera trama del worker
	send(w, &flag, sizeof(flag), 0);

	while(datarecv[0] == '1')
	{
		fwrite(&datarecv[35], 1, 100, fp);   //Escribiendo en el archivo 
		recv(w, datarecv, sizeof(datarecv), 0); //Esperando otra trama
		send(w, &flag, sizeof(flag), 0);
	}
	//Copiando los datos de la trama final
	memcpy(&fileSize, &datarecv[1], 4); 
	fwrite(&datarecv[35], 1, fileSize, fp);






	///////PIDIENDO ARCHIVO A WORKER O MIRROR 3/////////////////
	worker_port = 8888;       //Puerto del worker
	workermirror_port = 8889; //Puerto del mirror
	if((w=workerConnection(worker_addr, workermirror_addr, worker_port, workermirror_port)) < 0)
	{
		printf("Error al conectar con worker y mirror 1 \n");
	}

	//Pidiendo datos a worker o mirror 1
	printf("Conexion con worker o mirror exitosa, solicitando parte 1 a worker o mirror 1.....\n");
	b_ini = '2';                                 //Indicamos al worker que queremos un archivo
	memcpy(&datasend, &b_ini, 1);				 //Copiando byte inicial a la trama
	memcpy(&datasend[5], &fileName, 30);		 //Copiando nombre del archivo a la trama
	send(w, &datasend, sizeof(datasend), 0);     //Enviando trama solicitando archivo al worker
	recv(w, flag, sizeof(flag), 0);

	recv(w, datarecv, sizeof(datarecv), 0);      //Recibiendo primera trama del worker
	send(w, &flag, sizeof(flag), 0);

	while(datarecv[0] == '1')
	{
		fwrite(&datarecv[35], 1, 100, fp);   //Escribiendo en el archivo 
		recv(w, datarecv, sizeof(datarecv), 0); //Esperando otra trama
		send(w, &flag, sizeof(flag), 0);
	}
	//Copiando los datos de la trama final
	memcpy(&fileSize, &datarecv[1], 4); 
	fwrite(&datarecv[35], 1, fileSize, fp);


	fclose(fp); //Cerrando archivo que se tiene en local 
	

	///////////ENVIANDO ARCHIVO AL CLIENTE/////////
	printf("fileName : %s\n", fileName);
	fp = fopen(fileName, "r");
	printf("fp: %d", fp);

	while((bytes_read = fread(fileData, sizeof(char), 100, fp))==100) //Mientras existan mas bytes por leer
	{
		b_ini = '1';                                 //Indicamos que no es la ultima trama
		memcpy(&datasend[0], &b_ini, 1);             //Copiando byte inical a la trama que se va a enviar       
		memcpy(&datasend[1], &bytes_read, 4);        //Copiando numero de bytes que se envian a la trama que se va a enviar
		memcpy(&datasend[5], &fileName, 30);         //Copiando nombre del archivo a la trama que se va a enviar
		memcpy(&datasend[35], &fileData, 100);       //Copiando Datos del archivo a la trama que se va a enviar
		send(canal, &datasend, sizeof(datasend), 0);  //Enviando trama con 100 bytes de datos
		recv(canal, &flag, sizeof(flag), 0);
		printf("Se metio al while\n");
	}
	printf("bytesread %d\n", bytes_read);
	b_ini = '0';                                  //Indicamos que es la ultima trama
	memcpy(&datasend[0], &b_ini, 1);              //Copiando byte inical a la trama que se va a enviar 
	memcpy(&datasend[1], &bytes_read, 4);         //Copiando numero de bytes que se envian a la trama que se va a enviar
	memcpy(&datasend[5], &fileName, 30);          //Copiando nombre del archivo a la trama que se va a enviar
	memcpy(&datasend[35], &fileData, 100);        //Copiando Datos del archivo a la trama que se va a enviar
	send(canal, &datasend, sizeof(datasend), 0);   //Enviando trama con bytes_read datos 
	recv(canal, &flag, sizeof(flag), 0);

	fclose(fp); //Cerrando archivo

	//Eliminamos el archivo creado ya que ha sido enviado
	strcpy(command, "rm \0");
	strcat(command, fileName);
	system(command);


}
