#include <iostream>
#include <fstream>
#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <cstdio>
#include <time.h>

#define ARR 1
#define DEP 2
#define TS 3

struct event{
    float time;
    int   type;
    struct process* proc;
    struct event* next;
};

struct process{
    int id;
    float arr_time;
    float dep_time;
    float ser_time;
    float rem_ser_time;
    float run_time;
    float preempt_time;
    float response_ratio;
    float turnaround_time;
    struct process* next;
};

void fcfs_init(float lambda, float mu);
int run_fcfs(float lambda, float mu);
void fcfs_schedule_event(int event_type, process* p);
int fcfs_process_ARR(struct event* eve, float lambda, float mu);
int fcfs_process_DEP(struct event* eve, float lambda, float mu);

void srtf_init(float lambda, float mu);
int run_srtf(float lambda, float mu);
void srtf_schedule_event(int event_type, process* p);
int srtf_process_ARR(struct event* eve, float lambda, float mu);
int srtf_process_DEP(struct event* eve, float lambda, float mu);

void hrrn_init(float lambda, float mu);
int run_hrrn(float lambda, float mu);
void hrrn_schedule_event(int event_type, process* p);
int hrrn_process_ARR(struct event* eve, float lambda, float mu);
int hrrn_process_DEP(struct event* eve, float lambda, float mu);

void rr_init(float lambda, float mu);
int run_rr(float lambda, float mu);
void rr_schedule_event(int event_type, process* p);
int rr_process_ARR(struct event* eve, float lambda, float mu);
int rr_process_DEP(struct event* eve, float lambda, float mu);

void generate_report(int scheduler, int lambda, float turnaround, float throughput, float utilization);
float urand();
float genexp(float lambda);

event* e_queue; // head of event queue
process* r_queue; //head of ready queue
process* process_list; //head of process list
process* running_process;
float sim_clock; // simulation clock
float quantum;
float idle_start;
float idle_end;
float total_idle;
float rq_change;
float rq_avg;
int rq_size;
int CPU_idle;
int process_count;
int finished_count;

/////////////////////////////////////FCFS//////////////////////////////////////
void fcfs_init(float lambda, float mu)
{
    CPU_idle = 1;
    sim_clock = 0;
    finished_count = 0;
    process_count = 0;
    r_queue = NULL;
    e_queue = NULL;
    process_list = NULL;
    idle_start = 0.0;
    idle_end = 0.0;
    total_idle = 0.0;
    rq_change = 0.0;
    rq_size = 0;
    rq_avg = 0.0;


    process* p = new process;
    p->id = process_count;
    p->arr_time = genexp(lambda);
    p->ser_time = genexp(mu);
    p->next = NULL;
	process_count++;

    fcfs_schedule_event(ARR, p);
}

void fcfs_schedule_event(int event_type, process* p)
{
    //std::cout << "Begin scheduling " << event_type << " event." << std::endl;
    event* eve = new event;
    eve->type = event_type;
    eve->proc = p;
    eve->next = NULL;

    if(event_type == ARR)
        eve->time = p->arr_time;
    else if(event_type == DEP){
	    p->dep_time = sim_clock + p->ser_time;
        eve->time = p->dep_time;
    }

    if(e_queue == NULL){
        e_queue = eve;
        e_queue->next = NULL;
    }else if(eve->time < e_queue->time){
        eve->next = e_queue;
        e_queue = eve;
    }else{
        event* curr = e_queue;
        while(curr->next != NULL && curr->next->time < eve->time) {
            curr = curr->next;
        }
        eve->next = curr->next;
        curr->next = eve;
    }

    /*
    std::cout << "Done scheduling " << event_type << " event." << std::endl;
	event* print = e_queue;
    while(print != NULL){
	std::cout << "ID: " << print->proc->id << " Type: " << print->type << " Time: " << print->time << std::endl;
        print = print->next;
    }*/
}

