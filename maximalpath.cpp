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

    edges = new std::vector<nodeid_t>[nodes.size()];
    for (unsigned int i = 0; i < nodes.size(); ++i)
      edges[i].reserve(10);
  }


  nodeid_t Graph::nodeIndex(const std::string& label) const
  {
    return (nodeid_t)(std::find(nodes.begin(), nodes.end(), std::string(label)) - nodes.begin());
  }


  const std::string& Graph::nodeLabel(nodeid_t node) const
  {
    return nodes[node];
  }


  void Graph::addEdge(const std::string& fromLabel, const std::string& toLabel)
  {
    unsigned int from = nodeIndex(fromLabel);
    unsigned int to = nodeIndex(toLabel);
    if (std::find(edges[from].begin(), edges[from].end(), to) != edges[from].end())
      return;
    edges[from].push_back((nodeid_t)to);
    edges[to].push_back((nodeid_t)from);
  }


  //
  // SearchTree struct
  //

  SearchTree::SearchTree() :
    parent(NULL), node(0)
  {
  }


  SearchTree::SearchTree(SearchTree* iparent, nodeid_t inode) :
    parent(iparent), node(inode)
  {
  }


  bool SearchTree::alreadyVisited(nodeid_t nextNode) const
  {
    return (node == nextNode) || (parent != NULL && parent->alreadyVisited(nextNode));
  }


  void SearchTree::printPath(const Graph& g) const
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


  void CountPathsFunctor::operator() (const tbb::blocked_range<SearchTree**>& r)
  {
    for (SearchTree** prefix = r.begin(); prefix != r.end(); ++prefix) {
      memset(_visited, 0, sizeof(bool) * _kMaxNodes);
      SearchTree* tmp = (*prefix)->parent;
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


  uint64_t CountPathsFunctor::pathsFrom(nodeid_t node)
  {
    const unsigned int kNumEdges = _graph.edges[node].size();
    const nodeid_t* kEdges = _graph.edges[node].data();
    _visited[node] = true;

    uint64_t newCount = 0;
    for (unsigned int i = 0; i < kNumEdges; ++i) {
      nodeid_t nextNode = kEdges[i];
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
    for (nodeid_t i = 0; i < g.nodes.size(); ++i) {
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
  
  
  uint64_t PrintPathsFrom(Graph& g, nodeid_t node, uint64_t count, bool* visited, char* path, unsigned int depth)
  {
    const unsigned int kNumEdges = g.edges[node].size();
    const nodeid_t* kEdges = g.edges[node].data();
    const char* kNodeLabel = g.nodes[node].c_str();

    unsigned int pathIndex = depth * 3;
    visited[node] = true;
    strncpy(path + pathIndex, kNodeLabel, 3);
    path[pathIndex + 3] = '\0';
  
    uint64_t newCount = 0;
    for (unsigned int i = 0; i < kNumEdges; ++i) {
      if (count + newCount >= g.pathsToPrint)
        break;
  
      nodeid_t nextNode = kEdges[i];
      if (visited[nextNode])
        continue;
  
      newCount += PrintPathsFrom(g, nextNode, count + newCount, visited, path, depth + 1);
    }
  
    if (newCount == 0 && count < g.pathsToPrint) {
      printf("%s\n", path);
      newCount = 1;
    }
  
    visited[node] = false;
    return newCount;
  }


  void PrintPaths(Graph& g, nodeid_t startNode)
  {
    bool* printVisited = new bool[g.nodes.size()];
    char* path = new char[g.nodes.size() * 3 + 1];
    memset(printVisited, 0, sizeof(bool) * g.nodes.size());
  
    PrintPathsFrom(g, startNode, 0, printVisited, path, 0);
  }
  
  
  uint64_t CountPaths(Graph& g, nodeid_t node)
  {
    const unsigned int kNumPrefixes = std::thread::hardware_concurrency() * 8;
  
    uint64_t count = 0;
  
    std::list<SearchTree*> prefixes;
    prefixes.push_back(new SearchTree(NULL, node));
    while (!prefixes.empty() && prefixes.size() < kNumPrefixes) {
      SearchTree* parent = prefixes.front();
      prefixes.pop_front();
  
      node = parent->node;
      const unsigned int kNumEdges = g.edges[node].size();
      const nodeid_t* kEdges = g.edges[node].data();
  
      bool maximal = true;
      for (unsigned int i = 0; i < kNumEdges; ++i) {
        nodeid_t nextNode = kEdges[i];
        if (parent->alreadyVisited(nextNode))
          continue;
  
        maximal = false;
        prefixes.push_back(new SearchTree(parent, nextNode));
      }
  
      if (maximal)
        ++count;
    }
  
    std::vector<SearchTree*> prefixVec(prefixes.size());
    std::copy(prefixes.begin(), prefixes.end(), prefixVec.begin());
  
    CountPathsFunctor counter(g);
    tbb::parallel_reduce(tbb::blocked_range<SearchTree**>(
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

