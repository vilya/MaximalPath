#ifndef maximalpath_h
#define maximalpath_h

#include <cstring>
#include <set>
#include <string>
#include <vector>

#include <tbb/blocked_range.h>

namespace mxp {

  //
  // Typedefs
  //

  typedef unsigned short nodeid_t;
  typedef bool visited_t;


  //
  // Data structures
  //

  template <typename T>
  class FastVector
  {
  public:
    FastVector() :  _size(0), _capacity(8), _data(new T[10])
    {}

    ~FastVector()
    {
      delete[] _data;
    }

    inline unsigned int size() const
    {
      return _size;
    }

    inline const T& operator [] (unsigned int index) const
    {
      return _data[index];
    }

    inline T& operator [] (unsigned int index)
    {
      return _data[index];
    }

    inline void reserve(unsigned int numItems)
    {
      if (numItems > _capacity) {
        T* newData = new T[numItems];
        memcpy(newData, _data, sizeof(T) * numItems);
        delete[] _data;
        _data = newData;
        _capacity = numItems;
      }
    }

    inline T* begin()
    {
      return _data;
    }

    inline T* end()
    {
      return _data + _size;
    }

    inline void push_back(const T& val)
    {
      if (_size == _capacity)
        reserve(_capacity * 2);
      _data[_size] = val;
      ++_size;
    }

    inline T* data()
    {
      return _data;
    }

  private:
    unsigned int _size;
    unsigned int _capacity;
    T* _data;
  };


  struct Graph
  {
    // Note that node indexes only require 15 bits to represent, so they fit
    // comfortably into an unsigned short (or a signed one).
    std::vector<std::string> nodes;
    FastVector<nodeid_t>* edges;
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
    visited_t* _visited;
    uint64_t _count;
  };


  //
  // Functions
  //

  bool ParseGraph(const char* filename, Graph& g);
  bool ParseNodes(const char* filename, Graph& g);
  void PrintGraph(const Graph& g);

  uint64_t PrintPathsFrom(Graph& g, nodeid_t node, uint64_t count, visited_t* visited, char* path, unsigned int depth);

  void PrintPaths(Graph& g, nodeid_t node);
  uint64_t CountPaths(Graph& g, nodeid_t node);
  void MaximalPaths(Graph& g);


} // namespace mxp

#endif

