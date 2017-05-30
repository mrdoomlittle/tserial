# include <boost/interprocess/shared_memory_object.hpp>
# include <boost/interprocess/mapped_region.hpp>
# include <cstdio>
# include "shm_obj.h"
# include <iostream>
int main(int argc, char const *argv[]) {
	boost::interprocess::shared_memory_object shm_obj(boost::interprocess::open_only, "sh", boost::interprocess::read_write);
	boost::interprocess::mapped_region region(shm_obj, boost::interprocess::read_write);
	shm_obj_t *shm_obj_ptr = (shm_obj_t*)region.get_address();

	if (argc > 1) {
		while(1) {
			if (shm_obj_ptr-> reading_wait) {
				shm_obj_ptr-> reading_paused = true;
				continue;
			} else shm_obj_ptr-> reading_paused = false;

			shm_obj_ptr-> task_id = TID_READ;
			while(shm_obj_ptr-> task_id != TID_NONE){if (shm_obj_ptr-> reading_wait)break;}
			if (!shm_obj_ptr-> reading_wait) printf("%s", shm_obj_ptr-> rx_buff);
		}
	} else {
		while(1) {
			std::cin.getline((char*)shm_obj_ptr-> tx_buff, 200);
			shm_obj_ptr-> reading_wait = true;
			while(!shm_obj_ptr-> reading_paused) {}

			shm_obj_ptr-> task_id = TID_WRITE;
			while(shm_obj_ptr-> task_id != TID_NONE){}
			shm_obj_ptr-> reading_wait = false;
		}
	}
}
