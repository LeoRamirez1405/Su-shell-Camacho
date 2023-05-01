# Su-Shell 

Una shell es un **programa o interfaz que traduce los comandos del usuario a instrucciones que el sistema operativo puede entender y ejecutar**. Con el objetivo de desarrollar una instancia simple de este tipo de programa realizamos el presente proyecto en el lenguaje C.
&nbsp;
### Bibliotecas utilizadas (a modo de comentario a continuación de las bibliotecas se muestran las funciones utilizadas de cada una de ellas) :

```c
#include <sys/wait.h>
//waitpid() Espera a que un proceso hijo termine su ejecución.

#include <unistd.h>
//chdir() Cambia el directorio actual de trabajo del proceso en ejecución.
//fork() Crear un nuevo proceso mediante la duplicación del proceso que la llama. 
//pid_t Es un tipo de datos que se utiliza para representar el identificador de un proceso en el sistema operativo. 

#include <stdlib.h>
//malloc() Asigna memoria dinámicamente durante la ejecución del programa.
//realloc() Cambia el tamaño de la memoria asignada previamente con malloc().
//free() Liberar la memoria asignada previamente con malloc() o realloc().
//exit() Termina la ejecución del programa.
//execvp() Ejecutar un programa en el mismo proceso.
//EXIT_SUCCESS, EXIT_FAILURE Constantes predefinidas que se utilizan para indicar el estado de salida del programa. 

#include <stdio.h>
//fprintf() Imprime mensajes en la pantalla.
//printf() Imprimie mensajes en un archivo de salida.
//stderr Archivo de salida estándar que se utiliza para imprimir mensajes de error en la pantalla o en un archivo. 
//perror() Imprime un mensaje de error en la pantalla o en un archivo.

#include <string.h>
//strcmp() Compara dos cadenas de caracteres retornando un valor entero que indica si las cadenas son iguales o no.
//strtok() Divide una cadena de caracteres en subcadenas más pequeñas, o "tokens".

#include <fcntl.h>
//open() Abre un archivo en modo lectura o escritura.
//close() Cierra un archivo que se ha abierto previamente con la función "open()".
```


&nbsp;
#### Ciclo de ejecución:

```C
void loop(void)
{
    char *line;
    char **args;
    int status;
    do {
        printf("Su-Shell $ ");
        line = rev_comment(read_line());
        if(strchr(line, '|')) 
        {
            pipes_handler(line);
        }
        else 
        {
        	args = split_line(line);
        	status = execute(args);
        	free(args);
        }
        free(line);  
    }while (status);
}
```

Este método se utiliza para ejecutar una shell interactiva que lee la entrada del usuario, analiza la existencia de comentarios, divide la entrada en tokens, ejecuta los comandos ingresados por el usuario y repite el proceso hasta que el usuario decide salir de la shell. La función **pipes_handler()** se utiliza para manejar los comandos que contienen pipes (|), mientras que la función **execute()** se utiliza para ejecutar los comandos ingresados por el usuario.

La función **strchr(line, '|')** en C se utiliza para buscar la primera aparición de un caracter específico (en este caso, '|') en una cadena de caracteres (en este caso, "line").


&nbsp;
#### Análisis de comentarios :

```c
char *rev_comment(char * line)
{
    int cantchar = strlen(line);
    char * newline = line;

    for (int i = 0; i < cantchar; i++)
    {
        
        if(line[i] == '#' && (i == 0 || line[i-1] == ' '))
        {
            newline = (char*) malloc((i * sizeof(char)));
            newline = strncpy(newline,line,i);
            break;
        }
    }  
    return newline;
}
```

 Esta función se utiliza para eliminar comentarios de una línea de texto suministrada por el usuario. Se recorre la cadena de caracteres en busca del caracter **"#"**  que esté precedido por un espacio en blanco o que esté al principio de la línea. Cuando se encuentra un comentario, la función utiliza la función **malloc** para asignar memoria para una nueva cadena de caracteres **"newline"** que contiene todos los caracteres de **line** antes del comentario.

La función utiliza la función **strncpy** para copiar los caracteres de line a **"newline"**, deteniéndose en el carácter **"#"** que indica el inicio del comentario.

Para concluir la función devuelve la cadena de caracteres **"newline"** desprovista de comentarios.


&nbsp;
#### Lectura de la entrada del usuario :

```c
char *read_line()
{
    char *line = NULL;
    ssize_t bufsize = 0; 
    if (getline(&line, &bufsize, stdin) == -1)
    {
        if (feof(stdin)) 
        {
            exit(EXIT_SUCCESS); 
        } 
        else 
        {
            perror("readline");
            exit(EXIT_FAILURE);
        }
    }
    return line;
}
```

