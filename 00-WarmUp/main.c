#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>

typedef struct Person {
	int time_waiting_for_elevator;
	int desired_floor; // 0 if going down, 1 <= desired_floor <= 150 
} Person;

typedef struct Elevator {
	short id;
	int floor;
	int people_count;
	long distance_traveled;
	long total_wait_in_elev;
	long total_wait_for_elev;
	long people_off;
	bool upwards; // true if up, false if down (obviously)
	bool stopped; 
	short time_waiting;
	Person* riders[20];
} Elevator;

struct ll_node;

typedef struct ll_node {
	Person* p;
	struct ll_node* next;
} ll_node;

typedef struct linked_list {
	ll_node* head;
	ll_node* tail;
	int size;
} linked_list;

void linked_list_add_end(linked_list* ll, ll_node* node) {
	if (ll->head == NULL) {
		printf("adding to null ll\n");
		node->next = NULL;
		ll->head = node;
		ll->tail = node;
		ll->size++;
		// printf("%p <- new head/tail = %d -> %p\n", ll->tail->prev, ll->tail->p.desired_floor, ll->tail->next);
		return;
	}

	//printf("adding %d to end of linked list\n", node->p->desired_floor);
	// printf("%p <- curtail = %d -> %p\n", ll->tail->prev, ll->tail->p.desired_floor, ll->tail->next);
	node->next = NULL;
	ll->tail->next = node;
	ll->tail = node;
	ll->size++;
	// printf("%p <- newtail = %d -> %p\n", node->prev, node->p.desired_floor, node->next);
	/*
	ll_node* cur = ll->head;
	while (cur != NULL) {
		printf("iterated to find person: %d\n", cur->p.desired_floor);
		cur = cur->next;
	}
	*/
}

ll_node* linked_list_remove_first(linked_list* ll) {
	ll_node* h = ll->head;
	if (ll->size == 1) {
		ll->head = NULL;
		ll->tail = NULL;
		ll->size = 0;
		printf("new size = 0\n");
		return h;
	}
	// printf("old head = %d -> %p\n", ll->head->p->desired_floor, ll->head->next);
	ll->head = ll->head->next;
	ll->size--;
	if (ll->head != NULL) {
		// printf("new head = %d -> %p\n", ll->head->p->desired_floor, ll->head->next);
	}
	printf("new size = %d\n", ll->size);
	return h;
}

// sort in ascending order
void add_rider_chronological(Person* queue[], Person* p, bool upwards, int filled_spots) {
	bool found_spot = false;
	if (filled_spots == 0) {
		printf("empty elevator, adding to first spot\n");
		queue[0] = p;
		return;
	}

	Person* tmp = p;
	for (int i = 0; i < filled_spots; i++) {
		Person* cur = queue[i];

		if (cur->desired_floor > tmp->desired_floor) {
			queue[i] = tmp;
			tmp = cur;
		}
	}
	queue[filled_spots] = tmp;
}

void remove_rider_beginning(Elevator* elev, int filled_spots) {
	elev->riders[0] = NULL;

	for (int i = 1; i < filled_spots; i++) {
		elev->riders[i - 1] = elev->riders[i]; // shift
	}
}

void increment_wait(linked_list* ll) {
	ll_node* cur = ll->head;
	while (cur != NULL) {
		// printf("cur node: %d\n", cur->p->desired_floor);
		// printf("cur = %d -> %p\n", cur->p->desired_floor, cur->next);
		// printf("<- cur = %p -> \n", cur->p);
		cur->p->time_waiting_for_elevator++;
		// printf("new time waiting is: %d\n", cur->p->time_waiting_for_elevator);
		cur = cur->next;
	}
}

bool keep_going(linked_list* floors[], int floor, bool upwards) {
	if (upwards) {
		for (int i = floor; i < 150; i++) {
			if (floors[i]->size != 0) {
				return true;
			}
		}
		return false;
	} else {
		for (int i = floor; i >= 0; i--) {
			if (floors[i]->size != 0) {
				return true;
			}
		}
		return false;
	}
}

void print_array(Person* arr[], int size) {
	for (int i = 0; i < size; i++) {
		printf("%d ", arr[i]->desired_floor);
	}
	printf("\n");
}

