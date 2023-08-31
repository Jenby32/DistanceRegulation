#include <iostream>
#include <windows.h>
#include <fstream>
#include <string>
#include <cmath>
#include <array>

bool bIsRunning = true;

namespace defaults {
	const std::string pathToFile = "/pathToExcelFile.csv";
	const int driverObjectCount = 2;
	const int dataArraySize = 1000;
	const int roundResolution = 10000;
	const int simulationInterval = 100; // ms 
}

class Fahrer {
private:
	float traveledDistance = 0;			// -- km
	float traveledDistanceBefore = 0;	// -- km

	float desiredSpeed = 0; // -- km/h
	float currentSpeed = 0; // -- km/h
	float limit = 0;		// -- km/h

	float limitChangePoints[6] = { 0, 10, 20, 30, 40, 50};			// km
	float limitsAtChangePoints[6] = { 0, 70, 100, 70, 50, 130 };	// -- km/h 

	float desiredDistanceToFrontman = 0;	// -- m
	float actualDistanceToFrontman = 0;		// -- m

	int driverId = 0;

	std::array<int, defaults::dataArraySize> pastSecondsArr{};
	std::array<int, defaults::dataArraySize> drivenSpeedsArr{};
	std::array<float, defaults::dataArraySize> distanceTraveledArr{};
	std::array<int, defaults::dataArraySize> limitsArr{};
	std::array<int, defaults::dataArraySize> distanceKeptToFrontmanArr{};


	bool IsInRange(int low, int high, int x)
	{
		return ((x - high) * (x - low) <= 0);
	}

	void GetSpeedAtChangePoint() {
		for (int i = 0; i < std::size(limitChangePoints); i++) {
			/* i.e. 
			* 
			* you have this:
			* limitChangePoints[6] = { 0, 10, 20, 30, 40, 50};
			* limitsAtChangePoints[6] = { 0, 70, 100, 70, 50, 130 };
			* 
			* and current driver location(->traveledDistance<-) is:  25km
			* then IsRange function will trigger when limitChangePoints[i] = 20 and limitChangePoints[i + 1] = 30
			* as the limitsAtChangePoints array is the same size like the limitChangePoints we get our limit and so our desired speed with -- limitsAtChangePoints[i + 1]
			* 
			*/
			if (IsInRange(limitChangePoints[i], limitChangePoints[i + 1], traveledDistance)) {
				desiredSpeed = limitsAtChangePoints[i + 1];
				limit = desiredSpeed;
				break;
			}
		}
	}

	void PushDataIntoArrays(float currentSpeed, float traveledDistance, int second, float limit, float distanceToFrontman) {
		pastSecondsArr[second] = second;
		drivenSpeedsArr[second] = currentSpeed;
		distanceTraveledArr[second] = traveledDistance;
		limitsArr[second] = limit;
		distanceKeptToFrontmanArr[second] = distanceToFrontman;
	}

public:
	std::array<int, defaults::dataArraySize> GetPastSecondsArr() {
		return pastSecondsArr;
	}

	std::array<int, defaults::dataArraySize> GetDrivenSpeedsArr() {
		return drivenSpeedsArr;
	}

	std::array<float, defaults::dataArraySize> GetDistanceTraveledArr() {
		return distanceTraveledArr;
	}

	std::array<int, defaults::dataArraySize> GetLimitsArr() {
		return limitsArr;
	}

	std::array<int, defaults::dataArraySize> GetDistanceKeptToFrontmanArr() {
		return distanceKeptToFrontmanArr;
	}

	int GetDriverId() {
		return driverId;
	}

	void SetId(int idx) {
		driverId = idx;
	}

