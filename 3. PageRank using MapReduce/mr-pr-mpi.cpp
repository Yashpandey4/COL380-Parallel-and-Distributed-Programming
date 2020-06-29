//
// Created by Pratyush on 03-05-2020.
//

#include <cstdio>
#include <cstdlib>
#include <ctime>
#include <cmath>
#include <string>
#include <bits/stdc++.h>
#include <unordered_map>
using namespace std;

#include "mpi.h"
//#include "MapReduceMPI.h"

#define MASTER 0

void print_graph(unordered_map<int,vector<int> >& graph){
    for(auto it=graph.begin();it!=graph.end();it++){
        cout << it->first << " : ";
        vector<int> graph_vec = it->second;
        int sizes = it->second.size();
        for(int i=0;i<sizes;i++){
            cout << graph_vec[i] << " ";
        }
        cout << endl;
    }
}

void map_reduce_mpi(unordered_map<int, vector<int> >& adj_list,
        unordered_map<int, double>& page_rank, int rank, int num_processes) {

    unordered_map<int, vector<double> > map_res;
    int id;

    if(rank == MASTER) {
        unordered_map<int, vector<int> >::iterator it1;
        unordered_map<int, vector<int> >::iterator it2;

        cout << "fine";
        for (it1 = adj_list.begin(); it1 != adj_list.end(); it1++) {
//            vector<int> out_edges = it1->second;
            vector<double> in_edges;
//            for (int i = 0; i < out_edges.size(); i++) {
//                map_res[out_edges[i]] = ((double) page_rank[it1->first]) / ((double) out_edges.size());
//            }
            for(it2 = adj_list.begin(); it2 != adj_list.end(); it2++) {
                vector<int> temp_out_edges = it2->second;

                if(find(temp_out_edges.begin(), temp_out_edges.end(), it1->first)!=temp_out_edges.end())
                    in_edges.push_back(it2->first);
            }

            cout << it1->first << "-----" << endl;

            for(double & in_edge : in_edges)
            {
                in_edge = ((double) page_rank[in_edge]) / ((double) adj_list[in_edge].size());
                cout << in_edge << endl;
            }

            if (in_edges.empty())
                in_edges.push_back(0);

            map_res[it1->first] = in_edges;
        }

        for(it1 = adj_list.begin(); it1 != adj_list.end(); it1++) {
            id = it1->first;
            vector<double> page_rank_node_map = map_res[id];

            MPI_Send(&id, 1, MPI_INT, ((it1->first)%(num_processes-1))+1, ((it1->first+1)*10), MPI_COMM_WORLD);
            MPI_Send(&page_rank_node_map[0], page_rank_node_map.size(), MPI_DOUBLE, ((it1->first)%(num_processes-1))+1, ((it1->first+1)*10)+1, MPI_COMM_WORLD);
        }

        MPI_Status status;

        for(it1 = adj_list.begin(); it1 != adj_list.end(); it1++) {
            float pgrank = 0;
            MPI_Recv(&pgrank, 1, MPI_DOUBLE, ((it1->first)%(num_processes-1))+1, (it1->first)+1, MPI_COMM_WORLD, &status);
            page_rank[it1->first] = pgrank;
        }
    }

    else if(rank > MASTER)
    {
        cout << "fine2\n";
        MPI_Status status;

        for(int i = rank; i <= adj_list.size(); i+=(num_processes-1))
        {
            MPI_Recv(&id, INT_MAX, MPI_INT, 0, i*10, MPI_COMM_WORLD, &status);
            cout <<"fine3\n";
            vector<double> pgrank_values;
            pgrank_values.resize(adj_list.size());
            MPI_Recv(&pgrank_values[0], INT_MAX, MPI_DOUBLE, 0, i*10+1, MPI_COMM_WORLD, &status);
            double sum = 0;
            for(double & pgrank_value : pgrank_values)
            {
                sum += pgrank_value;
            }
            double pg_rank_temp = (double)(0.15/(double)adj_list.size()) + (double)(0.85*sum);

            MPI_Send(&pg_rank_temp, 1, MPI_DOUBLE, 0, i, MPI_COMM_WORLD);
        }
    }

}

int main(int argc, char *argv[]) {
    //  input output from files

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


    unordered_map<int, vector<int> > adj_list; // graph as adjacency list
    //    unordered_map<int, vector<int> > inv_adj_list; // graph as adjacency list
    unordered_map<int, double> page_rank;
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


    double start = MPI_Wtime();

    for(int i=0; i < 5; i++) {

        if(rank == MASTER) {
            for (auto &it : page_rank) {
                cout << it.first << " : " << it.second << endl;
            }
            cout << "---------------" << endl;
        }

        map_reduce_mpi(adj_list, page_rank, rank, num_processes);
    }

    double end = MPI_Wtime();
    double time = end - start;


    double sum = 0.0;
    if(rank == MASTER)
    {
        for(auto & it1 : page_rank) {
            output << it1.first << " : " << it1.second << endl;
            sum += it1.second;
        }
        output << "Total Sum is " << sum << endl;
        printf("The time taken for %d graph size and %d processes using Blocking P2P Communication is %0.4fs\n"
        , graph_size, num_processes, time);
    }

    input.close();
    output.close();
    MPI_Finalize();
    return 0;
}
