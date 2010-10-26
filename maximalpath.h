#ifndef maximalpath_h
#define maximalpath_h

#include <set>
#include <string>
#include <vector>

#include <tbb/blocked_range.h>

namespace mxp {

  //
  // Typedefs
  //

  typedef unsigned short nodeid_t;


  //
  // Data structures
  //

  struct Graph
  {
    // Note that node indexes only require 15 bits to represent, so they fit
    // comfortably into an unsigned short (or a signed one).
    std::vector<std::string> nodes;
    std::vector<nodeid_t>* edges;
    std::vector<nodeid_t> startNodes;
    unsigned int pathsToPrint;

    Graph();

    void setNodes(const std::set<std::string>& newNodes);
    inline nodeid_t nodeIndex(const std::string& label) const;
    inline const std::string& nodeLabel(nodeid_t node) const;
    inline void addEdge(const std::string& fromLabel, const std::string& toLabel);
  };


  struct SearchTree
  {
    SearchTree* parent;
    nodeid_t node;

    SearchTree();
    SearchTree(SearchTree* iparent, nodeid_t inode);

    bool alreadyVisited(nodeid_t nextNode) const;
    void printPath(const Graph& g) const;
  };


  //
  // Functors
  //

  class CountPathsFunctor
  {
  public:
    CountPathsFunctor(const Graph& g);
    CountPathsFunctor(const CountPathsFunctor& c, tbb::split);
    ~CountPathsFunctor();
  
    void operator() (const tbb::blocked_range<SearchTree**>& r);
    void join(CountPathsFunctor& rhs);

    uint64_t getCount() const;
    void resetCount();
  
  private:
    uint64_t pathsFrom(nodeid_t node);
  
  private:
    const nodeid_t _kMaxNodes;
    const Graph& _graph;
    bool* _visited;
    uint64_t _count;
  };


  //
  // Functions
  //

  bool ParseGraph(const char* filename, Graph& g);
  bool ParseNodes(const char* filename, Graph& g);
  void PrintGraph(const Graph& g);

  uint64_t PrintPathsFrom(Graph& g, nodeid_t node, uint64_t count, bool* visited, char* path, unsigned int depth);

  void PrintPaths(Graph& g, nodeid_t node);
  uint64_t CountPaths(Graph& g, nodeid_t node);
  void MaximalPaths(Graph& g);


} // namespace mxp

#endif