	void AdjustDriverSpeed(int second, float accelerationPerSecond , int driverId, Fahrer driverObj) {
		float traveledDistance_rounded = 0;
		float actualDistanceToFrontman_rounded = 0;
		float& last_limit = limit;

		traveledDistanceBefore = traveledDistance;

		GetSpeedAtChangePoint();

		//	the current location of driver can be calculated with this formula
		traveledDistance = traveledDistanceBefore + (currentSpeed * (second - (second-1)))/1000; 

		//	only if driverId is bigger than 0 the distance to frontman gets calculated !!First driver (driverId = 0) has no Frontman!!
		if (driverId > 0) {
			// !!	desired distance to frontman is always the speed that is currently driven	!!
			desiredDistanceToFrontman = currentSpeed;
			actualDistanceToFrontman = (driverObj.traveledDistance * 1000) - (traveledDistance * 1000);

			//	decrease speed when actual distance is smaller than desired distance
			if (actualDistanceToFrontman < desiredDistanceToFrontman) {
				if (currentSpeed == 0) {
					currentSpeed = currentSpeed;
				}
				else {
					currentSpeed -= accelerationPerSecond;
				}
			}
			/*	accelerate when actual distance is bigger than desired distance to frontman and current speed is less than the limit
			*	deccelerate when speed is higher than limit
			*/
			else {
				if (currentSpeed < limit) {
					currentSpeed += accelerationPerSecond;
				}
				else {
					currentSpeed -= accelerationPerSecond;
				}
			}
		}
		else {
			if (currentSpeed < limit) {
				currentSpeed += accelerationPerSecond;
			}
			else {
				currentSpeed -= accelerationPerSecond;
			}
		}

		traveledDistance_rounded = round(traveledDistance * defaults::roundResolution) / defaults::roundResolution;
		actualDistanceToFrontman_rounded = round(actualDistanceToFrontman * defaults::roundResolution) / defaults::roundResolution;
			
		PushDataIntoArrays(currentSpeed, traveledDistance_rounded, second, limit, actualDistanceToFrontman_rounded);

		std::cout << "\n----------------------driver " << driverId << "--------------------------" << std::endl;
		std::cout << "Desired Distance"<< ": " << desiredDistanceToFrontman << std::endl;
		std::cout << "Actual Distance"<< ": " << actualDistanceToFrontman << std::endl;
		std::cout << "Distance Traveled: " << traveledDistance << std::endl;
		std::cout << "Current Speed: " << currentSpeed << std::endl;
		std::cout << "Limit: " << limit << std::endl;
		std::cout << "\n-------------------------------------------------------------------------" << std::endl;

	}
};

void WriteDataArraysIntoFile(std::array<Fahrer, defaults::driverObjectCount> driver, std::string pathToFile) {

	int j = 0;
	int currentPosition = 0;
	int k = 0;
	int second = 0;

	std::array<int, driver.size()> watchedDriver = { 1001 };

	std::string writeToFileStr = "";

	std::ofstream file(pathToFile, std::ios::app);

	if (file.is_open()) {
		//	logic that goes through all drivers in the driver array and gets i.e. the same data which is at second 33
		//	for all drivers then hops to the next second
		/* in Example:
		*	Driver One:
		*		secondArr = [0,1,2,3,4,5]
		*		speedArr = [25, 26, 27, 28, 29, 30]
		*	Driver Two:
		*		secondArr = [0,1,2,3,4,5]
		*		speedArr = [45, 46, 47, 48, 49, 50]
		* 
		*	The desired Output is something like this
		*		0;25;;0;45 
		*		1;26;;1;46
		*		2;27;;2;47
		*	the delimiters are for the .csv output file
		* 
		*/
		do {
			int arraySize = driver.size();

			std::array<int, defaults::dataArraySize> secondArr = driver[k].GetPastSecondsArr();
			std::array<int, defaults::dataArraySize> speedArr = driver[k].GetDrivenSpeedsArr();
			std::array<float, defaults::dataArraySize> wayArr = driver[k].GetDistanceTraveledArr();
			std::array<int, defaults::dataArraySize> limitArr = driver[k].GetLimitsArr();
			std::array<int, defaults::dataArraySize> abstandArr = driver[k].GetDistanceKeptToFrontmanArr();

			while (j < defaults::dataArraySize) {
				int currsecond = secondArr[j];

				//	every "data paket" in the same cycle(second 0,1,2,3,4,5...n) gets added to the string
				//	which will be written into the file when the cycle has ended

				writeToFileStr = writeToFileStr + std::to_string(currsecond) + ";" + std::to_string(speedArr[j]) + ";" + std::to_string(wayArr[j]) + ";" + std::to_string(limitArr[j]) + ";" + std::to_string(abstandArr[j]) + ";;";
				k++;
				currentPosition++;

				//	if the current position is bigger than array it means that there is no more drivers in this cycle
				//	everything will be resetted and writeTiFileStr will be written into the next free line of the file
				if (currentPosition >= arraySize) {
					k = 0;
					currentPosition = 0;
					file.seekp(0, std::ios::end);
					file << writeToFileStr << std::endl;

					writeToFileStr = "";

				}

				//	loop through watchedDriver
				//	if k(can be defined as current driverId) has already been watched then we can
				//	hop a second further and reset the watchedDriver array
				for (int x = 0; x < watchedDriver.size(); x++) {
					if (k == watchedDriver[x]) {
						j++;
						watchedDriver = { 1001 };
					}
				}

				// add driver to watchedDriver
				watchedDriver[k] = k;

				break;
			}
		} while (k < driver.size());

		file.close();
	}
	else {
		std::cout << "Error when opening file." << std::endl;
	}
}


