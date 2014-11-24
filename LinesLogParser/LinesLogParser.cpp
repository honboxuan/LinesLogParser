#include <fstream>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <vector>

#define DEBUG_OUTPUT		0

#define UNEXPECTED_EOF		-1
#define WRONG_ID			-2
#define WRONG_HEADER_SIZE	-3
#define BAD_ARGUMENT		-4
#define OUTPUT_OPEN_FAILED	-5

int main(int argc, char* argv[]) {
	for (int i = 1; i < argc; i++) {
		
		std::ifstream lines_log;
		lines_log.open(argv[i], std::ios::binary);
		if (!lines_log.is_open()) {
			return BAD_ARGUMENT;
		}

		std::ofstream lines_log_output;
		std::string output_filename(argv[i]);
		output_filename.append(".csv");
		lines_log_output.open(output_filename);
		if (!lines_log_output.is_open()) {
			return OUTPUT_OPEN_FAILED;
		}

		uint8_t bytes_count;
		bool mag_data = false;

		int16_t gyro_offset[3];
		int16_t accel_offset[3];
			
		uint32_t time;
		int16_t accel[3];
		int16_t temperature;
		int16_t gyro[3];
		int16_t mag[3];
			
		//Header
		if (lines_log.read((char*)(&bytes_count), 1)) {
			if (bytes_count == 11 + 6 + 6) {
				uint8_t header_raw[11 + 6 + 6];
				if (lines_log.read((char*)(header_raw), bytes_count)) {
					uint8_t index_offset = 0;
						
					char id_string[] = "LinesLogger";
					for (uint8_t j = 0; j < 11; j++) {
						if (header_raw[j] == id_string[j]) {
#if DEBUG_OUTPUT
							printf("%c", header_raw[j]);
#endif
						} else {
							return WRONG_ID;
						}
					}
#if DEBUG_OUTPUT
					printf("\n");
#endif
					index_offset += 11;
						
					//Gyroscope offset
					for (uint8_t j = 0; j < 3; j++) {
						gyro_offset[j] = (int16_t(header_raw[j*2 + index_offset]) << 8) | header_raw[j*2 + 1 + index_offset];
					}
					index_offset += 6;
#if DEBUG_OUTPUT
					printf("G Offset: %d, %d, %d\n", gyro_offset[0], gyro_offset[1], gyro_offset[2]);
#endif
					//Accelerometer offset
					for (uint8_t j = 0; j < 3; j++) {
						accel_offset[j] = (int16_t(header_raw[j*2 + index_offset]) << 8) | header_raw[j*2 + 1 + index_offset];
					}
					index_offset += 6;
#if DEBUG_OUTPUT
					printf("A Offset: %d, %d, %d\n", accel_offset[0], accel_offset[1], accel_offset[2]);
#endif
				}
			} else {
				return WRONG_HEADER_SIZE;
			}
		} else {
			return UNEXPECTED_EOF;
		}

		//Body
		while (lines_log.read((char*)(&bytes_count), 1)) {
			if (bytes_count) {
#if DEBUG_OUTPUT
				//printf("Length: %u\n", bytes_count);
#endif
				//Read line
				std::vector<uint8_t> raw(bytes_count);
				if (lines_log.read((char*)(raw.data()), bytes_count)) {
					//Parse line
					uint8_t index_offset = 0;
					//Time
					time = (uint32_t(raw[3]) << 24) | (uint32_t(raw[2]) << 16) | (uint32_t(raw[1]) << 8) | uint32_t(raw[0]); //LSB first
					index_offset += 4;
#if DEBUG_OUTPUT
					printf("Time: %u\n", time);
#endif

					//Accelerometer
					for (uint8_t j = 0; j < 3; j++) {
						accel[j] = (int16_t(raw[j*2 + index_offset]) << 8) | raw[j*2 + 1 + index_offset];
					}
					index_offset += 6;
#if DEBUG_OUTPUT
					printf("A: %d, %d, %d\n", accel[0], accel[1], accel[2]);
#endif

					//Temperature
					temperature = (int16_t(raw[index_offset]) << 8) | raw[1 + index_offset];
					index_offset += 2;
#if DEBUG_OUTPUT
					printf("T: %d\n", temperature);
#endif

					//Gyroscope
					for (uint8_t j = 0; j < 3; j++) {
						gyro[j] = (int16_t(raw[j*2 + index_offset]) << 8) | raw[j*2 + 1 + index_offset];
					}
					index_offset += 6;
#if DEBUG_OUTPUT
					printf("G: %d, %d, %d\n", gyro[0], gyro[1], gyro[2]);
#endif
						
					//Magnetometer
					mag_data = false;
					if (bytes_count > index_offset) {
						mag_data = true;
						for (uint8_t j = 0; j < 3; j++) {
							mag[j] = (int16_t(raw[j*2 + 1 + index_offset]) << 8) | raw[j*2 + index_offset];
						}
						index_offset += 6;
					}
					if (mag_data) {
#if DEBUG_OUTPUT
						printf("M: %d, %d, %d\n", mag[0], mag[1], mag[2]);
#endif
					}

#if DEBUG_OUTPUT
					printf("\n");
#endif

					//Output
					lines_log_output << "Time," << time << ",";
					lines_log_output << "Accelerometer," << accel[0] << "," << accel[1] << "," << accel[2] << ",";
					lines_log_output << "Temperature," << temperature << ",";
					lines_log_output << "Gyroscope," << gyro[0] << "," << gyro[1] << "," << gyro[2] << ",";
					if (mag_data) {
						lines_log_output << "Magnetometer," << mag[0] << "," << mag[1] << "," << mag[2] << ",";
					}
					lines_log_output << "\n";
				} else {
					return UNEXPECTED_EOF;
				}
			}
		}
		
		lines_log.close();
		lines_log_output.close();

#if DEBUG_OUTPUT
		printf("EOF\n");
#endif
		//std::getchar(); //HACK
	}
	return 0;
}