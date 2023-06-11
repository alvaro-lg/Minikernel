/*
 *  kernel/kernel.c
 *
 *  Minikernel. Versi�n 1.0
 *
 *  Fernando P�rez Costoya
 *
 */

/*
 *
 * Fichero que contiene la funcionalidad del sistema operativo
 *
 */

#include "kernel.h"	/* Contiene defs. usadas por este modulo */
#include <string.h> /* Para emplear la funcion strdup */

/* Funciones auxiliares relacionadas con los mutex */
/*
 * Devuelve el numero de descriptores que posee un proceso.
 */
int num_mutex_desc() {

	// Variables
	int i, n=0;

	// Recorremos los descriptores de mutex asociados al proceso
	for (i = 0; i < NUM_MUT_PROC; i++)
		if (p_proc_actual->mutex_ids[i] != MTX_DESC_NO_USADO)
			n++;

	return n;
}

/*
 * Añade un descriptor a la tabla de descriptores de mutex 
 * asociados al proceso. Devuelve 0 si todo va bien, 
 * MUTEX_MAX_DESC en caso de que no haya hueco.
 */
int add_mutex_desc(int id) {

	// Variables
	int i;

	// Recorremos los descriptores de mutex asociados al proceso
	for (i = 0; i < NUM_MUT_PROC; i++)
		if (p_proc_actual->mutex_ids[i] == MTX_DESC_NO_USADO) {
			p_proc_actual->mutex_ids[i] = id;
			return 0;
		}

	// Error
	return MUTEX_MAX_DESC;
}

/*
 * Elimina un descriptor a la tabla de descriptores de mutex 
 * asociados al proceso. Devuelve 0 si todo va bien, 
 * MUTEX_CLOSED en caso de que no lo tenga asociado.
 */
int del_mutex_desc(int id) {

	// Variables
	int i;

	// Recorremos los descriptores de mutex asociados al proceso
	for (i = 0; i < NUM_MUT_PROC; i++)
		if (p_proc_actual->mutex_ids[i] == id) {
			p_proc_actual->mutex_ids[i] = MTX_DESC_NO_USADO;
			return 0;
		}

	// Error
	return MUTEX_CLOSED;
}

/*
 * Devuelve el número de procesos que tienen abierto un determinado
 * mutex.
 */
int open_mutex_count(int id) {

	// Variables
	int i, j, n=0;

	// Recorremos los descriptores de mutex asociados al proceso
	for (i = 0; i < MAX_PROC; i++)
		for (j = 0; j < NUM_MUT_PROC; j++)
			if (tabla_mutex[tabla_procs[i].mutex_ids[j]].estado != MTX_NO_USADO
				&& tabla_procs[i].mutex_ids[j] == id) {
				n++;
			}

	// Error
	return n;
}

/*
 * Devuelve 0 si el nombre es válido, MUTEX_NAME_EXIST si ya existe
 * un con ese nombre, y MUTEX_NAME_LONG si el nombre es demasiado largo.
 */
int mutex_valid_name(char* nombre) {

	// Variables
	int i, j, len=0;

	// Comprobar longitud del nombre
	while (nombre[len] != '\0') len++;
	if (len+1 > MAX_NOM_MUT)
		return MUTEX_NAME_LONG;

	// Comprobar que no haya otro nombre igual
	for (i = 0; i < NUM_MUT; i++){
		if (tabla_mutex[i].estado != MTX_NO_USADO){
			for (j = 0; j <= len; j++){
				// Si son distintos
				if (nombre[j] != tabla_mutex[i].nombre[j] ||
					tabla_mutex[i].nombre[j] == '\0')
					break;

			}
			// Si son iguales
			if (nombre[j] == '\0')
				return MUTEX_NAME_EXIST;
		}
	}

	return 0;
}

/*
 * Busca un mutex por su nombre y devuelve su id si existe,
 * MUTEX_NAME_NO_EXIST si no.
 */
int mutex_search_name(char* nombre) {

	// Variables
	int i, j;

	// Comprobar que no haya otro nombre igual
	for (i = 0; i < NUM_MUT; i++){
		if (tabla_mutex[i].estado != MTX_NO_USADO){
			for (j = 0; j <= MAX_NOM_MUT; j++){
				// Si son distintos
				if (nombre[j] != tabla_mutex[i].nombre[j] ||
					tabla_mutex[i].nombre[j] == '\0')
					break;

			}
			// Si son iguales
			if (nombre[j] == tabla_mutex[i].nombre[j] && 
			nombre[j] == '\0')
				return i;
		}
	}

	// Error
	return MUTEX_NO_EXIST;
}

