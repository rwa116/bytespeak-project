/*
	implementation of the pot_controller class
*/
#include "hal/pot_controller.h"

#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>
#include <fstream>
#include <mutex>

pot_controller::pot_controller(double& min, double& max)
{
    min = 0;
    max = 1.8;
}

pot_controller::~pot_controller()
{
    // nothing to do
}

double pot_controller::getVoltage0Reading()
{
	// Open file
	FILE *f = fopen(A2D_FILE_VOLTAGE0.c_str(), "r");
	if (!f) {
		printf("ERROR: Unable to open voltage input file. Cape loaded?\n");
		printf("       Check /boot/uEnv.txt for correct options.\n");
		exit(-1);
	}

	// Get reading
	int a2dReading = 0;
	int itemsRead = fscanf(f, "%d", &a2dReading);
	if (itemsRead <= 0) {
		printf("ERROR: Unable to read values from voltage input file.\n");
		exit(-1);
	}

	// Close file
	fclose(f);
	
	history.push_back(a2dReading);

	return a2dReading;
}

std::vector<double> pot_controller::getLast20Values()
{
	std::vector<double> last20Values;

	// get mutex
	std::lock_guard<std::mutex> lock1(this->history_mutex);

    size_t size = history.size();
    
    // Check if the input vector has at least 20 elements
    if (size >= 20) {
        std::copy(history.end() - 20, history.end(), std::back_inserter(last20Values));
    } else {
        last20Values = history;
    }
    
    return last20Values;
}