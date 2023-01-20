#include "page_rank.h"

#include <cstring>
#include <stdlib.h>
#include <cmath>
#include <omp.h>
#include <utility>
#include <vector>

#include "../common/CycleTimer.h"
#include "../common/graph.h"
#include "common/graph_internal.h"


// pageRank --
//
// g:           graph to process (see common/graph.h)
// solution:    array of per-vertex vertex scores (length of array is num_nodes(g))
// damping:     page-rank algorithm's damping parameter
// convergence: page-rank algorithm's convergence threshold
//
void pageRank(Graph g, double* solution, double damping, double convergence)
{


  // initialize vertex weights to uniform probability. Double
  // precision scores are used to avoid underflow for large graphs
  
  /*
     CS149 students: Implement the page rank algorithm here.  You
     are expected to parallelize the algorithm using openMP.  Your
     solution may need to allocate (and free) temporary arrays.

     Basic page rank pseudocode is provided below to get you started:

     // initialization: see example code above
     score_old[vi] = 1/numNodes;

     while (!converged) {

       // compute score_new[vi] for all nodes vi:
       score_new[vi] = sum over all nodes vj reachable from incoming edges
                          { score_old[vj] / number of edges leaving vj  }
       score_new[vi] = (damping * score_new[vi]) + (1.0-damping) / numNodes;

       score_new[vi] += sum over all nodes v in graph with no outgoing edges
                          { damping * score_old[v] / numNodes }

       // compute how much per-node scores have changed
       // quit once algorithm has converged

       global_diff = sum over all nodes vi { abs(score_new[vi] - score_old[vi]) };
       converged = (global_diff < convergence)
     }

   */

  int numNodes = num_nodes(g);
  double equal_prob = 1.0 / numNodes;

  bool converged = false;
  double *score_old = (double *)malloc(sizeof(double) * g->num_nodes);
  double *score_new = (double *)malloc(sizeof(double) * g->num_nodes);
  std::vector<Vertex> no_outgoing_vertex;

  for (Vertex i = 0; i < numNodes; ++i) {
    score_old[i] = equal_prob;
    if (outgoing_size(g, i) == 0) {
      no_outgoing_vertex.push_back(i);
    }
  }

  while (!converged) {

    double no_outgoing_sum = 0.0;
#pragma omp parallel for reduction(+ : no_outgoing_sum)
    for (Vertex &v : no_outgoing_vertex) {
      no_outgoing_sum += damping * score_old[v] / numNodes;
    }

#pragma omp parallel for
    for (Vertex vi = 0; vi < numNodes; vi++) {
      score_new[vi] = 0.0;
      const Vertex *beg = incoming_begin(g, vi), *end = incoming_end(g, vi);
      for (const Vertex *vj = beg; vj != end; vj++) {
        score_new[vi] += score_old[*vj] / outgoing_size(g, *vj);
      }

      score_new[vi] = (damping * score_new[vi]) + (1.0 - damping) / numNodes;

      score_new[vi] += no_outgoing_sum;
    }

    double global_diff = 0.0;
#pragma omp parallel for reduction(+ : global_diff)
    for (Vertex vi = 0; vi < numNodes; vi++) {
      global_diff += std::fabs(score_new[vi] - score_old[vi]);
    }
    converged = (global_diff < convergence);

    memcpy(score_old, score_new, sizeof(double) * g->num_nodes);
  }

  memcpy(solution, score_new, sizeof(double) * g->num_nodes);
  free(score_old);
  free(score_new);
}
