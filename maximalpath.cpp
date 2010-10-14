#include <algorithm>
#include <cstdlib>
#include <cstdio>
#include <vector>

#include <tbb/blocked_range.h>
#include <tbb/parallel_for.h>
#include <tbb/parallel_reduce.h>
#include <tbb/tick_count.h>


//
// Constants
//

const unsigned int kMaxNodes = 26 * 26 * 26;


//
// Classes
//

struct Graph {
  // Note that node indexes only require 15 bits to represent, so they fit
  // comfortably into an unsigned short (or a signed one).
  std::vector<unsigned short> edges[kMaxNodes];
  std::vector<unsigned short> startNodes;
  unsigned int pathsToPrint;


  Graph()  
  {
    for (unsigned int i = 0; i < kMaxNodes; ++i)
      edges[i].reserve(8);
  }


  inline unsigned int nodeIndex(const char* label) const
  {
    return ((label[0] - 'A') * 26 + label[1] - 'A') * 26 + label[2] - 'A';
  }


  inline void addEdge(const char* fromLabel, const char* toLabel)
  {
    unsigned int from = nodeIndex(fromLabel);
    unsigned int to = nodeIndex(toLabel);
    edges[from].push_back(to);
    edges[to].push_back(from);
  }
};


//
// Forward declarations
//

bool ParseGraph(const char* filename);
bool ParseNodes(const char* filename, unsigned int& numPaths, std::vector<unsigned int>& startNodes);


//
// Globals
//

Graph gGraph;


//
// Functions
//

bool ParseGraph(const char* filename, Graph& g)
{
  FILE* f = fopen(filename, "r");
  if (f == NULL) {
    fprintf(stderr, "Error opening %s: %s\n", filename, strerror(ferror(f)));
    return false;
  }

  char line[10];
  while (fgets(line, 10, f) != NULL)
    g.addEdge(line, line + 3);

  fclose(f);
  return true;
}


bool ParseNodes(const char* filename, Graph& g)
{
  FILE* f = fopen(filename, "r");
  if (f == NULL) {
    fprintf(stderr, "Error opening %s: %s\n", filename, strerror(ferror(f)));
    return false;
  }

  char line[32];

  fgets(line, 32, f);
  g.pathsToPrint = (unsigned int)atoi(line);

  g.startNodes.clear();
  while (fgets(line, 5, f) != NULL)
    g.startNodes.push_back(g.nodeIndex(line));

  fclose(f);
  return true;
}


int main(int argc, char** argv)
{
  if (argc != 3) {
    fprintf(stderr, "Usage: %s <graph> <nodes>\n", argv[0]);
    return -1;
  }

  // Start timing.
  tbb::tick_count startTime = tbb::tick_count::now();
  
  // Parse the graph.
  if (!ParseGraph(argv[1], gGraph))
    return -2;

  // Parse the nodes file.
  if (!ParseNodes(argv[2], gGraph))
    return -3;

  // For each start node
  //    Calculate and print the maximal paths
  // Print the results.

  // Stop timing.
  tbb::tick_count endTime = tbb::tick_count::now();
  fprintf(stderr, "\nExecution completed in %5.4f seconds.\n", (endTime - startTime).seconds());

  return 0;
}

