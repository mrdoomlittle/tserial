# include <boost/asio/serial_port.hpp>
# include <boost/asio.hpp>
# include <cstdio>
# include <mdlint.h>
# include <boost/interprocess/shared_memory_object.hpp>
# include <boost/interprocess/mapped_region.hpp>
# include "shm_obj.h"
# include <signal.h>
bool is_running = true;
void ctrl_handle(int sig) {
	is_running = false;
}

int main() {
	struct sigaction sig;

	sig.sa_handler = ctrl_handle;
	sigemptyset(&sig.sa_mask);
	sig.sa_flags = 0;

	sigaction(SIGINT, &sig, NULL);


	boost::asio::io_service io_service;
	boost::asio::serial_port serial_port(io_service, "/dev/ttyUSB0");
	serial_port.set_option(boost::asio::serial_port_base::baud_rate(9600));

	char const *shm_name = "sh";

	boost::interprocess::shared_memory_object::remove(shm_name);
	boost::interprocess::shared_memory_object shm_obj(boost::interprocess::create_only, shm_name, boost::interprocess::read_write);
	shm_obj.truncate(sizeof(shm_obj_t));

	boost::interprocess::mapped_region region(shm_obj, boost::interprocess::read_write);

	shm_obj_t *shm_obj_ptr = (shm_obj_t*)region.get_address();
	shm_obj_ptr-> reading_paused = false;
	shm_obj_ptr-> reading_wait = false;
	shm_obj_ptr-> task_id = TID_NONE;

	mdl::u8_t incomming_byte = 0;
	mdl::u8_t outgoing_byte = 0;

	mdl::uint_t rx_bfs = 0, tx_bfs = 0;
	do {
		boost::system::error_code any_err;
		try {
			if (shm_obj_ptr-> task_id == TID_READ) {
				boost::asio::read(serial_port, boost::asio::buffer(&incomming_byte, 1), any_err);
				if (any_err == boost::asio::error::eof) continue;
				*(shm_obj_ptr-> rx_buff + rx_bfs) = incomming_byte;

				if (incomming_byte == '\n') {
					*(shm_obj_ptr-> rx_buff + (rx_bfs + 1)) = '\0';
					shm_obj_ptr-> task_id = TID_NONE;
					printf("recved: %s, size: %d\n", shm_obj_ptr-> rx_buff, rx_bfs);
					rx_bfs = 0;
				} else {
					if (rx_bfs != sizeof(shm_obj_ptr-> rx_buff))
						rx_bfs += sizeof(mdl::u8_t);
					else break;
				}
			}

			if (shm_obj_ptr-> task_id == TID_WRITE) {
				outgoing_byte = *(shm_obj_ptr-> tx_buff + tx_bfs);
				boost::asio::write(serial_port, boost::asio::buffer(&outgoing_byte, 1), any_err);

				if (*(shm_obj_ptr-> tx_buff + (tx_bfs + 1)) == '\0') {
					shm_obj_ptr-> task_id = TID_NONE;
					printf("sent: %s, size: %d\n", shm_obj_ptr-> tx_buff, tx_bfs);
					tx_bfs = 0;
				} else {
					if (tx_bfs != sizeof(shm_obj_ptr-> tx_buff))
						tx_bfs += sizeof(mdl::u8_t);
					else break;
				}
			}
		} catch(boost::system::system_error const& e){}
	} while(is_running);

	serial_port.close();
}