int fcfs_process_ARR(struct event* eve, float lambda, float mu)
{
    //std::cout << "Begin processing arrival." << std::endl;
    if(CPU_idle == 1){
		CPU_idle = 0;
		idle_end = sim_clock;
		total_idle = total_idle + (idle_end - idle_start);
		fcfs_schedule_event(DEP, eve->proc);
    }else{
        if(r_queue == NULL){
            r_queue = eve->proc;
            r_queue->next = NULL;
        }
		else{
			process* curr = r_queue;
			while(curr->next != NULL){
				curr = curr->next;
			}
			curr->next = eve->proc;
			eve->proc->next = NULL;
		}
		rq_avg = rq_avg + (rq_size * (sim_clock - rq_change));
		rq_size++;
		rq_change = sim_clock;
    }

    process* p = new process;
    p->id = process_count;
    p->arr_time = sim_clock + genexp(lambda);
    p->ser_time = genexp(mu);
    p->next = NULL;
	process_count++;

    fcfs_schedule_event(ARR, p);

    //std::cout << "Done processing arrival." << std::endl;

    return 0;
}

int fcfs_process_DEP(struct event* eve, float lambda, float mu)
{
    //std::cout << "Begin processing departure." << std::endl;
    if(r_queue == NULL){
		CPU_idle = 1;
		idle_start = sim_clock;
    }
    else{
		process* p = r_queue;
        fcfs_schedule_event(DEP, p);
		r_queue = p->next;

		rq_avg = rq_avg + (rq_size * (sim_clock - rq_change));
        rq_size--;
        rq_change = sim_clock;
    }
	finished_count++;

    // Create copy of departed process and insert at end of list for report data
    process* p_copy = new process;
    p_copy->id = eve->proc->id;
    p_copy->arr_time = eve->proc->arr_time;
    p_copy->dep_time = eve->proc->dep_time;
    p_copy->ser_time = eve->proc->ser_time;
    p_copy->next = NULL;

    if(process_list == NULL) {
        process_list = p_copy;
    }
    else{
        process* curr = process_list;
        while(curr->next != NULL){
            curr = curr->next;
        }
        curr->next = p_copy;
    }

    //std::cout << "Done processing departure." << std::endl;
    return 0;
}

int run_fcfs(float lambda, float mu)
{
    event* eve;
    while (finished_count != 10000)
    {
        eve = e_queue;
        sim_clock = eve->time;
        //std::cout << "Sim clock: " << sim_clock << std::endl;
        switch (eve->type)
        {
            case ARR:
                fcfs_process_ARR(eve, lambda, mu);
                break;
            case DEP:
                fcfs_process_DEP(eve, lambda, mu);
                break;
            default:
                return 0;
                break;
                // error
        }

        e_queue = eve->next;
        free(eve);
        eve = NULL;
    }

    //std::cout << sim_clock << std::endl;

    return 0;
}
///////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////SRTF//////////////////////////////////////
void srtf_init(float lambda, float mu)
{
    CPU_idle = 1;
    sim_clock = 0;
    finished_count = 0;
    process_count = 0;
    r_queue = NULL;
    e_queue = NULL;
    process_list = NULL;
    running_process = NULL;
    idle_start = 0.0;
    idle_end = 0.0;
    total_idle = 0.0;
    rq_change = 0.0;
    rq_size = 0;
    rq_avg = 0.0;

    process* p = new process;
    p->id = process_count;
    p->arr_time = genexp(lambda);
    p->ser_time = genexp(mu);
    p->rem_ser_time = p->ser_time;
    p->next = NULL;
    process_count++;

    srtf_schedule_event(ARR, p);
}

void srtf_schedule_event(int event_type, process* p)
{
    //std::cout << "Begin scheduling " << event_type << " event." << std::endl;
    event* eve = new event;
    eve->type = event_type;
    eve->proc = p;
    eve->next = NULL;

    if(event_type == ARR)
        eve->time = p->arr_time;
    else if(event_type == DEP){
        p->dep_time = sim_clock + p->rem_ser_time;
        eve->time = p->dep_time;
    }

    if(e_queue == NULL){
        e_queue = eve;
        e_queue->next = NULL;
    }else if(eve->time < e_queue->time){
        eve->next = e_queue;
        e_queue = eve;
    }else{
        event* curr = e_queue;
        while(curr->next != NULL && curr->next->time < eve->time) {
            curr = curr->next;
        }
        eve->next = curr->next;
        curr->next = eve;
    }

    /*
    std::cout << "Done scheduling " << event_type << " event." << std::endl;
    event* print = e_queue;
    while(print != NULL){
        std::cout << "ID: " << print->proc->id << " Type: " << print->type << " Time: " << print->time << std::endl;
        print = print->next;
    }*/
}