/* explenation of method WriteFirstRows()
*	we have 2 Drivers
*	then the output csv file should look like this:
* 
* 
*	-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
*	|		s			|		km/h			|		km			|		km/h			|		m				|		|		s			|		km/h			|		km			|		km/h			|		m				|
*	-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
*	|		time		|		speedF1			|		wayF1		|		limitF1			|		distanceF1		|		|		time		|		speedF2			|		wayF2		|		limitF2			|		distanceF2		|
*	-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
*	|		datax		|		datax			|		datax		|		datax			|		datax			|		|		datax		|		datax			|		datax		|		datax			|		datax			|
*	-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
*
* 
*	this method creates the first two rows for every driver
*/
void WriteFirstTwoRows(std::array<Fahrer, defaults::driverObjectCount> driver, std::string pathToFile) {
	const int arraySize = sizeof(driver)/ sizeof(driver);
	std::string unitsStr = "";
	std::string nameStr = "";
	for (int i = 0; i <= arraySize; i++) {
		std::string units = "s;km/h;km;km/h;m;;";
		std::string driverId = std::to_string(driver[i].GetDriverId());
		std::string names = "time;speedF" + driverId +";wayF"+ driverId + ";limitF"+ driverId+";distanceF"+driverId+";;";

		unitsStr += units;
		nameStr += names;
	}
	std::ofstream file(pathToFile, std::ios::app);
	if (file.is_open()) {
		file.seekp(0, std::ios::end);
		file << unitsStr << std::endl;
		file << nameStr << std::endl;
		file.close();
	}
	else {
		std::cout << "Error when opening file." << std::endl;
	}
}


int main() {
	Fahrer driver1;
	Fahrer driver2;
	std::array<Fahrer, defaults::driverObjectCount> driver= {driver1, driver2};
	driver[0].SetId(0);
	driver[1].SetId(1);

	WriteFirstTwoRows(driver, defaults::pathToFile);

	int second = 0;
	while (bIsRunning) {
		// if escape is pressed the Speed Regulation is done.
		if (GetKeyState(VK_ESCAPE) & 0x8000) {
			bIsRunning = false;
		}
		else {
			driver[0].AdjustDriverSpeed(second, 1, driver[0].GetDriverId(), driver[0]);
			driver[1].AdjustDriverSpeed(second, 2, driver[1].GetDriverId(), driver[0]);

			Sleep(defaults::simulationInterval);

			second++;
		}
	}

	WriteDataArraysIntoFile(driver, defaults::pathToFile);
}