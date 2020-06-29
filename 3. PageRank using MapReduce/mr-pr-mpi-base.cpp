//
// Created by prats on 5/5/20.
//

#include "mpi.h"
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include "sys/stat.h"
#include "mapreduce.h"
#include "keyvalue.h"
#include <bits/stdc++.h>
#include <unordered_map>
using namespace std;

#define MASTER 0

struct MY_STRUCT {
    unordered_map<int, vector<int> > adj_list;
    vector<int> node_list;
    vector <double> page_rank;
    int num_processes;
    int graph_size;
    int num;
    int val;
};

void mapper(int rank, MAPREDUCE_NS::KeyValue *kv, void *mystr) {
    MY_STRUCT *S = (MY_STRUCT *) mystr;
    int graph_size = S->graph_size;
    int num_processes = S->num_processes;
    int start = rank*(graph_size/num_processes), end;

    if(rank == S->num_processes-1) {
        end = graph_size;
    }
    else {
        end = (rank+1)*(graph_size/num_processes);
    }
    for(int i=start; i<end; ++i)
    {
        kv->add((char *) &i, sizeof(int), (char *)&(S->adj_list[i]), S->adj_list[i].size()*sizeof(int));
    }
    if(rank == MASTER)
    {
        for(int i=0; i<S->graph_size; ++i) {
            S->num += S->page_rank[i]/((double)S->graph_size);
        }
        for(int i=0; i<S->node_list.size(); ++i) {
            S->val += S->page_rank[S->node_list[i]]/((double) S->graph_size);
        }
    }
}

void reducer(char *key, int keybytes, char *multivalue, int nvalues, const int *valuebytes, MAPREDUCE_NS::KeyValue *kv, void *mystr) {
    double res = 0;

    if(*valuebytes == 0) {
        kv->add(key, keybytes, (char *)&res, sizeof(double));
        return;
    }
    MY_STRUCT *S = (MY_STRUCT *) mystr;

    auto *row = (vector<pair<unsigned , double > > *) multivalue;
    auto val = *row;

    for(int i=0; i<val.size(); ++i)
    {
        res += S->page_rank[val[i].first]*val[i].second;
    }
    kv->add(key, keybytes, (char *)&res, sizeof(double));
}

int main(int argc, char *argv[]) {
    ifstream input;
    input.open(argv[1]);
    ofstream output;
    output.open(argv[2]);

    int from_page, to_page;

    int rank, num_processes;
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &num_processes);

    if(num_processes < 1)
    {
        MPI_Abort(MPI_COMM_WORLD, 0);
        exit(1);
    }

    MAPREDUCE_NS::MapReduce *map_red = new MAPREDUCE_NS::MapReduce(MPI_COMM_WORLD);

    unordered_map<int, vector<int> > adj_list; // graph as adjacency list
    //    unordered_map<int, vector<int> > inv_adj_list; // graph as adjacency list
    vector <double> page_rank;
    string line;

    if(input.is_open()) {
        while (getline(input, line)) {
            istringstream input_line(line);
            input_line >> from_page >> to_page;
            //        adj_list[from_page].push_back(to_page);
            //        inv_adj_list[to_page].push_back(from_page);

            if (adj_list.find(from_page) == adj_list.end()) {
                vector<int> temp_out_edge;
                temp_out_edge.push_back(to_page);
                adj_list[from_page] = temp_out_edge;
                if (adj_list.find(to_page) == adj_list.end()) {
                    vector<int> temp;
                    adj_list[to_page] = temp;
                }
            }
            else {
                adj_list[from_page].push_back(to_page);
                if (adj_list.find(to_page) == adj_list.end()) {
                    vector<int> temp;
                    adj_list[to_page] = temp;
                }
            }
        }
    }

    int graph_size = adj_list.size();
    unordered_map<int, vector<int> >::iterator it;

    for (it = adj_list.begin(); it != adj_list.end(); it++) {
        page_rank[it->first] = 1.0 / (double) graph_size;
    }

    MY_STRUCT my_struct;
    my_struct.page_rank = page_rank;
    my_struct.adj_list = adj_list;
    my_struct.num_processes = num_processes;
    my_struct.graph_size = graph_size;
    my_struct.num = 0;
    my_struct.val = 0;

    MPI_Barrier(MPI_COMM_WORLD);
    double start = MPI_Wtime();

    for(int i=0; i<20; i++)
    {
        map_red->map(num_processes, &mapper, &my_struct);
        map_red->convert();
        map_red->reduce(reducer, &my_struct)
        map_red->gather(1);
        delete mr;
    }

    MPI_Barrier(MPI_COMM_WORLD);
    double stop = MPI_Wtime();
    double time = stop - start;

    if(rank == MASTER)
    {
        for(int i=0; i<my_struct.graph_size; i++)
        {
            output << i << " = " << my_struct.page_rank[i] << endl;
        }
        printf("The time taken for %d graph size and %d processes using Blocking P2P Communication is %0.4fs\n"
                , graph_size, num_processes, time);
    }

    MPI_Finalize();

}