/*
 * Función que devuelve el índice de un hueco en la tabla de mutex,
 * MUTEX_TABLE_FULL si no quedan.
 */
int get_avail_mutex() {

	// Variables
	int i;

	for (i = 0; i < NUM_MUT; i++)
		if (tabla_mutex[i].estado == MTX_NO_USADO)
			return i;

	// Error
	return MUTEX_TABLE_FULL;
}

/*
 * Devuelve 0 si el proceso que la invoca tiene abierto el
 * mutex cuyo id se pasa como argumento, MUTEX_CLOSED si no.
 */
int mutex_is_opened(int id) {

	// Variables
	int i;

	// Recorremos los descriptores de mutex asociados al proceso
	for (i = 0; i < NUM_MUT_PROC; i++)
		if (p_proc_actual->mutex_ids[i] == id) {
			return 0;
		}

	// Error
	return MUTEX_CLOSED;
}

/*
 *
 * Funciones relacionadas con la tabla de procesos:
 *	iniciar_tabla_proc buscar_BCP_libre
 *
 */

/*
 * Funci�n que inicia la tabla de procesos
 */
static void iniciar_tabla_proc(){
	int i;

	for (i=0; i<MAX_PROC; i++) 
		tabla_procs[i] = (BCP) {
			.estado=NO_USADA,
			.mutex_ids ={[0 ... NUM_MUT_PROC-1] = MTX_DESC_NO_USADO}
		};
}

/*
 * Funci�n que busca una entrada libre en la tabla de procesos
 */
static int buscar_BCP_libre(){
	int i;

	for (i=0; i<MAX_PROC; i++)
		if (tabla_procs[i].estado==NO_USADA)
			return i;
	return -1;
}

/*
 *
 * Funciones que facilitan el manejo de las listas de BCPs
 *	insertar_ultimo eliminar_primero eliminar_elem
 *
 * NOTA: PRIMERO SE DEBE LLAMAR A eliminar Y LUEGO A insertar
 */

/*
 * Inserta un BCP al final de la lista.
 */
static void insertar_ultimo(lista_BCPs *lista, BCP * proc){
	if (lista->primero==NULL)
		lista->primero= proc;
	else
		lista->ultimo->siguiente=proc;
	lista->ultimo= proc;
	proc->siguiente=NULL;
}

/*
 * Elimina el primer BCP de la lista.
 */
static void eliminar_primero(lista_BCPs *lista){

	if (lista->ultimo==lista->primero)
		lista->ultimo=NULL;
	lista->primero=lista->primero->siguiente;
}

/*
 * Elimina un determinado BCP de la lista.
 */
static void eliminar_elem(lista_BCPs *lista, BCP * proc){
	BCP *paux=lista->primero;

	if (paux==proc)
		eliminar_primero(lista);
	else {
		for ( ; ((paux) && (paux->siguiente!=proc));
			paux=paux->siguiente);
		if (paux) {
			if (lista->ultimo==paux->siguiente)
				lista->ultimo=paux;
			paux->siguiente=paux->siguiente->siguiente;
		}
	}
}

/*
 *
 * Funciones relacionadas con la planificacion
 *	espera_int planificador
 */

/*
 * Espera a que se produzca una interrupcion
 */
static void espera_int(){
	int nivel;

	// TODO
	// printk("[%f] \tNO HAY LISTOS. ESPERA INT\n", (float) t_ticks/TICK);

	/* Baja al m�nimo el nivel de interrupci�n mientras espera */
	nivel=fijar_nivel_int(NIVEL_1);
	halt();
	fijar_nivel_int(nivel);
}

/*
 * Funci�n de planificacion que implementa un algoritmo FIFO.
 */
static BCP * planificador(){
	while (lista_listos.primero==NULL)
		espera_int();		/* No hay nada que hacer */
	return lista_listos.primero;
}

/*
 * Función que realiza el cambio de contexto al proceso 
 * de la siguiente rodaja
 */
