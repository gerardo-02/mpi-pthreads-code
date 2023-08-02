#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <time.h>

#define NUM_THREADS 4

double *vec;
double max_hilos = 0;
pthread_mutex_t lock;

/*
 * A cada hebra se le pasarán dos números enteros que conforman el rango del
 * vector en el que la hebra
 * va a buscar el mayor elemento.
 */
typedef struct t_struct{
	int primer_elemento;
	int ultimo_elemento;
} rango;

/*
 * Función ejecutada por cada hebra. Los parámetros de entrada constituyen
 * el rango del vector en el que se va a buscar el mayor elemento.
 * Simplemente se recorre el vector entero actualizando la variable max
 * si se encuentra un elemento mayor que ella, y cuando ha terminado la devuelve
 */
void* body(void* param){
	rango* MiRango = (rango*) param;
	for (int i = MiRango->primer_elemento; i < MiRango->ultimo_elemento; i++){
		if (vec[i] > max_hilos){
			pthread_mutex_lock(&lock);
			max_hilos = vec[i];
			pthread_mutex_unlock(&lock);
		}
	}
	return 0;
}

int main(int argc, char* argv[]){

	// Declaración de variables y asignación de valores al vector
	srand(time(NULL));
	int NUM_ELEMENTOS = atoi(argv[1]);
	struct t_struct rangos[NUM_THREADS];
	int cociente = NUM_ELEMENTOS / NUM_THREADS;
	int resto = NUM_ELEMENTOS % NUM_THREADS;

	vec = (double*)malloc(NUM_ELEMENTOS * sizeof(double));

	if (vec == NULL){
		printf("Memoria no reservada\n");
		exit(0);
	}

	for (int i=0; i<NUM_ELEMENTOS; i++){
		vec[i] = (double)rand();
	}

	for (int i=0; i<NUM_THREADS; i++){
		rangos[i].primer_elemento = i * cociente;
		if (i == NUM_THREADS-1)
			rangos[i].ultimo_elemento = rangos[i].primer_elemento + cociente + resto;
		else
			rangos[i].ultimo_elemento = rangos[i].primer_elemento + cociente;

	}

	// Búsqueda con hilos
	pthread_t threads[NUM_THREADS];
	pthread_attr_t attr;
	pthread_attr_init(&attr);

	pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);

	for(long int i = 0; i<NUM_THREADS; i++){
		pthread_create(&threads[i], 0, body, &(rangos[i]));
	}

	pthread_attr_destroy(&attr);

	// Búsqueda secuencial
	double max_secuencial = 0.0;
	for (int i=0; i<NUM_ELEMENTOS; i++){
		if (vec[i] > max_secuencial)
			max_secuencial = vec[i];
	}
	printf("El elemento mayor del vector es %f\n", max_secuencial);
	printf("El elemento mayor del vector es %f\n", max_hilos);

	pthread_exit(0);

}