int srtf_process_ARR(struct event* eve, float lambda, float mu)
{
    //std::cout << "Begin processing arrival." << std::endl;
    if(CPU_idle == 1){
        CPU_idle = 0;
        idle_end = sim_clock;
        total_idle = total_idle + (idle_end - idle_start);
        running_process = eve->proc;
        running_process->run_time = sim_clock;
        srtf_schedule_event(DEP, eve->proc);
    }else{
        running_process->rem_ser_time = running_process->rem_ser_time - (sim_clock - running_process->run_time);
        if(running_process->rem_ser_time < eve->proc->rem_ser_time){
            if(r_queue == NULL) {
                r_queue = eve->proc;
                r_queue->next = NULL;
            }else if(eve->proc->rem_ser_time < r_queue->rem_ser_time){
                eve->proc->next = r_queue;
                r_queue = eve->proc;
            }else{
                process* curr = r_queue;
                while(curr->next != NULL && curr->next->rem_ser_time < eve->proc->rem_ser_time){
                    curr = curr->next;
                }
                eve->proc->next = curr->next;
                curr->next = eve->proc;
            }
        }else{
            eve->proc->run_time = sim_clock;
            if(r_queue == NULL) {
                r_queue = running_process;
                r_queue->next = NULL;
            }else if(running_process->rem_ser_time < r_queue->rem_ser_time) {
                running_process->next = r_queue;
                r_queue = running_process;
            }else{
                process* curr = r_queue;
                while(curr->next != NULL && curr->next->rem_ser_time < running_process->rem_ser_time){
                    curr = curr->next;
                }
                running_process->next = curr->next;
                curr->next = running_process;
            }
            event* curr = e_queue;
            if(curr->proc->id == running_process->id){
                e_queue = curr->next;
                free(curr);
            }else{
                event* pred = curr;
                curr = curr->next;
                while (curr != NULL && curr->proc->id != running_process->id) {
                    pred = curr;
                    curr = curr->next;
                }
                pred->next = curr->next;
                free(curr);
            }
            running_process = eve->proc;
            srtf_schedule_event(DEP, eve->proc);
        }

        rq_avg = rq_avg + (rq_size * (sim_clock - rq_change));
        rq_size++;
        rq_change = sim_clock;
    }

    process* p = new process;
    p->id = process_count;
    p->arr_time = sim_clock + genexp(lambda);
    p->ser_time = genexp(mu);
    p->rem_ser_time = p->ser_time;
    p->next = NULL;
    process_count++;

    srtf_schedule_event(ARR, p);

    //std::cout << "Done processing arrival." << std::endl;

    return 0;
}

int srtf_process_DEP(struct event* eve, float lambda, float mu)
{

    //std::cout << "Begin processing departure." << std::endl;
    if(r_queue == NULL){
        CPU_idle = 1;
        idle_start = sim_clock;
    }
    else{
        /*
        process* print = r_queue;
        while(print != NULL){
            std::cout << "ID: " << print->id << "\tRemaining_time: " << print->rem_ser_time << std::endl;
            print = print->next;
        }*/

        running_process = r_queue;
        r_queue = r_queue->next;
        running_process->next = NULL;

        //std::cout << running_process->id <<std::endl;
        srtf_schedule_event(DEP, running_process);

        rq_avg = rq_avg + (rq_size * (sim_clock - rq_change));
        rq_size--;
        rq_change = sim_clock;
    }
    finished_count++;

    // Create copy of departed process and insert at end of list for report data
    process* p_copy = new process;
    p_copy->id = eve->proc->id;
    p_copy->arr_time = eve->proc->arr_time;
    p_copy->dep_time = eve->proc->dep_time;
    p_copy->ser_time = eve->proc->ser_time;
    p_copy->next = NULL;

    if(process_list == NULL) {
        process_list = p_copy;
    }
    else{
        process* curr = process_list;
        while(curr->next != NULL){
            curr = curr->next;
        }
        curr->next = p_copy;
    }

    //std::cout << "Done processing departure." << std::endl;
    return 0;
}

