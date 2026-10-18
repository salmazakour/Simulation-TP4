// a fast simulation of a rabbit population growth.  

#include <stdio.h>

void main(){
    int AdultRabbits[100];
    int YoungRabbits[100];
    int months = 0;
    
    AdultRabbits[0] = 0;
    YoungRabbits[0] = 1;
    for (int i = 0; i < 12; i++){

        AdultRabbits[i+1] = AdultRabbits[i] + YoungRabbits[i];
        YoungRabbits[i+1] = AdultRabbits[i];

        months++;
        printf("Month %d: \t Adult Rabbits: %d \t, Young Rabbits: %d\n", 
            i, AdultRabbits[i], YoungRabbits[i]);
    }

    
}

//pour compliler : gcc main.c -o main
// pour exécuter : ./main