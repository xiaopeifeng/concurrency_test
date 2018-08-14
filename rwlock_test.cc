#include <pthread.h>
#include <thread>
#include <iostream>
#include <vector>
#include <memory>
#include <string.h>
#include <errno.h>
#include <time.h>
#include <unistd.h>

using std::cerr;
using std::cout;
using std::thread;
using std::make_shared;
using std::shared_ptr;

uint64_t gNR = 0;
pthread_rwlock_t mtx = PTHREAD_RWLOCK_INITIALIZER;

uint16_t reader_count = 8;
uint16_t writer_count = 2;

uint32_t R_operation_cnt = 1000000;
uint32_t W_operation_cnt = 100000;

int main()
{
	std::vector<shared_ptr<thread> > pool;

	auto t1 = time(NULL);
	for (auto i = 0; i < reader_count; ++i) {

		auto reader = make_shared<thread>([&]() {

			auto op_cnt = R_operation_cnt;

			while (op_cnt--) {

				if (pthread_rwlock_rdlock(&mtx) == 0) {
					int x = gNR;
//					cout << "reading gNR:" << gNR << "\n";
				} else {
					cerr << "reader rwlock error:" << strerror(errno) << "\n";
				}

				pthread_rwlock_unlock(&mtx);
			}

			cout << "one reader finished\n";
		});

		pool.push_back(reader);
	}

	for (auto i = 0; i < writer_count; ++i) {

		auto writer = make_shared<thread>([&]() {
			auto op_cnt = W_operation_cnt;

			while (op_cnt--) { 
				if (pthread_rwlock_wrlock(&mtx) == 0) {

					gNR++;
					pthread_rwlock_unlock(&mtx);

					usleep(1);

				} else {
					cerr << "writer rwlock error:" << strerror(errno) << "\n";
				}
			}

			cout << "one writer finished\n";
		});

		pool.push_back(writer);
	}

	for (auto t : pool) {
		t->join();
	}

	assert(gNR == W_operation_cnt*writer_count);

	auto t2 = time(NULL);
	std::cout << "perf finished, duration " << t2 - t1 << " s\n";

}

