#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <time.h>
#include <string.h>

#define MAX 1000000
#define MAXPALABRASBUSCAR 100
pthread_mutex_t mutex=PTHREAD_MUTEX_INITIALIZER; //inicializando mutex
int num_palabras[MAXPALABRASBUSCAR];
char *palabras[MAXPALABRASBUSCAR];
int numero_palabras=0;


//retorna el numero de lineas del archivo y guarda en tam_lineas el numero de caracteres de cada linea
int numero_lineas (char *ruta, int *tam_lineas){
	if (ruta != NULL){
		FILE* ar= fopen (ruta, "r");
		int lineas = 0;
		int tam_linea=0;
		while (!feof(ar)){
			tam_linea++;
			char c = getc(ar);
			if (c== '\n'){
				if (tam_lineas != NULL) {
					tam_lineas[lineas] = tam_linea;
				}
				lineas++;
				tam_linea =0;
			}
		}
		fclose (ar);
		return lineas;
	}
	return -1;
}

//estructura con la informacion necesario a enviar a cada hilo creado
typedef struct mi_estructuraTDA{
	int iniciocaracter;
	int inilinea;
	int finlinea;
	FILE * fp;
	int *tam_lineas;
} estructura;

//funcion a enviar a cada hilo para que procese una porcion del arreglo
void * contarpalabras(void *arg){
	estructura *argumentos = (estructura *)arg;		//convertimos al tipo de dato correcto
	int ini=argumentos->iniciocaracter;
    for(int i = argumentos->inilinea; i <= argumentos->finlinea; i++){ 
		int cant = argumentos->tam_lineas[i]+1;
		char *buffer = (char *)malloc(cant*sizeof(char));
		char * token;
		//region critica
		pthread_mutex_lock(&mutex);
		fseek(argumentos->fp,ini,SEEK_SET);
		fgets(buffer, cant,argumentos->fp);
		pthread_mutex_unlock(&mutex);
		//fin region critica
		while( (token = strtok_r(buffer," \n\e\t\\\?\f\'\"\v\b\r!^~><·$%&/,()=º¡¢£¤¥¦§¨©ª«®¯|@#~½¬-_ç`+*[]{}ḉç¿,.!?:;",&buffer))){
			for(int j=0; j<numero_palabras;j++){ 
				int comp = strcmp(token,palabras[j]);
				if(  comp == 0 ) {
					//region critica
					pthread_mutex_lock(&mutex);
					(num_palabras[j])++;
					pthread_mutex_unlock(&mutex);
					//fin region critica
				}
			}	
		}
		ini=ini+cant-1;
    }
  	return (void *)0;
}


int main(int argc, char *argv[]){
	if(argc>3){
		numero_palabras=argc-3;  //numero de palabras a buscar  
		//validando ruta de archivo
		char* pathfile=argv[1];
		FILE *fp = fopen(pathfile,"r");
		if(fp==NULL){ 
			printf("direccion invalida de archivo");
			return -1;
		}
		//validando numero de hilos
		int num_hilos=atoi(argv[2]);  //numero de hilos a utilizar para la busqueda
		if(num_hilos<1){
			printf("numero de hijo debe ser mayor o igual a 1");
			return -1;
		}
		//agregando al arreglo las palabras a buscar
		for(int i=0;i<numero_palabras;i++){
			palabras[i]=argv[3+i];
			num_palabras[i]=0;
		}

		int *tam_lineas = (int *)malloc(MAX*sizeof(int));
		int totallineas = numero_lineas(pathfile,tam_lineas);
		printf(" total lineas de archivo: %d \n",totallineas);

		//determinando cantidad de caracteres y la seccion para cada hilo
		int totalcaracteres=0;
		for(int i=0;i<totallineas;i++)
      		totalcaracteres+=tam_lineas[i];
      	printf("total caracteres: %d \n", totalcaracteres);

      	int residuo=0,tamanobloque;
		pthread_t *hilos =malloc(num_hilos*sizeof(pthread_t));
		int numlineas[num_hilos];
		int numcaracter[num_hilos];
		int finlinea[num_hilos];
		int i=0,j=0,total=0;
		//validar tamano de bloque y determinando informacion a enviar en cada hilo
		//se trata de enviar un cantidad de caracteres aproximidamente igual a cada hilo
		if(totallineas<=num_hilos)
			num_hilos=totallineas;
		if(totallineas%num_hilos!=0){
			tamanobloque=totallineas/num_hilos;
			residuo=totallineas%num_hilos;
		}
		tamanobloque=(totallineas/num_hilos);   //numero de lineas para cada hilo
		printf("bloque de lineas %d \n", tamanobloque);
		while(i<num_hilos ){
			numlineas[i]=j;
			numcaracter[i]=total;
			int num=0;
			while(j<tamanobloque+numlineas[i] && j<=totallineas){
				num+=tam_lineas[j];
				j++;
			}
			finlinea[i]=j-1;
			if(i==num_hilos-1){
				finlinea[i]=j-1+residuo;
			}
			total=total+num;
			i++;
		}
		
		//imprimiendo la informacion a enviar a cada hilo
		for(int i=0;i<num_hilos;i++){
      		printf("hilo : %d numero linea: %d num caracteres: %d  fin linea: %d  \n",
      		 i+1,numlineas[i],numcaracter[i],finlinea[i]);
		}
		
		
		//creando hilo para imprimir estado cada segundo
		void* retorno=0;
		pthread_t hilomjs ;
		int status = pthread_create(&hilomjs, NULL,Estado, NULL);
			if(status < 0){
				fprintf(stderr, "Error al crear el hilo mensaje: \n");
				exit(-1);	
			}
		//creando hilos
		int  hilo;
		for(hilo=0;hilo<num_hilos;hilo++) {
			estructura *mi_argumento_estructura = malloc(sizeof(estructura));
			mi_argumento_estructura->iniciocaracter  = numcaracter[hilo] ;
			mi_argumento_estructura->inilinea = numlineas[hilo] ;
			mi_argumento_estructura->finlinea  = finlinea[hilo] ;
			mi_argumento_estructura->fp  = fp ;
			mi_argumento_estructura->tam_lineas  = tam_lineas ;
			status = pthread_create(&hilos[hilo], NULL,contarpalabras, (void *)mi_argumento_estructura);
			if(status < 0){
				fprintf(stderr, "Error al crear el hilo : %d\n", hilo);
				exit(-1);	
			}
		}

		//esperando la finalizacion de los hilos
		for(hilo=0;hilo<num_hilos;hilo++) {
			int status1 = pthread_join(hilos[hilo], retorno);
			if(status1 < 0){
				fprintf(stderr, "Error al esperar por el hilo 1\n");
				exit(-1);
			}
		}

		//terminar hilo que imprimi mensaje cada segundo
		pthread_cancel(hilomjs);

		printf("\n \n RESULTADO FINAL busqueda de palabras:\n");
		for(int i=0;i<numero_palabras;i++){
      		printf("palabra  %s: frecuencia final: %d \n",palabras[i],num_palabras[i]);
      	}
		pthread_exit(NULL);
		
	}else{
		printf("necesita enviar mas parametros: ./buscar dir_archivo num_hilos palabra1 palabra2..... palabraN.");
	}
}
