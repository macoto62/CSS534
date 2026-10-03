#include <iostream>  // cout
#include <stdlib.h>  // rand
#include <math.h>    // sqrt, pow
#include <omp.h>     // OpenMP
#include <string.h>  // memset
#include "Timer.h"
#include "Trip.h"
#include <algorithm> // sort

#define CHROMOSOMES    50000 // 50000 different trips
#define CITIES         36    // 36 cities = ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789
#define TOP_X          25000 // top optimal 25%
#define MUTATE_RATE    50    // optimal 50%

using namespace std;

/*
 * Evaluates each trip (or chromosome) and sort them out
 */
int cityIndex(char city) {
    if (city >= 'A' && city <= 'Z')
        return city - 'A';

    return city - '0' + 26;
}

void evaluate(Trip trip[CHROMOSOMES], int coordinates[CITIES][2]) {
    for (int i = 0; i < CHROMOSOMES; i++) {
        float totalDistance = 0.0f;

        int previousX = 0;
        int previousY = 0;

        for (int gene = 0; gene < CITIES; gene++) {
            int city = cityIndex(trip[i].itinerary[gene]);

            int currentX = coordinates[city][0];
            int currentY = coordinates[city][1];

            int xDiff = currentX - previousX;
            int yDiff = currentY - previousY;

            totalDistance += sqrtf(
                static_cast<float>(xDiff * xDiff + yDiff * yDiff)
            );

            previousX = currentX;
            previousY = currentY;
        }

        trip[i].fitness = totalDistance;
    }

    sort(trip, trip + CHROMOSOMES,
         [](const Trip& a, const Trip& b) {
             return a.fitness < b.fitness;
         });
}

/*
 * Generates new TOP_X offsprings from TOP_X parents.
 * Noe that the i-th and (i+1)-th offsprings are created from the i-th and (i+1)-th parents
 */
void crossover( Trip parents[TOP_X], Trip offsprings[TOP_X], int coordinates[CITIES][2] ) {

}

/*
 * Mutate a pair of genes in each offspring.
 */
void mutate( Trip offsprings[TOP_X] ) {

}

