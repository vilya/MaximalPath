#include "maximalpath.h"

#include <algorithm>
#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <list>

#include <tbb/compat/thread>
#include <tbb/parallel_reduce.h>
#include <tbb/tick_count.h>

namespace mxp {


  //
  // Graph class
  //

  Graph::Graph() : nodes(), edges(NULL), startNodes(), pathsToPrint(0)
  {
  }


  void Graph::setNodes(const std::set<std::string>& newNodes)
  {
    nodes.resize(newNodes.size());
    std::copy(newNodes.begin(), newNodes.end(), nodes.begin());

    edges = new std::vector<unsigned short>[nodes.size()];
    for (unsigned int i = 0; i < nodes.size(); ++i)
      edges[i].reserve(10);
  }


  unsigned short Graph::nodeIndex(const std::string& label) const
  {
    return (unsigned short)(std::find(nodes.begin(), nodes.end(), std::string(label)) - nodes.begin());
  }


  const std::string& Graph::nodeLabel(unsigned short node) const
  {
    return nodes[node];
  }


  void Graph::addEdge(const std::string& fromLabel, const std::string& toLabel)
  {
    unsigned int from = nodeIndex(fromLabel);
    unsigned int to = nodeIndex(toLabel);
    if (std::find(edges[from].begin(), edges[from].end(), to) != edges[from].end())
      return;
    edges[from].push_back(to);
    edges[to].push_back(from);
  }


  //
  // DFSTree struct
  //

  DFSTree::DFSTree() :
    parent(NULL), node(0)
  {
  }


  DFSTree::DFSTree(DFSTree* iparent, unsigned short inode) :
    parent(iparent), node(inode)
  {
  }


  bool DFSTree::alreadyVisited(unsigned short nextNode) const
  {
    return (node == nextNode) || (parent != NULL && parent->alreadyVisited(nextNode));
  }


  void DFSTree::printPath(const Graph& g) const
  {
    if (parent != NULL)
      parent->printPath(g);
    printf("%s", g.nodeLabel(node).c_str());
  }


  //
  // CountPathsFunctor class
  //

  CountPathsFunctor::CountPathsFunctor(const Graph& g) :
    _kMaxNodes(g.nodes.size()),
    _graph(g),
    _visited(NULL),
    _count(0)
  {
    _visited = new bool[_kMaxNodes];
  }


  CountPathsFunctor::CountPathsFunctor(const CountPathsFunctor& c, tbb::split) :
    _kMaxNodes(c._kMaxNodes),
    _graph(c._graph),
    _visited(NULL),
    _count(0)
  {
    _visited = new bool[_kMaxNodes];
    memcpy(_visited, c._visited, sizeof(bool) * _kMaxNodes);
  }


  CountPathsFunctor::~CountPathsFunctor()
  {
    delete _visited;
  }


  void CountPathsFunctor::operator() (const tbb::blocked_range<DFSTree**>& r)
  {
    for (DFSTree** prefix = r.begin(); prefix != r.end(); ++prefix) {
      memset(_visited, 0, sizeof(bool) * _kMaxNodes);
      DFSTree* tmp = (*prefix)->parent;
      while (tmp != NULL) {
        _visited[tmp->node] = true;
        tmp = tmp->parent;
      }
      _count += pathsFrom((*prefix)->node);
    }
  }


  void CountPathsFunctor::join(CountPathsFunctor& rhs)
  {
    _count += rhs._count;
  }


  uint64_t CountPathsFunctor::getCount() const
  {
    return _count;
  }


  /*
  void CountPathsFunctor::resetCount()
  {
    _count = 0;
  }
  */

  uint64_t CountPathsFunctor::pathsFrom(unsigned short node)
  {
    const unsigned int kNumEdges = _graph.edges[node].size();
    const unsigned short* kEdges = _graph.edges[node].data();
    _visited[node] = true;

    uint64_t newCount = 0;
    for (unsigned int i = 0; i < kNumEdges; ++i) {
      unsigned short nextNode = kEdges[i];
      if (_visited[nextNode])
        continue;

      newCount += pathsFrom(nextNode);
    }

    _visited[node] = false;
    return newCount ? newCount : 1;
  }


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
  