static void siguiente_rodaja() {

	// Variables
	BCPptr old_p;
	int n_int;

	printk("[%f] \tSIGUIENTE RODAJA\n", (float) t_ticks/TICK);

	// Reestablecemos el contador de la rodaja
	t_proc = 0;

	// Inhibir interrupciones
	n_int = fijar_nivel_int(NIVEL_3);

	// Modificar listas de BCPs
	old_p = p_proc_actual;
	eliminar_primero(&lista_listos);

	if (p_proc_actual->estado == EJECUCION) { 						// Caso de que agote la rodaja
		p_proc_actual->estado = LISTO;
		//printk("ejecucion\n"); // TODO
		insertar_ultimo(&lista_listos, p_proc_actual);
	} else if (p_proc_actual->estado == BLOQUEADO) {
		if (p_proc_actual->t_wake > t_ticks){						// Caso bloqueado por dormir
			//printk("dormir\n"); // TODO
			insertar_ultimo(&lista_dormidos, p_proc_actual);
		} else if (!(mutex_is_opened((int)leer_registro(1)) < 0)){ 	// Caso bloqueado por lock
			//printk("lock %d\n", (int)leer_registro(1)); // TODO
			insertar_ultimo(&tabla_mutex[(int)leer_registro(1)].lista_bloqueados, p_proc_actual);
		} else if (get_avail_mutex() < 0) { 												// Caso bloqueado por open
			//printk("open \n"); // TODO
			insertar_ultimo(&lista_dormidos_mtx, p_proc_actual);
		} else {													// Caso bloqueado por terminal
			//printk("term \n"); // TODO
			insertar_ultimo(&lista_dormidos_term, p_proc_actual);
		}
	}

	// Deshinibir interrupciones
	fijar_nivel_int(n_int);

	// Invocar planificador para obtener nuevo proceso
	p_proc_actual= planificador();
	p_proc_actual->estado = EJECUCION;

	// Si es el unico proceso en el sistema, se duerme y se despierta no se deberia hacer c. contexto
	if (old_p->id != p_proc_actual->id) {
		if (old_p->estado == TERMINADO)
			printk("[%f] \tC.CONTEXTO POR FIN:", (float) t_ticks/TICK);
		else if (old_p->estado == BLOQUEADO)
			printk("[%f] \tC.CONTEXTO VOLUNTARIO:", (float) t_ticks/TICK);
		else
			printk("[%f] \tC.CONTEXTO INVOLUNTARIO:", (float) t_ticks/TICK);
		printk("%d a %d\n", old_p->id, p_proc_actual->id);

		// Cambio de contexto
		cambio_contexto(&(old_p->contexto_regs), &(p_proc_actual->contexto_regs));
	}
}

/*
 *
 * Funcion auxiliar que termina proceso actual liberando sus recursos.
 * Usada por llamada terminar_proceso y por rutinas que tratan excepciones
 *
 */
static void liberar_proceso(){

	// Variables
	int i;

	// Cerrando mutexes que tenga asociados
	for (i = 0; i < NUM_MUT_PROC; i++){
		if (p_proc_actual->mutex_ids[i] != MTX_DESC_NO_USADO){
			escribir_registro(1, p_proc_actual->mutex_ids[i]);
			sis_cerrar_mutex();
		}
		p_proc_actual->mutex_ids[i] = MTX_DESC_NO_USADO;
	}

	liberar_imagen(p_proc_actual->info_mem); /* liberar mapa */
	liberar_pila(p_proc_actual->pila); /* liberar pila */

	p_proc_actual->estado=TERMINADO;

	// Siguiente proceso
	siguiente_rodaja();

    return; /* no deber�a llegar aqui */
}

/*
 *
 * Funciones relacionadas con el tratamiento de interrupciones
 *	excepciones: exc_arit exc_mem
 *	interrupciones de reloj: int_reloj
 *	interrupciones del terminal: int_terminal
 *	llamadas al sistemas: llam_sis
 *	interrupciones SW: int_sw
 *
 */

/*
 * Tratamiento de excepciones aritmeticas
 */
static void exc_arit(){

	if (!viene_de_modo_usuario())
		panico("excepcion aritmetica cuando estaba dentro del kernel");


	printk("[%f] \tEXCEPCION ARITMETICA EN PROC %d\n", (float) t_ticks/TICK, p_proc_actual->id);
	liberar_proceso();

        return; /* no deber�a llegar aqui */
}

/*
 * Tratamiento de excepciones en el acceso a memoria
 */
static void exc_mem(){

	if (!viene_de_modo_usuario() && acc_param == 0)
		panico("excepcion de memoria cuando estaba dentro del kernel");

	printk("[%f] \tEXCEPCION DE MEMORIA EN PROC %d\n", (float) t_ticks/TICK, p_proc_actual->id);
	liberar_proceso();

    return; /* no deber�a llegar aqui */
}

/*
 * Tratamiento de interrupciones de terminal
 */
