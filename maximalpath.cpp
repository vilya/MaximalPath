#include <algorithm>
#include <cstdlib>
#include <cstdio>
#include <list>
#include <set>
#include <string>
#include <vector>

#include <tbb/atomic.h>
#include <tbb/task_group.h>
#include <tbb/tick_count.h>


//
// Graph class
//

struct Graph
{
  // Note that node indexes only require 15 bits to represent, so they fit
  // comfortably into an unsigned short (or a signed one).
  std::vector<std::string> nodes;
  std::vector<unsigned short>* edges;
  std::vector<unsigned short> startNodes;
  unsigned int pathsToPrint;


  Graph() : nodes(), edges(NULL), startNodes(), pathsToPrint(0)
  {
  }


  void setNodes(const std::set<std::string>& newNodes)
  {
    nodes.resize(newNodes.size());
    std::copy(newNodes.begin(), newNodes.end(), nodes.begin());

    edges = new std::vector<unsigned short>[nodes.size()];
    for (unsigned int i = 0; i < nodes.size(); ++i)
      edges[i].reserve(10);
  }


  inline unsigned short nodeIndex(const std::string& label) const
  {
    return (unsigned short)(std::find(nodes.begin(), nodes.end(), std::string(label)) - nodes.begin());
  }


  inline const std::string& nodeLabel(unsigned short node) const
  {
    return nodes[node];
  }


  inline void addEdge(const std::string& fromLabel, const std::string& toLabel)
  {
    unsigned int from = nodeIndex(fromLabel);
    unsigned int to = nodeIndex(toLabel);
    if (std::find(edges[from].begin(), edges[from].end(), to) != edges[from].end())
      return;
    edges[from].push_back(to);
    edges[to].push_back(from);
  }
};


struct DFSTree
{
  DFSTree* parent;
  unsigned short node;
  uint64_t visitedCache;


  DFSTree() :
    parent(NULL), node(0), visitedCache(0)
  {
  }


  DFSTree(DFSTree* iparent, unsigned short inode) :
    parent(iparent), node(inode), visitedCache(iparent->visitedCache)
  {
    visitedCache |= ((uint64_t)1 << inode);
  }


  bool alreadyVisited(unsigned short nextNode) const
  {
    return (nextNode & ((uint64_t)1 << nextNode)) || (node == nextNode) || (parent != NULL && parent->alreadyVisited(nextNode));
  }


  void printPath(const Graph& g) const
  {
    if (parent != NULL)
      parent->printPath(g);
    printf("%s", g.nodeLabel(node).c_str());
  }
};


//
// Forward declarations
//

bool ParseGraph(const char* filename, Graph& g);
bool ParseNodes(const char* filename, Graph& g);
void PrintGraph(const Graph& g);
//void PrintPath(const Graph& g, const std::vector<unsigned short>& path);
//uint64_t PrintPathsFrom(Graph& g, std::vector<unsigned short>& path, uint64_t count, bool visited[]);
//uint64_t PathsFrom(Graph& g, unsigned short node, const std::vector<bool>& visited);
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
  char label[4] = { 0, 0, 0, 0 };
  std::set<std::string> labels;
  while (fgets(line, 10, f) != NULL) {
    snprintf(label, 4, "%s", line);
    labels.insert(std::string(label));

    snprintf(label, 4, "%s", line + 3);
    labels.insert(std::string(label));
  }
  g.setNodes(labels);

  fseek(f, 0, SEEK_SET);
  while (fgets(line, 10, f) != NULL) {
    snprintf(label, 4, "%s", line);
    std::string from(label);

    snprintf(label, 4, "%s", line + 3);
    std::string to(label);

    g.addEdge(from, to);
  }

  const unsigned int kMaxNodes = g.nodes.size();
  for (unsigned int i = 0; i < kMaxNodes; ++i)
    std::sort(g.edges[i].begin(), g.edges[i].end());

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
  char label[4] = { 0, 0, 0, 0 };

  fgets(line, 32, f);
  g.pathsToPrint = (unsigned int)atoi(line);

  g.startNodes.clear();
  while (fgets(line, 5, f) != NULL) {
    snprintf(label, 4, "%s", line);
    g.startNodes.push_back(g.nodeIndex(label));
  }

  fclose(f);
  return true;
}


void PrintGraph(const Graph& g)
{
  for (unsigned short i = 0; i < g.nodes.size(); ++i) {
    if (g.edges[i].size() == 0)
      continue;

    if (g.edges[i][0] == i)
      printf("dodgy!\n");

    printf("%s: ", g.nodeLabel(i).c_str());
    printf("%s", g.nodeLabel(g.edges[i][0]).c_str());
    for (unsigned int j = 1; j < g.edges[i].size(); ++j)
      printf(", %s", g.nodeLabel(g.edges[i][j]).c_str());
    printf("\n");
  }
}