int run_srtf(float lambda, float mu)
{
    event* eve;
    while (finished_count != 10000)
    {
        eve = e_queue;
        sim_clock = eve->time;
        //std::cout << "Sim clock: " << sim_clock << std::endl;
        switch (eve->type)
        {
            case ARR:
                srtf_process_ARR(eve, lambda, mu);
                break;
            case DEP:
                srtf_process_DEP(eve, lambda, mu);
                break;
            default:
                return 0;
                break;
                // error
        }

        e_queue = eve->next;
        free(eve);
        eve = NULL;
    }

    //std::cout << sim_clock << std::endl;

    return 0;
}
///////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////HRRN//////////////////////////////////////
void hrrn_init(float lambda, float mu)
{
    CPU_idle = 1;
    sim_clock = 0;
    finished_count = 0;
    process_count = 0;
    r_queue = NULL;
    e_queue = NULL;
    process_list = NULL;
    idle_start = 0.0;
    idle_end = 0.0;
    total_idle = 0.0;
    rq_change = 0.0;
    rq_size = 0;
    rq_avg = 0.0;

    process* p = new process;
    p->id = process_count;
    p->arr_time = genexp(lambda);
    p->ser_time = genexp(mu);
    p->next = NULL;
    process_count++;

    hrrn_schedule_event(ARR, p);
}

void hrrn_schedule_event(int event_type, process* p)
{
    //std::cout << "Begin scheduling " << event_type << " event." << std::endl;
    event* eve = new event;
    eve->type = event_type;
    eve->proc = p;
    eve->next = NULL;

    if(event_type == ARR)
        eve->time = p->arr_time;
    else if(event_type == DEP){
        p->dep_time = sim_clock + p->ser_time;
        eve->time = p->dep_time;
    }

    if(e_queue == NULL){
        e_queue = eve;
        e_queue->next = NULL;
    }else if(eve->time < e_queue->time){
        eve->next = e_queue;
        e_queue = eve;
    }else{
        event* curr = e_queue;
        while(curr->next != NULL && curr->next->time < eve->time) {
            curr = curr->next;
        }
        eve->next = curr->next;
        curr->next = eve;
    }

    /*
    std::cout << "Done scheduling " << event_type << " event." << std::endl;
    event* print = e_queue;
    while(print != NULL){
        std::cout << "ID: " << print->proc->id << " Type: " << print->type << " Time: " << print->time << std::endl;
        print = print->next;
    }*/
}

int hrrn_process_ARR(struct event* eve, float lambda, float mu)
{
    //std::cout << "Begin processing arrival." << std::endl;
    if(CPU_idle == 1){
        CPU_idle = 0;
        idle_end = sim_clock;
        total_idle = total_idle + (idle_end - idle_start);
        hrrn_schedule_event(DEP, eve->proc);
    }else{
        if(r_queue == NULL){
            r_queue = eve->proc;
            eve->proc->next = NULL;
        }
        else{
            process* curr = r_queue;
            while(curr->next != NULL){
                curr = curr->next;
            }
            curr->next = eve->proc;
            eve->proc->next = NULL;
        }

        rq_avg = rq_avg + (rq_size * (sim_clock - rq_change));
        rq_size++;
        rq_change = sim_clock;
    }

    process* p = new process;
    p->id = process_count;
    p->arr_time = sim_clock + genexp(lambda);
    p->ser_time = genexp(mu);
    process_count++;

    hrrn_schedule_event(ARR, p);

    //std::cout << "Done processing arrival." << std::endl;

    return 0;
}

