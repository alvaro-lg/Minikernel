/*
 *  minikernel/include/kernel.h
 *
 *  Minikernel. Versi�n 1.0
 *
 *  Fernando P�rez Costoya
 *
 */

/*
 *
 * Fichero de cabecera que contiene definiciones usadas por kernel.c
 *
 *      SE DEBE MODIFICAR PARA INCLUIR NUEVA FUNCIONALIDAD
 *
 */

#ifndef _KERNEL_H
#define _KERNEL_H

#include "const.h"
#include "HAL.h"
#include "llamsis.h"

/*
 * Posibles estados de un mutex
 */
#define MTX_NO_USADO 0		/* Entrada de tabla de mutex no usada */
#define MTX_BLOQUEADO 1	
#define MTX_DESBLOQUEADO 2

#define MTX_DESC_NO_USADO -1	/* Entrada de la lista de descriptores asociados a un proceso no usada */

/*
 * Errores asociados a un mutex
 */
#define MUTEX_NAME_EXIST -1
#define MUTEX_NAME_LONG -2
#define MUTEX_MAX_DESC -3
#define MUTEX_TABLE_FULL -4
#define MUTEX_CLOSED -5
#define MUTEX_NO_EXIST -6
#define MUTEX_LOCK_FAIL -7
#define MUTEX_UNLOCK_FAIL -8

/* constantes con los tipos de mutex que se pueden definir */
#define NO_RECURSIVO 0
#define RECURSIVO 1

/*
 *
 * Definici�n del tipo que corresponde con la entrada para la función tiempos_proceso().
 *
 */
struct tiempos_ejec {
    int usuario;
    int sistema;
};

/*
 *
 * Definicion del tipo que corresponde con el BCP.
 * Se va a modificar al incluir la funcionalidad pedida.
 *
 */
typedef struct BCP_t *BCPptr;

typedef struct BCP_t {
        int id;						/* ident. del proceso */
        int estado;					/* TERMINADO|LISTO|EJECUCION|BLOQUEADO */
        contexto_t contexto_regs;	/* copia de regs. de UCP */
        void * pila;				/* dir. inicial de la pila */
	BCPptr siguiente;				/* puntero a otro BCP */
	void *info_mem;					/* descriptor del mapa de memoria */
	unsigned int t_wake;			/* tiempo (ticks) en que el proceso se despertara */
	int mutex_ids[NUM_MUT_PROC];	/* descriptores e los mutex que posee el proceso */
} BCP;

/*
 *
 * Definicion del tipo que corresponde con la cabecera de una lista
 * de BCPs. Este tipo se puede usar para diversas listas (procesos listos,
 * procesos bloqueados en sem�foro, etc.).
 *
 */

typedef struct{
	BCP *primero;
	BCP *ultimo;
} lista_BCPs;


/*
 * Definición del tipo correspondiente con el mutex;
 */
typedef struct mutex_t *mutexptr;

typedef struct mutex_t {
	int p_id;					/* ident. del proceso que lo esta usando */
	int estado; 				/* MTX_NO_USADO|MTX_BLOQUEADO|MTX_DESBLOQUEADO */
	int tipo;					/* NO_RECURSIVO|RECURSIVO */
	int n_anidamiento;			/* Nº de veces que se ha bloqueado un mutex recursivo */
	char *nombre;				/* nombre asociado al mutex */
	lista_BCPs lista_bloqueados;/* representa la cola de procesos bloqueados por un mutex */
} mutex;

/*
 * Variable global que identifica el proceso actual
 */

BCP * p_proc_actual=NULL;

/*
 * Variable global que representa la tabla de procesos
 */

BCP tabla_procs[MAX_PROC];

/*
 * Variable global que representa la tabla de mutex
 */

mutex tabla_mutex[NUM_MUT];

/*
 * Variable global que representa la cola de procesos listos
 */
lista_BCPs lista_listos= {NULL, NULL};

/*
 * Variable global que representa la cola de procesos dormidos
 */
lista_BCPs lista_dormidos= {NULL, NULL};

/*
 * Variable global que representa la cola de procesos dormidos
 * a la espera de liberar el hueco de un mutex
 */
lista_BCPs lista_dormidos_mtx= {NULL, NULL};

/*
 *
 * Definici�n del tipo que corresponde con una entrada en la tabla de
 * llamadas al sistema.
 *
 */
typedef struct{
	int (*fservicio)();
} servicio;

/*
 *
 * Variable global empleada para gestionar el número de ticks pasados
 * en total (para debugging principalmente), en modo usuario y sistema
 * 
 */
unsigned long long int t_ticks = 0, t_usr = 0, t_sys = 0;

/*
 *
 * Variable global empleada para indicarcuando se esta accediendo a un parámetro
 * y poder gestionar debidamente una futurible excepción de memoria.
 * 
 */
unsigned int acc_param = 0;


/*
 * Prototipos de las rutinas que realizan cada llamada al sistema
 */
int sis_crear_proceso();
int sis_terminar_proceso();
int sis_escribir();
int sis_obtener_id_pr();
int sis_dormir();
int sis_tiempos_proceso();
int sis_crear_mutex();
int sis_abrir_mutex();
int sis_lock();
int sis_unlock();
int sis_cerrar_mutex();

/*
 * Variable global que contiene las rutinas que realizan cada llamada
 */
servicio tabla_servicios[NSERVICIOS]={	{sis_crear_proceso},
					{sis_terminar_proceso},
					{sis_escribir},
					{sis_obtener_id_pr},
					{sis_dormir},
					{sis_tiempos_proceso},
					{sis_crear_mutex},
					{sis_abrir_mutex},
					{sis_lock},
					{sis_unlock},
					{sis_cerrar_mutex}};

#endif /* _KERNEL_H */