Esta función se utiliza para leer una línea completa de entrada desde stdin y manejar posibles errores que puedan ocurrir durante la lectura.

La función **getline(&line, &bufsize, stdin)** lee una línea completa de entrada desde stdin y la almacena en la variable **line**, que es un puntero a un puntero a un caracter.

La función **feof(stdin)** se utiliza para verificar si se ha alcanzado el final del flujo de entrada stdin.


&nbsp;
#### Separación en "tokens" :

```C
char **split_line(char *line)
{
    int bufsize = TOK_BUFSIZE, position = 0;
    char **tokens = malloc(bufsize * sizeof(char*));
    char *token;
    if (!tokens) 
    {
        fprintf(stderr, "Error al hacer malloc\n");
        exit(EXIT_FAILURE);
    }
    token = strtok(line, TOK_DELIM);
    
    while (token != NULL) 
    {
        tokens[position] = token;
        position++;
        if (position >= bufsize) 
        {
            bufsize += TOK_BUFSIZE;
            tokens = realloc(tokens, bufsize * sizeof(char*));
            if (!tokens) 
            {
                fprintf(stderr, "Error al hacer realloc\n");
                exit(EXIT_FAILURE);
            }
        }
        token = strtok(NULL, TOK_DELIM);
    }

    tokens[position] = NULL;
    no_elem_tok = position;
    return tokens;
}
```

Este método divide una cadena de caracteres en tokens utilizando caracteres delimitadores específicos y devuelve un arreglo de punteros a caracteres que apuntan a los tokens encontrados.

La función **split_line()** se utiliza para dividir una cadena de caracteres en tokens (palabras).

**Constantes** : **\#define TOK_BUFSIZE 64** Especifica el tamaño inicial del búfer de tokens;  **\#define TOK_DELIM " \t\r\n\a"** que se utiliza para especificar los caracteres delimitadores que se utilizarán para dividir la cadena de entrada en tokens.

La función comienza asignando memoria para el arreglo de punteros a caracteres "tokens" utilizando la función **malloc()**. Si la asignación de memoria falla, se imprime un mensaje de error y se llama a la función "exit()" para terminar el programa.

Luego, con la función "strtok()" se divide la cadena de entrada en tokens utilizando los caracteres delimitadores especificados en **TOK_DELIM**. La función **strtok()** devuelve un puntero al primer token encontrado en la cadena de entrada. Luego se llama a este función repetidamente en un  while hasta que se hayan encontrado todos los tokens en la cadena de entrada.

Dentro del while, se asigna el puntero al token actual al arreglo de punteros a caracteres "tokens". Si el número de tokens encontrados es mayor o igual al tamaño del búfer de tokens, se asigna más memoria al arreglo "tokens" utilizando la función **realloc()**. Si la asignación de memoria falla, se imprime un mensaje de error y se llama a la función **exit()** para terminar el programa.

Después de que se han encontrado todos los tokens, se asigna un puntero nulo (NULL) al final del arreglo "tokens" para indicar el final de los tokens. Además, se guarda la cantidad de elementos en el arreglo en la variable global **no_elem_tok**.


&nbsp;
#### Manejo de los pipes :

```c
void pipes_handler(char *line) 
{
    //split
    char *arg; 
    char **args = malloc(RL_BUFSIZE);  
    int count; 
    int pipe_number = count_pipes(line); 
    count = 0;  
    while((arg = strtok_r(line, "|", &line))) 
    {
       args[count] = arg; 
       count ++; 
    }

    //handler
    int exit_value;  
    int infd; 
    int pipefd[2]; 

    //Ciclo por todos los comandos entre pipes
    for (int i = 0; i <= pipe_number; i++) 
    {
        //Crea un nuevo pipe 
        if (pipe(pipefd) == -1) { 
            perror("pipe"); 
            exit(EXIT_FAILURE); 
        }
        //Hacemos fork 
        pid_t pid; 
        pid = fork(); 
        if (pid == -1) {
            perror("fork"); 
            return; 
        } else if(pid == 0)  //Hijo
        { 
            //Para todos menos el primer comando, conecta stdin con pipefd[0]
            if(i != 0) { 
                dup2(infd, 0); 
            }
            //Para todos menos el ultimo comando, conecta stdout con pipefd[1] 
            if (i != pipe_number) { 
                dup2(pipefd[1], 1); 
            }
            //Ejecutar el comando actual
            execute(split_line(args[i])); 
            exit(1); 
        } else {
            //Espera a que termine el hijo, guarda pipefd[0] para la siguiente iteracion y cierra pipefd[1]
            wait(&exit_value); 
            infd = pipefd[0]; 
            close(pipefd[1]); 
        }
    }
    return; 
}
```