uint64_t PrintPathsFrom(Graph& g, DFSTree* parent, uint64_t count)
{
  const unsigned short node = parent->node;
  const unsigned int kNumEdges = g.edges[node].size();
  const unsigned short* kEdges = g.edges[node].data();

  uint64_t newCount = 0;
  for (unsigned int i = 0; i < kNumEdges; ++i) {
    if (count + newCount >= g.pathsToPrint)
      break;

    unsigned short nextNode = kEdges[i];
    if (parent->alreadyVisited(nextNode))
      continue;

    DFSTree child(parent, nextNode);
    newCount += PrintPathsFrom(g, &child, count + newCount);
  }

  if (newCount == 0 && count < g.pathsToPrint) {
    parent->printPath(g);
    printf("\n");
    return 1;
  }
  else {
    return newCount;
  }
}


struct PathsFromFunctor
{
  Graph& g;
  DFSTree* parent;
  uint64_t& count;


  PathsFromFunctor(Graph& ig, DFSTree* iparent, uint64_t& ocount) :
    g(ig), parent(iparent), count(ocount)
  {}


  void operator () ()
  {
    const unsigned short node = parent->node;
    const unsigned int kNumEdges = g.edges[node].size();
    const unsigned short* kEdges = g.edges[node].data();

    const unsigned int kNumChildren = 16; // Must be an exact power of 2.
    const unsigned int kChildMask = kNumChildren - 1;

    DFSTree children[kNumChildren];
    uint64_t newCounts[kNumChildren];
    memset(newCounts, 0, sizeof(newCounts));

    bool maximal = true;
    tbb::task_group grp;
    for (unsigned int i = 0; i < kNumEdges; ++i) {
      unsigned short nextNode = kEdges[i];
      if (parent->alreadyVisited(nextNode))
        continue;

      maximal = false;
      DFSTree& child = children[i & kChildMask];
      child.parent = parent;
      child.node = nextNode;
      child.visitedCache = parent->visitedCache | ((uint64_t)1 << child.node);
      grp.run(PathsFromFunctor(g, &child, newCounts[i & kChildMask]));

      if ((i & kChildMask) == kChildMask) {
        grp.wait();
        uint64_t sum = 0;
        for (unsigned int s = 0; s < kNumChildren; ++s) {
          sum += newCounts[s];
          newCounts[s] = 0;
        }
        count += sum;
      }
    }

    if ((kNumEdges & kChildMask) != kChildMask) {
      grp.wait();
      uint64_t sum = 0;
      for (unsigned int s = 0; s < kNumChildren; ++s) {
        sum += newCounts[s];
        newCounts[s] = 0;
      }
      count += sum;
    }

    if (maximal)
      ++count;
  }
};


uint64_t PathsFrom(Graph& g, unsigned short node, bool visited[])
{
  const unsigned int kNumEdges = g.edges[node].size();
  const unsigned short* kEdges = g.edges[node].data();
  visited[node] = true;

  uint64_t newCount = 0;
  for (unsigned int i = 0; i < kNumEdges; ++i) {
    unsigned short nextNode = kEdges[i];
    if (visited[nextNode])
      continue;

    newCount += PathsFrom(g, nextNode, visited);
  }

  visited[node] = false;
  return newCount ? newCount : 1;
}


void MaximalPaths(Graph& g, bool threaded)
{
  const unsigned int kMaxNodes = g.nodes.size();
  bool* visited = NULL;
  if (!threaded) {
    visited = new bool[kMaxNodes];
    memset(visited, 0, sizeof(bool) * kMaxNodes);
  }

  for (unsigned int i = 0; i < g.startNodes.size(); ++i) {
    printf("First %u lexicographic paths from %s:\n", g.pathsToPrint, g.nodeLabel(g.startNodes[i]).c_str());

    DFSTree root;
    root.node = g.startNodes[i];
    root.visitedCache = ((uint64_t)1 << root.node);

    PrintPathsFrom(g, &root, 0);

    uint64_t count = 0;
    if (threaded) {
      PathsFromFunctor pathsFrom(g, &root, count);
      pathsFrom();
    }
    else {
      count = PathsFrom(g, g.startNodes[i], visited);
    }

    printf("Total maximal paths starting from %s: %lu\n\n",
        g.nodeLabel(g.startNodes[i]).c_str(), (unsigned long int)count);
  }
}


int main(int argc, char** argv)
{
  bool threaded = false;
  if (argc == 4 && strcmp(argv[1], "-t") == 0) {
    threaded = true;
    argv[1] = argv[2];
    argv[2] = argv[3];
    --argc;
  }

  if (argc != 3) {
    fprintf(stderr, "Usage: %s [-t] <graph> <nodes>\n", argv[0]);
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
  MaximalPaths(graph, threaded);

  // Stop timing.
  tbb::tick_count endTime = tbb::tick_count::now();
  fprintf(stderr, "\nExecution completed in %5.4f seconds.\n", (endTime - startTime).seconds());

  return 0;
}