int hrrn_process_DEP(struct event* eve, float lambda, float mu)
{
    //std::cout << "Begin processing departure." << std::endl;
    if(r_queue == NULL){
        CPU_idle = 1;
        idle_start = sim_clock;
    }else if(r_queue->next == NULL){
        /*
        process* print = r_queue;
        while(print != NULL){
            std::cout << "ID: " << print->id << "\tR: " << print->response_ratio << std::endl;
            print = print->next;
        }*/

        hrrn_schedule_event(DEP, r_queue);
        r_queue = r_queue->next;

        rq_avg = rq_avg + (rq_size * (sim_clock - rq_change));
        rq_size--;
        rq_change = sim_clock;
    }else{
        process* highest_ratio = r_queue;
        while(highest_ratio != NULL){
            highest_ratio->response_ratio = ((sim_clock - highest_ratio->arr_time) + highest_ratio->ser_time) / highest_ratio->ser_time;
            highest_ratio = highest_ratio->next;
        }

        /*
        process* print = r_queue;
        while(print != NULL){
            std::cout << "ID: " << print->id << "\tR: " << print->response_ratio << std::endl;
            print = print->next;
        }*/

        highest_ratio = r_queue;
        process* curr = r_queue->next;
        while(curr != NULL){
            if(curr->response_ratio > highest_ratio->response_ratio)
                highest_ratio = curr;
            curr = curr->next;
        }
        //std::cout << highest_ratio->id << std::endl;
        if(r_queue == highest_ratio){
            r_queue = r_queue->next;
            highest_ratio->next = NULL;
        }else{
            curr = r_queue;
            while (curr->next != highest_ratio) {
                curr = curr->next;
            }
            curr->next = highest_ratio->next;
            highest_ratio->next = NULL;
        }

        hrrn_schedule_event(DEP, highest_ratio);

        rq_avg = rq_avg + (rq_size * (sim_clock - rq_change));
        rq_size--;
        rq_change = sim_clock;

    }
    finished_count++;

    // Create copy of departed process and insert at end of list for report data
    process* p_copy = new process;
    p_copy->id = eve->proc->id;
    p_copy->arr_time = eve->proc->arr_time;
    p_copy->dep_time = eve->proc->dep_time;
    p_copy->ser_time = eve->proc->ser_time;
    p_copy->next = NULL;

    if(process_list == NULL) {
        process_list = p_copy;
    }
    else{
        process* curr = process_list;
        while(curr->next != NULL){
            curr = curr->next;
        }
        curr->next = p_copy;
    }

    //std::cout << "Done processing departure." << std::endl;
    return 0;
}

int run_hrrn(float lambda, float mu)
{
    event* eve;
    while (finished_count != 10000)
    {
        eve = e_queue;
        sim_clock = eve->time;
        //std::cout << "Sim clock: " << sim_clock << std::endl;
        switch (eve->type)
        {
            case ARR:
                hrrn_process_ARR(eve, lambda, mu);
                break;
            case DEP:
                hrrn_process_DEP(eve, lambda, mu);
                break;
            default:
                return 0;
                break;
                // error
        }

        e_queue = eve->next;
        free(eve);
        eve = NULL;
    }

    //std::cout << sim_clock << std::endl;

    return 0;
}
///////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////RR///////////////////////////////////////
void rr_init(float lambda, float mu)
{
    CPU_idle = 1;
    sim_clock = 0;
    finished_count = 0;
    process_count = 0;
    r_queue = NULL;
    e_queue = NULL;
    process_list = NULL;
    idle_start = 0.0;
    idle_end = 0.0;
    total_idle = 0.0;
    rq_change = 0.0;
    rq_size = 0;
    rq_avg = 0.0;

    process* p = new process;
    p->id = process_count;
    p->arr_time = genexp(lambda);
    p->ser_time = genexp(mu);
    p->rem_ser_time = p->ser_time;
    p->next = NULL;
    process_count++;

    rr_schedule_event(ARR, p);
}

void rr_schedule_event(int event_type, process* p)
{
    //std::cout << "Begin scheduling " << event_type << " event." << std::endl;
    event* eve = new event;
    eve->type = event_type;
    eve->proc = p;
    eve->next = NULL;

    if(event_type == ARR)
        eve->time = p->arr_time;
    else if(event_type == DEP){
        p->dep_time = sim_clock + p->rem_ser_time;
        eve->time = p->dep_time;
    }else if(event_type == TS){
        eve->time = sim_clock + quantum;
    }

    if(e_queue == NULL){
        e_queue = eve;
        e_queue->next = NULL;
    }else if(eve->time < e_queue->time){
        eve->next = e_queue;
        e_queue = eve;
    }else{
        event* curr = e_queue;
        while(curr->next != NULL && curr->next->time < eve->time) {
            curr = curr->next;
        }
        eve->next = curr->next;
        curr->next = eve;
    }

    /*
    std::cout << "Done scheduling " << event_type << " event." << std::endl;
    event* print = e_queue;
    while(print != NULL){
        std::cout << "ID: " << print->proc->id << " Type: " << print->type << " Time: " << print->time << std::endl;
        print = print->next;
    }*/
}