Esta función  se encarga de manejar los comandos que contienen pipes ("|") en una línea de entrada. La función toma la línea de entrada como argumento y la divide en una matriz de argumentos, donde cada argumento es un comando separado por pipes. Luego, la función itera a través de cada comando y crea un nuevo proceso hijo para manejar cada comando. Los procesos hijos se conectan mediante pipes para que la salida del comando anterior se convierta en la entrada del siguiente. Luego la función espera a que todos los procesos hijos terminen y devuelve el control al proceso padre.

La función **count_pipes** es una función auxiliar que cuenta el número de pipes en la línea de entrada.


&nbsp;
#### Ejecución de comandos :

```c
int execute(char **args)
{
    if (args[0] == NULL) 
    {
        //Linea vacia.
        return 1;
    }
    int cambiofd = -1;
    int cambiofdin = -1;
    int archivo;
    int archivoin;
    int mask_result = -1;
    int result;
    int fd = dup(1);

    
    if(no_elem_tok>2)
    {
        for (int k = 0; k < len_red_mod(); k++) 
        {
            if (strcmp(args[no_elem_tok-2], red_mod_srt[k]) == 0) 
            {
                char ** newargs = (char**) malloc((no_elem_tok - 1) * sizeof(char*));
                cambiofd = 1;   
                archivo = red_out_init(args[no_elem_tok-1],red_mod_srt[k]);

                for (int i = 0; i < no_elem_tok - 2; i++) 
                {
                    newargs[i] = (char*) malloc((strlen(args[i]) + 1) * sizeof(char));
                    strncpy(newargs[i], args[i], strlen(args[i]));
                    newargs[i][strlen(args[i])] = '\0';
                }

                newargs[no_elem_tok - 2] = NULL;    
                args = newargs;
                no_elem_tok = no_elem_tok -2;
            }
            if (cambiofd == 1)
            {
                break;
            }
        }
    }
    
    if(no_elem_tok>2)
    {
        if (strcmp(args[no_elem_tok-2],"<") == 0) 
        {
            char ** newargs = (char**) malloc((no_elem_tok - 1) * sizeof(char*));
            cambiofdin = 1;   
            archivoin = red_in_init(args[no_elem_tok-1]); 
            if (archivoin == -3)
            {
                perror("Mensaje de error:");
                if (cambiofd == 1)
                {
                    red_out_end(archivo,fd);
                }
                return 1;
            }

            for (int i = 0; i < no_elem_tok - 2; i++) 
            {
                newargs[i] = (char*) malloc((strlen(args[i]) + 1) * sizeof(char));
                strncpy(newargs[i], args[i], strlen(args[i]));
                newargs[i][strlen(args[i])] = '\0';
            }
            newargs[no_elem_tok - 2] = NULL;    
            args = newargs;
            no_elem_tok = no_elem_tok -2;
        }
    }
    for (int i = 0; i < num_builtins(); i++) 
    {
        if (strcmp(args[0], builtin_str[i]) == 0) 
        {
            mask_result = 0;
            result = (*builtin_func[i])(args);
        }
    }

        if(mask_result == -1)
        {
            result = launch(args);
        }

    if(cambiofd == 1)
    {
        red_out_end(archivo,fd);
    }
    if(cambiofdin == 1)
    {
        red_in_end(archivoin,fd);
    }
    return result;
}

```

Esta función toma un conjunto de argumentos (que se pasan como una matriz de cadenas de caracteres), verifica si se trata de un comando interno (cd, help, exit), ejecuta el comando interno si es uno de los comandos internos conocidos, o de lo contrario lanza un proceso externo para ejecutar el comando.

Se comienza comprobando si se ha ingresado un comando vacío y en caso de no ser así la función busca si hay redirecciones de entrada o salida en los argumentos ingresados y realiza los cambios necesarios en los descriptores de archivo para redirigir la entrada o salida del comando (**red_in_init**, **red_out_init**, **red_in_end**, **red_out_end**).

A continuación, se busca si el comando es uno de los comandos internos conocidos, que se definen en otras funciones del programa. Si es así, la función llama a la función correspondiente para ejecutar el comando interno. Si no es un comando interno, la función llama a **launch** para lanzar un proceso externo y ejecutar el comando.

Finalmente, la función devuelve el resultado de la ejecución del comando y restaura los descriptores de archivo originales si se realizaron cambios debido a redirecciones de entrada o salida.


&nbsp;
#### Redireccionamientos :

*in* : Simboliza que se está redireccionando la entrada.