static void int_terminal(){

	// Variables
	BCPptr p;
	int n_int;
	char car;

	// Impedimos c. de contexto involuntarios por int sw
	n_int = fijar_nivel_int(NIVEL_1);

	car = leer_puerto(DIR_TERMINAL);

	// Almacenamos el caracter si hay hueco en el buffer
	if (read_chars - start_char < TAM_BUF_TERM) {
		char_buff[read_chars % TAM_BUF_TERM] = car;
		read_chars++;
	}

	printk("[%f] \tTRATANDO INT. DE TERMINAL %c CARACTERES LEIDOS %d\n", 
		(float) t_ticks/TICK, car, read_chars - start_char);

	// Reestablecemos el nivel de ejecucion
	fijar_nivel_int(n_int);

	// Despertando proceso bloqueado a la espera de leer un carcter si los hay
	p = lista_dormidos_term.primero;
	if (p != NULL) {
		// Cambiamos su estado
		p->estado = LISTO;

		// Inhibir interrupciones
		n_int = fijar_nivel_int(NIVEL_3);

		// Modificar listas de BCPs
		eliminar_elem(&lista_dormidos_term,p);
		insertar_ultimo(&lista_listos,p);

		// Deshinibir interrupciones
		fijar_nivel_int(n_int);
	}

    return;
}

/*
 * Tratamiento de interrupciones de reloj
 */
static void int_reloj(){

	// Variables
	BCPptr p = lista_dormidos.primero, p_next;
	unsigned int n_int;

	// TODO
	// printk("[%f] \tTRATANDO INT. DE RELOJ (TIEMPO RESTANTE DE RODAJA: %d)\n", 
	//  	(float) t_ticks/TICK, TICKS_POR_RODAJA - t_proc);

	// Incrementamos el numero de ticks actuales del kernel
	t_ticks++;

	// Gestion de tiempos si hay procesos activos
	if (lista_listos.primero != NULL) {
		if (viene_de_modo_usuario())
			t_usr++;
		else
			t_sys++;
		// Si el proceso no actua como el nulo, esta consumiendo su rodaja
		t_proc++;
	}

	// Tratando procesos esperando un plazo
	while (p != NULL) {

		// Reservamos el valor del siguiente
		p_next = p->siguiente;

		// Si se cumple el plazo, desbloqueamos el proceso
		if (p->t_wake <= t_ticks) {
			printk("[%f] \tPROCESO %d LISTO\n", (float) t_ticks/TICK, p->id);

			// Cambiamos su estado
			p->estado = LISTO;

			// Inhibir interrupciones
			n_int = fijar_nivel_int(NIVEL_3);

			// Modificar listas de BCPs
			eliminar_elem(&lista_dormidos,p);
			insertar_ultimo(&lista_listos,p);

			// Deshinibir interrupciones
			fijar_nivel_int(n_int);
		}

		// Avanzamos el puntero
		p = p_next;
	}

	// Pasado el suficiente tiempo, el proceso agota su rodaja
	if (t_proc >= TICKS_POR_RODAJA)
		activar_int_SW();
}

/*
 * Tratamiento de llamadas al sistema
 */
static void tratar_llamsis(){

	// Variables
	int nserv, res, n_int;

	// Impedimos c. de contexto involuntarios por int sw
	n_int = fijar_nivel_int(NIVEL_1);

	nserv=leer_registro(0);
	if (nserv<NSERVICIOS)
		res=(tabla_servicios[nserv].fservicio)();
	else
		res=-1;		/* servicio no existente */
	escribir_registro(0,res);

	// Reestablecemos el nivel de ejecucion
	fijar_nivel_int(n_int);

	return;
}

/*
 * Tratamiento de interrupciones software
 */
static void int_sw(){

	printk("[%f] \tTRATANDO INT. SW\n", (float) t_ticks/TICK);
	siguiente_rodaja();

	return;
}

/*
 *
 * Funcion auxiliar que crea un proceso reservando sus recursos.
 * Usada por llamada crear_proceso.
 *
 */
