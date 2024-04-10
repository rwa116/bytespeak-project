/*
    * interface for the pot
*/

#ifndef HAL_POT_CONTROLLER_H
#define HAL_POT_CONTROLLER_H

#include <string>
#include <vector>
#include <mutex>
/**
 * @brief The pot_controller class represents a controller for a potentiometer.
 * 
 * This class provides functionality to read the voltage reading from a potentiometer,
 * store the reading history, and retrieve the last 20 values from the history.
 */
class pot_controller {
    private:    
    std::string A2D_FILE_VOLTAGE0 = "/sys/bus/iio/devices/iio:device0/in_voltage0_raw";
    double A2D_VOLTAGE_REF_V = 1.8;
    double A2D_MAX_READING = 4095;

    // vector to store history
    std::vector<double> history;

    public: 
    pot_controller(double& min, double& max);
    ~pot_controller();

    /**
     * @brief Get the voltage reading from the potentiometer.
     * 
     * @return The voltage reading from the potentiometer.
     */
    double getVoltage0Reading();

    // mutex
    std::mutex history_mutex;

    /**
     * @brief Get the last 20 values from the history.
     * 
     * @return A vector containing the last 20 values from the history.
     */
    std::vector<double> getLast20Values();

};

#endif // HAL_POT_CONTROLLER_H