*out* : Simboliza que se está redireccionado la salida.

 *init* : Se cambia la entrada estándar original por otra entrada dada directamente por el usuario o como consecuencia de la ejecución de un comando.

*end* : Una vez ejecutado el comando volver los valores de entrada y salida estandar a su valor original.

#### Redireción de la entrada estándar :
```c
int red_in_init(char * nameFile)
{
    char *nombreArchivo = nameFile;
    int archivo = open(nombreArchivo, O_RDONLY ,0644);

    if (archivo == -1) 
    {
        printf("Error abriendo archivo %s", nombreArchivo);
        return -3;
    }
    dup2(archivo,0);
    return archivo;
}
```

 Esta función se utiliza para redirigir la entrada estándar de un programa a un archivo especificado por el usuario. La función toma un argumento de tipo **char*** que es el nombre del archivo que se va a abrir, utiliza la función **open()** para abrir el archivo en modo de solo lectura y asigna el descriptor de archivo devuelto por **open()** a una variable.

Si la función **open()** devuelve -1 significa que hubo un error al intentar abrir el archivo. En ese caso, la función imprime un mensaje de error y devuelve -3.

Si se pudo abrir correctamente el archivo, la función utiliza la función **dup2** para duplicar el descriptor de archivo "archivo" en el descriptor de archivo 0, que es la entrada estándar del programa. Esto redirige la entrada estándar del programa al archivo especificado.

Finalmente, la función devuelve el descriptor de archivo "archivo", que se puede utilizar más adelante para cerrar el archivo o realizar otras operaciones de entrada y salida en el archivo redirigido.

&nbsp;
#### Redirección de la salida estándar :
```c
int red_out_init(char * nameFile,char * type)
{
    char *nombreArchivo = nameFile;
    int archivo=NULL; 
    if( type == ">")
    {
        archivo = open(nombreArchivo, O_WRONLY | O_CREAT | O_TRUNC,0644);
    } 
    else if(type == ">>")
    { 
        archivo = open(nombreArchivo, O_APPEND | O_CREAT | O_WRONLY,0644);
    }
    // Si por alguna razón el archivo no fue abierto, salimos inmediatamente
    if (archivo == -1) 
    {
        printf("Error abriendo archivo %s", nombreArchivo);
        return 1;
    }
    dup2(archivo,1);
    
    return archivo;
}
```

Esta función redirige la salida estándar a un archivo especificado por el usuario. 

La función recibe dos parámetros: **nameFile**, que es una cadena de caracteres que representa el nombre del archivo que se va a crear o sobrescribir, y **type**, que es una cadena de caracteres que indica el tipo de redirección de salida que se debe realizar (> para sobrescribir el archivo o >> para agregar al archivo existente).

Luego se determina el tipo de redirección que se debe realizar. Si **type** es igual a **">"**, entonces la función **open** se utiliza para crear un nuevo archivo o sobrescribir un archivo existente. Si type es igual a **">>"**, entonces la función open se utiliza para agregar datos al final de un archivo existente o crear el archivo en caso de que este no exista con anterioridad.

Si la función **open** no puede abrir el archivo especificado, se imprime un mensaje de error y la función devuelve 1.

Después de abrir el archivo, se utiliza la función **dup2** para duplicar el descriptor de archivo del archivo abierto en el descriptor de archivo 1, que es el descriptor de archivo estándar de salida. Esto redirige la salida estándar a **"archivo"**.

Finalmente, la función devuelve el descriptor de archivo del archivo abierto.

&nbsp;
#### Finalización de la redirección de la entrada estándar (se regresa al estado original):
```c
int red_in_end(int nameFile,int fd)
{
    dup2(fd,0);
    close(nameFile);
    return 0;
}
```

Esta función finaliza la redirección de la entrada estándar a un archivo. Recibe dos parámetros: **"nameFile"**, que es un descriptor de archivo que representa el archivo que se ha abierto para redirigir la entrada estándar, y **"fd"**, que es el descriptor de archivo original de la entrada estándar antes de la redirección.

Se utiliza **dup2** para duplicar el descriptor de archivo **"fd"** en el descriptor de archivo 0, que es el descriptor de archivo estándar de entrada. Esto restaura la entrada estándar original. Luego se cierra el archivo **"nameFile"** utilizando la función **close** para cerrar el archivo **"nameFile"**. Esto libera los recursos utilizados por el archivo.

Finalmente, la función devuelve 0 para indicar que la operación se ha completado correctamente.

&nbsp;
#### Finalización de la redirección de la salida estándar (se regresa al estado original)
```c
int red_out_end(int nameFile,int fd)
{
    dup2(fd,1);
    close(nameFile);
    return 0;
}
```