static int crear_tarea(char *prog){
	void * imagen, *pc_inicial;
	int error=0;
	int proc, n_int;
	BCP *p_proc;

	proc=buscar_BCP_libre();
	if (proc==-1)
		return -1;	/* no hay entrada libre */

	/* A rellenar el BCP ... */
	p_proc=&(tabla_procs[proc]);

	/* crea la imagen de memoria leyendo ejecutable */
	imagen=crear_imagen(prog, &pc_inicial);
	if (imagen) {
		p_proc->info_mem=imagen;
		p_proc->pila=crear_pila(TAM_PILA);
		fijar_contexto_ini(p_proc->info_mem, p_proc->pila, TAM_PILA,
			pc_inicial,
			&(p_proc->contexto_regs));
		p_proc->id=proc;
		p_proc->estado=LISTO;

		// Inhibir interrupciones
		n_int = fijar_nivel_int(NIVEL_3);

		// Modificar listas de BCPs
		/* lo inserta al final de cola de listos */
		insertar_ultimo(&lista_listos, p_proc);

		// Deshinibir interrupciones
		fijar_nivel_int(n_int);

		error= 0;
	}
	else
		error= -1; /* fallo al crear imagen */

	return error;
}

/*
 *
 * Rutinas que llevan a cabo las llamadas al sistema
 *	sis_crear_proceso sis_escribir
 *
 */

/*
 * Tratamiento de llamada al sistema crear_proceso. Llama a la
 * funcion auxiliar crear_tarea sis_terminar_proceso
 */
int sis_crear_proceso(){
	char *prog;
	int res;

	printk("[%f] \tPROC %d: CREAR PROCESO\n", (float) t_ticks/TICK, p_proc_actual->id);
	prog=(char *)leer_registro(1);
	res=crear_tarea(prog);
	return res;
}

/*
 * Tratamiento de llamada al sistema escribir. Llama simplemente a la
 * funcion de apoyo escribir_ker
 */
int sis_escribir()
{
	char *texto;
	unsigned int longi;

	texto=(char *)leer_registro(1);
	longi=(unsigned int)leer_registro(2);

	escribir_ker(texto, longi);
	return 0;
}

/*
 * Tratamiento de llamada al sistema terminar_proceso. Llama a la
 * funcion auxiliar liberar_proceso
 */
int sis_terminar_proceso(){

	printk("[%f] \tFIN PROCESO %d\n", (float) t_ticks/TICK, p_proc_actual->id);

	liberar_proceso();

    return 0; /* no deber�a llegar aqui */
}

/*
 * Función que implementa la primera funcionalidad a desarrollar (obtener ID).
 */
int sis_obtener_id_pr() {
	return p_proc_actual->id;
}

/*
 * Función que implementa la segunda funcionalidad a desarrollar (dormir).
 */
int sis_dormir() {

	// Variables
	unsigned int segundos;

	// Lectura de argumentos
	segundos=(unsigned int)leer_registro(1);

	// Bloquear proceso
	p_proc_actual->estado=BLOQUEADO;
	p_proc_actual->t_wake=t_ticks + segundos*TICK;

	// Siguiente proceso
	siguiente_rodaja();

	return 0;
}

/*
 * Función que implementa la tercera funcionalidad a desarrollar 
 * (contabilidad de uso del procesador).
 */
int sis_tiempos_proceso() {

	// Variables
	struct tiempos_ejec* tiempos;
	int n_int;

	// Lectura de argumentos
	tiempos=(struct tiempos_ejec *)leer_registro(1);

	// Gestionando argumentos erroneos
	if (tiempos != NULL && acc_param == 0) {

		// Inhibir interrupciones
		n_int = fijar_nivel_int(NIVEL_3);

		// Concurrencia mientras se accede a parametros
		acc_param = 1;
		tiempos->usuario=t_usr;
		tiempos->sistema=t_sys;
		acc_param = 0;

		// Deshinibir interrupciones
		fijar_nivel_int(n_int);
	}

	// Returning elapsed ticks
	return t_ticks;
}

/* Llamadas relacionadas con los mutexes */
/*
 * Función que elimina un determinado mutex cuyo id se pasa 
 * como parámetro.
 */
void eliminar_mutex(int id) {

	// Variables
	BCPptr p;
	int n_int;

	// Marcando el hueco como libre
	tabla_mutex[id].estado = MTX_NO_USADO;

	// Despertando a uno de los porcesos esperando a liberar un hueco
	p = lista_dormidos_mtx.primero;
	if (p != NULL) {
		p->estado = LISTO;

		// Inibir interrupciones
		n_int = fijar_nivel_int(NIVEL_3);

		// Modificar listas de BCPs
		eliminar_elem(&lista_dormidos_mtx,p);
		insertar_ultimo(&lista_listos,p);

		// Deshinibir interrupciones
		fijar_nivel_int(n_int);
	}
}

/*
 * Función que implementa la operación de crear un mutex de 
 * la cuarta funcionalidad a desarrollar (mutex).
 */