void handle_stop(Elevator* elevator, linked_list* ll) {
	print_array(elevator->riders, elevator->people_count);
	printf("old elevator people_count is %d\n", elevator->people_count);
	if (elevator->upwards) {
		int curInd = 0;
		while (curInd < elevator->people_count && elevator->riders[curInd]->desired_floor == elevator->floor) {
			printf("removed rider (%d in elevator for %d) on floor %d\n"
					, elevator->riders[curInd]->desired_floor
					, elevator->riders[curInd]->time_waiting_for_elevator
					, elevator->floor);
			remove_rider_beginning(elevator, elevator->people_count);
			elevator->people_count--;
			elevator->people_off++;
		}
	} else {
		int curInd = elevator->people_count - 1;
		while (curInd >= 0 && elevator->riders[curInd]->desired_floor == elevator->floor) {
			printf("removed rider (%d in elevator for %d) on floor %d\n"
					, elevator->riders[curInd]->desired_floor
					, elevator->riders[curInd]->time_waiting_for_elevator
					, elevator->floor);
			elevator->riders[curInd] = NULL;
			elevator->people_count--;
			elevator->people_off++;
			curInd--;
		}
	}
	while (elevator->people_count < 20 && ll->size > 0) {
		ll_node* removed = linked_list_remove_first(ll);
		printf("adding %d rider from linked list to elevator\n", removed->p->desired_floor);
		printf("added %d to time wait for elevator\n", removed->p->time_waiting_for_elevator);
		add_rider_chronological(elevator->riders, removed->p, elevator->upwards, elevator->people_count);
		elevator->total_wait_for_elev += removed->p->time_waiting_for_elevator;
		elevator->people_count++;
		free(removed);
	}

	printf("new elevator people_count is %d\n", elevator->people_count);
}

