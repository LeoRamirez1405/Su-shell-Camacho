#include <sys/wait.h>
//waitpid() and associated macros
#include <unistd.h>
//chdir()
//fork()
//pid_t
#include <stdlib.h>
//malloc()
//realloc()
//free()
//exit()
//execvp()
//EXIT_SUCCESS, EXIT_FAILURE
#include <stdio.h>
//fprintf()
//printf()
//stderr
//getchar()
//perror()
#include <string.h>
//strcmp()
//strtok()

#include <fcntl.h>
//open()
//close()

void loop(void);
char *read_line(void);
char **split_line(char *);
int execute(char **);
void pipes_handler(char *line); 
char *rev_comment(char * line);



int red_out_init(char * nameFile,char * type);
int red_out_end(int nameFile,int fd);
int red_in_init(char * nameFile);
int red_in_end(int nameFile,int fd);
char *red_mod_srt[] = {">",">>"};

int cd(char **args);
int help(char **args);
int bi_exit(char **args);
char *builtin_str[] = {"cd","help","exit"};
int (*builtin_func[]) (char **) = {&cd, &help, &exit};

#define RL_BUFSIZE 1024
#define TOK_BUFSIZE 64
#define TOK_DELIM " \t\r\n\a"
int no_elem_tok;


int main(int argc, char **argv)
{
    loop();
    return EXIT_SUCCESS;
}

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
    } 
    while (status);
}

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

int count_pipes(char *args) 
{
    int count = 0; 
    for (int i=0; i < strlen(args); i++) {
        count += (args[i] == '|');
    }
    return count; 
}

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

int num_builtins() 
{
    return sizeof(builtin_str) / sizeof(char *);
}

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

int bi_exit(char **args)
{
    return 0;
}

int len_red_mod() 
{
    return sizeof(red_mod_srt) / sizeof(char *);
}

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

int red_in_end(int nameFile,int fd)
{
    dup2(fd,0);
    close(nameFile);
    return 0;
}

int red_out_end(int nameFile,int fd)
{
    dup2(fd,1);
    close(nameFile);
    return 0;
}