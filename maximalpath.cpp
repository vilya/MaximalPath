#include <algorithm>
#include <cstdlib>
#include <cstdio>
#include <vector>

#include <tbb/blocked_range.h>
#include <tbb/parallel_for.h>
#include <tbb/parallel_reduce.h>
#include <tbb/tick_count.h>


//
// Typedefs
//

typedef uint64_t EdgeChunk;


//
// Constants
//

const unsigned int kMaxNodes = 26 * 26 * 26;
const unsigned int kEdgeChunkBits = sizeof(EdgeChunk) * 8;
const unsigned int kEdgeChunkShift = __builtin_ctz(kEdgeChunkBits);
const unsigned int kEdgeChunkMask = kEdgeChunkBits - 1;
const unsigned int kNumEdgeChunks = kMaxNodes / kEdgeChunkBits + 1;


//
// Globals
//

EdgeChunk gEdges[kMaxNodes][kNumEdgeChunks];
unsigned int gEdgeCounts[kMaxNodes];
// Maybe have a per-node list of which chunks contain edges, so that we can iterate through the edges more quickly?
// Maybe it would actually be better to have an explicit list of connected nodes? Would only use more memory if every node had, on average, 1099 or more edges.
// Note that node indexes only require 15 bits to represent, so they fit comfortably into an unsigned short (or a signed one).


//
// Forward declarations
//

unsigned int NodeIndex(const char* label);
void AddEdge(unsigned int from, unsigned int to);
bool ParseGraph(const char* filename);
bool ParseNodes(const char* filename, unsigned int& numPaths, std::vector<unsigned int>& startNodes);


//
// Functions
//

inline unsigned int NodeIndex(const char* label)
{
  return ((label[0] - 'A') * 26 + label[1] - 'A') * 26 + label[2] - 'A';
}


inline void AddEdge(unsigned int from, unsigned int to)
{
  unsigned int itemNum = to >> kEdgeChunkShift;
  unsigned int itemBit = to & kEdgeChunkMask;
  gEdges[from][itemNum] |= itemBit;
}


bool ParseGraph(const char* filename)
{
  FILE* f = fopen(filename, "r");
  if (f == NULL) {
    fprintf(stderr, "Error opening %s: %s\n", filename, strerror(ferror(f)));
    return false;
  }

  memset(gEdges, 0, sizeof(gEdges));
  memset(gEdgeCounts, 0, sizeof(gEdgeCounts));

  char line[10];
  while (fgets(line, 10, f) != NULL) {
    unsigned int from = NodeIndex(line);
    unsigned int to = NodeIndex(line + 3);
    AddEdge(from, to);
    AddEdge(to, from);
    ++gEdgeCounts[from];
    ++gEdgeCounts[to];
  }

  fclose(f);
  return true;
}


bool ParseNodes(const char* filename, unsigned int& numPaths, std::vector<unsigned int>& startNodes)
{
  FILE* f = fopen(filename, "r");
  if (f == NULL) {
    fprintf(stderr, "Error opening %s: %s\n", filename, strerror(ferror(f)));
    return false;
  }

  char line[32];

  fgets(line, 32, f);
  numPaths = (unsigned int)atoi(line);

  startNodes.clear();
  while (fgets(line, 5, f) != NULL)
    startNodes.push_back(NodeIndex(line));

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
  if (!ParseGraph(argv[1]))
    return -2;

  // Parse the nodes file.
  unsigned int numPaths = 0;
  std::vector<unsigned int> startNodes;
  if (!ParseNodes(argv[2], numPaths, startNodes))
    return -3;

  // For each start node
  //    Calculate and print the maximal paths
  // Print the results.

  // Stop timing.
  tbb::tick_count endTime = tbb::tick_count::now();
  fprintf(stderr, "\nExecution completed in %5.4f seconds.\n", (endTime - startTime).seconds());

  return 0;
}

