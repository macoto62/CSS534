#include <iostream>  // cout
#include <stdlib.h>  // rand
#include <math.h>    // sqrt, pow
#include <omp.h>     // OpenMP
#include <string.h>  // memset
#include "Timer.h"
#include "Trip.h"
#include <algorithm> // sort
#include <ctime>
#include <vector>

#define CHROMOSOMES    50000 // 50000 different trips
#define CITIES         36    // 36 cities = ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789
#define TOP_X          25000 // top optimal 25%
#define MUTATE_RATE    50    // optimal 50%

using namespace std;

const char cities[CITIES+1] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";

/*
 * Evaluates each trip (or chromosome) and sort them out
 */
int findCityIndex(Trip trip, char city) {
    for (int i = 0; i < CITIES; i++) {
        if (trip.itinerary[i] == city) {
            return i;
        }
    }
    return -1;
}

char followingCity(Trip trip, char city) {
    int index = findCityIndex(trip, city);
    return trip.itinerary[(index + 1) % CITIES];
}

int cityIndex(char city) {
    if (city >= 'A' && city <= 'Z')
        return city - 'A';

    return city - '0' + 26;
}

char indexToCity(int index) {
    if (index < 26) {
        return 'A' + index;
    }
    return '0' + (index - 26);
}

float distance(int x1, int y1, int x2, int y2) {
    int xDiff = x2 - x1;
    int yDiff = y2 - y1;

    return sqrtf(static_cast<float>(xDiff * xDiff + yDiff * yDiff));
}


void evaluate(Trip trip[CHROMOSOMES], int coordinates[CITIES][2]) {
    #pragma omp parallel for
    for (int i = 0; i < CHROMOSOMES; i++) {
        float totalDistance = 0.0f;

        int previousX = 0;
        int previousY = 0;

        for (int gene = 0; gene < CITIES; gene++) {
            int city = cityIndex(trip[i].itinerary[gene]);

            int currentX = coordinates[city][0];
            int currentY = coordinates[city][1];

            totalDistance += distance(previousX, previousY, currentX, currentY);

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
/*
 * 1. select first city of the first parent
 * 2. choose the closer next city of parent[i] and parent[i+1]
 * 3. if appear in the trip, choose other city, if both appear, choose randomly
 * 4. iterate until all cities are filled
 */


void crossover( Trip parents[TOP_X], Trip offsprings[TOP_X], int coordinates[CITIES][2] ) {
    #pragma omp parallel for
    for (int i = 0; i < TOP_X; i += 2) {
        vector<bool> used(CITIES, false);
        char firstCity = parents[i].itinerary[0];
        used[cityIndex(firstCity)] = true;
        offsprings[i].itinerary[0] = firstCity;
        offsprings[i + 1].itinerary[0] = cities[CITIES - 1 - cityIndex(firstCity)];

        char nextCity1 = followingCity(parents[i], firstCity);
        char nextCity2 = followingCity(parents[i + 1], firstCity);

        for (int j = 1; j < CITIES; j++) {
            char previousCity = offsprings[i].itinerary[j - 1];
            int previousCityIndex = cityIndex(previousCity);
            int nextCityDiff1 = distance(coordinates[cityIndex(nextCity1)][0],
                                         coordinates[cityIndex(nextCity1)][1],
                                         coordinates[previousCityIndex][0],
                                         coordinates[previousCityIndex][1]);
            int nextCityDiff2 = distance(coordinates[cityIndex(nextCity2)][0],
                                         coordinates[cityIndex(nextCity2)][1],
                                         coordinates[previousCityIndex][0],
                                         coordinates[previousCityIndex][1]);

            bool nextCity1Used = used[cityIndex(nextCity1)];
            bool nextCity2Used = used[cityIndex(nextCity2)];
            int selectedCityIndex;

            if (!nextCity1Used && !nextCity2Used) {
                selectedCityIndex = nextCityDiff1 < nextCityDiff2
                    ? cityIndex(nextCity1)
                    : cityIndex(nextCity2);
            } else if (!nextCity1Used) {
                selectedCityIndex = cityIndex(nextCity1);
            } else if (!nextCity2Used) {
                selectedCityIndex = cityIndex(nextCity2);
            } else {
                // Both candidates are used; choose uniformly from unused cities.
                vector<int> unusedCityIndices;
                for (int city = 0; city < CITIES; city++) {
                    if (!used[city]) {
                        unusedCityIndices.push_back(city);
                    }
                }
                selectedCityIndex = unusedCityIndices[rand() % unusedCityIndices.size()];
            }

            used[selectedCityIndex] = true;
            offsprings[i].itinerary[j] = indexToCity(selectedCityIndex);
            offsprings[i + 1].itinerary[j] = cities[CITIES - 1 - selectedCityIndex];
            nextCity1 = followingCity(parents[i], offsprings[i].itinerary[j]);
            nextCity2 = followingCity(parents[i + 1], offsprings[i].itinerary[j]);
        }

        offsprings[i].itinerary[CITIES] = '\0';
        offsprings[i + 1].itinerary[CITIES] = '\0';
    }
}

/*
 * Mutate a pair of genes in each offspring.
 */
void mutate( Trip offsprings[TOP_X] ) {
    static bool seeded = false;
    if (!seeded) {
        srand(static_cast<unsigned>(time(0)));
        seeded = true;
    }
    #pragma omp parallel for
    for (int i = 0; i < TOP_X; i++) {
        if (rand() % 100 >= MUTATE_RATE) {
            continue;
        }

        int gene1 = rand() % CITIES;
        int gene2;
        do {
            gene2 = rand() % CITIES;
        } while (gene1 == gene2);
        char temp = offsprings[i].itinerary[gene1];
        offsprings[i].itinerary[gene1] = offsprings[i].itinerary[gene2];
        offsprings[i].itinerary[gene2] = temp;
    }
}