    if(!fgets(line, 32, f))
      return false;
  
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
  
  
  uint64_t PrintPathsFrom(Graph& g, DFSTree* parent, uint64_t count, bool* visited)
  {
    const unsigned short node = parent->node;
    const unsigned int kNumEdges = g.edges[node].size();
    const unsigned short* kEdges = g.edges[node].data();
  
    visited[node] = true;
  
    uint64_t newCount = 0;
    for (unsigned int i = 0; i < kNumEdges; ++i) {
      if (count + newCount >= g.pathsToPrint)
        break;
  
      unsigned short nextNode = kEdges[i];
      if (visited[nextNode])
        continue;
  
      DFSTree child(parent, nextNode);
      newCount += PrintPathsFrom(g, &child, count + newCount, visited);
    }
  
    if (newCount == 0 && count < g.pathsToPrint) {
      parent->printPath(g);
      printf("\n");
      newCount = 1;
    }
  
    visited[node] = false;
    return newCount;
  }


  void PrintPaths(Graph& g, unsigned short startNode)
  {
    bool* printVisited = new bool[g.nodes.size()];
    memset(printVisited, 0, sizeof(bool) * g.nodes.size());
  
    DFSTree root;
    root.node = startNode;
  
    PrintPathsFrom(g, &root, 0, printVisited);
  }
  
  
  uint64_t CountPaths(Graph& g, unsigned short node)
  {
    const unsigned int kNumPrefixes = std::thread::hardware_concurrency() * 4;
  
    uint64_t count = 0;
  
    std::list<DFSTree*> prefixes;
    prefixes.push_back(new DFSTree(NULL, node));
    while (!prefixes.empty() && prefixes.size() < kNumPrefixes) {
      DFSTree* parent = prefixes.front();
      prefixes.pop_front();
  
      node = parent->node;
      const unsigned int kNumEdges = g.edges[node].size();
      const unsigned short* kEdges = g.edges[node].data();
  
      bool maximal = true;
      for (unsigned int i = 0; i < kNumEdges; ++i) {
        unsigned short nextNode = kEdges[i];
        if (parent->alreadyVisited(nextNode))
          continue;
  
        maximal = false;
        prefixes.push_back(new DFSTree(parent, nextNode));
      }
  
      if (maximal)
        ++count;
    }
  
    std::vector<DFSTree*> prefixVec(prefixes.size());
    std::copy(prefixes.begin(), prefixes.end(), prefixVec.begin());
  
    CountPathsFunctor counter(g);
    tbb::parallel_reduce(tbb::blocked_range<DFSTree**>(
          prefixVec.data(), prefixVec.data() + prefixVec.size()), counter);
    count += counter.getCount();
  
    return count;
  }
  

  void MaximalPaths(Graph& g)
  {
    for (unsigned int i = 0; i < g.startNodes.size(); ++i) {
      printf("First %u lexicographic paths from %s:\n", g.pathsToPrint, g.nodeLabel(g.startNodes[i]).c_str());
  
      std::thread th(PrintPaths, g, g.startNodes[i]);
      uint64_t count = CountPaths(g, g.startNodes[i]);
      th.join();
  
      printf("Total maximal paths starting from %s: %lu\n\n",
          g.nodeLabel(g.startNodes[i]).c_str(), (unsigned long int)count);
    }
  
    //delete[] printVisited;
  }


} // namespace mxp


int main(int argc, char** argv)
{
  if (argc != 3) {
    fprintf(stderr, "Usage: %s <graph> <nodes>\n", argv[0]);
    return -1;
  }

  // Start timing.
  tbb::tick_count startTime = tbb::tick_count::now();
  
  mxp::Graph graph;

  // Parse the graph.
  if (!mxp::ParseGraph(argv[1], graph))
    return -2;
  //PrintGraph(graph);

  // Parse the nodes file.
  if (!mxp::ParseNodes(argv[2], graph))
    return -3;

  // Calculate and print the maximal paths
  mxp::MaximalPaths(graph);

  // Stop timing.
  tbb::tick_count endTime = tbb::tick_count::now();
  fprintf(stderr, "\nExecution completed in %5.4f seconds.\n", (endTime - startTime).seconds());

  return 0;
}