int sis_crear_mutex(){

	// Variables
	mutexptr m;
	char* nombre;
	int tipo, ret, id;

	// Lectura de argumentos
	nombre=(char*)leer_registro(1);
	tipo=(int)leer_registro(2);

	// Comprobar que el proceso puede tener mutex asignados
	if (num_mutex_desc() >= NUM_MUT_PROC) {
		printk("[%f] \tPROCESO %d POSEE EL MAXIMO DE DESCRIPTORES\n", (float) t_ticks/TICK, p_proc_actual->id);
		return MUTEX_MAX_DESC;
	}
	
	// Comprobar longitud del nombre y que no exista
	ret = mutex_valid_name(nombre);
	if (ret < 0) {
		if (ret == MUTEX_NAME_LONG) {
			printk("[%f] \tNOMBRBE DEL MUTEX DEMASIADO LARGO\n", (float) t_ticks/TICK);
			return MUTEX_NAME_LONG;
		} else if (ret == MUTEX_NAME_EXIST) {
			printk("[%f] \tYA EXISTE UN MUTEX LLAMADO %s\n", (float) t_ticks/TICK, nombre);
			return MUTEX_NAME_EXIST;
		}
	}

	// Comprobar que queden huecos libres
	ret = get_avail_mutex();
	while (ret < 0) {

		// Bloquar el proceso
		p_proc_actual->estado=BLOQUEADO;

		printk("[%f] \tPROCESO %d ESPERA A ELIMINAR UN MUTEX\n", (float) t_ticks/TICK, p_proc_actual->id);

		// Siguiente proceso
		siguiente_rodaja();
		
		// Comprobar de nuevo que no exista otro con ese nombre despues de la espera
		ret = mutex_valid_name(nombre);
		if (ret < 0) {
			printk("[%f] \tAL DESPERTAR %d EXISTE UN MUTEX LLAMADO %s\n", (float) t_ticks/TICK, p_proc_actual->id, nombre);
			return MUTEX_NAME_EXIST;
		}

		// Comprobar que siguen habiendo libres
		ret = get_avail_mutex();
	}

	// Inicializacion del mutex
	id = get_avail_mutex();
	m = &tabla_mutex[id];
	m->estado = MTX_DESBLOQUEADO;
	m->tipo = tipo;
	m->nombre = strdup(nombre);
	m->lista_bloqueados= (lista_BCPs) {NULL, NULL};
	if (tipo == RECURSIVO) m->n_anidamiento = 0;

	printk("[%f] \tPROCESO %d CREA EL MUTEX %d (%s)\n", (float) t_ticks/TICK, p_proc_actual->id, id, m->nombre);

	// Abriendo el mutex
	escribir_registro(1, (long) nombre); // Paso de parametros
	sis_abrir_mutex();

	// Devolver descriptor
	return id;
}

/*
 * Función que implementa la operación de abrir un mutex de 
 * la cuarta funcionalidad a desarrollar (mutex).
 */
int sis_abrir_mutex(){

	// Variables
	char* nombre;
	int ret;

	// Lectura de argumentos
	nombre=(char*)leer_registro(1);

	// Comprobar que el proceso puede tener mutex asignados
	if (num_mutex_desc() >= NUM_MUT_PROC) {
		printk("[%f] \tPROCESO %d POSEE EL MAXIMO DE DESCRIPTORES\n", (float) t_ticks/TICK, p_proc_actual->id);
		return MUTEX_MAX_DESC;
	}

	// Buscar el mutex
	ret = mutex_search_name(nombre);
	if (ret == MUTEX_NO_EXIST) {
		// Error
		printk("[%f] \tNO EXISTE EL MUTEX %s\n", (float) t_ticks/TICK, nombre);
		return MUTEX_NO_EXIST;
	}

	// Asociando el descriptor al proceso
	add_mutex_desc(ret);
	
	printk("[%f] \tPROCESO %d ABRE EL MUTEX %d (%s)\n", (float) t_ticks/TICK, sis_obtener_id_pr(), ret, tabla_mutex[ret].nombre);

	return ret;
}

/*
 * Función que implementa la operación de bloquear un mutex de 
 * la cuarta funcionalidad a desarrollar (mutex).
 */
