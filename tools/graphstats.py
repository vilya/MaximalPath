import sys

def main():
  if len(sys.argv) != 2:
    print >> sys.stderr, "Usage: %s <graph>" % sys.argv[0]
    sys.exit(-1)
  
  graph = {}
  with open(sys.argv[1]) as f:
    for line in f:
      line = line.upper()
      fromNode = line[:3]
      toNode = line[3:6]

      if fromNode not in graph:
        graph[fromNode] = set([toNode])
      else:
        graph[fromNode].add(toNode)

      if toNode not in graph:
        graph[toNode] = set([fromNode])
      else:
        graph[toNode].add(fromNode)

  print "%d nodes" % len(graph)
  print "%d to %d edges per node" % (
    min([len(edges) for _, edges in graph.iteritems()]),
    max([len(edges) for _, edges in graph.iteritems()])
  )
  print "%d self edges" % sum([1 for node, edges in graph.iteritems() if node in edges])


if __name__ == '__main__':
  main()

