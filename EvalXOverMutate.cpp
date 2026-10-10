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
#include <random>

#define CHROMOSOMES    50000 // 50000 different trips
#define CITIES         36    // 36 cities = ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789
#define TOP_X          25000 // top optimal 25%
#define MUTATE_RATE    50    // optimal 50%

using namespace std;

const char cities[CITIES+1] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
float distanceMatrix[CITIES+1][CITIES];
/*
 * Evaluates each trip (or chromosome) and sort them out
 */
int findCityIndex(const Trip &trip, char city) {
    for (int i = 0; i < CITIES; i++) {
        if (trip.itinerary[i] == city) {
            return i;
        }
    }
    return -1;
}

char followingCity(const Trip &trip, char city) {
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

void calculateDistanceMatrix(int coordinates[CITIES][2]) {
    for (int i = 0; i < CITIES+1; i++) {
        for (int j = 0; j < CITIES; j++) {
            if (i == CITIES ) {
                distanceMatrix[i][j] = distance(0, 0, coordinates[j][0], coordinates[j][1]); 
            }else {
                distanceMatrix[i][j] = distance(coordinates[i][0], coordinates[i][1], coordinates[j][0], coordinates[j][1]);
            }
        }
    }
}

mt19937& crossoverRng() {
    // 每個執行緒獨立擁有 rng，並用「固定基底 + 執行緒編號」當作種子
    thread_local std::mt19937 rng(12345678u + omp_get_thread_num());
    return rng;
}

// Evaluates each trip (or chromosome) and sort them out
void evaluate(Trip trip[CHROMOSOMES], int coordinates[CITIES][2]) {
    #pragma omp parallel for
    for (int i = 0; i < CHROMOSOMES; i++) {
        float totalDistance = 0.0f;

        int previousCity = CITIES; // City at (0,0)

        for (int gene = 0; gene < CITIES; gene++) {
            int city = cityIndex(trip[i].itinerary[gene]);

            totalDistance += distanceMatrix[previousCity][city];

            previousCity = city;
        }

        trip[i].fitness = totalDistance;
    }
    //May be optimized
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
        mt19937& rng = crossoverRng();
        vector<bool> used(CITIES, false);
        // 1. Select the first city from the first parent
        char firstCity = parents[i].itinerary[0];
        used[cityIndex(firstCity)] = true;
        offsprings[i].itinerary[0] = firstCity;
        offsprings[i + 1].itinerary[0] = cities[CITIES - 1 - cityIndex(firstCity)];
        // 2. Choose the closer next city of parent[i] and parent[i+1]
        char nextCity1 = followingCity(parents[i], firstCity);
        char nextCity2 = followingCity(parents[i + 1], firstCity);

        // 3. If the next city is already used, choose the other one
        for (int j = 1; j < CITIES; j++) {
            char previousCity = offsprings[i].itinerary[j - 1];
            int previousCityIndex = cityIndex(previousCity);
            bool nextCity1Used = used[cityIndex(nextCity1)];
            bool nextCity2Used = used[cityIndex(nextCity2)];
            int selectedCityIndex;
            if (nextCity1Used && nextCity2Used) {
                // Both next cities are used, so choose a random unused city.
                vector<int> unusedCityIndices;
                for (int city = 0; city < CITIES; city++) {
                    if (!used[city]) {
                        unusedCityIndices.push_back(city);
                    }
                }
                uniform_int_distribution<int> dist(0, unusedCityIndices.size() - 1);
                selectedCityIndex = unusedCityIndices[dist(rng)];
            }
            else if(nextCity1Used) {
                selectedCityIndex = cityIndex(nextCity2);
            }
            else if(nextCity2Used) {
                selectedCityIndex = cityIndex(nextCity1);
            }
            else{
                int nextCityDiff1 = distanceMatrix[cityIndex(nextCity1)][previousCityIndex];
                int nextCityDiff2 = distanceMatrix[cityIndex(nextCity2)][previousCityIndex];
                selectedCityIndex = nextCityDiff1 < nextCityDiff2
                    ? cityIndex(nextCity1)
                    : cityIndex(nextCity2);
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
    #pragma omp parallel for
    for (int i = 0; i < TOP_X; i++) {
        mt19937& rng = crossoverRng();
        uniform_int_distribution<int> dist(0, 99);
        if (dist(rng) >= MUTATE_RATE) {
            continue;
        }
        
        uniform_int_distribution<int> geneDist(0, CITIES - 1);
        int gene1 = geneDist(rng);
        int gene2;
        do {
            gene2 = geneDist(rng);
        } while (gene1 == gene2);
        char temp = offsprings[i].itinerary[gene1];
        offsprings[i].itinerary[gene1] = offsprings[i].itinerary[gene2];
        offsprings[i].itinerary[gene2] = temp;
    }
}