int sis_lock(){
	
	// Variables
	int id;

	// Lectura de argumentos
	id=(int)leer_registro(1);

	// Caso de que el mutex no se haya creado
	if (tabla_mutex[id].estado == MTX_NO_USADO) {
		printk("[%f] \tMUTEX %d NO CREADO\n", (float) t_ticks/TICK, id);
		return MUTEX_NO_EXIST;
	}

	// Caso de que el proceso no tenga abierto el mutex
	if (mutex_is_opened(id) < 0){
		printk("[%f] \tPROCESO %d NO TIENE EL MUTEX %d ABIERTO\n", (float) t_ticks/TICK, p_proc_actual->id, id);
		return MUTEX_CLOSED;
	}

	// Caso de que el mutex sea no recursivo y lo quiera bloquear el dueño
	if (tabla_mutex[id].tipo == NO_RECURSIVO && tabla_mutex[id].p_id == sis_obtener_id_pr()) {
		printk("[%f] \tPROCESO %d INTENTA TOMAR EL MUTEX NO RECURSIVO %d (%s)\n", 
			(float) t_ticks/TICK, sis_obtener_id_pr(), id, tabla_mutex[id].nombre);
		return MUTEX_LOCK_FAIL;
	}

	// No se bloquea si es recursivo y lo quiere bloquear el dueño, si no se intenta tomar
	if (!(tabla_mutex[id].tipo == RECURSIVO && tabla_mutex[id].p_id == sis_obtener_id_pr()))
		while (tabla_mutex[id].estado == MTX_BLOQUEADO){
			// Bloquar el proceso
			p_proc_actual->estado=BLOQUEADO;

			printk("[%f] \tPROCESO %d ESPERA A LIBERAR MUTEX %d (%s)\n", (float) t_ticks/TICK, p_proc_actual->id, id, tabla_mutex[id].nombre);

			// Siguiente proceso
			siguiente_rodaja();
		}

	// Se toma el mutex
	tabla_mutex[id].estado = MTX_BLOQUEADO;
	tabla_mutex[id].p_id = sis_obtener_id_pr();

	printk("[%f] \tPROCESO %d TOMA EL MUTEX %d (%s)\n", (float) t_ticks/TICK, p_proc_actual->id, id, tabla_mutex[id].nombre);
	
	// Si es recursivo
	if (tabla_mutex[id].tipo == RECURSIVO){
		tabla_mutex[id].n_anidamiento++;
		printk("\t\tNIVEL DE ANIDAMIENTO DEL MUTEX RECURSIVO: %d\n", (float) t_ticks/TICK, tabla_mutex[id].n_anidamiento);
	}

	return 0;
}

/*
 * Función que implementa la operación de desbloquear un mutex de 
 * la cuarta funcionalidad a desarrollar (mutex).
 */
int sis_unlock(){
	
	// Variables
	int id, n_int;
	BCPptr p;

	// Lectura de argumentos
	id=(int)leer_registro(1);

	// Comprobar que es dueño del mutex
	if (sis_obtener_id_pr() != tabla_mutex[id].p_id) {
		printk("[%f] \tPROCESO %d INTENTA LIBERAR EL MUTEX %d (%s) PERO NO LO POSEE (LO POSEE %d)\n", 
			(float) t_ticks/TICK, p_proc_actual->id, id, tabla_mutex[id].nombre, tabla_mutex[id].p_id);
		return 0; // Se ignora
	}

	printk("[%f] \tPROCESO %d LIBERA EL MUTEX %d (%s)\n", (float) t_ticks/TICK, p_proc_actual->id, id, tabla_mutex[id].nombre);

	// Si es recursivo
	if (tabla_mutex[id].tipo == RECURSIVO) {
		if (tabla_mutex[id].n_anidamiento > 0)
			tabla_mutex[id].n_anidamiento--;
		else {
			printk("[%f] \tPROCESO %d INTENTA LIBERAR EL MUTEX %d (%s) DEMASIADAS VECES\n", 
			(float) t_ticks/TICK, p_proc_actual->id, id, tabla_mutex[id].nombre);
			return MUTEX_UNLOCK_FAIL;
		}
		printk("\t\tNIVEL DE ANIDAMIENTO DEL MUTEX RECURSIVO: %d\n", (float) t_ticks/TICK, tabla_mutex[id].n_anidamiento);		
	}

	// Sólo se libera realmente si el nivel de anidamiento es 0
	if (tabla_mutex[id].tipo == NO_RECURSIVO || 
		(tabla_mutex[id].tipo == RECURSIVO && tabla_mutex[id].n_anidamiento == 0)){
		// Se libera el mutex
		tabla_mutex[id].estado = MTX_DESBLOQUEADO;

		// Despertando proceso bloqueado a la espera de liberar el mutex si los hay
		p = tabla_mutex[id].lista_bloqueados.primero;
		if (p != NULL) {
			// Cambiamos su estado
			p->estado = LISTO;

			// Inhibir interrupciones
			n_int = fijar_nivel_int(NIVEL_3);

			// Modificar listas de BCPs
			eliminar_elem(&tabla_mutex[id].lista_bloqueados,p);
			insertar_ultimo(&lista_listos,p);

			// Deshinibir interrupciones
			fijar_nivel_int(n_int);
		}
	}

	return 0;
}

