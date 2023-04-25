#include <sys/wait.h>
//waitpid() and associated macros
#include <unistd.h>
//chdir()
//fork()
//exec()
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

void lsh_loop(void);
char *lsh_read_line(void);
char **lsh_split_line(char *);
int lsh_execute(char **);

int main(int argc, char **argv)
{
// Load config files, if any.
// Run command loop.
    lsh_loop();
// Perform any shutdown/cleanup.
    return EXIT_SUCCESS;
}

void lsh_loop(void)
{
    char *line;
    char **args;
    int status;
    do {
        printf("Su-Shell $ ");
        line = lsh_read_line();
        args = lsh_split_line(line);
        status = lsh_execute(args);
        free(line);
        free(args);
    } 
    while (status);
}

#define LSH_RL_BUFSIZE 1024


char *lsh_read_line()
{
    
    char *line = NULL;
    ssize_t bufsize = 0; // have getline allocate a buffer for us
    if (getline(&line, &bufsize, stdin) == -1)
    {
        if (feof(stdin)) 
        {
            exit(EXIT_SUCCESS); // We recieved an EOF
        } 
        else 
        {
            perror("readline");
            exit(EXIT_FAILURE);
        }
    }
    return line;
}

#define LSH_TOK_BUFSIZE 64
#define LSH_TOK_DELIM " \t\r\n\a"
int no_elem_tok;
char **lsh_split_line(char *line)
{
    int bufsize = LSH_TOK_BUFSIZE, position = 0;
    char **tokens = malloc(bufsize * sizeof(char*));
    char *token;
    if (!tokens) 
    {
        fprintf(stderr, "lsh: allocation error\n");
        exit(EXIT_FAILURE);
    }
    token = strtok(line, LSH_TOK_DELIM);
    
    while (token != NULL) 
    {
        tokens[position] = token;
        position++;
        if (position >= bufsize) 
        {
            bufsize += LSH_TOK_BUFSIZE;
            tokens = realloc(tokens, bufsize * sizeof(char*));
            if (!tokens) 
            {
                fprintf(stderr, "lsh: allocation error\n");
                exit(EXIT_FAILURE);
            }
        }
        token = strtok(NULL, LSH_TOK_DELIM);
    }

    tokens[position] = NULL;
    no_elem_tok = position;
    return tokens;
}

int lsh_launch(char **args)
{
    pid_t pid, wpid;
    int status;
    pid = fork();
    if (pid == 0) 
    {
        // Child process
        if (execvp(args[0], args) == -1) 
        {
            perror("lsh");
        }
    exit(EXIT_FAILURE);
    }

    else if (pid < 0) 
    {
    // Error forking
    perror("lsh");
    } 

    else 
    {
    // Parent process
        do 
        {
            wpid = waitpid(pid, &status, WUNTRACED);
        }
        while (!WIFEXITED(status) && !WIFSIGNALED(status));
    }

    return 1;
}


/*
Function Declarations for builtin shell commands:
*/
int lsh_cd(char **args);
int lsh_help(char **args);
int lsh_exit(char **args);
/*
List of builtin commands, followed by their corresponding functions.
*/
char *builtin_str[] = {"cd","help","exit"};
int (*builtin_func[]) (char **) = {&lsh_cd, &lsh_help, &lsh_exit};


int red_out_init(char * nameFile,char * type);
int red_out_end(char * nameFile,int fd);

int lsh_num_builtins() 
{
    return sizeof(builtin_str) / sizeof(char *);
}
/*
Builtin function implementations.
*/
int lsh_cd(char **args)
{
    if (args[1] == NULL) 
    {
        fprintf(stderr, "lsh: expected argument to \"cd\"\n");
    } 
    else 
    {
        if (chdir(args[1]) != 0) 
        {
            perror("lsh");
        }
    }  
    return 1;
}

int lsh_help(char **args)
{
    int i;
    printf("Stephen Brennan's LSH\n");
    printf("Type program names and arguments, and hit enter.\n");
    printf("The following are built in:\n");
    
    for (i = 0; i < lsh_num_builtins(); i++) 
    {
        printf(" %s\n", builtin_str[i]);
    }
    printf("Use the man command for information on other programs.\n");
    return 1;
}

int lsh_exit(char **args)
{
    return 0;
}

char *red_mod_srt[] = {"<",">",">>"};
int len_red_mod() 
{
    return sizeof(red_mod_srt) / sizeof(char *);
}
int lsh_execute(char **args)
{
    int i;
    if (args[0] == NULL) 
    {
        // An empty command was entered.
        return 1;
    }
    int cambiofd = -1;
    int archivo;
    int result = -1;
    int fd = dup(1);
    
    

    for (i = 0; i < len_red_mod(); i++) 
    {
        if (strcmp(args[no_elem_tok-2], red_mod_srt[i]) == 0) 
        {
            cambiofd = 1;   
            archivo = red_out_init(args[no_elem_tok-1],red_mod_srt[i]);
        }
    }

    for (i = 0; i < lsh_num_builtins(); i++) 
    {
        if (strcmp(args[0], builtin_str[i]) == 0) 
        {
            int result = (*builtin_func[i])(args);
        }
    }
    
    if(result == -1)
    {
        result = lsh_launch(args);
    }

    if(cambiofd == 1)
    {
       for (i = 0; i < len_red_mod(); i++) 
        {
            if (strcmp(args[no_elem_tok-2], red_mod_srt[i]) == 0) 
            {  
                red_out_end(args[no_elem_tok-1],fd);
            }
        }
    }
    return result;
}

int red_out_init(char * nameFile,char * type)
{
    char *nombreArchivo = nameFile;
    char *modo;
  
    if( type == ">")
    {
        modo = "w";// w es para sobrescribir, a+ es para añadir al existente 
    } 
    else if(type == ">>")
    {
        modo = "a+";// w es para sobrescribir, a+ es para añadir al existente
    }
   
    int archivo = open(nombreArchivo, modo);

    // Si por alguna razón el archivo no fue abierto, salimos inmediatamente
    if (archivo == NULL) 
    {
        printf("Error abriendo archivo %s", nombreArchivo);
        return 1;
    }
    dup2(archivo,1);
    /*
     * Escribir el contenido usando fprintf.
     * */
    
    // Al final, cerramos el archivo
    return archivo;
}

int red_out_end(char * nameFile,int fd)
{
    dup2(1,fd);
    fclose(nameFile);
    puts("Contenido escrito correctamente");
    return 0;
}