int rr_process_ARR(struct event* eve, float lambda, float mu)
{
    //std::cout << "Begin processing arrival." << std::endl;
    if(CPU_idle == 1){
        CPU_idle = 0;
        idle_end = sim_clock;
        total_idle = total_idle + (idle_end - idle_start);
        if(eve->proc->rem_ser_time <= quantum)
            rr_schedule_event(DEP, eve->proc);
        else
            rr_schedule_event(TS, eve->proc);
    }else{
        if(r_queue == NULL){
            r_queue = eve->proc;
            r_queue->next = NULL;
        }
        else{
            process* curr = r_queue;
            while(curr->next != NULL){
                curr = curr->next;
            }
            curr->next = eve->proc;
            eve->proc->next = NULL;
        }

        rq_avg = rq_avg + (rq_size * (sim_clock - rq_change));
        rq_size++;
        rq_change = sim_clock;
    }

    process* p = new process;
    p->id = process_count;
    p->arr_time = sim_clock + genexp(lambda);
    p->ser_time = genexp(mu);
    p->rem_ser_time = p->ser_time;
    p->next = NULL;
    process_count++;

    rr_schedule_event(ARR, p);

    //std::cout << "Done processing arrival." << std::endl;

    return 0;
}

int rr_process_DEP(struct event* eve, float lambda, float mu)
{

    //std::cout << "Begin processing departure." << std::endl;
    if(r_queue == NULL){
        CPU_idle = 1;
        idle_start = sim_clock;
    }
    else{
        /*
        process* print = r_queue;
        while(print != NULL){
            std::cout << "ID: " << print->id << "\tRemaining_time: " << print->rem_ser_time << std::endl;
            print = print->next;
        }*/

        running_process = r_queue;
        r_queue = r_queue->next;
        running_process->next = NULL;

        //std::cout << running_process->id <<std::endl;
        if(running_process->rem_ser_time <= quantum)
            rr_schedule_event(DEP, running_process);
        else
            rr_schedule_event(TS, running_process);

        rq_avg = rq_avg + (rq_size * (sim_clock - rq_change));
        rq_size--;
        rq_change = sim_clock;
    }
    finished_count++;

    // Create copy of departed process and insert at end of list for report data
    process* p_copy = new process;
    p_copy->id = eve->proc->id;
    p_copy->arr_time = eve->proc->arr_time;
    p_copy->dep_time = eve->proc->dep_time;
    p_copy->ser_time = eve->proc->ser_time;
    p_copy->next = NULL;

    if(process_list == NULL) {
        process_list = p_copy;
    }
    else{
        process* curr = process_list;
        while(curr->next != NULL){
            curr = curr->next;
        }
        curr->next = p_copy;
    }

    //std::cout << "Done processing departure." << std::endl;
    return 0;
}

int rr_process_TS(struct event* eve, float lambda, float mu)
{
    //std::cout << "Begin processing timeslice." << std::endl;
    eve->proc->rem_ser_time = eve->proc->rem_ser_time - quantum;
    if(r_queue == NULL){
        if(eve->proc->rem_ser_time <= quantum)
            rr_schedule_event(DEP, eve->proc);
        else
            rr_schedule_event(TS, eve->proc);
    }
    else{
        /*
        process* print = r_queue;
        while(print != NULL){
            std::cout << "ID: " << print->id << "\tRemaining_time: " << print->rem_ser_time << std::endl;
            print = print->next;
        }*/

        process* curr = r_queue;
        while(curr->next != NULL){
            curr = curr->next;
        }
        curr->next = eve->proc;
        eve->proc->next = NULL;

        running_process = r_queue;
        r_queue = r_queue->next;
        running_process->next = NULL;

        //std::cout << running_process->id <<std::endl;
        if(running_process->rem_ser_time <= quantum)
            rr_schedule_event(DEP, running_process);
        else
            rr_schedule_event(TS, running_process);
    }

    //std::cout << "Done processing timeslice." << std::endl;

    return 0;
}