/*
 * Función que implementa la operación de cerrar un mutex de 
 * la cuarta funcionalidad a desarrollar (mutex).
 */
int sis_cerrar_mutex(){
	
	// Variables
	int id, ret;

	// Lectura de argumentos
	id=(int)leer_registro(1);

	// Comprobando que el proceso tiene el mutex abierto
	ret = del_mutex_desc(id);
	if (ret == MUTEX_CLOSED) {
		printk("[%f] \tPROCESO %d NO TIENE EL MUTEX %d (%s) ABIERTO\n", (float) t_ticks/TICK, p_proc_actual->id, id, tabla_mutex[id].nombre);
		return MUTEX_CLOSED;
	}

	printk("[%f] \tPROCESO %d CIERRA EL MUTEX %d (%s)\n", (float) t_ticks/TICK, p_proc_actual->id, id, tabla_mutex[id].nombre);

	// Si el dueño es quien lo cierra, se desbloquea
	if (tabla_mutex[id].p_id == sis_obtener_id_pr() &&
		tabla_mutex[id].estado == MTX_BLOQUEADO) {
		// Reestablecemos el numero de anidamientos si es recursivo
		if (tabla_mutex[id].tipo == RECURSIVO)
			tabla_mutex[id].n_anidamiento = 1; // Para que al hacer el unlock sea 0 y se despierte a los demas
		sis_unlock();
	}

	// Desasociando el descriptor del proceso
	del_mutex_desc(id);

	// Si no hay nadie que tenga abierto el mutex, se elimnina
	if (open_mutex_count(id) == 0) {
		printk("\t\tSE ELIMINA EL MUTEX %d (%s), NINGUN PROCESO LO USA\n", id, tabla_mutex[id].nombre);
		eliminar_mutex(id);
	}

	return 0;
}

int sis_leer_caracter() {

	// Variables
	char ret;
	int n_int, eval;

	// Inhibir interrupciones de terminal para evaluar condicion
	n_int = fijar_nivel_int(NIVEL_2);

	eval = start_char - read_chars; 
	
	// Deshinibir interrupciones de terminal
	fijar_nivel_int(n_int);

	// Caso de  que no queden caracteres en el buffer
	while (eval >= 0) {
		// Bloquar el proceso
		p_proc_actual->estado=BLOQUEADO;

		printk("[%f] \tPROCESO %d ESPERA A LEER UN CARACTER\n", (float) t_ticks/TICK, p_proc_actual->id);

		// Siguiente proceso
		siguiente_rodaja();

		// Inhibir interrupciones de terminal para evaluar condicion
		n_int = fijar_nivel_int(NIVEL_2);

		eval = start_char - read_chars; 
		
		// Deshinibir interrupciones de terminal
		fijar_nivel_int(n_int);
	}

	// Leemos el caracter
	ret = char_buff[start_char % TAM_BUF_TERM];
	start_char++;

	return ret;
}

/*
 *
 * Rutina de inicializaci�n invocada en arranque
 *
 */
int main(){
	/* se llega con las interrupciones prohibidas */

	instal_man_int(EXC_ARITM, exc_arit); 
	instal_man_int(EXC_MEM, exc_mem); 
	instal_man_int(INT_RELOJ, int_reloj); 
	instal_man_int(INT_TERMINAL, int_terminal); 
	instal_man_int(LLAM_SIS, tratar_llamsis); 
	instal_man_int(INT_SW, int_sw); 

	iniciar_cont_int();		/* inicia cont. interr. */
	iniciar_cont_reloj(TICK);	/* fija frecuencia del reloj */
	iniciar_cont_teclado();		/* inici cont. teclado */

	iniciar_tabla_proc();		/* inicia BCPs de tabla de procesos */

	/* crea proceso inicial */
	if (crear_tarea((void *)"init")<0)
		panico("no encontrado el proceso inicial");
	
	/* activa proceso inicial */
	p_proc_actual=planificador();
	cambio_contexto(NULL, &(p_proc_actual->contexto_regs));
	panico("S.O. reactivado inesperadamente");
	return 0;
}
