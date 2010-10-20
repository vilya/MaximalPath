#include <algorithm>
#include <cstdlib>
#include <cstdio>
#include <vector>

#include <tbb/tick_count.h>


//
// Constants
//

const unsigned int kMaxNodes = 26 * 26 * 26;


//
// Graph class
//

struct Graph
{
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


  inline unsigned short nodeIndex(const char* label) const
  {
    return ((label[0] - 'A') * 26 + label[1] - 'A') * 26 + label[2] - 'A';
  }


  inline void addEdge(const char* fromLabel, const char* toLabel)
  {
    unsigned int from = nodeIndex(fromLabel);
    unsigned int to = nodeIndex(toLabel);
    if (std::find(edges[from].begin(), edges[from].end(), to) != edges[from].end())
      return;
    edges[from].push_back(to);
    edges[to].push_back(from);
  }
};


//
// Forward declarations
//

bool ParseGraph(const char* filename, Graph& g);
bool ParseNodes(const char* filename, Graph& g);
const char* NodeLabel(unsigned short node);
void PrintGraph(const Graph& g);
void PrintPath(const std::vector<unsigned short>& path);
uint64_t PrintPathsFrom(Graph& g, std::vector<unsigned short>& path, uint64_t count);
//uint64_t PathsFrom(Graph& g, unsigned short node);
void MaximalPaths(Graph& g);


//
// Functions
//

bool ParseGraph(const char* filename, Graph& g)
{
  FILE* f = fopen(filename, "r");
  if (f == NULL) {
    fprintf(stderr, "Error opening %s\n", filename);
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
    fprintf(stderr, "Error opening %s\n", filename);
    return false;
  }

  char line[32];

  fgets(line, 32, f);
  g.pathsToPrint = (unsigned int)atoi(line);

  g.startNodes.clear();
  while (fgets(line, 5, f) != NULL)
    g.startNodes.push_back(g.nodeIndex(line));

  for (unsigned int i = 0; i < kMaxNodes; ++i)
    std::sort(g.edges[i].begin(), g.edges[i].end());

  fclose(f);
  return true;
}


// FIXME: this function is not threadsafe!
const char* NodeLabel(unsigned short node)
{
  static char sLabel[4] = { 0, 0, 0, 0 };
  sLabel[0] = (node / 676) + 'A';
  sLabel[1] = ((node / 26) % 26) + 'A';
  sLabel[2] = (node % 26) + 'A';
  return sLabel;
}


void PrintGraph(const Graph& g)
{
  for (unsigned short i = 0; i < kMaxNodes; ++i) {
    if (g.edges[i].size() == 0)
      continue;

    if (g.edges[i][0] == i)
      printf("dodgy!\n");

    printf("%s: ", NodeLabel(i));
    printf("%s", NodeLabel(g.edges[i][0]));
    for (unsigned int j = 1; j < g.edges[i].size(); ++j)
      printf(", %s", NodeLabel(g.edges[i][j]));
    printf("\n");
  }
}


void PrintPath(const std::vector<unsigned short>& path)
{
  const unsigned int kLength = path.size();
  const unsigned short* kNodes = path.data();

  for (unsigned int i = 0; i < kLength; ++i)
    printf("%s", NodeLabel(kNodes[i]));
  printf("\n");
}


uint64_t PrintPathsFrom(Graph& g, std::vector<unsigned short>& path, uint64_t count, bool visited[])
{
  unsigned short node = path.back();
  visited[node] = true;
  
  const unsigned int kNumEdges = g.edges[node].size();
  const unsigned short* kEdges = g.edges[node].data();

  uint64_t newCount = 0;
  for (unsigned int i = 0; i < kNumEdges; ++i) {
    if (count + newCount >= g.pathsToPrint)
      break;

    unsigned short nextNode = kEdges[i];
    if (visited[nextNode])
      continue;

    path.push_back(nextNode);
    newCount += PrintPathsFrom(g, path, count + newCount, visited);
    path.pop_back();
  }

  if (newCount == 0 && count < g.pathsToPrint)
    PrintPath(path);
  
  visited[node] = false;
  return (newCount > 0) ? newCount : 1;
}


uint64_t PathsFrom(Graph& g, unsigned short node, bool visited[])
{
  const unsigned int kNumEdges = g.edges[node].size();
  const unsigned short* kEdges = g.edges[node].data();

  visited[node] = true;
  uint64_t newCount = 0;
  for (unsigned int i = 0; i < kNumEdges; ++i) {
    unsigned short nextNode = kEdges[i];
    if (!visited[nextNode])
      newCount += PathsFrom(g, nextNode, visited);
  }
  visited[node] = false;

  return newCount ? newCount : 1;
}


void MaximalPaths(Graph& g)
{
  std::vector<unsigned short> path;
  path.reserve(kMaxNodes);

  bool visited[kMaxNodes];
  memset(visited, 0, sizeof(visited));

  for (unsigned int i = 0; i < g.startNodes.size(); ++i) {
    printf("First %u lexicographic paths from %s:\n", g.pathsToPrint, NodeLabel(g.startNodes[i]));

    path.push_back(g.startNodes[i]);
    PrintPathsFrom(g, path, 0, visited);
    path.pop_back();

    uint64_t count = PathsFrom(g, g.startNodes[i], visited);
    
    printf("Total maximal paths starting from %s: %lu\n\n", NodeLabel(g.startNodes[i]), (unsigned long int)count);
  }
}


int main(int argc, char** argv)
{
  if (argc != 3) {
    fprintf(stderr, "Usage: %s <graph> <nodes>\n", argv[0]);
    return -1;
  }

  // Start timing.
  tbb::tick_count startTime = tbb::tick_count::now();
  
  Graph graph;

  // Parse the graph.
  if (!ParseGraph(argv[1], graph))
    return -2;
  //PrintGraph(graph);

  // Parse the nodes file.
  if (!ParseNodes(argv[2], graph))
    return -3;

  // Calculate and print the maximal paths
  MaximalPaths(graph);

  // Print the results.

  // Stop timing.
  tbb::tick_count endTime = tbb::tick_count::now();
  fprintf(stderr, "\nExecution completed in %5.4f seconds.\n", (endTime - startTime).seconds());

  return 0;
}