int main() {
	Elevator* elevators[8];

	// initialize elevators
	for (int i = 0; i < 8; i++) {
		Elevator* e = malloc(sizeof(Elevator));
		e->id = i;
		e->floor = 0;
		e->people_count = 0;
		e->distance_traveled = 0;
		e->upwards = true;
		e->stopped = false;
		e->time_waiting = 0;
		e->total_wait_in_elev = 0;
		e->total_wait_for_elev = 0;
		e->people_off = 0;
		elevators[i] = e;
	}

	int time = 0;
	// srand(time(NULL));
	
	printf("initialized elevators\n");

	linked_list *floors[150];
	bool elevator_here[150];

	for (int i = 0; i < 150; i++) {
		linked_list* ll = malloc(sizeof(linked_list));
		ll->head = NULL;
		ll->tail = NULL;
		ll->size = 0;
		floors[i] = ll;
	}

	printf("initialized floors\n");

	// simulation start
	while (time < 3600) {
		printf("\nITERATION %d\n", time);
		int num_people_in = rand() % 5; // 0-4
		int num_people_out = rand() % 5; // 0-4
		printf("%d people coming in, %d people coming out\n", num_people_in, num_people_out);

		for (int i = 0; i < num_people_in; i++) {
			linked_list* ll = floors[0];
			ll_node* new_node = malloc(sizeof(ll_node));
			new_node->next = NULL;
			Person* pers = malloc(sizeof(Person));
			pers->desired_floor = rand() % 150;
			pers->time_waiting_for_elevator = 0;
			new_node->p = pers;
			// printf("adding person wanting to go from floor 0 to floor %d\n", new_node->p->desired_floor);
			linked_list_add_end(ll, new_node);
		}

		for (int i = 0; i < num_people_out; i++) {
			int cur_passenger_floor = 1 + rand() % 149; // not on ground floor
			linked_list* ll = floors[cur_passenger_floor];
			ll_node* new_node = malloc(sizeof(ll_node));
			new_node->next = NULL;
			Person* pers = malloc(sizeof(Person));
			pers->desired_floor = 0;
			pers->time_waiting_for_elevator = 0;
			new_node->p = pers;
			// printf("adding person wanting to go from floor %d to floor 0\n", cur_passenger_floor);
			linked_list_add_end(ll, new_node);
		}

		printf("LOOKING AT ELEVATORS *******\n");
		for (int i = 0; i < 8; i++) {
			Elevator* cur = elevators[i];
			cur->total_wait_in_elev += cur->people_count;
			printf("*** LOOKING AT ELEVATOR %d ON FLOOR %d WITH %d PEOPLE ON IT\n", cur->id, cur->floor, cur->people_count);
			print_array(cur->riders, cur->people_count);

			if (cur->stopped) {
				printf("elevator %d is stopped\n", cur->id);
				if (cur->time_waiting == 10) {
					printf("elevator %d has waited for 10 seconds, moving on\n", cur->id);
					cur->stopped = false;
					elevator_here[cur->floor] = false;
					cur->time_waiting = 0;
					continue;
				}
				printf("elevator %d has waited for only %d seconds, picking up people on floor %d\n", cur->id, cur->time_waiting, cur->floor);
				handle_stop(cur, floors[cur->floor]);
				cur->time_waiting++;
			} else {
				printf("elevator %d is moving\n", cur->id);
				if (cur->upwards) { 
					if (cur->riders[0] != NULL && cur->floor == cur->riders[0]->desired_floor) {
						printf("stopping to drop off people on floor %d \n", cur->floor);
						cur->stopped = true;
						elevator_here[cur->floor] = true;
						continue;
					} 
					if (cur->riders[19] !=NULL) {
						printf("%d people_count and last person desired floor = %d\n", cur->people_count, cur->riders[19]->desired_floor);
					}
					if (cur->floor == 149 || (cur->people_count == 20 && cur->riders[19]->desired_floor == 0)) {
						printf("UPWARDS: going down because at floor %d \n", cur->floor);
						elevator_here[cur->floor] = true;
						cur->upwards = false;
						continue;
					}
					if (cur->people_count == 20) {
						printf("Incrementing floor count\n");
						cur->distance_traveled++;
						cur->floor++;
						continue;
					}
					if (floors[cur->floor]->size > 0 && !elevator_here[cur->floor]) {
						printf("Stopping to pick up people\n");
						elevator_here[cur->floor] = true;
						cur->stopped = true;
						continue;
					}
					printf("checking other conditions \n");
					if (!keep_going(floors,cur->floor,cur->upwards) && cur->riders[0] == NULL) { // at end 
						printf("Going down because no one is in elevator\n");
						cur->upwards = false;
						continue;
					}
					printf("Incrementing floor count\n");
					printf("END OF THING: %d people_count\n", cur->people_count);
					cur->distance_traveled++;
					cur->floor++;
				} else { // going down
					if (cur->floor == 0) {
						printf("DOWNWARDS: stopping to drop off people on floor %d \n", cur->floor);
						elevator_here[cur->floor] = true;
						cur->stopped = true;
						cur->upwards = true;
						continue;
					}
					if (cur->people_count == 20) {
						printf("Decrementing floor count\n");
						cur->distance_traveled++;
						cur->floor--;
						continue;
					}
					if (floors[cur->floor]->size > 0 && !elevator_here[cur->floor]) {
						elevator_here[cur->floor] = true;
						cur->stopped = true;
						continue;
					}
					cur->distance_traveled++;
					cur->floor--;
				}
			}
		}

		printf("incrementing wait time\n");
		for (int i = 0; i < 150; i++) {
			// printf("floor %d\n", i);
			linked_list* cur = floors[i];
			// printf("%d\n", cur->size);
			// printf("%p\n", cur);
			increment_wait(cur);
		}
		time++;
	}

	long distance_traveled_tot = 0;
	long total_wait_in = 0;
	long total_wait_for = 0;
	long tot_off = 0;
	for (int i = 0; i < 8; i++) {
		printf("Elevator %d distance traveled: %ld\n", i, elevators[i]->distance_traveled);
		distance_traveled_tot += elevators[i]->distance_traveled;
		printf("Elevator %d wait in elevator: %ld\n", i, elevators[i]->total_wait_in_elev);
		total_wait_in += elevators[i]->total_wait_in_elev;
		printf("Elevator %d wait for elevator: %ld\n", i, elevators[i]->total_wait_for_elev);
		total_wait_for += elevators[i]->total_wait_for_elev;
		printf("Elevator %d total off: %ld\n\n", i, elevators[i]->people_off);
		tot_off += elevators[i]->people_off;
	}
	printf("Average distance traveled: %ld\n", distance_traveled_tot / 8);
	printf("Average wait in elevator: %ld\n", total_wait_in / tot_off);
	printf("Average wait for elevator: %ld\n", total_wait_for / tot_off);
	printf("Total guests off elevator: %ld\n", tot_off);
}
