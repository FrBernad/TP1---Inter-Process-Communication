// This is a personal academic project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

/*
● DEBE recibir el/los paths de los archivos a procesar y DEBE iniciar el programa
correspondiente para procesarlos (minisat).
● DEBE enviar la información relevante del procesamiento (ver proceso vista) al
proceso aplicación.
● NO DEBE volcar el resultado de minisat a un archivo en disco, para luego leerlo
desde el esclavo, DEBERÁ recibir el output de minisat utilizando algún mecanismo
de IPC más sofisticado.
● No es necesario escribir un parser del output de minisat, se pueden utilizar
comandos como grep, sed, awk, etc.
○ Hint: popen
○ Hint: grep -o -e "Number of.*[0-9]\+" -e "CPU time.*" -e
".*SATISFIABLE"
*/

#define _POSIX_C_SOURCE 2

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define ERROR_CODE 1
#define MAX_TASKS_LENGTH 4096
#define ERROR_MANAGER(ERROR_STRING) \
    do {                            \
        fprintf(stderr,"Error in %s, line %d", ERROR_STRING,__LINE__);       \
        exit(EXIT_FAILURE);         \
    } while (0)

#define SAT_SOLVER "minisat"


static void processTask(char *tasks);

int main(int argc, char const *argv[]) {

    // Disable buffering on stdout
    if (setvbuf(stdout, NULL, _IONBF, 0)!=0)
        ERROR_MANAGER("slave > main > setvbuff");


    for (size_t i = 1; i < argc; i++) 
        processTask((char *)argv[i]);
    

    char tasks[MAX_TASKS_LENGTH + 1] = {0};
    ssize_t count;

    while ((count = read(STDIN_FILENO, tasks, MAX_TASKS_LENGTH)) != 0) {
        if (count == -1)
            ERROR_MANAGER("slave > main > read input");
        
        tasks[count] = 0;
        processTask(tasks);
    }

    return 0;
}

static void processTask(char *tasks) {
    char *task = strtok(tasks, "\t");

    char command[MAX_TASKS_LENGTH + 1];
    char output[MAX_TASKS_LENGTH + 1];

    while (task != NULL) {
        sprintf(command, "%s %s | grep -o -e \"Number of.*[0 - 9]\\+\" -e \"CPU time.*\" -e \".*SATISFIABLE\"", SAT_SOLVER, task);

        FILE *outputStream;
        if ((outputStream = popen(command, "r")) == NULL)
            ERROR_MANAGER("slave>processTask>popen");

        int count = fread(output, sizeof(char), MAX_TASKS_LENGTH, outputStream);

        if (ferror(outputStream))
            ERROR_MANAGER("slave>processTask>fread");

        //pisa el stack anterior y sino ponemos el 0 imprime basura
        output[count] = 0;

        printf("%s\t", output);

        pclose(outputStream);

        task = strtok(NULL, "\t");
    }
}