int run_rr(float lambda, float mu)
{
    event* eve;
    while (finished_count != 10000)
    {
        eve = e_queue;
        sim_clock = eve->time;
        //std::cout << "Sim clock: " << sim_clock << std::endl;
        switch (eve->type)
        {
            case ARR:
                rr_process_ARR(eve, lambda, mu);
                break;
            case DEP:
                rr_process_DEP(eve, lambda, mu);
                break;
            case TS:
                rr_process_TS(eve, lambda, mu);
                break;
            default:
                return -1;
                break;
                // error
        }

        e_queue = eve->next;
        free(eve);
        eve = NULL;
    }

    //std::cout << "End time: " << sim_clock << std::endl;

    return 0;
}
///////////////////////////////////////////////////////////////////////////////
void generate_report(int scheduler, int lambda, float turnaround, float throughput, float utilization)
{
    std::ofstream outfile ("sim.csv");

    outfile << scheduler << "," << lambda << "," << turnaround << "," << throughput << "," << utilization << "," << rq_avg << std::endl;

    outfile.close();
}

float urand()
{
    return( (float) rand()/RAND_MAX );
}

float genexp(float lambda)
{
    float u,x;
    x = 0;
    while (x == 0)
    {
        u = urand();
        x = (-1/lambda)*log(u);
    }
    return(x);
}

int main(int argc, char* argv[])
{
    if (argc != 5) {fprintf(stderr, "USAGE: %s scheduler[1-4] arrival_rate service_time quantum\n", argv[0]); exit(-1);}
    int scheduler = atoi(argv[1]);
    if (scheduler < 1 || scheduler > 4) {fprintf(stderr, "ERROR: scheduler must be between 1 and 4.\n"); exit(-1);}
    int lambda = atoi(argv[2]);
    if (lambda < 1) {fprintf(stderr, "ERROR: arrival_rate must be at least 1.\n"); exit(-1);}
    std::cout << "Arrival_rate: " << lambda << std::endl;
    float mu = atof(argv[3]);
    if (mu <= 0) {fprintf(stderr, "ERROR: service_time must be greater than 0.\n"); exit(-1);}
    mu = 1 / mu;
    std::cout << "Service_time: " << mu << std::endl;
    quantum = atof(argv[4]);
    if (quantum <= 0) {fprintf(stderr, "ERROR: quantum must be greater than 0.\n"); exit(-1);}

    float avg_turnaround = 0.0;
    float throughput = 0.0;
    float utilization = 0.0;
    int count = 0;

    switch(scheduler)
    {
        case 1:
            std::cout << "FCFS" << std::endl;
            fcfs_init(lambda, mu);
            run_fcfs(lambda, mu);
            break;
        case 2:
            std::cout << "SRTF" << std::endl;
            srtf_init(lambda, mu);
            run_srtf(lambda, mu);
            break;
        case 3:
            std::cout << "HRRN" << std::endl;
            hrrn_init(lambda, mu);
            run_hrrn(lambda, mu);
            break;
        case 4:
            std::cout << "Quantum: " << quantum << std::endl;
            std::cout << "RR" << std::endl;
            rr_init(lambda, mu);
            run_rr(lambda, mu);
            break;
    }

    //std::cout << finished_count << std::endl;

    process* calc = process_list;
    while(calc != NULL){
        calc->turnaround_time = calc->dep_time - calc->arr_time;
        avg_turnaround = avg_turnaround + calc->turnaround_time;
        calc = calc->next;
    }
    avg_turnaround = avg_turnaround / finished_count;
    std::cout <<  "Average turnaround: " << avg_turnaround << std::endl;

    throughput = finished_count / sim_clock;
    std::cout <<  "Throughput: " << throughput << std::endl;

    utilization = total_idle / sim_clock;
    utilization = 1 - utilization;
    std::cout <<  "Utilization: " << utilization << std::endl;
    //std::cout <<  "Idle time: " << total_idle << std::endl;

    rq_avg = rq_avg / sim_clock;
    std::cout <<  "Average ready queue size: " << rq_avg << std::endl;

    generate_report(scheduler, lambda, avg_turnaround, throughput, utilization);

    /*
    process* print = process_list;
    while(print != NULL){
        std::cout << print->id << "\t" << print->arr_time << "\t\t" << print->dep_time << std::endl;
        print = print->next;
        count++;
    }

    std::cout << std::endl;
    std::cout << count << std::endl;
    */
    return 0;
}