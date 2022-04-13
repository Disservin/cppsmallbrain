#include "timecontroller.h"

int time_left(long long time, int inc, int movestogo) {
	long long search_time{};
	if (movestogo > 0) {
		search_time = time / movestogo;
	}
	else {
		search_time = time / 10;
	}
	search_time += inc / 2;
	
	if (search_time >= time) {
		return time / 10;
	}
	return search_time;
};