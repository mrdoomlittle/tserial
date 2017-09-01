# include <mdlint.h>
# include <atomic>
enum task_id {
	TID_NONE,
	TID_READ,
	TID_WRITE
};

typedef enum task_id task_id_t;

typedef struct {
	bool reading_paused = false;
	bool reading_wait = false;
	task_id_t task_id = TID_NONE;
	mdl::u8_t rx_buff[200];
	mdl::uint_t rxb_size = 0;
	mdl::u8_t tx_buff[200];
	mdl::uint_t txb_size = 0;
} shm_obj_t;
