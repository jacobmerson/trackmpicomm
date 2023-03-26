#include <mpi.h>
#include <iostream>
#define BOOST_STACKTRACE_USE_ADDR2LINE
#include <boost/stacktrace.hpp>
#include <map>
#include <unordered_map>
#include <numeric>
#include <sstream>



std::map<boost::stacktrace::basic_stacktrace<>, int> stacktrace_counts;
int call_count = 0;

struct CountResult {
  int created;
  int destroyed;
};
CountResult current_counts() {
  CountResult results;
    results.created = std::accumulate(stacktrace_counts.begin(), stacktrace_counts.end(), 0, [](int a, auto&b) -> int {
        auto val = b.second;
        return a + ((val>0)?val:0);
        });
    results.destroyed = std::accumulate(stacktrace_counts.begin(), stacktrace_counts.end(), 0, [](int a, auto&b) -> int {
        auto val = b.second;
        return a + ((val>0)?0:val);
        });
    return results;
}




#define FUNC_CREATE_MPI_COMM \
    stacktrace_counts[boost::stacktrace::stacktrace()] += 1; \
    call_count += 1; \
    int rank; \
    std::stringstream ss; \
    MPI_Comm_rank(MPI_COMM_WORLD, &rank); \
    if((call_count%10) == 0 ) { \
      auto [created, destroyed] = current_counts(); \
      ss<<"Current: "<<rank<<" "<<created<<" "<<destroyed<<"\n"; \
      std::cerr<<ss.str(); \
    }
  
#define FUNC_DESTROY_MPI_COMM \
    stacktrace_counts[boost::stacktrace::stacktrace()] -= 1;

extern "C" {
  int MPI_Init(int *argc, char ***argv) {
    return PMPI_Init(argc, argv);
  }
  int MPI_Comm_accept(const char *port_name, MPI_Info info, int root, MPI_Comm comm,
                          MPI_Comm * newcomm) {
    FUNC_CREATE_MPI_COMM
    return PMPI_Comm_accept(port_name, info, root, comm, newcomm);
  }
  int MPI_Comm_connect(const char *port_name, MPI_Info info, int root, MPI_Comm comm,
                           MPI_Comm * newcomm) {
    FUNC_CREATE_MPI_COMM
    return PMPI_Comm_connect(port_name, info, root, comm, newcomm);
  }
  int MPI_Comm_create(MPI_Comm comm, MPI_Group group, MPI_Comm * newcomm) {
    FUNC_CREATE_MPI_COMM
    return PMPI_Comm_create(comm, group, newcomm);
  }
  int MPI_Comm_dup_with_info(MPI_Comm comm, MPI_Info info, MPI_Comm * newcomm) {
    FUNC_CREATE_MPI_COMM
    return PMPI_Comm_dup_with_info(comm, info, newcomm);
  }
  int MPI_Comm_create_group(MPI_Comm comm, MPI_Group group, int tag, MPI_Comm * newcomm) {
    FUNC_CREATE_MPI_COMM
    return PMPI_Comm_create_group(comm, group, tag, newcomm);
  }
  int MPI_Comm_free(MPI_Comm * comm) {
    FUNC_DESTROY_MPI_COMM
    return PMPI_Comm_free(comm);
  }
  int MPI_Comm_disconnect(MPI_Comm * comm) {
    FUNC_DESTROY_MPI_COMM
    return PMPI_Comm_disconnect(comm);
  }

  int MPI_Comm_idup(MPI_Comm comm, MPI_Comm * newcomm, MPI_Request * request) {
    FUNC_CREATE_MPI_COMM
    return PMPI_Comm_idup(comm, newcomm, request);
  }
  int MPI_Comm_join(int fd, MPI_Comm * intercomm) {
    FUNC_CREATE_MPI_COMM
    return MPI_Comm_join(fd, intercomm);
  }
  int MPI_Comm_split(MPI_Comm comm, int color, int key, MPI_Comm * newcomm) {
    FUNC_CREATE_MPI_COMM
    return PMPI_Comm_split(comm, color, key, newcomm);
  }
  int MPI_Comm_split_type(MPI_Comm comm, int split_type, int key, MPI_Info info, MPI_Comm * newcomm) {
    FUNC_CREATE_MPI_COMM
    return PMPI_Comm_split_type(comm, split_type, key, info, newcomm);
  }
  int MPI_Dist_graph_create(MPI_Comm comm_old, int n, const int sources[],
                            const int degrees[], const int destinations[],
                            const int weights[],
                            MPI_Info info, int reorder, MPI_Comm * comm_dist_graph) {
    FUNC_CREATE_MPI_COMM
    return PMPI_Dist_graph_create(comm_old, n, sources, degrees, destinations, weights, info,
                                  reorder, comm_dist_graph);
  }
  int MPI_Dist_graph_create_adjacent(MPI_Comm comm_old,
                                     int indegree, const int sources[],
                                     const int sourceweights[],
                                     int outdegree, const int destinations[],
                                     const int destweights[],
                                     MPI_Info info, int reorder, MPI_Comm * comm_dist_graph) {
    FUNC_CREATE_MPI_COMM
    return PMPI_Dist_graph_create_adjacent(comm_old, indegree, sources, sourceweights,
                                    outdegree, destinations, destweights,
                                    info, reorder, comm_dist_graph);
  }
  int MPI_Graph_create(MPI_Comm comm_old, int nnodes, const int indx[],
                           const int edges[], int reorder, MPI_Comm * comm_graph) {
    FUNC_CREATE_MPI_COMM
    return PMPI_Graph_create(comm_old, nnodes, indx, edges, reorder, comm_graph);
  }
  int MPI_Intercomm_create(MPI_Comm local_comm, int local_leader,
                               MPI_Comm peer_comm, int remote_leader, int tag,
                               MPI_Comm * newintercomm) {
    FUNC_CREATE_MPI_COMM
    return PMPI_Intercomm_create(local_comm, local_leader, peer_comm, remote_leader,
                                 tag, newintercomm);
  }
  int MPI_Intercomm_merge(MPI_Comm intercomm, int high, MPI_Comm * newintracomm) {
    FUNC_CREATE_MPI_COMM
    return PMPI_Intercomm_merge(intercomm, high, newintracomm);
  }

  int MPI_Comm_dup(MPI_Comm comm, MPI_Comm* newcomm) {
    FUNC_CREATE_MPI_COMM
    return PMPI_Comm_dup(comm, newcomm);
  }

  int MPI_Finalize() {
    auto [total_create, total_destroy] = current_counts();
    int rank;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    int leaks = total_create+total_destroy;
    std::stringstream ss;
    ss << "Rank: "<<rank <<"\n"<<"Create: "<<total_create <<" Destroy: "<<abs(total_destroy)<<"\n";
    if ( leaks != 0) {
      ss <<"Application had "<<leaks << " MPI Communicator leaks\n";
      std::unordered_map<std::string, int> function_counts;
      ss << "Stack Traces\n";
      for(auto& [st,count] : stacktrace_counts) {
        function_counts[st[0].name()] += count;
        ss <<"Count "<<count<<"\n";
        ss << st;
        ss <<"\n";
      }
      ss << "Function Counts\n";
      for(auto& [name,count] : function_counts) {
        ss << name <<" "<<abs(count)<<"\n";
      }
      ss <<"\n";
    } else {
      ss <<"Application had no MPI Communicator leaks\n";
    }
    std::cerr<<ss.str();
    return PMPI_Finalize();
  }

}
#undef FUNC_CREATE_MPI_COMM