Esta función se utiliza para finalizar la redirección de la salida estándar a un archivo. Recibe dos parámetros: **"nameFile"**, que es un descriptor de archivo que representa el archivo que se ha creado o abierto para redirigir la salida estándar, y **"fd"**, que es el descriptor de archivo original de la salida estándar antes de la redirección.

Se utiliza **dup2** para duplicar el descriptor de archivo **"fd"** en el descriptor de archivo 1, que es el descriptor de archivo estándar de salida. Esto restaura la salida estándar original. Luego se utiliza la función **close()** para cerrar el archivo **"nameFile"**. Esto libera los recursos utilizados por el archivo.

Finalmente, la función devuelve 0 para indicar que la operación se ha completado correctamente.

&nbsp;
#### Comandos no internos :

```c
int launch(char **args)
{
    pid_t pid, wpid;
    int status;    
    pid = fork();
    if (pid == 0) 
    {
        //Proceso hijo
        if (execvp(args[0], args) == -1) 
        {
            perror("Mensaje de error:");
        }
    exit(EXIT_FAILURE);
    }

    else if (pid < 0) 
    {
    perror("Error en pid haciendo fork: ");
    } 

    else 
    {
    //Proceso padre
        do 
        {
            wpid = waitpid(pid, &status, WUNTRACED);
        }
        while (!WIFEXITED(status) && !WIFSIGNALED(status));
    }

    return 1;
}

```

Inicialmente se crea un nuevo proceso utilizando la función **fork()** y almacena el identificador del proceso hijo en una variable llamada **pid**. Luego si **pid** es igual a 0, significa que el proceso actual es el proceso hijo y se ejecuta el código dentro del condicional **if**.

Dentro del **if** se utiliza la función **execvp()** para ejecutar el comando pasado como argumento en **args[0]**. Esta función reemplaza el proceso actual con el proceso del comando ejecutado. Si **execvp()** devuelve -1, significa que se produjo un error al ejecutar el comando y se imprime un mensaje de error.

Si **pid** es menor que 0, significa que se produjo un error al crear el proceso hijo y se imprime un mensaje de error.

Si **pid** es mayor que 0, significa que el proceso actual es el proceso padre y se ejecuta el código dentro del **else**. El padre espera a que el proceso hijo termine de ejecutar utilizando la función **waitpid()**. Esta función suspende la ejecución del proceso padre hasta que el proceso hijo especificado por **pid** termine de ejecutar. El estado de salida del proceso hijo se almacena en **status**. El ciclo se repite hasta que el proceso hijo termine de ejecutar.

Finalmente, la función devuelve 1.


&nbsp;
#### Comandos Internos :

#### cd

```c
int cd(char **args)
{
    if (args[1] == NULL) 
    {
        fprintf(stderr, "Se esperaban argumentos para \"cd\"\n");
    } 
    else 
    {
        if (chdir(args[1]) != 0) 
        {
            perror("Error:");
        }
    }  
    return 1;
}
```

Esta función se utiliza para cambiar el directorio de trabajo actual del proceso.

Inicialmente, la función verifica en el **if** si se proporcionó un argumento para la función **cd** . Si no se proporcionó un argumento, se imprime un mensaje de error en **stderr** utilizando la función **fprintf()**.

Si se proporcionó un argumento, la función utiliza la función **chdir()** para cambiar el directorio de trabajo actual a la ruta especificada por el argumento. Si **chdir()** devuelve un valor distinto de cero, significa que se produjo un error al cambiar el directorio y se imprime un mensaje de error utilizando la función **perror()**.

Finalmente, la función devuelve 1.

&nbsp;
#### help

```c
int help(char **args)
{
    int i;
    printf("Leonardo Ramirez Calatayud\nNaommi Lahera Champagne\nYisell Martinez Noa\n");
    printf("Los built in son:\n");
    
    for (i = 0; i < num_builtins(); i++) 
    {
        printf(" %s\n", builtin_str[i]);
    }
    return 1;
}
```

Esta es una función se utiliza para imprimir información de ayuda sobre el programa. 

La función comienza imprimiendo los nombres de los creadores del programa con la función **printf()**. Luego imprime una lista con los comandos internos del programa. Más adelante se recorre la matriz **builtin_str[]** que contiene los nombres de los comandos internos y los imprime en la pantalla utilizando la función **printf()**. 

Finalmente, la función devuelve 1.

&nbsp;
#### exit

```c

int bi_exit(char **args)
{
    return 0;
}
```

Esta función devuelve 0 cambiando el estado del programa a 0.
